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
#ifndef JSJITC_H_
#define JSJITC_H_

#include "jsutils.h"
#include "jsjit.h"

typedef enum {
  JSJVT_INT,
  JSJVT_JSVAR
} JsjValueType;

// Add 32 bit literal
void jsjcLiteral32(int reg, uint32_t data);
// Add 64 bit literal in reg,reg+1
void jsjcLiteral64(int reg, uint64_t data);
// Call a function
#ifdef DEBUG
void _jsjcCall(void *c, const char *name);
#define jsjcCall(c) _jsjcCall(c, STRINGIFY(c))
#else
void jsjcCall(void *c);
#endif
// Move one register to another
void jsjcMov(int regFrom, int regTo);
// Push a register onto the stack
void jsjcPush(int reg, JsjValueType type);
// Pop off teh stack to a register
JsjValueType jsjcPop(int reg);

#endif /* JSJIT_H_ */
#endif /* ESPR_JIT */
