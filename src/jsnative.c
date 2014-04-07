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

#define MAX_ARGS 10


/** Call a function with a set of data that's all packed up
 *  paramsAre64Bit is a bit mask specifying which parameters are 64 bit
 */
uint64_t jswCallTypedFunction(void *function, JsnArgumentType returnType, unsigned int paramsAre64Bit, uint32_t *argData, int paramCount) {
  if (paramCount==1) {
    if (paramsAre64Bit==1 && returnType==JSWAT_JSVARINT) {
      return ((uint64_t (*)(uint64_t))function)(*(uint64_t*)&argData[0]);;
    }
    if (paramsAre64Bit==1 && returnType==JSWAT_JSVARFLOAT) {
      JsVarFloat f = ((JsVarFloat (*)(uint64_t))function)(*(uint64_t*)&argData[0]);
      return *(uint64_t*)&f;
    }
  }

  jsiConsolePrint(returnType==JSWAT_JSVARFLOAT ? "double": (returnType==JSWAT_JSVARINT? "64":"32"));
  jsiConsolePrint(" call(");
  int i;
  for (i=0;i<paramCount;i++) {
    jsiConsolePrint((paramsAre64Bit&(1<<(paramCount-(i+1)))) ? "64":"32");
    if (i<paramCount-1) jsiConsolePrint(",");
  }
  jsiConsolePrint(") not implemented\n");
  return 0;
}

/** Call a function with the given argument specifiers */
JsVar *jsnCallFunction(void *function, unsigned int argumentSpecifier, JsVar *thisParam, JsVar **paramData, int paramCount) {
  JsnArgumentType returnType = (JsnArgumentType)(argumentSpecifier&JSWAT_MASK);
  JsVar *argsArray = 0; // if JSWAT_ARGUMENT_ARRAY is ever used (note it'll only ever be used once)
  argumentSpecifier >>= JSWAT_BITS;
  int paramNumber = 0; // how many parameters we have
  uint32_t param64BitFlags = 0; // each bit is one if that parameter is 64 bits
  int argCount = 0;
  uint32_t argData[MAX_ARGS];

  // prepend the 'this' link if we need one
  bool hasThis = false;
  if (argumentSpecifier&JSWAT_THIS_ARG) {
    argumentSpecifier &= ~JSWAT_THIS_ARG;
    hasThis = true;
    param64BitFlags = (param64BitFlags<<1) | JSWAT_IS_64BIT(JSWAT_JSVAR);
#if __WORDSIZE == 64
      uint64_t i = (uint64_t)thisParam;
      argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
      argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
#else
      argData[argCount++] = (uint32_t)thisParam;
#endif
  }

  // run through all arguments
  while (argumentSpecifier) {
    // Get the parameter data
    JsVar *param = (paramNumber<paramCount) ? paramData[paramNumber] : (JsVar *)0;
    paramNumber++;
    // try and pack it:
    JsnArgumentType argType = (JsnArgumentType)(argumentSpecifier&JSWAT_MASK);
    param64BitFlags = (param64BitFlags<<1) | JSWAT_IS_64BIT(argType);
    switch (argType) {
      case JSWAT_JSVAR: { // standard variable
#if __WORDSIZE == 64
        uint64_t i = (uint64_t)param;
        argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
        argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
#else
        argData[argCount++] = (uint32_t)param;
#endif
        break;
      }
      case JSWAT_ARGUMENT_ARRAY: { // a JsVar array containing all subsequent arguments
        argsArray = jsvNewWithFlags(JSV_ARRAY);
        if (argsArray) {
          // push everything into the array
          while (paramNumber<=paramCount) {
            jsvArrayPush(argsArray, param);
            param = (paramNumber<paramCount) ? paramData[paramNumber] : 0;
            paramNumber++;
          }
        }
        // push the array
#if __WORDSIZE == 64
        uint64_t i = (uint64_t)argsArray;
        argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
        argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
#else
        argData[argCount++] = (uint32_t)argsArray;
#endif
        break;
      }
      case JSWAT_BOOL: // boolean
        argData[argCount++] = jsvGetBool(param);
        break;
      case JSWAT_INT32: // 32 bit int
        argData[argCount++] = (uint32_t)(jsvGetInteger(param) & 0xFFFFFFFF);
        break;
      case JSWAT_PIN: // 16 bit int
        argData[argCount++] = (uint32_t)(jshGetPinFromVar(param));
        break;
      case JSWAT_JSVARINT: { // 64 bit int
        uint64_t i = (uint64_t)jsvGetInteger(param);
        argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
        argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
        break;
      }
      case JSWAT_JSVARFLOAT: { // 64 bit float
        JsVarFloat f = jsvGetFloat(param);;
        uint64_t i = *(uint64_t*)&f;
        argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
        argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
        break;
      }
      default:
        assert(0);
        break;
    }
    // on to next!
    argumentSpecifier >>= JSWAT_BITS;
  }

  // because of the param we shoved on at the start
  if (hasThis) paramCount++;

  uint64_t result;

  // this is the simple case...
  if (param64BitFlags==0 && paramNumber<=4 &&  !JSWAT_IS_64BIT(returnType)) {
    result = ((uint32_t (*)(uint32_t,uint32_t,uint32_t,uint32_t))function)(argData[0],argData[1],argData[2],argData[3]);
  } else { // else it gets tricky...
    JsnArgumentType simpleReturnType = returnType;
#ifndef ARM
    if (simpleReturnType!=JSWAT_JSVARFLOAT) {
      // we have to deal with 3 distinct return types in x86 (we can forget the others)
#else
    if (true) {
      // on ARM we only care about 32/64 bit
#endif
      if (JSWAT_IS_64BIT(simpleReturnType))
        simpleReturnType = JSWAT_JSVARINT;
      else
        simpleReturnType = JSWAT_INT32;
    }
    // just use calls that were pre-made by build_jswrapper
    result = jswCallTypedFunction(function, simpleReturnType, param64BitFlags, argData, paramCount);
  }

  jsvUnLock(argsArray);

  switch (returnType) {
    case JSWAT_VOID:
      return 0;
    case JSWAT_JSVAR: // standard variable
    case JSWAT_ARGUMENT_ARRAY: // a JsVar array containing all subsequent arguments
      return (JsVar*)result;
    case JSWAT_BOOL: // boolean
      return jsvNewFromBool(result!=0);
    case JSWAT_PIN:
      return jsvNewFromPin((Pin)result);
    case JSWAT_INT32: // 32 bit int
    case JSWAT_JSVARINT: // 64 bit int
      return jsvNewFromInteger((JsVarInt)result);
    case JSWAT_JSVARFLOAT: // 64 bit float
      return jsvNewFromFloat(*(JsVarFloat*)&result);
    default:
      assert(0);
      return 0;
  }
}
