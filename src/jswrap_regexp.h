/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * JavaScript methods for Regular Expressions
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_regexp_constructor(JsVar *str, JsVar *flags);
JsVar *jswrap_regexp_exec(JsVar *parent, JsVar *str);
bool jswrap_regexp_test(JsVar *parent, JsVar *str);

/// Does this regex have the given flag?
bool jswrap_regexp_hasFlag(JsVar *parent, char flag);
