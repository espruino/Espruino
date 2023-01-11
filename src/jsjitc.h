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

// Write debug info to the console
#define DEBUG_JIT jsjcDebugPrintf
// Write debug info to the console IF we're in the 'emit' phase
#define DEBUG_JIT_EMIT if (jit.phase == JSJP_EMIT) jsjcDebugPrintf
// Called to print debug info - best to use DEBUG_JIT so we can disable debug lines for final compiles though
void jsjcDebugPrintf(const char *fmt, ...);

#define JSJ_TYPE_STACK_SIZE 64 // Most amount of types stored on stack

typedef enum {
  JSJVT_INT,
  JSJVT_JSVAR,        ///< A JsVar
  JSJVT_JSVAR_NO_NAME ///< A JsVar, and we know it's not a name so it doesn't need SkipName
} PACKED_FLAGS JsjValueType;

typedef enum {
  JSJAC_EQ, // Equal / equals zero
  JSJAC_NE, // Not equal
  JSJAC_CS, // Carry set / unsigned higher or same
  JSJAC_CC, // Carry clear / unsigned lower
  JSJAC_MI, // Minus / negative
  JSJAC_PL, // Plus / positive or zero
  JSJAC_VS, // Overflow
  JSJAC_VC, // No overflow
  JSJAC_HI, // Unsigned higher
  JSJAC_LI, // Unsigned lower or same
  JSJAC_GE, // Signed greater than or equal
  JSJAC_LT, // Signed less than
  JSJAC_GT, // Signed greater than
  JSJAC_LE, // Signed less than or equal
  JSJAC_AL  // Always
} JsjAsmCondition;

typedef enum {
  JSJAR_r0,
  JSJAR_r1,
  JSJAR_r2,
  // ...
  JSJAR_SP = 13,
  JSJAR_LR = 14,
  JSJAR_PC = 15,
} JsjAsmReg;

typedef enum {
  JSJP_UNKNOWN,
  JSJP_SCAN, /// scan for variables used
  JSJP_EMIT  /// emit code
} JsjPhase;


typedef struct {
  /// Which compilation phase are we in?
  JsjPhase phase;
  /// The ARM Thumb-2 code we're in the process of creating
  JsVar *code;
  /// The ARM Thumb-2 variable init code block (this goes right at the start of our function)
  JsVar *initCode;
  /// How many blocks deep are we? blockCount=0 means we're writing to the 'code' var
  int blockCount;
  /// An Object mapping var name -> index on the stack
  JsVar *vars;
  /// How many words (not bytes) are on the stack reserved for variables?
  int varCount;
  /// How much stuff has been pushed on the stack so far? (including variables)
  int stackDepth;
  /// For each item on the stack, we store its type
  JsjValueType typeStack[JSJ_TYPE_STACK_SIZE];
} JsjInfo;

// JIT state
extern JsjInfo jit;

// Called before start of JIT output
void jsjcStart();
// Called when JIT output stops
JsVar *jsjcStop();
// Called before start of a block of code. Returns the old code jsVar that should be passed into jsjcStopBlock. Ignored unless in JSJP_EMIT phase
JsVar *jsjcStartBlock();
// Called to start writing to 'init code' (which is inserted before everything else). Returns the old code jsVar that should be passed into jsjcStopBlock
JsVar *jsjcStartInitCodeBlock();
// Called when JIT output stops, pass it the return value from jsjcStartBlock. Returns the code parsed in the block. Ignored unless in JSJP_EMIT phase
JsVar *jsjcStopBlock(JsVar *oldBlock);
// Emit a whole block of code
void jsjcEmitBlock(JsVar *block);
// Get what byte we're at in our code
int jsjcGetByteCount();

// Add 8 bit literal
void jsjcLiteral8(int reg, uint8_t data);
// Add 16 bit literal
void jsjcLiteral16(int reg, bool hi16, uint16_t data);
// Add 32 bit literal
void jsjcLiteral32(int reg, uint32_t data);
// Add 64 bit literal in reg,reg+1
void jsjcLiteral64(int reg, uint64_t data);
// Call a function
#ifdef DEBUG_JIT_CALLS
void _jsjcCall(void *c, const char *name);
#define jsjcCall(c) _jsjcCall(c, STRINGIFY(c))
#else
void jsjcCall(void *c);
#endif
// Store a string of data and put the address in a register. Returns the length
int jsjcLiteralString(int reg, JsVar *str, bool nullTerminate);
// Compare a register with a literal. jsjcBranchConditionalRelative can then be called
void jsjcCompareImm(int reg, int literal);
// Jump a number of bytes forward or back
void jsjcBranchRelative(int bytes);
// Jump a number of bytes forward or back, based on condition flags
void jsjcBranchConditionalRelative(JsjAsmCondition cond, int bytes);
// Move one register to another
void jsjcMov(int regTo, int regFrom);
// Move negated register
void jsjcMVN(int regTo, int regFrom);
// regTo = regTo & regFrom
void jsjcAND(int regTo, int regFrom);
// Convert the var type in the given reg to a JsVar
void jsjcConvertToJsVar(int reg, JsjValueType varType);
// Push a register onto the stack
void jsjcPush(int reg, JsjValueType type);
// Get the type of the variable on the top of the stack
JsjValueType jsjcGetTopType();
// Pop off the stack to a register
JsjValueType jsjcPop(int reg);
// Add a value to the stack pointer (only multiple of 4)
void jsjcAddSP(int amt);
// Subtract a value from the stack pointer (only multiple of 4)
void jsjcSubSP(int amt);
// reg = mem[regAddr + offset]
void jsjcLoadImm(int reg, int regAddr, int offset);
// mem[regAddr + offset] = reg
void jsjcStoreImm(int reg, int regAddr, int offset);

void jsjcPushAll();
void jsjcPopAllAndReturn();

#endif /* JSJITC_H_ */
#endif /* ESPR_JIT */
