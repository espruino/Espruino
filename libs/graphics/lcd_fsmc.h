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
 * Graphics Backend for 16 bit parallel LCDs (ILI9325 and similar)
 * ----------------------------------------------------------------------------
 */
#include "graphics.h"
#include "jsvariterator.h"

void lcdInit_FSMC(JsGraphics *gfx);
void lcdSetCallbacks_FSMC(JsGraphics *gfx);

/// Starts a blit operation - call this, then blitPixel (a lot) then blitEnd. No bounds checking. Based on lcdST7789
void lcdFSMC_blitStart(JsGraphics *gfx, int x, int y, int w, int h);
void lcdFSMC_setCursor(JsGraphics *gfx, int x, int y);
void lcdFSMC_blitPixel(unsigned int col);
void lcdFSMC_blitEnd();

void lcdFSMC_setPower(bool isOn);

/// special case for sending a pixel-doubled 4 bit image, with even no of pixels
void lcdFSMC_blit4Bit(JsGraphics *gfx, int x, int y, int w, int h, int scale, JsvStringIterator *pixels, const uint16_t *palette);