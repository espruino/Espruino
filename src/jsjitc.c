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

 optimisations to do:

 * Allow us to check what the last instruction was, and to replace it. Can then do peephole optimisations:
   * 'push+pop' is just a 'mov' (or maybe even nothing)
   *
 * Use a String iterator for writing to jit.code - it'll be a lot faster

 */
#ifdef ESPR_JIT

#if defined(LINUX) && defined(DEBUG)
#define JIT_OUTPUT_FILE "jit.bin"
#endif

#include "jsjitc.h"
#include "jsinteractive.h"
#include "jsflags.h"

#ifdef JIT_OUTPUT_FILE
#include <stdio.h>
#endif

// JIT state
JsjInfo jit;

const char *jsjcGetTypeName(JsjValueType t) {
  switch(t) {
    case JSJVT_INT: return "int";
    case JSJVT_JSVAR: return "JsVar";
    case JSJVT_JSVAR_NO_NAME: return "JsVar-value";
    default: return "unknown";
  }
}

void jsjcDebugPrintf(const char *fmt, ...) {
  if (jsFlags & JSF_JIT_DEBUG) {
    if (!jit.blockCount) jsiConsolePrintf("%6x: ", jsjcGetByteCount());
    else jsiConsolePrintf("       : ");
    va_list argp;
    va_start(argp, fmt);
    vcbprintf((vcbprintf_callback)jsiConsolePrint,0, fmt, argp);
    va_end(argp);
  }
}

void jsjcStart() {
  jit.phase = JSJP_UNKNOWN;
  jit.code = jsvNewFromEmptyString();
  jit.initCode = jsvNewFromEmptyString(); // FIXME: maybe we don't need this?
  jit.blockCount = 0;
  jit.vars = jsvNewObject();
  jit.varCount = 0;
  jit.stackDepth = 0;
}

JsVar *jsjcStop() {
  jsjcDebugPrintf("VARS: %j\n", jit.vars);
  jsvUnLock(jit.vars);
  jit.vars = 0;
  assert(jit.stackDepth == 0);

  assert(jit.blockCount==0);
#ifdef JIT_OUTPUT_FILE
  FILE *f = fopen(JIT_OUTPUT_FILE, "wb");
  JSV_GET_AS_CHAR_ARRAY(icPtr, icLen, jit.initCode);
  if (icPtr) fwrite(icPtr, 1, icLen, f);
  JSV_GET_AS_CHAR_ARRAY(cPtr, cLen, jit.code);
  if (cPtr) fwrite(cPtr, 1, cLen, f);
  fclose(f);
#endif
  // Like AsFlatString but we need to concat two blocks instead
  size_t len = jsvGetStringLength(jit.code) + jsvGetStringLength(jit.initCode);
  JsVar *flat = jsvNewFlatStringOfLength((unsigned int)len);
  if (flat) {
    JsvStringIterator src;
    JsvStringIterator dst;
    jsvStringIteratorNew(&src, jit.initCode, 0);
    jsvStringIteratorNew(&dst, flat, 0);
    while (jsvStringIteratorHasChar(&src)) {
      jsvStringIteratorSetCharAndNext(&dst, jsvStringIteratorGetCharAndNext(&src));
    }
    jsvStringIteratorFree(&src);
    jsvStringIteratorNew(&src, jit.code, 0);
    while (jsvStringIteratorHasChar(&src)) {
      jsvStringIteratorSetCharAndNext(&dst, jsvStringIteratorGetCharAndNext(&src));
    }
    jsvStringIteratorFree(&src);
    jsvStringIteratorFree(&dst);
  }
  jsvUnLock(jit.code);
  jit.code = 0;
  jsvUnLock(jit.initCode);
  jit.code = 0;
  return flat;
}

// Called before start of a block of code. Returns the old code jsVar that should be passed into jsjcStopBlock
JsVar *jsjcStartBlock() {
  if (jit.phase != JSJP_EMIT) return 0; // ignore block changes if not in emit phase
  JsVar *v = jit.code;
  jit.code = jsvNewFromEmptyString();
  jit.blockCount++;
  return v;
}

// Called to start writing to 'init code' (which is inserted before everything else). Returns the old code jsVar that should be passed into jsjcStopBlock
JsVar *jsjcStartInitCodeBlock() {
  JsVar *v = jit.code;
  jit.code = jsvLockAgain(jit.initCode);
  jit.blockCount++;
  return v;
}

// Called when JIT output stops, pass it the return value from jsjcStartBlock. Returns the code parsed in the block
JsVar *jsjcStopBlock(JsVar *oldBlock) {
  if (jit.phase != JSJP_EMIT) return 0; // ignore block changes if not in emit phase
  JsVar *v = jit.code;
  jit.code = oldBlock;
  jit.blockCount--;
  return v;
}

void jsjcEmit16(uint16_t v) {
  //DEBUG_JIT("> %04x\n", v);
  jsvAppendStringBuf(jit.code, (char *)&v, 2);
}

// Emit a whole block of code
void jsjcEmitBlock(JsVar *block) {
  DEBUG_JIT("... code block ...\n");
  JsvStringIterator it;
  jsvStringIteratorNew(&it, block, 0);
  while (jsvStringIteratorHasChar(&it)) {
    unsigned int v = (unsigned)jsvStringIteratorGetCharAndNext(&it);
    v = v | (((unsigned)jsvStringIteratorGetCharAndNext(&it)) << 8);
    jsjcEmit16((uint16_t)v);
  }
  jsvStringIteratorFree(&it);
}

int jsjcGetByteCount() {
  return jsvGetStringLength(jit.code);
}

void jsjcLiteral8(int reg, uint8_t data) {
  assert(reg<8);
  // https://web.eecs.umich.edu/~prabal/teaching/eecs373-f11/readings/ARMv7-M_ARM.pdf page 347
  // https://developer.arm.com/documentation/ddi0308/d/Thumb-Instructions/Alphabetical-list-of-Thumb-instructions/MOV--immediate-
  int n = 0b0010000000000000 | (reg<<8) | data;
  jsjcEmit16((uint16_t)n);
}

void jsjcLiteral16(int reg, bool hi16, uint16_t data) {
  assert(reg<16);
  // https://web.eecs.umich.edu/~prabal/teaching/eecs373-f11/readings/ARMv7-M_ARM.pdf page 347
  // https://developer.arm.com/documentation/ddi0308/d/Thumb-Instructions/Alphabetical-list-of-Thumb-instructions/MOV--immediate-
  int imm4,i,imm3,imm8;
  imm4 = (data>>12)&15;
  i = (data>>11)&1;
  imm3 = (data>>8)&7;
  imm8 = data&255;
  jsjcEmit16((uint16_t)(0b1111001001000000 | (hi16?(1<<7):0)|  (i<<10) | imm4));
  jsjcEmit16((uint16_t)((imm3<<12) | imm8 | (reg<<8)));
}

void jsjcLiteral32(int reg, uint32_t data) {
  DEBUG_JIT("MOV r%d,#0x%08x\n", reg,data);
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
  DEBUG_JIT("... %d bytes data (%q) ...\n", (uint32_t)(realLen), str);
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

// Compare a register with a literal. jsjcBranchConditionalRelative can then be called
void jsjcCompareImm(int reg, int literal) {
  DEBUG_JIT("CMP r%d,#%d\n", reg, literal);
  assert(reg<16);
  assert(literal>=0 && literal<256); // only multiples of 2 bytes
  int imm8 = literal & 255;
  jsjcEmit16((uint16_t)(0b0010100000000000 | (reg<<8) | imm8)); // unconditional branch
}

void jsjcBranchRelative(int bytes) {
  DEBUG_JIT("B %s%d (addr 0x%04x)\n", (bytes>0)?"+":"", (uint32_t)(bytes), jsjcGetByteCount()+bytes);
  bytes -= 2; // because PC is ahead by 2
  assert(!(bytes&1)); // only multiples of 2 bytes
  assert(bytes>=-2048 && bytes<2048); // check it's in range...
  if (bytes<-2048 || bytes>=2048) {
    // out of range, so use jsjcBranchConditionalRelative that can handle big jumps
    jsjcBranchConditionalRelative(JSJAC_AL, bytes+2);
  } else {
    int imm11 = ((unsigned int)(bytes)>>1) & 2047;
    jsjcEmit16((uint16_t)(0b1110000000000000 | imm11)); // unconditional branch
  }
}

// Jump a number of bytes forward or back, based on condition flags
void jsjcBranchConditionalRelative(JsjAsmCondition cond, int bytes) {
  bytes -= 2; // because PC is ahead by 2
  assert(!(bytes&1)); // only multiples of 2 bytes
  if (bytes>=-256 && bytes<256) { // B<c>
    DEBUG_JIT("B<%d> %s%d (addr 0x%04x)\n", cond, (bytes>0)?"+":"", (uint32_t)(bytes), jsjcGetByteCount()+bytes);
    int imm8 = (bytes>>1) & 255;
    jsjcEmit16((uint16_t)(0b1101000000000000 | (cond<<8) | imm8)); // conditional branch
  } else if (bytes>=-1048576 && bytes<(1048576-2)) { // B<c>.W
    bytes += 2; // must pad out by 1 byte because this is a double-length instruction!
    DEBUG_JIT("B<%d>.W %s%d (addr 0x%04x)\n", cond, (bytes>0)?"+":"", (uint32_t)(bytes), jsjcGetByteCount()+bytes);
    int imm20 = (bytes>>1);
    int S = (imm20>>19) & 1;
    int J2 = (imm20>>18) & 1;
    int J1 = (imm20>>17) & 1;
    int imm6 = (imm20>>11) & 63;
    int imm11 = imm20 & 2047;
    jsjcEmit16((uint16_t)(0b1111000000000000 | (S<<10) | (cond<<6) | imm6)); // conditional branch
    jsjcEmit16((uint16_t)(0b1000000000000000 | (J1<<13) | (J2<<11) | imm11)); // conditional branch
  } else
    jsExceptionHere(JSET_ERROR, "JIT: B<> jump (%d) out of range", bytes);
}

#ifdef DEBUG_JIT_CALLS
void _jsjcCall(void *c, const char *name) {
#else
void jsjcCall(void *c) {
#endif
 /* if (((uint32_t)c) < 0x7FFFFF) { // BL + immediate(PC relative!)
    uint32_t v = ((uint32_t)c)>>1;
    jsjcEmit16((uint16_t)(0b1111000000000000 | ((v>>11)&0x7FF)));
    jsjcEmit16((uint16_t)(0b1111100000000000 | (v&0x7FF)));
  } else */{
    jsjcLiteral32(7, (uint32_t)(size_t)c); // save address to r7
#ifdef DEBUG_JIT_CALLS
    DEBUG_JIT("BLX r7 (%s)\n", name);
#else
    DEBUG_JIT("BLX r7\n");
#endif
    jsjcEmit16((uint16_t)(0b0100011110000000 | (7<<3))); // BL reg 7 - BROKEN?
  }

}

void jsjcMov(int regTo, int regFrom) {
  DEBUG_JIT("MOV r%d <- r%d\n", regTo, regFrom);
  assert(regTo>=0 && regTo<16);
  assert(regFrom>=0 && regFrom<16);
  jsjcEmit16((uint16_t)(0b0100011000000000 | ((regTo&8)?128:0) | (regFrom<<3) | (regTo&7)));
                        //        TFFFFTTT
}

// Move negated register
void jsjcMVN(int regTo, int regFrom) {
  DEBUG_JIT("MVNS r%d <- r%d\n", regTo, regFrom);
  assert(regTo>=0 && regTo<8);
  assert(regFrom>=0 && regFrom<8);
  jsjcEmit16((uint16_t)(0b0100001111000000 | (regFrom<<3) | (regTo&7)));
}

// regTo = regTo & regFrom
void jsjcAND(int regTo, int regFrom) {
  DEBUG_JIT("ANDS r%d <- r%d\n", regTo, regFrom);
  assert(regTo>=0 && regTo<8);
  assert(regFrom>=0 && regFrom<8);
  jsjcEmit16((uint16_t)(0b0100000000000000 | (regFrom<<3) | (regTo&7)));
}

// Convert the var type in the given reg to a JsVar
void jsjcConvertToJsVar(int reg, JsjValueType varType) {
  if (varType==JSJVT_JSVAR || varType==JSJVT_JSVAR_NO_NAME) return; // no conversion needed
  if (varType==JSJVT_INT) {
    if (reg) jsjcMov(0, reg);
    jsjcCall(jsvNewFromInteger); // FIXME: what about clobbering r1-r3? Do a push/pop?
    if (reg) jsjcMov(reg, 0);
    return;
  }
  assert(0);
}

void jsjcPush(int reg, JsjValueType type) {
  DEBUG_JIT("PUSH {r%d}   (%s => stack depth %d)\n", reg, jsjcGetTypeName(type), jit.stackDepth);
  if (jit.stackDepth>=JSJ_TYPE_STACK_SIZE) { // not enough space on type staclk
    DEBUG_JIT("!!! not enough space on type stack - converting to JsVar\n");
    jsjcConvertToJsVar(reg, type);
    type = JSJVT_JSVAR;
  } else
    jit.typeStack[jit.stackDepth] = type;
  jit.stackDepth++;
  assert(reg>=0 && reg<8);
  jsjcEmit16((uint16_t)(0b1011010000000000 | (1<<reg)));
}

// Get the type of the variable on the top of the stack
JsjValueType jsjcGetTopType() {
  assert(jit.stackDepth>0);
  if (jit.stackDepth==0) return JSJVT_INT; // Error!
  if (jit.stackDepth>JSJ_TYPE_STACK_SIZE) return JSJVT_JSVAR; // If too many types, assume JSVAR (we convert when we push)
  return jit.typeStack[jit.stackDepth-1];
}

JsjValueType jsjcPop(int reg) {
  JsjValueType varType = jsjcGetTopType();
  jit.stackDepth--;
  DEBUG_JIT("POP {r%d}   (%s <= stack depth %d)\n", reg, jsjcGetTypeName(varType), jit.stackDepth);
  assert(reg>=0 && reg<8);
  jsjcEmit16((uint16_t)(0b1011110000000000 | (1<<reg)));
  return varType;
}

void jsjcAddSP(int amt) {
  assert((amt&3)==0 && amt>0 && amt<512);
  jit.stackDepth -= (amt>>2); // stack grows down -> negate
  DEBUG_JIT("ADD SP,SP,#%d   (stack depth now %d)\n", amt, jit.stackDepth);
  jsjcEmit16((uint16_t)(0b1011000000000000 | (amt>>2)));
}

void jsjcSubSP(int amt) {
  assert((amt&3)==0 && amt>0 && amt<512);
  jit.stackDepth += (amt>>2); // stack grows down -> negate
  DEBUG_JIT("SUB SP,SP,#%d   (stack depth now %d)\n", amt, jit.stackDepth);
  jsjcEmit16((uint16_t)(0b1011000010000000 | (amt>>2)));
}

void jsjcLoadImm(int reg, int regAddr, int offset) {
  assert((offset&3)==0 && offset>=0);
  // https://developer.arm.com/documentation/ddi0308/d/Thumb-Instructions/Alphabetical-list-of-Thumb-instructions/LDR--immediate-
  if (regAddr == JSJAR_SP) {
    assert(reg<2);
    assert(offset<4096);
    DEBUG_JIT("LDR r%d,[SP,#%d]\n", reg, offset);
    jsjcEmit16((uint16_t)(0b1001100000000000 | (offset>>2) | (reg<<10)));
  } else {
    assert(reg<8);
    assert(regAddr<8);
    assert(offset<128);
    DEBUG_JIT("LDR r%d,[r%d,#%d]\n", reg, regAddr, offset);
    jsjcEmit16((uint16_t)(0b0110100000000000 | ((offset>>2)<<6) | (regAddr<<3) | reg));
  }
}

void jsjcStoreImm(int reg, int regAddr, int offset) {
  assert((offset&3)==0 && offset>=0 && offset<128);
  assert(reg<8);
  assert(regAddr<8);
  DEBUG_JIT("STR r%d,r%d,#%d\n", reg, regAddr, offset);
  jsjcEmit16((uint16_t)(0b0110000000000000 | ((offset>>2)<<6) | (regAddr<<3) | reg));
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

#endif /* ESPR_JIT */
