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
 * JavaScript methods for Arrays
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_array_constructor(JsVar *args);
bool jswrap_array_contains(JsVar *parent, JsVar *value);
JsVar *jswrap_array_indexOf(JsVar *parent, JsVar *value, JsVarInt startIdx);
JsVar *jswrap_array_join(JsVar *parent, JsVar *filler);
JsVarInt jswrap_array_push(JsVar *parent, JsVar *args);
JsVar *jswrap_array_map(JsVar *parent, JsVar *funcVar, JsVar *thisVar);
JsVar *jswrap_array_shift(JsVar *parent);
JsVarInt jswrap_array_unshift(JsVar *parent, JsVar *elements);
JsVar *jswrap_array_slice(JsVar *parent, JsVarInt start, JsVar *endVar);
JsVar *jswrap_array_splice(JsVar *parent, JsVarInt index, JsVar *howManyVar, JsVar *elements);
void jswrap_array_forEach(JsVar *parent, JsVar *funcVar, JsVar *thisVar);
JsVar *jswrap_array_filter(JsVar *parent, JsVar *funcVar, JsVar *thisVar);
JsVar *jswrap_array_some(JsVar *parent, JsVar *funcVar, JsVar *thisVar);
JsVar *jswrap_array_every(JsVar *parent, JsVar *funcVar, JsVar *thisVar);
JsVar *jswrap_array_reduce(JsVar *parent, JsVar *funcVar, JsVar *initialValue);
JsVar *jswrap_array_sort (JsVar *array, JsVar *compareFn);
JsVar *jswrap_array_concat(JsVar *parent, JsVar *args);
JsVar *jswrap_array_fill(JsVar *parent, JsVar *value, JsVarInt start, JsVar *endVar);
JsVar *jswrap_array_reverse(JsVar *parent);
