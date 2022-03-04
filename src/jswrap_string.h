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
 * JavaScript String Functions
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_string_constructor(JsVar *a);
JsVar *jswrap_string_fromCharCode(JsVar *arr);
JsVar *jswrap_string_charAt(JsVar *parent, JsVarInt idx);
int jswrap_string_charCodeAt(JsVar *parent, JsVarInt idx);
int jswrap_string_indexOf(JsVar *parent, JsVar *substring, JsVar *fromIndex, bool lastIndexOf);
JsVar *jswrap_string_match(JsVar *parent, JsVar *subStr);
JsVar *jswrap_string_replace(JsVar *parent, JsVar *subStr, JsVar *newSubStr);
JsVar *jswrap_string_substring(JsVar *parent, JsVarInt pStart, JsVar *vEnd);
JsVar *jswrap_string_substr(JsVar *parent, JsVarInt pStart, JsVar *vLen);
JsVar *jswrap_string_slice(JsVar *parent, JsVarInt pStart, JsVar *vEnd);
JsVar *jswrap_string_split(JsVar *parent, JsVar *split);
JsVar *jswrap_string_toUpperLowerCase(JsVar *parent, bool upper);
JsVar *jswrap_string_trim(JsVar *parent);
JsVar *jswrap_string_concat(JsVar *parent, JsVar *args);
bool jswrap_string_startsWith(JsVar *parent, JsVar *search, int position);
bool jswrap_string_endsWith(JsVar *parent, JsVar *search, JsVar *length);
JsVar *jswrap_string_repeat(JsVar *parent, int count);
JsVar *jswrap_string_padX(JsVar *str, int targetLength, JsVar *padString, bool padStart);
