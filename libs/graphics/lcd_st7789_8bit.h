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
  LCDST7789_MODE_UNBUFFERED, // normal, 240x240
  LCDST7789_MODE_DOUBLEBUFFERED, // 240x160 widescreen, double-buffered
} LCDST7789Mode;

/// Send a command direct to the screen
void lcdST7789_cmd(int cmd, int dataLen, const uint8_t *data);
/// Set double buffered or normal modes
void lcdST7789_setMode(LCDST7789Mode mode);
/// When in double-buffered mode, flip the screen
void lcdST7789_flip();
