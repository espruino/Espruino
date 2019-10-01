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

void jswrap_hackstrap_lcdWr(JsVarInt cmd, JsVar *data);
void jswrap_hackstrap_setLCDPower(bool isOn);
void jswrap_hackstrap_setLCDTimeout(JsVarFloat timeout);
bool jswrap_hackstrap_isLCDOn();

void jswrap_hackstrap_setGPSPower(bool isOn);

void jswrap_hackstrap_accelWr(JsVarInt reg, JsVarInt data);
int jswrap_hackstrap_accelRd(JsVarInt reg);
void jswrap_hackstrap_ioWr(JsVarInt mask, bool on);
JsVar *jswrap_hackstrap_getPressure();
void jswrap_hackstrap_off();

void jswrap_hackstrap_init();
void jswrap_hackstrap_kill();
bool jswrap_hackstrap_idle();
bool jswrap_hackstrap_gps_character(char ch);
