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
 * Contains JavaScript function just for the Dickens smartwatch
 * ----------------------------------------------------------------------------
 */

#include "jsvar.h"

JsVar *jswrap_graphics_fillArc(JsVar *parent, double a1, double a2, double r);
JsVar *jswrap_graphics_fillSeg(JsVar *parent, double a, double ar, double r1, double r2);
JsVar *jswrap_graphics_drawSeg(JsVar *parent, double a, double ar, double r);

void jswrap_banglejs_setLCDRotation(int d);