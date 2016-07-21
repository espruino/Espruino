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

#include "jsnative.h"
#include "jshardware.h"
#include "jsinteractive.h"

// none of this is used at the moment

#define MAX_ARGS 12

#if defined(__i386__) || defined(__x86_64__)
    #define USE_SEPARATE_DOUBLES // cdecl on x86 puts FP args elsewhere!
#endif

#if defined(__WORDSIZE) && __WORDSIZE == 64
  #define USE_64BIT
#else // 32 bit
  #if defined(__gnu_linux__) && !defined(USE_X86_CDECL)
/* This is nuts. On rasbperry pi Linux:
 *
 * `uint32_t a, uint32_t b, uint64_t c` -> a,b,c - awesome
 * `uint64_t a, uint32_t b, uint32_t c` -> a,b,c - awesome
 * `uint32_t a, uint64_t b, uint32_t c` -> a,c,b - NOT awesome
 *
 * 64 bits are aligned, but 32 bits fill in the gaps inbetween!
 */
    #define USE_ARG_REORDERING
    #define USE_FLOAT_RETURN_FIX
  #endif
#endif

#ifdef __ARM_PCS_VFP
  #define USE_FLOAT_RETURN_FIX
  #define USE_SEPARATE_DOUBLES
#endif

/** Call a function with the given argument specifiers */
JsVar *jsnCallFunction(void *function, JsnArgumentType argumentSpecifier, JsVar *thisParam, JsVar **paramData, int paramCount) {
#ifndef SAVE_ON_FLASH
  // Handle common call types quickly:
  // ------- void(void)
  if (argumentSpecifier==JSWAT_VOID) {
    ((void (*)())function)();
    return 0;
  }
  // ------- JsVar*(void)
  if (argumentSpecifier==JSWAT_JSVAR) {
    return ((JsVar *(*)())function)();
  }
  // ------- void('this')
  if (argumentSpecifier==(JSWAT_VOID | JSWAT_THIS_ARG)) {
    ((void (*)(JsVar *))function)(thisParam);
    return 0;
  }
#endif
  // Now do it the hard way...

  JsnArgumentType returnType = (JsnArgumentType)(argumentSpecifier&JSWAT_MASK);
  JsVar *argsArray = 0; // if JSWAT_ARGUMENT_ARRAY is ever used (note it'll only ever be used once)
  int paramNumber = 0; // how many parameters we have
  int argCount = 0;
  size_t argData[MAX_ARGS];
#ifdef USE_SEPARATE_DOUBLES
  int doubleCount = 0;
  JsVarFloat doubleData[MAX_ARGS];
#endif

  // prepend the 'this' link if we need one
  if (argumentSpecifier&JSWAT_THIS_ARG)
    argData[argCount++] = (size_t)thisParam;
  argumentSpecifier = (argumentSpecifier & JSWAT_ARGUMENTS_MASK) >> JSWAT_BITS;

#ifdef USE_ARG_REORDERING
  size_t alignedLongsAfter = 0;
#endif


  // run through all arguments
  while (argumentSpecifier & JSWAT_MASK) {
    // Get the parameter data
    JsVar *param = (paramNumber<paramCount) ? paramData[paramNumber] : (JsVar *)0;
    paramNumber++;
    // try and pack it:
    JsnArgumentType argType = (JsnArgumentType)(argumentSpecifier&JSWAT_MASK);

#ifdef USE_ARG_REORDERING
    if (!JSWAT_IS_64BIT(argType) && !(argCount&1)) {
      argCount += alignedLongsAfter*2;
      alignedLongsAfter = 0;
    }
#endif

    if (argCount > MAX_ARGS - (JSWAT_IS_64BIT(argType)?2:1)) {
      // TODO: can we ever hit this because of JsnArgumentType's restrictions?
      jsError("INTERNAL: too many arguments for jsnCallFunction");
    }

    switch (argType) {
    case JSWAT_JSVAR: { // standard variable
      argData[argCount++] = (size_t)param;
      break;
    }
    case JSWAT_ARGUMENT_ARRAY: { // a JsVar array containing all subsequent arguments
      argsArray = jsvNewEmptyArray();
      if (argsArray) {
        // push everything into the array
        while (paramNumber<=paramCount) {
          jsvArrayPush(argsArray, param);
          param = (paramNumber<paramCount) ? paramData[paramNumber] : 0;
          paramNumber++;
        }
      }
      // push the array
      argData[argCount++] = (size_t)argsArray;
      break;
    }
    case JSWAT_BOOL: // boolean
      argData[argCount++] = jsvGetBool(param);
      break;
    case JSWAT_INT32: // 32 bit int
      argData[argCount++] = (uint32_t)jsvGetInteger(param);
      break;
    case JSWAT_PIN: // 16 bit int
      argData[argCount++] = (uint32_t)jshGetPinFromVar(param);
      break;
    case JSWAT_JSVARFLOAT: { // 64 bit float
      JsVarFloat f = jsvGetFloat(param);
#ifdef USE_SEPARATE_DOUBLES
      doubleData[doubleCount++] = f;
#else
      uint64_t i = *(uint64_t*)&f;
#if USE_64BIT
      argData[argCount++] = (size_t)i;
#else // 32 bit...
 #ifdef USE_ARG_REORDERING
      if (argCount&1) {
        size_t argC = argCount+1;
        argData[argC++] = (size_t)((i) & 0xFFFFFFFF);
        argData[argC++] = (size_t)((i>>32) & 0xFFFFFFFF);
        alignedLongsAfter++;
      } else {
        argData[argCount++] = (size_t)((i) & 0xFFFFFFFF);
        argData[argCount++] = (size_t)((i>>32) & 0xFFFFFFFF);
      }
 #else // no reordering
      if (argCount&1) argCount++;
      argData[argCount++] = (size_t)((i) & 0xFFFFFFFF);
      argData[argCount++] = (size_t)((i>>32) & 0xFFFFFFFF);
 #endif
#endif
#endif
      break;
    }
    default:
      assert(0);
      break;
    }
    // on to next!
    argumentSpecifier >>= JSWAT_BITS;
  }

  uint64_t result;

  // When args<=4 on ARM, everything is passed in registers (so we try and do this case first)
  if (argCount<=4) {
#ifdef USE_SEPARATE_DOUBLES
    assert(doubleCount<=4);
    if (doubleCount) {
      // We are passing doubles
      if (returnType==JSWAT_JSVARFLOAT) {
        // On x86, doubles are returned in a floating point unit register
        JsVarFloat f = ((JsVarFloat (*)(size_t,size_t,size_t,size_t,JsVarFloat,JsVarFloat,JsVarFloat,JsVarFloat))function)(argData[0],argData[1],argData[2],argData[3],doubleData[0],doubleData[1],doubleData[2],doubleData[3]);
        result = *(uint64_t *)&f;
      } else {
#ifdef USE_64BIT
        if (JSWAT_IS_64BIT(returnType))
          result = ((uint64_t (*)(size_t,size_t,size_t,size_t,JsVarFloat,JsVarFloat,JsVarFloat,JsVarFloat))function)(argData[0],argData[1],argData[2],argData[3],doubleData[0],doubleData[1],doubleData[2],doubleData[3]);
        else
#endif
          result = ((uint32_t (*)(size_t,size_t,size_t,size_t,JsVarFloat,JsVarFloat,JsVarFloat,JsVarFloat))function)(argData[0],argData[1],argData[2],argData[3],doubleData[0],doubleData[1],doubleData[2],doubleData[3]);
      }
    } else if (returnType==JSWAT_JSVARFLOAT) {
      // On x86, doubles are returned in a floating point unit register
      JsVarFloat f = ((JsVarFloat (*)(size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3]);
      result = *(uint64_t*)&f;
    } else
#endif
    {
      if (JSWAT_IS_64BIT(returnType)) {
#ifdef USE_FLOAT_RETURN_FIX
        assert(returnType==JSWAT_JSVARFLOAT);
        JsVarFloat f = ((JsVarFloat (*)(size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3]);
        result = *(uint64_t*)&f;
#else
        result = ((uint64_t (*)(size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3]);
#endif
    } else
        result = ((uint32_t (*)(size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3]);
    }
  } else { // else it gets tricky...
#ifdef USE_SEPARATE_DOUBLES
    assert(doubleCount==0);
    if (returnType==JSWAT_JSVARFLOAT) {
      // On x86, doubles are returned in a floating point unit register
      JsVarFloat f = ((JsVarFloat (*)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3],argData[4],argData[5],argData[6],argData[7],argData[8],argData[9],argData[10],argData[11]);
      result = *(uint64_t*)&f;
    } else
#endif
    {
      if (JSWAT_IS_64BIT(returnType)) {
#ifdef USE_FLOAT_RETURN_FIX
        assert(returnType==JSWAT_JSVARFLOAT);
        JsVarFloat f = ((JsVarFloat (*)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3],argData[4],argData[5],argData[6],argData[7],argData[8],argData[9],argData[10],argData[11]);
        result = *(uint64_t*)&f;
#else
        result = ((uint64_t (*)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3],argData[4],argData[5],argData[6],argData[7],argData[8],argData[9],argData[10],argData[11]);
#endif
      } else
        result = ((uint32_t (*)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3],argData[4],argData[5],argData[6],argData[7],argData[8],argData[9],argData[10],argData[11]);
    }
  }

  jsvUnLock(argsArray);

  switch (returnType) {
  case JSWAT_VOID:
    return 0;
  case JSWAT_JSVAR: // standard variable
  case JSWAT_ARGUMENT_ARRAY: // a JsVar array containing all subsequent arguments
    return (JsVar*)(size_t)result;
  case JSWAT_BOOL: // boolean
    return jsvNewFromBool(result!=0);
  case JSWAT_PIN:
    return jsvNewFromPin((Pin)result);
  case JSWAT_INT32: // 32 bit int
    return jsvNewFromInteger((JsVarInt)result);
  case JSWAT_JSVARFLOAT: // 64 bit float
    return jsvNewFromFloat(*(JsVarFloat*)&result);
  default:
    assert(0);
    return 0;
  }
}

// -----------------------------------------------------------------------------------------

JsVarFloat sanity_pi() { return 3.141592; }
int32_t sanity_int_pass(int32_t hello) { return (hello*10)+5; }
int32_t sanity_int_flt_int(int32_t a, JsVarFloat b, int32_t c) {
  return a + (int32_t)(b*100) + c*10000;
}

/** Perform sanity tests to ensure that  jsnCallFunction is working as expected */
void jsnSanityTest() {
  JsVar *args[4];
  if (jsvGetFloatAndUnLock(jsnCallFunction(sanity_pi, JSWAT_JSVARFLOAT, 0, 0, 0)) != 3.141592)
    jsiConsolePrint("WARNING: jsnative.c sanity check failed (returning double values)\n");

  args[0] = jsvNewFromInteger(1234);
  if (jsvGetIntegerAndUnLock(jsnCallFunction(sanity_int_pass, JSWAT_INT32|(JSWAT_INT32<<JSWAT_BITS), 0, args, 1)) != 12345)
      jsiConsolePrint("WARNING: jsnative.c sanity check failed (simple integer passing)\n");
  jsvUnLock(args[0]);

  args[0] = jsvNewFromInteger(56);
  args[1] = jsvNewFromFloat(34);
  args[2] = jsvNewFromInteger(12);
  if (jsvGetIntegerAndUnLock(jsnCallFunction(sanity_int_flt_int, JSWAT_INT32|(JSWAT_INT32<<(JSWAT_BITS*1))|(JSWAT_JSVARFLOAT<<(JSWAT_BITS*2))|(JSWAT_INT32<<(JSWAT_BITS*3)), 0, args, 3)) != 123456)
      jsiConsolePrint("WARNING: jsnative.c sanity check failed (int-float-int passing)\n");
  jsvUnLockMany(3, args);
}

