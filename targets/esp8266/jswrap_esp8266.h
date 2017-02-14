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
 * Contains ESP8266 board specific function definitions.
 * ----------------------------------------------------------------------------
 */
#ifndef TARGETS_ESP8266_JSWRAP_ESP8266_H_
#define TARGETS_ESP8266_JSWRAP_ESP8266_H_
#include "jsvar.h"
#include "jspin.h"
#include "log.h"

//===== ESP8266 Library
JsVar *jswrap_ESP8266_getResetInfo();
JsVar *jswrap_ESP8266_getState();
void   jswrap_ESP8266_dumpSocketInfo(void);
void   jswrap_ESP8266_ping(JsVar *jsIpAddr, JsVar *jsPingCallback);
void   jswrap_ESP8266_reboot();
void   jswrap_ESP8266_setCPUFreq(JsVar *jsFreq);
JsVar *jswrap_ESP8266_crc32(JsVar *jsData);
JsVar *jswrap_ESP8266_getFreeFlash();

void   jswrap_ESP8266_logDebug(bool Debug);
void   jswrap_ESP8266_setLog(int mode);
void   jswrap_ESP8266_printLog();

void   jswrap_ESP8266_wifi_pre_init();
void   jswrap_ESP8266_wifi_init();
void   jswrap_ESP8266_wifi_reset();

void   jswrap_ESP8266_neopixelWrite(Pin pin, JsVar *jsArrayOfData);

uint32_t crc32(uint8_t *buf, uint32_t len);

void   jswrap_ESP8266_deepSleep(JsVar *jsMicros, JsVar *jsOption);

#endif /* TARGETS_ESP8266_JSWRAP_ESP8266_H_ */
