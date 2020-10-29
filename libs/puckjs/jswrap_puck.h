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
 * Contains JavaScript interface for Puck.js
 * ----------------------------------------------------------------------------
 */
#include "jspin.h"

extern bool isPuckV2;

void jswrap_puck_magOn();
void jswrap_puck_magOff();
JsVar *jswrap_puck_mag();
JsVarFloat jswrap_puck_magTemp();
void jswrap_puck_magWr(JsVarInt reg, JsVarInt data);
int jswrap_puck_magRd(JsVarInt reg);
JsVarFloat jswrap_puck_getTemperature();

void jswrap_puck_accelOn(JsVarFloat hz);
void jswrap_puck_accelOff();
JsVar *jswrap_puck_accel();
void jswrap_puck_accelWr(JsVarInt reg, JsVarInt data);
int jswrap_puck_accelRd(JsVarInt reg);

void jswrap_puck_IR(JsVar *data, Pin cathode, Pin anode);
int jswrap_puck_capSense(Pin tx, Pin rx);
JsVarFloat jswrap_puck_light();
JsVarInt jswrap_puck_getBattery();
bool jswrap_puck_selfTest();

void jswrap_puck_init();
void jswrap_puck_kill();
bool jswrap_puck_idle();
