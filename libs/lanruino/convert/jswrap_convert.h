/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 lancer
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef LIBS_LANRUINO_CONVERT_H_
#define LIBS_LANRUINO_CONVERT_H_
#include "jsvar.h"
#include "jsinteractive.h"

bool jswrap_convert_isLittleEndian();

JsVar *jswrap_convert_floatToBytes(JsVar *floatVar);
JsVar *jswrap_convert_doubleToBytes(JsVar *doubleVar);
JsVar *jswrap_convert_bytesToFloat(JsVar *bytes);
JsVar *jswrap_convert_bytesToDouble(JsVar *bytes);

#endif