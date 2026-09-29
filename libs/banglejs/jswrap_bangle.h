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
#ifndef JSWRAP_BANGLE
#define JSWRAP_BANGLE
#include "jspin.h"

typedef enum {
  JSBF_NONE,
  JSBF_WAKEON_FACEUP = 1<<0,
  JSBF_WAKEON_BTN1   = 1<<1,
  JSBF_WAKEON_BTN2   = 1<<2,
  JSBF_WAKEON_BTN3   = 1<<3,
  JSBF_WAKEON_TOUCH  = 1<<4,
  JSBF_WAKEON_DBLTAP = 1<<5,
  JSBF_WAKEON_TWIST  = 1<<6,
  JSBF_BEEP_VIBRATE  = 1<<7, // use vibration motor for beep
  JSBF_ENABLE_BEEP   = 1<<8,
  JSBF_ENABLE_BUZZ   = 1<<9,
  JSBF_ACCEL_LISTENER = 1<<10, ///< we have a listener for accelerometer data
  JSBF_POWER_SAVE    = 1<<11, ///< if no movement detected for a while, lower the accelerometer poll interval
  JSBF_HRM_ON        = 1<<12,
  JSBF_GPS_ON        = 1<<13,
  JSBF_COMPASS_ON    = 1<<14,
  JSBF_BAROMETER_ON  = 1<<15,
  JSBF_LCD_ON        = 1<<16,
  JSBF_LCD_BL_ON     = 1<<17,
  JSBF_LOCKED        = 1<<18,
  JSBF_HRM_INSTANT_LISTENER = 1<<19,
  JSBF_LCD_DBL_REFRESH = 1<<20, ///< On Bangle.js 2, toggle extcomin twice for each poll interval (avoids screen 'flashing' behaviour off axis)
  JSBF_MANUAL_WATCHDOG = 1<<21, ///< If set, we don't kick the WDT from the interrupt, so users can call it from their JS to ensure JS always stays running
#ifdef BANGLEJS_Q3
  /** On some Bangle.js 2, BTN1 (which is used for reloading apps) gets a low resistance across it
  (possibly due to water damage) and the internal resistor can no longer overcome that resistance
  so the button appears stuck on. With this fix we force the button pin low just before reading to try
  and overcome that resistance, and we also disable the button watch interrupt. */
  JSBF_BTN_LOW_RESISTANCE_FIX = 1<<22,
#endif
#ifdef BANGLEJS3
   JSBF_WIFI_ON = 1<<22,
#endif
#ifdef MIC_PIN
   JSBF_MIC_ON  = 1<<23,
#endif

  JSBF_DEFAULT = ///< default at power-on
      JSBF_WAKEON_TWIST|
      JSBF_WAKEON_BTN1|JSBF_WAKEON_BTN2|JSBF_WAKEON_BTN3
} JsBangleFlags;
extern volatile JsBangleFlags bangleFlags;


void jswrap_banglejs_lcdWr(JsVarInt cmd, JsVar *data);
void jswrap_banglejs_setLCDPower(bool isOn);
void jswrap_banglejs_setLCDPowerBacklight(bool isOn);
void jswrap_banglejs_setLCDBrightness(JsVarFloat v);
void jswrap_banglejs_setLCDMode(JsVar *mode);
JsVar *jswrap_banglejs_getLCDMode();
void jswrap_banglejs_setLCDOffset(int y);
void jswrap_banglejs_setLCDOverlay(JsVar *imgVar, JsVar *xv, int y, JsVar *options);
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

bool jswrap_banglejs_setWiFiPower(bool isOn, JsVar *appId);
int jswrap_banglejs_isWiFiOn();

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
void jswrap_banglejs_touchWr(JsVarInt reg, JsVarInt data);
JsVar *jswrap_banglejs_touchRd(JsVarInt reg, JsVarInt cnt);
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
JsVar *jswrap_banglejs_haptic(JsVar* eventName);


void jswrap_banglejs_off();
void jswrap_banglejs_softOff();
JsVar *jswrap_banglejs_getLogo();
void jswrap_banglejs_factoryReset(bool noReboot);
void jswrap_banglejs_showLoadingScreen();

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
#ifdef MIC_PIN
  JSBE_MIC_BUFFER, // new buffer of data from microphone
#endif
} JsBangleEvent;

/// Called from jsinteractive when an event is parsed from the event queue for Bangle.js (executed outside IRQ)
void jsbangle_exec_pending(uint8_t *data, int dataLen);
/// queue an event for Bangle.js (usually called from inside an IRQ)
void jsbangle_push_event(JsBangleEvent type, uint16_t value);

void jswrap_banglejs_powerusage(JsVar *devices);

// Called when we have an interrupt from the touchscreen
void jswrap_banglejs_touchHandler(bool state, IOEventFlags flags);

/** This is called to set whether an app requests a device to be on or off.
 * The value returned is whether the device should be on.
 * Devices: GPS/Compass/HRM/Barom/Mic
 */
bool setDeviceRequested(const char *deviceName, JsVar *appID, bool powerOn);

#endif // JSWRAP_BANGLE