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
#include "jsutils.h"

#ifndef ESPR_NO_PROMISES

/// Create a new promise
JsVar *jspromise_create();
/// Resolve the given promise
void jspromise_resolve(JsVar *promise, JsVar *data);
/// Reject the given promise
void jspromise_reject(JsVar *promise, JsVar *data);

JsVar *jswrap_promise_constructor(JsVar *executor);
JsVar *jswrap_promise_all(JsVar *arr);
JsVar *jswrap_promise_reject(JsVar *data);
JsVar *jswrap_promise_resolve(JsVar *data);
JsVar *jswrap_promise_then(JsVar *parent, JsVar *onFulfilled, JsVar *onRejected);
JsVar *jswrap_promise_catch(JsVar *parent, JsVar *onRejected);

#endif
