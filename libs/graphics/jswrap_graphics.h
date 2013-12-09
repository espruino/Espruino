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
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript Graphics Draw Functions
 * ----------------------------------------------------------------------------
 */

#include "jsvar.h"
#include "graphics.h"

// For creating graphics classes
JsVar *jswrap_graphics_createArrayBuffer(int width, int height, int bpp,  JsVar *options);
JsVar *jswrap_graphics_createCallback(int width, int height, int bpp, JsVar *callback);
#ifdef USE_LCD_SDL
JsVar *jswrap_graphics_createSDL(int width, int height);
#endif


int jswrap_graphics_getWidthOrHeight(JsVar *parent, bool height);
void jswrap_graphics_clear(JsVar *parent);
void jswrap_graphics_fillRect(JsVar *parent, int x1, int y1, int x2, int y2);
void jswrap_graphics_drawRect(JsVar *parent, int x1, int y1, int x2, int y2);
int jswrap_graphics_getPixel(JsVar *parent, int x, int y);
void jswrap_graphics_setPixel(JsVar *parent, int x, int y, int color);
void jswrap_graphics_setColorX(JsVar *parent, JsVar *r, JsVar *g, JsVar *b, bool isForeground);
void jswrap_graphics_setFontSizeX(JsVar *parent, JsVarInt size, bool checkValid);
void jswrap_graphics_drawString(JsVar *parent, JsVar *str, int x, int y);
JsVarInt jswrap_graphics_stringWidth(JsVar *parent, JsVar *var);
void jswrap_graphics_drawLine(JsVar *parent, int x1, int y1, int x2, int y2);
void jswrap_graphics_lineTo(JsVar *parent, int x, int y);
void jswrap_graphics_moveTo(JsVar *parent, int x, int y);
void jswrap_graphics_fillPoly(JsVar *parent, JsVar *poly);
