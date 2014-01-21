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
 * Contains built-in functions for CC3000 WiFi Access
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_cc3000_connect();
JsVarInt jswrap_wlan_connect(JsVar *wlanObj, JsVar *vAP, JsVar *vKey, JsVar *callback);
void jswrap_wlan_disconnect(JsVar *wlanObj);
JsVar *jswrap_wlan_getIP(JsVar *wlanObj);

/// Check if the cc3000's socket has disconnected (clears flag as soon as is called)
bool cc3000_socket_has_closed(int socketNum);
