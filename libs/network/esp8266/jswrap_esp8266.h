/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains built-in functions for Espressif ESP8266 WiFi Access
 *
 * DEPRECATED - YOU SHOULD NOW USE THE ESP8266.js MODULE
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_esp8266_connect_device(JsVar *usart, JsVar *callback);
bool jswrap_esp8266_connect(JsVar *wlanObj, JsVar *vAP, JsVar *vKey, JsVar *callback);

JsVar *jswrap_esp8266_getIP(JsVar *wlanObj);
bool jswrap_esp8266_setIP(JsVar *wlanObj, JsVar *options);
