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

#ifdef SAVE_ON_FLASH
#define NO_VECTOR_FONT
#ifndef PIXLJS
#define NO_MODIFIED_AREA
#endif
#endif

typedef enum {
  JSGRAPHICSTYPE_ARRAYBUFFER, ///< Write everything into an ArrayBuffer
  JSGRAPHICSTYPE_JS,          ///< Call JavaScript when we want to write something
  JSGRAPHICSTYPE_FSMC,        ///< FSMC (or fake FSMC) ILI9325 16bit-wide LCDs
  JSGRAPHICSTYPE_SDL,         ///< SDL graphics library for linux
  JSGRAPHICSTYPE_SPILCD,      ///< SPI LCD library
  JSGRAPHICSTYPE_ST7789_8BIT, ///< ST7789 in 8 bit mode
  JSGRAPHICSTYPE_LCD_SPI_UNBUF ///< LCD SPI unbuffered 16 bit driver
} JsGraphicsType;

typedef enum {
  JSGRAPHICSFLAGS_NONE,
  JSGRAPHICSFLAGS_ARRAYBUFFER_ZIGZAG = 1, ///< ArrayBuffer: zig-zag (even rows reversed)
  JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE = 2, ///< ArrayBuffer: if 1 bpp, treat bytes as stacked vertically
  JSGRAPHICSFLAGS_ARRAYBUFFER_MSB = 4, ///< ArrayBuffer: store pixels MSB first
  JSGRAPHICSFLAGS_ARRAYBUFFER_INTERLEAVEX = 8, //< ArrayBuffer:  Pixels 0,2,4,etc are from the top half of the image, 1,3,5,etc from the bottom half. Used for P3 LED panels
  JSGRAPHICSFLAGS_SWAP_XY = 16, //< All devices: swap X and Y over
  JSGRAPHICSFLAGS_INVERT_X = 32, //< All devices: x = getWidth() - (x+1) - where x is DEVICE X
  JSGRAPHICSFLAGS_INVERT_Y = 64, //< All devices: y = getHeight() - (y+1) - where y is DEVICE Y
  JSGRAPHICSFLAGS_COLOR_BASE = 128,

  JSGRAPHICSFLAGS_COLOR_RGB = 0,
  JSGRAPHICSFLAGS_COLOR_BRG = JSGRAPHICSFLAGS_COLOR_BASE, //< All devices: color order is BRG
  JSGRAPHICSFLAGS_COLOR_BGR = JSGRAPHICSFLAGS_COLOR_BASE*2, //< All devices: color order is BGR
  JSGRAPHICSFLAGS_COLOR_GBR = JSGRAPHICSFLAGS_COLOR_BASE*3, //< All devices: color order is GBR
  JSGRAPHICSFLAGS_COLOR_GRB = JSGRAPHICSFLAGS_COLOR_BASE*4, //< All devices: color order is GRB
  JSGRAPHICSFLAGS_COLOR_RBG = JSGRAPHICSFLAGS_COLOR_BASE*5, //< All devices: color order is RBG
  JSGRAPHICSFLAGS_COLOR_MASK = JSGRAPHICSFLAGS_COLOR_BASE*7, //< All devices: color order is BRG

  /// If any bits here are set, X and Y get modified before being used
  JSGRAPHICSFLAGS_MAPPEDXY = JSGRAPHICSFLAGS_SWAP_XY|JSGRAPHICSFLAGS_INVERT_X|JSGRAPHICSFLAGS_INVERT_Y,
  /// If any bits in this mark are set, pixel data is not laid out linearly
  JSGRAPHICSFLAGS_NONLINEAR = JSGRAPHICSFLAGS_ARRAYBUFFER_ZIGZAG|JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE|JSGRAPHICSFLAGS_ARRAYBUFFER_INTERLEAVEX
} JsGraphicsFlags;

typedef enum {
 JSGRAPHICS_FONTSIZE_SCALE_MASK = 8191, ///< the size of the font
 JSGRAPHICS_FONTSIZE_FONT_MASK = 7 << 13, ///< the type of the font
 JSGRAPHICS_FONTSIZE_VECTOR = 0,
 JSGRAPHICS_FONTSIZE_4X6 = 1 << 13, // a bitmap font
#ifdef USE_FONT_6X8
 JSGRAPHICS_FONTSIZE_6X8 = 2 << 13, // a bitmap font
#endif
 JSGRAPHICS_FONTSIZE_CUSTOM = 3 << 13,// a custom bitmap font made from fields in the graphics object (See below)
} JsGraphicsFontSize;


#define JSGRAPHICS_CUSTOMFONT_BMP JS_HIDDEN_CHAR_STR"fnB"
#define JSGRAPHICS_CUSTOMFONT_WIDTH JS_HIDDEN_CHAR_STR"fnW"
#define JSGRAPHICS_CUSTOMFONT_HEIGHT JS_HIDDEN_CHAR_STR"fnH"
#define JSGRAPHICS_CUSTOMFONT_FIRSTCHAR JS_HIDDEN_CHAR_STR"fn1"

typedef struct {
  unsigned short x1,y1;
  unsigned short x2,y2;
} PACKED_FLAGS JsGraphicsClipRect;

typedef struct {
  JsGraphicsType type;
  JsGraphicsFlags flags;
  unsigned short width, height; // DEVICE width and height (flags could mean the device is rotated)
  unsigned char bpp;
  unsigned int fgColor, bgColor; ///< current foreground and background colors
  unsigned short fontSize; ///< See JSGRAPHICS_FONTSIZE_ constants
  short cursorX, cursorY; ///< current cursor positions
#ifndef SAVE_ON_FLASH
  unsigned char fontAlignX : 2;
  unsigned char fontAlignY : 2;
  unsigned char fontRotate : 2;
#endif
#ifndef NO_MODIFIED_AREA
  JsGraphicsClipRect clipRect;
  short modMinX, modMinY, modMaxX, modMaxY; ///< area that has been modified
#endif
} PACKED_FLAGS JsGraphicsData;

typedef struct JsGraphics {
  JsVar *graphicsVar; // this won't be locked again - we just know that it is already locked by something else
  JsGraphicsData data;
  void *backendData; ///< Data used by the graphics backend

  void (*setPixel)(struct JsGraphics *gfx, int x, int y, unsigned int col);
  void (*fillRect)(struct JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col);
  unsigned int (*getPixel)(struct JsGraphics *gfx, int x, int y);
  void (*scroll)(struct JsGraphics *gfx, int xdir, int ydir); // scroll - leave unscrolled area undefined
} PACKED_FLAGS JsGraphics;
typedef void (*JsGraphicsSetPixelFn)(struct JsGraphics *gfx, int x, int y, unsigned int col);

// ---------------------------------- these are in graphics.c
/// Reset graphics structure state (eg font size, color, etc)
void graphicsStructResetState(JsGraphics *gfx);
/// Completely reset graphics structure including flags
void graphicsStructInit(JsGraphics *gfx, int width, int height, int bpp);
/// Access the Graphics Instance JsVar and get the relevant info in a JsGraphics structure. True on success
bool graphicsGetFromVar(JsGraphics *gfx, JsVar *parent);
/// Access the Graphics Instance JsVar and set the relevant info from JsGraphics structure
void graphicsSetVar(JsGraphics *gfx);
// ----------------------------------------------------------------------------------------------



/// Get the memory requires for this graphics's pixels if everything was packed as densely as possible
size_t graphicsGetMemoryRequired(const JsGraphics *gfx);
// If graphics is flipped or rotated then the coordinates need modifying
void graphicsToDeviceCoordinates(const JsGraphics *gfx, int *x, int *y);
unsigned short graphicsGetWidth(const JsGraphics *gfx);
unsigned short graphicsGetHeight(const JsGraphics *gfx);
// Set the area modified (inclusive of x2,y2) by a draw command and also clip to the screen/clipping bounds. Returns true if clipped
bool graphicsSetModifiedAndClip(JsGraphics *gfx, int *x1, int *y1, int *x2, int *y2);
/// Get a setPixel function (assuming coordinates already clipped with graphicsSetModifiedAndClip) - if all is ok it can choose a faster draw function
JsGraphicsSetPixelFn graphicsGetSetPixelFn(JsGraphics *gfx);
/// Get a setPixel function and set modified area (assuming no clipping) (inclusive of x2,y2) - if all is ok it can choose a faster draw function
JsGraphicsSetPixelFn graphicsGetSetPixelUnclippedFn(JsGraphics *gfx, int x1, int y1, int x2, int y2);
/// Merge one color into another based on current bit depth
uint32_t graphicsBlendColor(JsGraphics *gfx, double amt);

// drawing functions - all coordinates are in USER coordinates, not DEVICE coordinates
void         graphicsSetPixel(JsGraphics *gfx, int x, int y, unsigned int col);
unsigned int graphicsGetPixel(JsGraphics *gfx, int x, int y);
void         graphicsClear(JsGraphics *gfx);
void         graphicsFillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col);
void graphicsFallbackFillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col); // Simple fillrect - doesn't call device-specific FR
void graphicsDrawRect(JsGraphics *gfx, int x1, int y1, int x2, int y2);
void graphicsDrawEllipse(JsGraphics *gfx, int x, int y, int x2, int y2);
void graphicsFillEllipse(JsGraphics *gfx, int x, int y, int x2, int y2);
void graphicsDrawLine(JsGraphics *gfx, int x1, int y1, int x2, int y2);
void graphicsDrawLineAA(JsGraphics *gfx, int ix1, int iy1, int ix2, int iy2); ///< antialiased drawline. each pixel is 1/16th
void graphicsFillPoly(JsGraphics *gfx, int points, short *vertices); ///< each pixel is 1/16th a pixel may overwrite vertices...
#ifndef NO_VECTOR_FONT
unsigned int graphicsFillVectorChar(JsGraphics *gfx, int x1, int y1, int size, char ch); ///< prints character, returns width
unsigned int graphicsVectorCharWidth(JsGraphics *gfx, unsigned int size, char ch); ///< returns the width of a character
#endif
/// Draw a simple 1bpp image in foreground colour
void graphicsDrawImage1bpp(JsGraphics *gfx, int x1, int y1, int width, int height, const unsigned char *pixelData);
/// Scroll the graphics device (in user coords). X>0 = to right, Y >0 = down
void graphicsScroll(JsGraphics *gfx, int xdir, int ydir);

void graphicsSplash(JsGraphics *gfx); ///< splash screen

void graphicsIdle(); ///< called when idling

#endif // GRAPHICS_H
