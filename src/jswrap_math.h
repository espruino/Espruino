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
 * Contains built-in functions for Maths
 * ----------------------------------------------------------------------------
 */
#ifndef JSWRAP_MATH_H_
#define JSWRAP_MATH_H_

#include "jsutils.h"
#include "jsvar.h"

#ifndef ESPR_EMBED
#include <math.h>
#endif

#define PI (3.141592653589793)


JsVarInt jswrap_integer_valueOf(JsVar *v);
JsVarFloat jswrap_math_abs(JsVarFloat x);
JsVarFloat jswrap_math_asin(JsVarFloat x);
double jswrap_math_mod(double x, double y);
double jswrap_math_pow(double x, double y);
JsVar *jswrap_math_round(double x);
double jswrap_math_sqrt(double x);
double jswrap_math_sin(double x);
double jswrap_math_cos(double x);
double jswrap_math_atan(double x);
double jswrap_math_atan2(double y, double x);
JsVarFloat jswrap_math_clip(JsVarFloat x, JsVarFloat min, JsVarFloat max);
JsVarFloat jswrap_math_minmax(JsVar *args, bool isMax);
int jswrap_math_sign(double x);

#endif // JSWRAP_MATH_H_
