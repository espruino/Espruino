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

void lcdInit_SPILCD(JsGraphics *gfx);
void lcdSetCallbacks_SPILCD(JsGraphics *gfx);

void lcdFlip_SPILCD(JsGraphics *gfx); // run this to flip the offscreen buffer to the screen
void lcdCmd_SPILCD(int cmd, int dataLen, const char *data); // to send specific commands to the display
void lcdSetPalette_SPILCD(const char *pal);
