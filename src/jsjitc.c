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

 https://developer.arm.com/documentation/ddi0308/d/Thumb-Instructions/Alphabetical-list-of-Thumb-instructions?lang=en
 https://web.eecs.umich.edu/~prabal/teaching/eecs373-f11/readings/ARMv7-M_ARM.pdf
 */
#ifdef ESPR_JIT
#include "jsjitc.h"
#include "jsinteractive.h"

#define DEBUG_JIT jsiConsolePrintf

void jsjcEmit16(uint16_t v) {
  DEBUG_JIT("> %04x\n", v);
}

void jsjcLiteral8(uint8_t reg, uint8_t data) {
  assert(reg<8);
  // https://web.eecs.umich.edu/~prabal/teaching/eecs373-f11/readings/ARMv7-M_ARM.pdf page 347
  int n = 0b0010000000000000 | (reg<<8) | data;
  jsjcEmit16(n);
}

void jsjcLiteral16(uint8_t reg, bool hi16, uint16_t data) {
  assert(reg<16);
  // https://web.eecs.umich.edu/~prabal/teaching/eecs373-f11/readings/ARMv7-M_ARM.pdf page 347
  int imm4,i,imm3,imm8;
  imm4 = (data>>12)&15;
  i = (data>>11)&1;
  imm3 = (data>>8)&7;
  imm8 = data&255;
  jsjcEmit16(0b1111001001000000 | (hi16?(1<<7):0)|  (i<<10) | imm4);
  jsjcEmit16((imm3<<12) | imm8 | (reg<<8));
}

void jsjcLiteral32(int reg, uint32_t data) {
  DEBUG_JIT("L32 r%d,0x%08x\n", reg,data);
  // bit shifted 8 bits? https://developer.arm.com/documentation/ddi0308/d/Thumb-Instructions/Immediate-constants/Encoding?lang=en
  // https://developer.arm.com/documentation/ddi0308/d/Thumb-Instructions/Alphabetical-list-of-Thumb-instructions/MOVT
  if (data<256) {
    jsjcLiteral8(reg, (uint8_t)data);
  } else if (data<65536) {
    jsjcLiteral16(reg, false, (uint16_t)data);
  } else {
    // FIXME - what about signed values?
    jsjcLiteral16(reg, false, (uint16_t)data);
    jsjcLiteral16(reg, true, (uint16_t)(data>>16));
  }
}

void jsjcLiteral64(int reg, uint64_t data) {
  jsjcLiteral32(reg, (uint32_t)(data>>32));
  jsjcLiteral32(reg+1, (uint32_t)data);
}

#ifdef DEBUG
void _jsjcCall(void *c, const char *name) {
  DEBUG_JIT("CALL 0x%08x %s\n", (uint32_t)c, name);
#else
void jsjcCall(void *c) {
  DEBUG_JIT("CALL 0x%08x\n", (uint32_t)c);
#endif
  jsjcLiteral32(7, (uint32_t)c); // save address to r7
  jsjcEmit16(0b0100011110000000 | (7<<3)); // BLX reg 7

}

void jsjcMov(int regFrom, int regTo) {
  DEBUG_JIT("MOV r%d r%d\n", regFrom, regTo);
  jsjcEmit16(0b0100011000000000 | ((regTo&8)?128:0) | (regFrom<<3) | (regTo&7));
}

void jsjcPush(int reg, JsjValueType type) {
  DEBUG_JIT("PUSH r%d\n", reg);
  jsjcEmit16(0b1011010000000000 | (1<<reg));
}

JsjValueType jsjcPop(int reg) {
  DEBUG_JIT("POP r%d\n", reg);
  jsjcEmit16(0b1011110000000000 | (1<<reg));
  return JSJVT_JSVAR; // FIXME
}

#endif /* ESPR_JIT */
