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

#include <stdint.h> // uint32_t

typedef enum {
  JSNAT_FINISH = 0, // no argument
  JSNAT_VOID = 0, // Only for return values
  JSNAT_JSVAR, // standard variable
  JSNAT_JSVARNAME, // variable without the name being stripped off
  JSNAT_ARGUMENT_ARRAY, // a JsVar array containing all subsequent arguments
  JSNAT_BOOL, // boolean
  JSNAT_INT16, // 16 bit int
  JSNAT_INT32, // 32 bit int
  JSNAT_PIN, // A pin
  JSNAT_JSVARINT, // 64 bit int
  JSNAT_JSVARFLOAT, // 64 bit float
  JSNAT__LAST = JSNAT_JSVARFLOAT,
  JSNAT_MASK = NEXT_POWER_2(JSNAT__LAST)-1
} JsnArgumentType;

// number of bits needed for each argument bit
#define JSNAT_BITS GET_BIT_NUMBER(JSNAT_MASK+1)

#if __WORDSIZE == 64
 #define JSNAT_IS_64BIT(N) (\
  (N)==JSNAT_JSVAR || \
  (N)==JSNAT_ARGUMENT_ARRAY || \
  (N)==JSNAT_JSVARINT || \
  (N)==JSNAT_JSVARFLOAT \
 )
#else
#define JSNAT_IS_64BIT(N) (\
  (N)==JSNAT_JSVARINT || \
  (N)==JSNAT_JSVARFLOAT \
 )
#endif

/** argumentSpecifier is actually a set of JsnArgumentType. The one at bit 0
 * is the return type
 */
JsVar *jsnCallFunction(void *function, unsigned int argumentSpecifier, JsVar **paramData, int paramCount);

#endif //JSNATIVE_H
