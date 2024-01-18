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

/// toggle EXTCOMIN to avoid burn-in
void lcdMemLCD_extcominToggle();
/// If backlight is on, we need to raise EXTCOMIN freq (use HW PWM)
void lcdMemLCD_extcominBacklight(bool isOn);
// Enable overlay mode (to overlay an image on top of the LCD contents)
void lcdMemLCD_setOverlay(JsVar *imgVar, int x, int y);
// return a pointer to the LCD's memory buffer
unsigned char *lcdMemLCD_getRowPtr(int row);