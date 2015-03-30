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
 * Implementation of JsNetwork for ESP8266 devices
 *
 * DEPRECATED - YOU SHOULD NOW USE THE ESP8266.js MODULE
 * ----------------------------------------------------------------------------
 */
#include "network.h"

void esp8266_send(JsVar *msg);
bool esp8266_wait_for(const char *text, int milliseconds, bool justTheStart);

bool net_esp8266_initialise(JsVar *callback);
bool net_esp8266_connect(JsVar *vAP, JsVar *vKey, JsVar *callback);

void netSetCallbacks_esp8266(JsNetwork *net);
