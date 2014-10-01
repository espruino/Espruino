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
 * Contains built-in functions for WIZnet Ethernet Access
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"
#include "jspin.h"

JsVar *jswrap_wiznet_connect(JsVar *spi, Pin cs);

JsVar *jswrap_ethernet_getIP(JsVar *wlanObj);
bool jswrap_ethernet_setIP(JsVar *wlanObj, JsVar *options);
