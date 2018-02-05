/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2016 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
* Contains JavaScript interface for Pixl.js (http://www.espruino.com/Pixl.js)
 * ----------------------------------------------------------------------------
 */
#include "jspin.h"

int jswrap_pixljs_getBatteryPercentage();
void jswrap_pixljs_lcdw(JsVarInt c);
void jswrap_pixljs_setContrast(JsVarFloat c);

void jswrap_pixljs_init();
void jswrap_pixljs_kill();
bool jswrap_pixljs_idle();
