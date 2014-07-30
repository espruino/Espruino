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

/** Call a function with the given argument specifiers */
JsVar *jsnCallFunction(void *function, JsnArgumentType argumentSpecifier, JsVar *thisParam, JsVar **paramData, int paramCount) {
  JsnArgumentType returnType = (JsnArgumentType)(argumentSpecifier&JSWAT_MASK);
  JsVar *argsArray = 0; // if JSWAT_ARGUMENT_ARRAY is ever used (note it'll only ever be used once)
  int paramNumber = 0; // how many parameters we have
  int argCount = 0;
  size_t argData[MAX_ARGS];
#ifndef ARM // cdecl on x86 puts FP args elsewhere!
  int doubleCount = 0;
  JsVarFloat doubleData[MAX_ARGS];
#endif

  // prepend the 'this' link if we need one
  if (argumentSpecifier&JSWAT_THIS_ARG)
    argData[argCount++] = (size_t)thisParam;
  argumentSpecifier = (argumentSpecifier & JSWAT_ARGUMENTS_MASK) >> JSWAT_BITS;


  // run through all arguments
  while (argumentSpecifier & JSWAT_MASK) {
    // Get the parameter data
    JsVar *param = (paramNumber<paramCount) ? paramData[paramNumber] : (JsVar *)0;
    paramNumber++;
    // try and pack it:
    JsnArgumentType argType = (JsnArgumentType)(argumentSpecifier&JSWAT_MASK);

#ifdef ARM
    if (JSWAT_IS_64BIT(argType))
      argCount = (argCount+1)&~1;
#endif

    if (argCount > MAX_ARGS - (JSWAT_IS_64BIT(argType)?2:1)) {
      jsError("INTERNAL: too many arguments for jsnCallFunction");
    }



    switch (argType) {
      case JSWAT_JSVAR: { // standard variable
        argData[argCount++] = (size_t)param;
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
#ifdef ARM
        uint64_t i = *(uint64_t*)&f;
#if __WORDSIZE == 64
        argData[argCount++] = (size_t)i;
#else
        argData[argCount++] = (size_t)((i) & 0xFFFFFFFF);
        argData[argCount++] = (size_t)((i>>32) & 0xFFFFFFFF);
#endif
#else
        doubleData[doubleCount++] = f;
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
#ifndef ARM
    assert(doubleCount<=4);
    if (doubleCount) {
      if (returnType==JSWAT_JSVARFLOAT) {
        // On x86, doubles are returned in a floating point unit register
        JsVarFloat f = ((JsVarFloat (*)(size_t,size_t,size_t,size_t,JsVarFloat,JsVarFloat,JsVarFloat,JsVarFloat))function)(argData[0],argData[1],argData[2],argData[3],doubleData[0],doubleData[1],doubleData[2],doubleData[3]);
        result = *(uint64_t*)&f;
      } else {
        if (JSWAT_IS_64BIT(returnType))
          result = ((uint64_t (*)(size_t,size_t,size_t,size_t,JsVarFloat,JsVarFloat,JsVarFloat,JsVarFloat))function)(argData[0],argData[1],argData[2],argData[3],doubleData[0],doubleData[1],doubleData[2],doubleData[3]);
        else
          result = ((uint32_t (*)(size_t,size_t,size_t,size_t,JsVarFloat,JsVarFloat,JsVarFloat,JsVarFloat))function)(argData[0],argData[1],argData[2],argData[3],doubleData[0],doubleData[1],doubleData[2],doubleData[3]);
      }
    } else if (returnType==JSWAT_JSVARFLOAT) {
      // On x86, doubles are returned in a floating point unit register
      JsVarFloat f = ((JsVarFloat (*)(size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3]);
      result = *(uint64_t*)&f;
    } else
#endif
    {
      if (JSWAT_IS_64BIT(returnType))
        result = ((uint64_t (*)(size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3]);
      else
        result = ((uint32_t (*)(size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3]);
    }
  } else { // else it gets tricky...
#ifndef ARM
    assert(doubleCount==0);
    if (returnType==JSWAT_JSVARFLOAT) {
      // On x86, doubles are returned in a floating point unit register
      JsVarFloat f = ((JsVarFloat (*)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3],argData[4],argData[5],argData[6],argData[7],argData[8],argData[9],argData[10],argData[11]);
      result = *(uint64_t*)&f;
    } else
#endif
    {
      if (JSWAT_IS_64BIT(returnType))
        result = ((uint64_t (*)(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3],argData[4],argData[5],argData[6],argData[7],argData[8],argData[9],argData[10],argData[11]);
      else
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
