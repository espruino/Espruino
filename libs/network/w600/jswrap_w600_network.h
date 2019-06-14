/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2019 BeanJS 
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains W600 and Wifi library specific functions.
 *
 * FOR DESCRIPTIONS OF THE WIFI FUNCTIONS IN THIS FILE, SEE
 * libs/network/jswrap_wifi.c (or http://www.espruino.com/Reference#Wifi)
 * ----------------------------------------------------------------------------
 */

#include "jswrap_wifi.h"

#ifndef __JSWRAP_W600_NETWORK_H__
#define __JSWRAP_W600_NETWORK_H__
#include "jsvar.h"

void   jswrap_wifi_init();
void   jswrap_wifi_soft_init();

#endif /* __JSWRAP_W600_NETWORK_H__ */