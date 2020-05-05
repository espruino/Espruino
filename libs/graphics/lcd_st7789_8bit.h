/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2019 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Graphics Backend for drawing to SPI displays
 * ----------------------------------------------------------------------------
 */
#include "graphics.h"

void lcdST7789_init(JsGraphics *gfx);
void lcdST7789_setCallbacks(JsGraphics *gfx);

typedef enum {
  LCDST7789_MODE_NULL, // ignore draw calls
  LCDST7789_MODE_UNBUFFERED, // normal, 240x240
  LCDST7789_MODE_DOUBLEBUFFERED, // 240x160 widescreen, double-buffered
  LCDST7789_MODE_BUFFER_120x120, // 120x120 8 bit buffer
  LCDST7789_MODE_BUFFER_80x80, // 80x80 8 bit buffer
} LCDST7789Mode;

#ifdef EMSCRIPTEN
extern int EMSCRIPTEN_GFX_YSTART;
extern char EMSCRIPTEN_GFX_BUFFER[240*320*2];
extern bool EMSCRIPTEN_GFX_CHANGED;
extern bool EMSCRIPTEN_GFX_WIDESCREEN; // are we 160px high?
#endif

/// Send a command direct to the screen
void lcdST7789_cmd(int cmd, int dataLen, const uint8_t *data);
/** Allow the LCD to be shifted vertically while still drawing in the normal position.
 * Use this to display notifications while keeping the original data on the screen */
void lcdST7789_setYOffset(int y);
/// Set double buffered or normal modes
void lcdST7789_setMode(LCDST7789Mode mode);
LCDST7789Mode lcdST7789_getMode();
/// When in double-buffered mode, flip the screen
void lcdST7789_flip(JsGraphics *gfx);

/// Starts a blit operation - call this, then blitPixel (a lot) then blitEnd. No bounds checking
void lcdST7789_blitStart(int x, int y, int w, int h);
void lcdST7789_blitPixel(unsigned int col);
void lcdST7789_blitEnd();

/// blit a 1 bit image direct to the screen
void lcdST7789_blit1Bit(int x, int y, int w, int h, int scale, uint8_t *pixels, const uint16_t *palette);
/// blit a 8 bit image direct to the screen
void lcdST7789_blit8Bit(int x, int y, int w, int h, int scale, uint8_t *pixels, const uint16_t *palette);
