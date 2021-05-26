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

void lcdMemLCD_init(JsGraphics *gfx);
void lcdMemLCD_setCallbacks(JsGraphics *gfx);

void lcdMemLCD_flip(JsGraphics *gfx); // run this to flip the offscreen buffer to the screen
void lcdMemLCD_cmd(int cmd, int dataLen, const char *data); // to send specific commands to the display
void lcdMemLCD_extcomin(); // toggle EXTCOMIN to avoid burn-in
