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
//#ifdef ESPR_JIT

#define DEBUG_JIT jsiConsolePrintf
#if defined(LINUX) && defined(DEBUG)
#define JIT_OUTPUT_FILE "jit.bin"
#endif

#include "jsjitc.h"
#include "jsinteractive.h"

#ifdef JIT_OUTPUT_FILE
#include <stdio.h>
FILE *f;
#endif
JsVar *jitCode = 0;

void jsjcStart() {
#ifdef JIT_OUTPUT_FILE
  f = fopen(JIT_OUTPUT_FILE, "wb");
#endif
  jitCode = jsvNewFromEmptyString();
}

JsVar *jsjcStop() {
#ifdef JIT_OUTPUT_FILE
  fclose(f);
#endif
  JsVar *v = jsvAsFlatString(jitCode);
  jsvUnLock(jitCode);
  jitCode = 0;
  return v;
}

void jsjcEmit16(uint16_t v) {
  DEBUG_JIT("> %04x\n", v);
#ifdef JIT_OUTPUT_FILE
  fputc(v&255, f);
  fputc(v>>8, f);
#endif
  jsvAppendStringBuf(jitCode, (char *)&v, 2);
}

void jsjcLiteral8(int reg, uint8_t data) {
  assert(reg<8);
  // https://web.eecs.umich.edu/~prabal/teaching/eecs373-f11/readings/ARMv7-M_ARM.pdf page 347
  int n = 0b0010000000000000 | (reg<<8) | data;
  jsjcEmit16((uint16_t)n);
}

void jsjcLiteral16(int reg, bool hi16, uint16_t data) {
  assert(reg<16);
  // https://web.eecs.umich.edu/~prabal/teaching/eecs373-f11/readings/ARMv7-M_ARM.pdf page 347
  int imm4,i,imm3,imm8;
  imm4 = (data>>12)&15;
  i = (data>>11)&1;
  imm3 = (data>>8)&7;
  imm8 = data&255;
  jsjcEmit16((uint16_t)(0b1111001001000000 | (hi16?(1<<7):0)|  (i<<10) | imm4));
  jsjcEmit16((uint16_t)((imm3<<12) | imm8 | (reg<<8)));
}

void jsjcLiteral32(int reg, uint32_t data) {
  DEBUG_JIT("L32 r%d,0x%08x\n", reg,data); // wrong?
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

int jsjcLiteralString(int reg, JsVar *str, bool nullTerminate) {
  /* We store the String data here in-line, so store the PC location then jump forward over the data. */
  int len = (int)jsvGetStringLength(str);
  int realLen = len + (nullTerminate?1:0);
  if (realLen&1) realLen++; // pad to even bytes
  // Write location of data to register
  jsjcMov(reg, JSJAR_PC);
  // jump over the data
  jsjcBranchRelative(realLen);
  // write the data
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  for (int i=0;i<realLen;i+=2) {
    unsigned int v = (unsigned)jsvStringIteratorGetCharAndNext(&it);
    v = v | (((unsigned)jsvStringIteratorGetCharAndNext(&it)) << 8);
    jsjcEmit16((uint16_t)v);
  }
  jsvStringIteratorFree(&it);
  // we should be fine now!
  return len;
}

void jsjcBranchRelative(int bytes) {
  DEBUG_JIT("B %d\n", (uint32_t)bytes);
  bytes -= 2; // because PC is ahead by 2
  assert(!(bytes&1)); // only multiples of 2 bytes
  assert(bytes>=-4096 && bytes<4096); // only multiples of 2 bytes
  int imm11 = (bytes>>1) & 2047;
  jsjcEmit16((uint16_t)(0b1110000000000000 | imm11)); // unconditional branch
  /*JsjAsmCondition cond = ...;
  int imm8 = (pos>>1) & 255;
  jsjcEmit16((uint16_t)(0b1101000000000000 | (cond<<8) | imm8));*/ // conditional branch
}

#ifdef DEBUG
void _jsjcCall(void *c, const char *name) {
  DEBUG_JIT("CALL 0x%08x %s\n", (uint32_t)(size_t)c, name);
#else
void jsjcCall(void *c) {
  DEBUG_JIT("CALL 0x%08x\n", (uint32_t)(size_t)c);
#endif
 /* if (((uint32_t)c) < 0x7FFFFF) { // BL + immediate(PC relative!)
    uint32_t v = ((uint32_t)c)>>1;
    jsjcEmit16((uint16_t)(0b1111000000000000 | ((v>>11)&0x7FF)));
    jsjcEmit16((uint16_t)(0b1111100000000000 | (v&0x7FF)));
  } else */{
    jsjcLiteral32(7, (uint32_t)(size_t)c); // save address to r7
    jsjcEmit16((uint16_t)(0b0100011110000000 | (7<<3))); // BL reg 7 - BROKEN?
  }

}

void jsjcMov(int regTo, int regFrom) {
  DEBUG_JIT("MOV r%d <- r%d\n", regTo, regFrom);
  jsjcEmit16((uint16_t)(0b0100011000000000 | ((regTo&8)?128:0) | (regFrom<<3) | (regTo&7)));
}

void jsjcPush(int reg, JsjValueType type) {
  DEBUG_JIT("PUSH {r%d}\n", reg);
  jsjcEmit16((uint16_t)(0b1011010000000000 | (1<<reg)));
}

JsjValueType jsjcPop(int reg) {
  DEBUG_JIT("POP {r%d}\n", reg);
  jsjcEmit16((uint16_t)(0b1011110000000000 | (1<<reg)));
  return JSJVT_JSVAR; // FIXME
}

void jsjcAddSP(int amt) {
  assert((amt&3)==0);
  DEBUG_JIT("ADD SP,SP,#%d\n", amt);
  jsjcEmit16((uint16_t)(0b1011000000000000 | (amt>>2)));
}

void jsjcPushAll() {
  DEBUG_JIT("PUSH {r4,r5,r6,r7,lr}\n");
  jsjcEmit16(0xb5f0);
}
void jsjcPopAllAndReturn() {
  DEBUG_JIT("POP {r4,r5,r6,r7,pc}\n");
  jsjcEmit16(0xbdf0);
}

/*void jsjcReturn() {
  DEBUG_JIT("BX LR\n");
  int reg = 14; // lr
  jsjcEmit16(0b0100011100000000 | (reg<<3));
}*/

//#endif /* ESPR_JIT */
