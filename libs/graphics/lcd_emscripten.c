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
 * Graphics Backend for drawing via Emscripten
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"
#include "jsutils.h"
#include "lcd_emscripten.h"

char EMSCRIPTEN_GFX_BUFFER[240*240*2];
bool EMSCRIPTEN_GFX_CHANGED;

void lcdEmscripten_SetPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
  EMSCRIPTEN_GFX_CHANGED = true;
  uint16_t *buf = (uint16_t*)EMSCRIPTEN_GFX_BUFFER;
  buf[x+y*240]=col;
}

void lcdEmscripten_setCallbacks(JsGraphics *gfx) {
  gfx->setPixel = lcdEmscripten_SetPixel;
}
