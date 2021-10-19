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
 * Graphics Backend for drawing to ArrayBuffer
 * ----------------------------------------------------------------------------
 */
#include "graphics.h"

void lcdInit_ArrayBuffer(JsGraphics *gfx);
void lcdSetCallbacks_ArrayBuffer(JsGraphics *gfx);

// these use gfx->backendData as a pointer to data. They're exported so lcd_st7789_8bit can use them for fast offscreen rendering
void lcdSetPixel_ArrayBuffer_flat8(JsGraphics *gfx, int x, int y, unsigned int col);
unsigned int lcdGetPixel_ArrayBuffer_flat8(struct JsGraphics *gfx, int x, int y);
void lcdFillRect_ArrayBuffer_flat8(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col);
void lcdScroll_ArrayBuffer_flat8(JsGraphics *gfx, int xdir, int ydir, int x1, int y1, int x2, int y2);

