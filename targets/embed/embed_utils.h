/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2022 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Exported functions that are useful when extending Espruino
 * ----------------------------------------------------------------------------
 */

#ifndef ESPRUINO_EMBEDDED_UTILS_H_
#define ESPRUINO_EMBEDDED_UTILS_H_

#include "espruino_embedded.h"

#ifdef __cplusplus
extern "C" {
#endif

extern JsVar *jsVars;

typedef int32_t JsVarInt;
typedef double JsVarFloat;

size_t jsvGetString(const JsVar *v, char *str, size_t len);
JsVar *jsvAsString(JsVar *v);
size_t jsvGetStringLength(const JsVar *v);
JsVar *jswrap_json_stringify(JsVar *v, JsVar *replacer, JsVar *space);
JsVar *jswrap_json_parse(JsVar *v);
void jsExceptionHere(JsExceptionType type, const char *fmt, ...);
JsVar *jsvObjectGetChild(JsVar *obj, const char *name, unsigned short createChild);
JsVar *jsvLockAgainSafe(JsVar *var);
void jsvUnLock(JsVar *var);
bool jsvIsMemoryFull();

bool jsvIsBoolean(const JsVar *v);
bool jsvIsString(const JsVar *v);
bool jsvIsFunction(const JsVar *v);
bool jsvIsNumeric(const JsVar *v);
bool jsvIsObject(const JsVar *v);
bool jsvIsArray(const JsVar *v);
bool jsvIsNull(const JsVar *v);

JsVar *jsvNewFromString(const char *str);
JsVar *jsvNewFromInteger(JsVarInt value);
JsVar *jsvNewFromBool(bool value);
JsVar *jsvNewFromFloat(JsVarFloat value);
JsVar *jsvNewFromLongInteger(long long value);
JsVar *jsvNewEmptyArray();
JsVar *jsvNewArray(JsVar **elements, int elementCount);

#ifdef __cplusplus
}
#endif

#endif  // ESPRUINO_EMBEDDED_UTILS_H_
