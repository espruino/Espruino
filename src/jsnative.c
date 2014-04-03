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

#if 0
// none of this is used at the moment

#define MAX_ARGS 10


typedef uint32_t (*fn_4_32)(size_t,size_t,size_t,size_t);
typedef uint64_t (*fn_4_64)(size_t,size_t,size_t,size_t);

JsVar *jsnCallFunction(void *function, unsigned int argumentSpecifier, JsVar **paramData, int paramCount) {
  JsnArgumentType returnType = (JsnArgumentType)(argumentSpecifier&JSWAT_MASK);
  JsVar *argsArray = 0; // if JSWAT_ARGUMENT_ARRAY is ever used (note it'll only ever be used once)
  argumentSpecifier >>= JSWAT_BITS;
  int paramNumber = 0;
  int argCount = 0;
  size_t argData[MAX_ARGS];
  while (argumentSpecifier) {
    // Get the parameter data
    JsVar *param = (paramNumber<paramCount) ? paramData[paramNumber] : (JsVar *)0;
    paramNumber++;
    // try and pack it:
    JsnArgumentType argType = (JsnArgumentType)(argumentSpecifier&JSWAT_MASK);
    switch (argType) {
      case JSWAT_JSVAR: { // standard variable
        argData[argCount++] = param;
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
        argData[argCount++] = argsArray;
        break;
      }
      case JSWAT_BOOL: // boolean
        argData[argCount++] = jsvGetBool(param);
        break;
      case JSWAT_INT16: // 16 bit int
        argData[argCount++] = (uint32_t)(jsvGetInteger(param) & 0xFFFF);
        break;
      case JSWAT_INT32: // 32 bit int
        argData[argCount++] = (uint32_t)(jsvGetInteger(param) & 0xFFFFFFFF);
        break;
      case JSWAT_JSVARINT: { // 64 bit int
        uint64_t i = (uint64_t)jsvGetInteger(param);
#if __WORDSIZE == 64
        argData[argCount++] = i;
#else
        argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
        argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
#endif
        break;
      }
      case JSWAT_JSVARFLOAT: { // 64 bit float
        JsVarFloat f = jsvGetFloat(param);;
        uint64_t i = *(uint64_t*)&f;
#if __WORDSIZE == 64
        argData[argCount++] = i;
#else
        argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
        argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
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

  bool return64 = JSWAT_IS_64BIT(returnType);

  uint64_t result;

  if (return64) { // 64 bit
    result = ((fn_4_64)function)(argData[0],argData[1],argData[2],argData[3]);
  } else { // 32 bit
    result = ((fn_4_32)function)(argData[0],argData[1],argData[2],argData[3]);
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
      break;
    case JSWAT_INT16: // 16 bit int
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

#endif
