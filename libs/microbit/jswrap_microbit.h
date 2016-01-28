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
 * Contains JavaScript interface to micro:bit
 * ----------------------------------------------------------------------------
 */

#include "jsvar.h"

void jswrap_microbit_init();
void jswrap_microbit_kill();
void jswrap_microbit_show(JsVar *image);

JsVar *jswrap_microbit_acceleration();
JsVar *jswrap_microbit_compass();
