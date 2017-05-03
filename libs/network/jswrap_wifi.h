/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains WiFi functions
 * ----------------------------------------------------------------------------
 */

#include "jsvar.h"

void   jswrap_wifi_connect(JsVar *jsSsid, JsVar *jsOptions, JsVar *jsCallback);
void   jswrap_wifi_disconnect(JsVar *jsCallback);
void   jswrap_wifi_startAP(JsVar *jsSsid, JsVar *jsOptions, JsVar *jsCallback);
void   jswrap_wifi_stopAP(JsVar *jsCallback);
void   jswrap_wifi_scan(JsVar *jsCallback);
void   jswrap_wifi_save(JsVar *what);
void   jswrap_wifi_restore(void);
JsVar *jswrap_wifi_getStatus(JsVar *jsCallback);
void   jswrap_wifi_setConfig(JsVar *jsOptions);
JsVar *jswrap_wifi_getDetails(JsVar *jsCallback);
JsVar *jswrap_wifi_getAPDetails(JsVar *jsCallback);
JsVar *jswrap_wifi_getIP(JsVar *jsCallback);
JsVar *jswrap_wifi_getAPIP(JsVar *jsCallback);
JsVar *jswrap_wifi_getHostname(JsVar *jsCallback);
void   jswrap_wifi_setHostname(JsVar *jsHostname);
void   jswrap_wifi_getHostByName(JsVar *jsHostname, JsVar *jsCallback);
void   jswrap_wifi_setSNTP(JsVar *zone, JsVar *server);
void   jswrap_wifi_setIP(JsVar *jsSettings, JsVar *jsCallback);
void   jswrap_wifi_setAPIP(JsVar *jsSettings, JsVar *jsCallback);
