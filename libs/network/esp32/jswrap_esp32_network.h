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
 * Contains ESP32 and Wifi library specific functions.
 *
 * FOR DESCRIPTIONS OF THE WIFI FUNCTIONS IN THIS FILE, SEE
 * libs/network/jswrap_wifi.c (or http://www.espruino.com/Reference#Wifi)
 * ----------------------------------------------------------------------------
 */

#include "jswrap_wifi.h"

void jswrap_ESP32_ping(
    JsVar *ipAddr,      //!< A string or integer representation of an IP address.
    JsVar *pingCallback //!< Optional callback function.
);

void esp32_wifi_init();
void esp32_wifi_soft_init();
