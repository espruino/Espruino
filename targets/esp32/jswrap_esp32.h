/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP32 board specific function definitions.
 * ----------------------------------------------------------------------------
 */
#ifndef TARGETS_ES32_JSWRAP_ESP32_H_
#define TARGETS_ESP32_JSWRAP_ESP32_H_
#include "jsvar.h"
#include "jspin.h"

//===== ESP32 Library
JsVar *jswrap_ESP32_getState();
JsVar *jswrap_ESP32_setBoot(JsVar *jsPartitionName);
void   jswrap_ESP32_reboot();
void   jswrap_ESP32_deepSleep(int us);
void   jswrap_ESP32_deepSleep_ext0(Pin pin,int level);
void   jswrap_ESP32_deepSleep_ext1(JsVar *pinVar, JsVarInt mode);
int    jswrap_ESP32_getWakeupCause();
void   jswrap_ESP32_setAtten(Pin pin,int atten);

#ifdef BLUETOOTH
void  jswrap_ESP32_setBLE_Debug(int level);
void  jswrap_ESP32_enableBLE(bool enable);
#endif
void jswrap_ESP32_enableWifi(bool enable);
void jswrap_ESP32_setOTAValid(bool isValid);
#endif /* TARGETS_ESP32_JSWRAP_ESP32_H_ */
