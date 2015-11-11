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
#ifndef TARGETS_ESP8266_JSWRAP_ESP8266_H_
#define TARGETS_ESP8266_JSWRAP_ESP8266_H_
#include "jsvar.h"
#include "jspin.h"

void   jswrap_ESP8266_neopixelWrite(Pin pin, JsVar *jsArrayOfData);

#endif /* TARGETS_ESP8266_JSWRAP_ESP8266_H_ */
