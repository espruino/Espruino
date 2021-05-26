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
 * Contains JavaScript interface for Bangle.js (http://www.espruino.com/Bangle.js)
 * ----------------------------------------------------------------------------
 */
#include "jspin.h"

void jswrap_banglejs_lcdWr(JsVarInt cmd, JsVar *data);
void jswrap_banglejs_setLCDPower(bool isOn);
bool jswrap_banglejs_getLCDPower();
void jswrap_banglejs_setLCDBrightness(JsVarFloat v);
void jswrap_banglejs_setLCDMode(JsVar *mode);
JsVar *jswrap_banglejs_getLCDMode();
void jswrap_banglejs_setLCDOffset(int y);
void jswrap_banglejs_setLCDTimeout(JsVarFloat timeout);
int jswrap_banglejs_isLCDOn();

void jswrap_banglejs_setPollInterval(JsVarFloat interval);
void jswrap_banglejs_setOptions(JsVar *options);
int jswrap_banglejs_isCharging();
JsVarInt jswrap_banglejs_getBattery();

bool jswrap_banglejs_setHRMPower(bool isOn, JsVar *appId);
int jswrap_banglejs_isHRMOn();
bool jswrap_banglejs_setGPSPower(bool isOn, JsVar *appId);
int jswrap_banglejs_isGPSOn();
bool jswrap_banglejs_setCompassPower(bool isOn, JsVar *appId);
int jswrap_banglejs_isCompassOn();
void jswrap_banglejs_resetCompass();
bool jswrap_banglejs_setBarometerPower(bool isOn, JsVar *appId);
int jswrap_banglejs_isBarometerOn();

int jswrap_banglejs_getStepCount();
JsVar *jswrap_banglejs_getCompass();
JsVar *jswrap_banglejs_getAccel();
JsVar *jswrap_banglejs_getPressure();

JsVar *jswrap_banglejs_dbg();
void jswrap_banglejs_accelWr(JsVarInt reg, JsVarInt data);
JsVar *jswrap_banglejs_accelRd(JsVarInt reg, JsVarInt cnt);
void jswrap_banglejs_barometerWr(JsVarInt reg, JsVarInt data);
JsVar *jswrap_banglejs_barometerRd(JsVarInt reg, JsVarInt cnt);
void jswrap_banglejs_compassWr(JsVarInt reg, JsVarInt data);
void jswrap_banglejs_ioWr(JsVarInt mask, bool on);

JsVar *jswrap_banglejs_project(JsVar *latlong);
void jswrap_banglejs_beep_callback(); // internal use only
JsVar *jswrap_banglejs_beep(int time, int freq);
void jswrap_banglejs_buzz_callback(); // internal use only
JsVar *jswrap_banglejs_buzz(int time, JsVarFloat amt);


void jswrap_banglejs_off();
void jswrap_banglejs_softOff();
JsVar *jswrap_banglejs_getLogo();

void jswrap_banglejs_init();
void jswrap_banglejs_kill();
bool jswrap_banglejs_idle();
bool jswrap_banglejs_gps_character(char ch);

#ifdef SMAQ3
// We use pins that are unused for 'fake' buttons to allow Bangle.js v1 apps to work
#define FAKE_BTN1_PIN 40
#define FAKE_BTN3_PIN 41
#endif
