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
#include "jsjitc.h"
#include "jsinteractive.h"

void jsjcLiteral32(int reg, uint32_t data) {
  jsiConsolePrintf("L32 r%d,0x%08x\n", reg,data);
}

void jsjcLiteral64(int reg, uint64_t data) {
  jsjcLiteral32(reg, (uint32_t)(data>>32));
  jsjcLiteral32(reg+1, (uint32_t)data);
}

#ifdef DEBUG
void _jsjcCall(void *c, const char *name) {
  jsiConsolePrintf("CALL 0x%08x %s\n", (uint32_t)c, name);
#else
void jsjcCall(void *c) {
  jsiConsolePrintf("CALL 0x%08x\n", (uint32_t)c);
#endif
}

void jsjcMov(int regFrom, int regTo) {
  jsiConsolePrintf("MOV r%d r%d\n", regFrom, regTo);
}

void jsjcPush(int reg, JsjValueType type) {
  jsiConsolePrintf("PUSH r%d\n", reg);
}

JsjValueType jsjcPop(int reg) {
  jsiConsolePrintf("POP r%d\n", reg);
  return JSJVT_JSVAR; // FIXME
}

#endif /* ESPR_JIT */
