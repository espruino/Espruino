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
 * JavaScript Hardware IO Functions
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"
#include "jsinteractive.h"

JsVar *jswrap_io_peek(JsVarInt addr, JsVarInt count, int wordSize);
void jswrap_io_poke(JsVarInt addr, JsVar *data, int wordSize);

void jswrap_io_analogWrite(Pin pin, JsVarFloat value, JsVar *options);
void jswrap_io_digitalPulse(Pin pin, bool value, JsVar *times);
void jswrap_io_digitalWrite(JsVar *pinVar, JsVarInt value);
JsVarInt jswrap_io_digitalRead(JsVar *pinVar);
void jswrap_io_pinMode(Pin pin, JsVar *mode);
JsVar *jswrap_io_getPinMode(Pin pin);

JsVar *jswrap_interface_setWatch(JsVar *funcVar, Pin pin, JsVar *repeatOrObject);
void jswrap_interface_clearWatch(JsVar *idVar);
