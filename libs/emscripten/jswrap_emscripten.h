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
 * Contains JavaScript interface for Emscripten JS port
 * ----------------------------------------------------------------------------
 */
#include "jspin.h"

void jswrap_em_init();
void jswrap_em_kill();
bool jswrap_em_idle();


JsVar *jswrap_banglejs_getLogo();
void jswrap_banglejs_setLCDPower(bool isOn);
void jswrap_banglejs_setLCDBrightness(JsVarFloat v);
void jswrap_banglejs_setLCDMode(JsVar *mode);
void jswrap_banglejs_setLCDTimeout(JsVarFloat timeout);
void jswrap_banglejs_setPollInterval(JsVarFloat interval);
void jswrap_banglejs_setOptions(JsVar *options);
int jswrap_banglejs_isLCDOn();
int jswrap_banglejs_isCharging();
JsVarInt jswrap_banglejs_getBattery();
JsVar *jswrap_banglejs_beep(int time, int freq);
JsVar *jswrap_banglejs_buzz(int time, JsVarFloat amt);
