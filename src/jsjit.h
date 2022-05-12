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
 * Recursive descent JIT
 * ----------------------------------------------------------------------------
 */
#ifdef ESPR_JIT
#ifndef JSJIT_H_
#define JSJIT_H_

#ifndef RELEASE
#define DEBUG_JIT_CALLS
#endif

#include "jsparse.h"

JsVar *jsjEvaluateVar(JsVar *str);
JsVar *jsjEvaluate(const char *str);

// parse a function and return a native string of the code. Assumes '{' has already been parsed
JsVar *jsjParseFunction();

#endif /* JSJIT_H_ */
#endif /* ESPR_JIT */
