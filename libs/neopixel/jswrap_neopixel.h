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
 * Contains JavaScript interface for Neopixel/WS281x/APA10x devices
 * ----------------------------------------------------------------------------
 */

#include "jspin.h"
#include "jsvar.h"

void jswrap_neopixel_write(Pin pin, JsVar *data);
