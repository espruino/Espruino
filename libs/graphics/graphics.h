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
} JsGraphicsFlags;

#define JSGRAPHICS_FONTSIZE_8X8 (-1) // a bitmap font
// Positive font sizes are Vector fonts

typedef struct {
  JsGraphicsType type;
  JsGraphicsFlags flags;
  unsigned short width, height;
  unsigned char bpp;
  unsigned int fgColor, bgColor; ///< current foreground and background colors
  short fontSize; ///< See JSGRAPHICS_FONTSIZE_ constants
  short cursorX, cursorY; ///< current cursor positions
} PACKED_FLAGS JsGraphicsData;

typedef struct JsGraphics {
  JsVar *graphicsVar; // this won't be locked again - we just know that it is already locked by something else
  JsGraphicsData data;
  unsigned char _blank; ///< this is needed as jsvGetString for 'data' wants to add a trailing zero  

  void (*setPixel)(struct JsGraphics *gfx, short x, short y, unsigned int col);
  void (*fillRect)(struct JsGraphics *gfx, short x1, short y1, short x2, short y2);
  void (*bitmap1bit)(struct JsGraphics *gfx, short x1, short y1, unsigned short width, unsigned short height, unsigned char *data);
  unsigned int (*getPixel)(struct JsGraphics *gfx, short x, short y);
} PACKED_FLAGS JsGraphics;

static inline void graphicsStructInit(JsGraphics *gfx) {
  gfx->data.fgColor = 0xFFFFFFFF;
  gfx->data.bgColor = 0;
  gfx->data.fontSize = JSGRAPHICS_FONTSIZE_8X8;
  gfx->data.cursorX = 0;
  gfx->data.cursorY = 0;
}

// ---------------------------------- these are in lcd.c
// Access a JsVar and get/set the relevant info in JsGraphics
bool graphicsGetFromVar(JsGraphics *gfx, JsVar *parent);
void graphicsSetVar(JsGraphics *gfx);
// ----------------------------------------------------------------------------------------------
// drawing functions
void         graphicsSetPixel(JsGraphics *gfx, short x, short y, unsigned int col);
unsigned int graphicsGetPixel(JsGraphics *gfx, short x, short y);
void         graphicsClear(JsGraphics *gfx);
void         graphicsFillRect(JsGraphics *gfx, short x1, short y1, short x2, short y2);
void         graphicsBitmap1bit(JsGraphics *gfx, short x1, short y1, unsigned short width, unsigned short height, unsigned char *data);
void graphicsDrawRect(JsGraphics *gfx, short x1, short y1, short x2, short y2);
void graphicsDrawChar(JsGraphics *gfx, short x1, short y1, char ch);
void graphicsDrawString(JsGraphics *gfx, short x1, short y1, const char *str);
void graphicsDrawLine(JsGraphics *gfx, short x1, short y1, short x2, short y2);
void graphicsFillPoly(JsGraphics *gfx, int points, const short *vertices);
unsigned int graphicsFillVectorChar(JsGraphics *gfx, short x1, short y1, short size, char ch); ///< prints character, returns width
unsigned int graphicsVectorCharWidth(JsGraphics *gfx, short size, char ch); ///< returns the width of a character
void graphicsFillVectorString(JsGraphics *gfx, short x1, short y1, short size, const char *str);
void graphicsSplash(JsGraphics *gfx); ///< splash screen

void graphicsIdle(); ///< called when idling

#endif // GRAPHICS_H
