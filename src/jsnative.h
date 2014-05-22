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
 * For calling built-in functions
 * ----------------------------------------------------------------------------
 */

#ifndef JSNATIVE_H
#define JSNATIVE_H

#include "jsutils.h"
#include "jsvar.h"
#include "jswrapper.h"

#include <stdint.h> // uint32_t

#if __WORDSIZE == 64
 #define JSWAT_IS_64BIT(N) (\
  (N)==JSWAT_JSVAR || \
  (N)==JSWAT_ARGUMENT_ARRAY || \
  (N)==JSWAT_JSVARFLOAT \
 )
#else
#define JSWAT_IS_64BIT(N) (\
  (N)==JSWAT_JSVARFLOAT \
 )
#endif

/** argumentSpecifier is actually a set of JsnArgumentType. The one at bit 0
 * is the return type
 */
JsVar *jsnCallFunction(void *function, JsnArgumentType argumentSpecifier, JsVar *thisParam, JsVar **paramData, int paramCount) ;


#endif //JSNATIVE_H
