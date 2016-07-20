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

#ifndef LIBS_NETWORK_ESP8266_JSWRAP_ESP8266_NETWORK_H_
#define LIBS_NETWORK_ESP8266_JSWRAP_ESP8266_NETWORK_H_
#include "jsvar.h"

//===== Wifi library
void   jswrap_ESP8266_wifi_init1();
void   jswrap_ESP8266_wifi_soft_init();
void   jswrap_ESP8266_wifi_connect(JsVar *jsSsid, JsVar *jsOptions, JsVar *jsCallback);
void   jswrap_ESP8266_wifi_disconnect(JsVar *jsCallback);
void   jswrap_ESP8266_wifi_startAP(JsVar *jsSsid, JsVar *jsOptions, JsVar *jsCallback);
void   jswrap_ESP8266_wifi_stopAP(JsVar *jsCallback);
void   jswrap_ESP8266_wifi_scan(JsVar *jsCallback);
void   jswrap_ESP8266_wifi_save(JsVar *what);
void   jswrap_ESP8266_wifi_restore(void);
JsVar *jswrap_ESP8266_wifi_getStatus(JsVar *jsCallback);
void   jswrap_ESP8266_wifi_setConfig(JsVar *jsOptions);
JsVar *jswrap_ESP8266_wifi_getDetails(JsVar *jsCallback);
JsVar *jswrap_ESP8266_wifi_getAPDetails(JsVar *jsCallback);
JsVar *jswrap_ESP8266_wifi_getIP(JsVar *jsCallback);
JsVar *jswrap_ESP8266_wifi_getAPIP(JsVar *jsCallback);
JsVar *jswrap_ESP8266_wifi_getHostname(JsVar *jsCallback);
void   jswrap_ESP8266_wifi_setHostname(JsVar *jsHostname);
void   jswrap_ESP8266_wifi_getHostByName(JsVar *jsHostname, JsVar *jsCallback);
void   jswrap_ESP8266_wifi_setSNTP(JsVar *zone, JsVar *server);

#endif /* LIBS_NETWORK_ESP8266_JSWRAP_ESP8266_NETWORK_H_ */
