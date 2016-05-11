/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2016 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * ES6 Promise implementation
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_promise_constructor(JsVar *executor);
JsVar *jswrap_promise_all(JsVar *arr);
JsVar *jswrap_promise_reject(JsVar *data);
JsVar *jswrap_promise_resolve(JsVar *data);
JsVar *jswrap_promise_then(JsVar *parent, JsVar *callback);
JsVar *jswrap_promise_catch(JsVar *parent, JsVar *callback);
