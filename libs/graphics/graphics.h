/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Graphics Draw Functions
 * ----------------------------------------------------------------------------
 */

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "jsutils.h"
#include "jsvar.h"

typedef enum {
  JSGRAPHICSTYPE_ARRAYBUFFER, ///< Write everything into an ArrayBuffer
  JSGRAPHICSTYPE_JS,          ///< Call JavaScript when we want to write something
  JSGRAPHICSTYPE_FSMC,        ///< FSMC (or fake FSMC) ILI9325 16bit-wide LCDs
  JSGRAPHICSTYPE_SDL,         ///< SDL graphics library for linux
} JsGraphicsType;

typedef enum {
  JSGRAPHICSFLAGS_NONE,
  JSGRAPHICSFLAGS_ARRAYBUFFER_ZIGZAG = 1, ///< ArrayBuffer: zig-zag (even rows reversed)
  JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE = 2, ///< ArrayBuffer: if 1 bpp, treat bytes as stacked vertically
  JSGRAPHICSFLAGS_ARRAYBUFFER_MSB = 4, ///< ArrayBuffer: store pixels MSB first
  JSGRAPHICSFLAGS_SWAP_XY = 8, //< All devices: swap X and Y over
  JSGRAPHICSFLAGS_INVERT_X = 16, //< All devices: x = getWidth() - (x+1) - where x is DEVICE X
  JSGRAPHICSFLAGS_INVERT_Y = 32, //< All devices: y = getHeight() - (y+1) - where y is DEVICE Y

  JSGRAPHICSFLAGS_COLOR_RGB = 0,
  JSGRAPHICSFLAGS_COLOR_BRG = 64, //< All devices: color order is BRG
  JSGRAPHICSFLAGS_COLOR_BGR = 128, //< All devices: color order is BGR
  JSGRAPHICSFLAGS_COLOR_GBR = 64+128, //< All devices: color order is GBR
  JSGRAPHICSFLAGS_COLOR_GRB = 256, //< All devices: color order is GRB
  JSGRAPHICSFLAGS_COLOR_RBG = 256+64, //< All devices: color order is RBG
  JSGRAPHICSFLAGS_COLOR_MASK = 64+128+256, //< All devices: color order is BRG
} JsGraphicsFlags;

#define JSGRAPHICS_FONTSIZE_4X6 (-1) // a bitmap font
#define JSGRAPHICS_FONTSIZE_CUSTOM (-2) // a custom bitmap font made from fields in the graphics object (See below)
// Positive font sizes are Vector fonts

#define JSGRAPHICS_CUSTOMFONT_BMP JS_HIDDEN_CHAR_STR"fnB"
#define JSGRAPHICS_CUSTOMFONT_WIDTH JS_HIDDEN_CHAR_STR"fnW"
#define JSGRAPHICS_CUSTOMFONT_HEIGHT JS_HIDDEN_CHAR_STR"fnH"
#define JSGRAPHICS_CUSTOMFONT_FIRSTCHAR JS_HIDDEN_CHAR_STR"fn1"

typedef struct {
  JsGraphicsType type;
  JsGraphicsFlags flags;
  unsigned short width, height; // DEVICE width and height (flags could mean the device is rotated)
  unsigned char bpp;
  unsigned int fgColor, bgColor; ///< current foreground and background colors
  short fontSize; ///< See JSGRAPHICS_FONTSIZE_ constants
  short cursorX, cursorY; ///< current cursor positions
  short modMinX, modMinY, modMaxX, modMaxY; ///< area that has been modified
} PACKED_FLAGS JsGraphicsData;

typedef struct JsGraphics {
  JsVar *graphicsVar; // this won't be locked again - we just know that it is already locked by something else
  JsGraphicsData data;
  unsigned char _blank; ///< this is needed as jsvGetString for 'data' wants to add a trailing zero
  void *backendData; ///< Data used by the graphics backend

  void (*setPixel)(struct JsGraphics *gfx, short x, short y, unsigned int col);
  void (*fillRect)(struct JsGraphics *gfx, short x1, short y1, short x2, short y2);
  unsigned int (*getPixel)(struct JsGraphics *gfx, short x, short y);
  void (*scroll)(struct JsGraphics *gfx, int xdir, int ydir); // scroll - leave unscrolled area undefined
} PACKED_FLAGS JsGraphics;

static inline void graphicsStructInit(JsGraphics *gfx) {
  // type/width/height/bpp should be set elsewhere...
  gfx->data.flags = JSGRAPHICSFLAGS_NONE;
  gfx->data.fgColor = 0xFFFFFFFF;
  gfx->data.bgColor = 0;
  gfx->data.fontSize = JSGRAPHICS_FONTSIZE_4X6;
  gfx->data.cursorX = 0;
  gfx->data.cursorY = 0;
  gfx->data.modMaxX = -32768;
  gfx->data.modMaxY = -32768;
  gfx->data.modMinX = 32767;
  gfx->data.modMinY = 32767;
}

// ---------------------------------- these are in graphics.c
// Access a JsVar and get/set the relevant info in JsGraphics
bool graphicsGetFromVar(JsGraphics *gfx, JsVar *parent);
void graphicsSetVar(JsGraphics *gfx);
// ----------------------------------------------------------------------------------------------
/// Get the memory requires for this graphics's pixels if everything was packed as densely as possible
size_t graphicsGetMemoryRequired(const JsGraphics *gfx);
// If graphics is flipped or rotated then the coordinates need modifying
void graphicsToDeviceCoordinates(const JsGraphics *gfx, short *x, short *y);
// drawing functions - all coordinates are in USER coordinates, not DEVICE coordinates
void         graphicsSetPixel(JsGraphics *gfx, short x, short y, unsigned int col);
unsigned int graphicsGetPixel(JsGraphics *gfx, short x, short y);
void         graphicsClear(JsGraphics *gfx);
void         graphicsFillRect(JsGraphics *gfx, short x1, short y1, short x2, short y2);
void graphicsFallbackFillRect(JsGraphics *gfx, short x1, short y1, short x2, short y2); // Simple fillrect - doesn't call device-specific FR
void graphicsDrawRect(JsGraphics *gfx, short x1, short y1, short x2, short y2);
void graphicsDrawCircle(JsGraphics *gfx, short posX, short posY, short rad);
void graphicsFillCircle(JsGraphics *gfx, short x, short y, short rad);
void graphicsDrawString(JsGraphics *gfx, short x1, short y1, const char *str);
void graphicsDrawLine(JsGraphics *gfx, short x1, short y1, short x2, short y2);
void graphicsFillPoly(JsGraphics *gfx, int points, short *vertices); // may overwrite vertices...
#ifndef SAVE_ON_FLASH
unsigned int graphicsFillVectorChar(JsGraphics *gfx, short x1, short y1, short size, char ch); ///< prints character, returns width
unsigned int graphicsVectorCharWidth(JsGraphics *gfx, short size, char ch); ///< returns the width of a character
#endif
/// Draw a simple 1bpp image in foreground colour
void graphicsDrawImage1bpp(JsGraphics *gfx, short x1, short y1, short width, short height, const unsigned char *pixelData);
/// Scroll the graphics device (in user coords). X>0 = to right, Y >0 = down
void graphicsScroll(JsGraphics *gfx, int xdir, int ydir);

void graphicsSplash(JsGraphics *gfx); ///< splash screen

void graphicsIdle(); ///< called when idling

#endif // GRAPHICS_H
