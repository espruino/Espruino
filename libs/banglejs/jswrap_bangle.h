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
void jswrap_banglejs_setLCDPowerBacklight(bool isOn);
void jswrap_banglejs_setLCDBrightness(JsVarFloat v);
void jswrap_banglejs_setLCDMode(JsVar *mode);
JsVar *jswrap_banglejs_getLCDMode();
void jswrap_banglejs_setLCDOffset(int y);
void jswrap_banglejs_setLCDOverlay(JsVar *imgVar, int x, int y);
void jswrap_banglejs_setLCDTimeout(JsVarFloat timeout);
int jswrap_banglejs_isLCDOn();
int jswrap_banglejs_isBacklightOn();
void jswrap_banglejs_setLocked(bool isLocked);
int jswrap_banglejs_isLocked();

void jswrap_banglejs_setPollInterval(JsVarFloat interval);
void jswrap_banglejs_setOptions(JsVar *options);
JsVar *jswrap_banglejs_getOptions();
int jswrap_banglejs_isCharging();
JsVarInt jswrap_banglejs_getBattery();

bool jswrap_banglejs_setHRMPower(bool isOn, JsVar *appId);
int jswrap_banglejs_isHRMOn();
bool jswrap_banglejs_setGPSPower(bool isOn, JsVar *appId);
int jswrap_banglejs_isGPSOn();
JsVar *jswrap_banglejs_getGPSFix();
bool jswrap_banglejs_setCompassPower(bool isOn, JsVar *appId);
int jswrap_banglejs_isCompassOn();
void jswrap_banglejs_resetCompass();
bool jswrap_banglejs_setBarometerPower(bool isOn, JsVar *appId);
int jswrap_banglejs_isBarometerOn();

int jswrap_banglejs_getStepCount();
void jswrap_banglejs_setStepCount(JsVarInt count);

JsVar *jswrap_banglejs_getCompass();
JsVar *jswrap_banglejs_getAccel();
JsVar *jswrap_banglejs_getPressure();
JsVar *jswrap_banglejs_getHealthStatus();

JsVar *jswrap_banglejs_dbg();
void jswrap_banglejs_accelWr(JsVarInt reg, JsVarInt data);
JsVar *jswrap_banglejs_accelRd(JsVarInt reg, JsVarInt cnt);
void jswrap_banglejs_barometerWr(JsVarInt reg, JsVarInt data);
JsVar *jswrap_banglejs_barometerRd(JsVarInt reg, JsVarInt cnt);
void jswrap_banglejs_compassWr(JsVarInt reg, JsVarInt data);
JsVar *jswrap_banglejs_compassRd(JsVarInt reg, JsVarInt cnt);
void jswrap_banglejs_hrmWr(JsVarInt reg, JsVarInt data);
JsVar *jswrap_banglejs_hrmRd(JsVarInt reg, JsVarInt cnt);
void jswrap_banglejs_ioWr(JsVarInt mask, bool on);

JsVar *jswrap_banglejs_project(JsVar *latlong);
void jswrap_banglejs_beep_callback(); // internal use only
JsVar *jswrap_banglejs_beep(int time, int freq);
void jswrap_banglejs_buzz_callback(); // internal use only
JsVar *jswrap_banglejs_buzz(int time, JsVarFloat amt);


void jswrap_banglejs_off();
void jswrap_banglejs_softOff();
JsVar *jswrap_banglejs_getLogo();
void jswrap_banglejs_factoryReset();

JsVar *jswrap_banglejs_appRect();

void jswrap_banglejs_hwinit();
void jswrap_banglejs_init();
void jswrap_banglejs_kill();
bool jswrap_banglejs_idle();
bool jswrap_banglejs_gps_character(char ch);

/* If we're busy and really don't want to be interrupted (eg clearing flash memory)
 then we should *NOT* allow the home button to set EXEC_INTERRUPTED (which happens
 if it was held, JSBT_RESET was set, and then 0.5s later it wasn't handled).
 */
void jswrap_banglejs_kickPollWatchdog();

#ifdef EMULATED
extern void touchHandlerInternal(int tx, int ty, int pts, int gesture);
#endif

// Used when pushing events/retrieving events from the event queue
typedef enum {
  JSBE_HRM_ENV, // new HRM environment reading
} JsBangleEvent;

/// Called from jsinteractive when an event is parsed from the event queue for Bangle.js (executed outside IRQ)
void jsbangle_exec_pending(IOEvent *event);
/// queue an event for Bangle.js (usually called from inside an IRQ)
void jsbangle_push_event(JsBangleEvent type, uint16_t value);

