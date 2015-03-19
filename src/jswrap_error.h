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
 * JavaScript methods for Errors
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_error_constructor(JsVar *msg);
JsVar *jswrap_syntaxerror_constructor(JsVar *msg);
JsVar *jswrap_typeerror_constructor(JsVar *msg);
JsVar *jswrap_internalerror_constructor(JsVar *msg);

JsVar *jswrap_error_toString(JsVar *parent);
