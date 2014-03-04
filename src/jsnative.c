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

#define MAX_ARGS 10


typedef uint32_t (*fn_4_32)(uint32_t,uint32_t,uint32_t,uint32_t);
typedef uint64_t (*fn_4_64)(uint32_t,uint32_t,uint32_t,uint32_t);

JsVar *jsnCallFunction(void *function, unsigned int argumentSpecifier, JsVar **paramData, int paramCount) {
  JsnArgumentType returnType = (JsnArgumentType)(argumentSpecifier&JSNAT_MASK);
  JsVar *argsArray = 0; // if JSNAT_ARGUMENT_ARRAY is ever used (note it'll only ever be used once)
  argumentSpecifier >>= JSNAT_BITS;
  int paramNumber = 0;
  int argCount = 0;
  uint32_t argData[MAX_ARGS];
  while (argumentSpecifier) {
    // Get the parameter data
    JsVar *param = (paramNumber<paramCount) ? paramData[paramNumber] : (JsVar *)0;
    paramNumber++;
    // try and pack it:
    JsnArgumentType argType = (JsnArgumentType)(argumentSpecifier&JSNAT_MASK);
    switch (argType) {
      case JSNAT_JSVAR: { // standard variable
#if __WORDSIZE == 64
        uint64_t i = (uint64_t)param;
        argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
        argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
#else
        argData[argCount++] = param;
#endif
        break;
      }
      case JSNAT_ARGUMENT_ARRAY: { // a JsVar array containing all subsequent arguments
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
        argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
        argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
#else
        argData[argCount++] = argsArray;
#endif
        break;
      }
      case JSNAT_BOOL: // boolean
        argData[argCount++] = jsvGetBool(param);
        break;
      case JSNAT_INT16: // 16 bit int
        argData[argCount++] = (uint32_t)(jsvGetInteger(param) & 0xFFFF);
        break;
      case JSNAT_INT32: // 32 bit int
        argData[argCount++] = (uint32_t)(jsvGetInteger(param) & 0xFFFFFFFF);
        break;
      case JSNAT_JSVARINT: { // 64 bit int
        uint64_t i = (uint64_t)jsvGetInteger(param);
        argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
        argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
        break;
      }
      case JSNAT_JSVARFLOAT: { // 64 bit float
        JsVarFloat f = jsvGetFloat(param);;
        uint64_t i = *(uint64_t*)&f;
        argData[argCount++] = (uint32_t)((i>>32) & 0xFFFFFFFF);
        argData[argCount++] = (uint32_t)((i) & 0xFFFFFFFF);
        break;
      }
      default:
        assert(0);
        break;
    }
    // on to next!
    argumentSpecifier >>= JSNAT_BITS;
  }

  bool return64 = JSNAT_IS_64BIT(returnType);

  uint64_t result;

  if (return64) { // 64 bit
    result = ((fn_4_64)function)(argData[0],argData[1],argData[2],argData[3]);
  } else { // 32 bit
    result = ((fn_4_32)function)(argData[0],argData[1],argData[2],argData[3]);
  }
  jsvUnLock(argsArray);

  switch (returnType) {
    case JSNAT_VOID:
      return 0;
    case JSNAT_JSVAR: // standard variable
    case JSNAT_ARGUMENT_ARRAY: // a JsVar array containing all subsequent arguments
      return (JsVar*)result;
    case JSNAT_BOOL: // boolean
      return jsvNewFromBool(result!=0);
      break;
    case JSNAT_INT16: // 16 bit int
    case JSNAT_INT32: // 32 bit int
    case JSNAT_JSVARINT: // 64 bit int
      return jsvNewFromInteger((JsVarInt)result);
    case JSNAT_JSVARFLOAT: // 64 bit float
      return jsvNewFromFloat(*(JsVarFloat*)&result);
    default:
      assert(0);
      return 0;
  }
}

