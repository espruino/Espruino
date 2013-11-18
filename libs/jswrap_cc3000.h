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

void jswrap_wlan_init();
void jswrap_wlan_start();
JsVarInt jswrap_wlan_connect(JsVar *vAP, JsVar *vKey);
JsVar *jswrap_wlan_getIP();

/// Check if the cc3000's socket has disconnected (clears flag as soon as is called)
bool cc3000_socket_has_closed(int socketNum);
