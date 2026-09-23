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
 * Contains JavaScript interface for Bangle.js 3 (http://www.espruino.com/Bangle.js)
 * ----------------------------------------------------------------------------
 */

#include "banglejs3_py32/src/const.h"

void jswrap_banglejs_setRGB(int r, int g, int b);
void jswrap_banglejs_enableUART(bool en);

void jswrap_banglejs3_hwinit();
bool jswrap_banglejs3_idle();


void jshPY32Transfer(uint8_t *buf, int count);
void jshPY32Update(PY32Command cmd, int data);