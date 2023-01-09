/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2022 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Platform Specific entry point
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"
#include "jswrapper.h"

extern volatile JsVarRef jsVarFirstEmpty;

// Fixing up undefined functions
void jshInterruptOn() {}
void jshInterruptOff() {}
bool jshIsInInterrupt() { return false; }

void jsiConsolePrintf(const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
#ifdef USE_FLASH_MEMORY
  // TODO
  assert(0);
#else
  vcbprintf(vcbprintf_callback_jsiConsolePrintString,0, fmt, argp);
#endif
  va_end(argp);
}
void jsiConsolePrintStringVar(JsVar *v) {
  // the jsinteractive one converts \n -> \r\n but we don't care here
  jsiConsolePrintf("%v", v);
}
bool jsiFreeMoreMemory() { return false; } // no extra memory is allocated
void jshKickWatchDog() { }
void jsiConsoleRemoveInputLine() {}
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime)(ms*1000);
}
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return ((JsVarFloat)time)/1000;
}
// ===============================
JsSysTime jshGetSystemTime() {
  return ejs_get_microseconds();
}
void jsiConsolePrintString(const char *str) {
  ejs_print(str);
}
void vcbprintf_callback_jsiConsolePrintString(const char *str, void* user_data) {
  jsiConsolePrintString(str);
}
// ===============================

void ejs_set_instance(struct ejs *ejs) {
  jsVarsSize = ejs->varCount;
  jsVars = ejs->vars;
  execInfo.hiddenRoot = ejs->hiddenRoot;
  execInfo.root = ejs->root;
  execInfo.baseScope = ejs->root;
  jsVarFirstEmpty = ejs->jsVarFirstEmpty;
  if(ejs->exception) {
    jsvUnLock(ejs->exception);
    ejs->exception = NULL;
  }
}

static void ejs_process_exceptions(struct ejs *ejs) {
  JsVar *exception = jspGetException();
  if (exception) {
    ejs->exception = exception;
    jsiConsolePrintf("Uncaught %v\n", exception);
    if (jsvIsObject(exception)) {
      JsVar *stackTrace = jsvObjectGetChild(exception, "stack", 0);
      if (stackTrace) {
        jsiConsolePrintf("%v\n", stackTrace);
        jsvUnLock(stackTrace);
      }
    }
  }
}

/* Create an instance */
struct ejs *ejs_create(unsigned int varCount) {
  struct ejs *ejs = (struct ejs*)malloc(sizeof(struct ejs));
  if (!ejs) return 0;
  ejs->exception = NULL;
  ejs->varCount = varCount;

  jsVars = NULL; // or else jsvInit will reuse the old jsVars

  jswHWInit();
  jsvInit(varCount);
  jspInit();
  
  ejs->vars = jsVars;
  ejs->hiddenRoot = execInfo.hiddenRoot;
  ejs->root = execInfo.root;
  ejs->jsVarFirstEmpty = jsVarFirstEmpty;
  return ejs;
}

/* Destroy the instance */
void ejs_destroy(struct ejs *ejs) {
  jspKill();
  jsvKill();
  
  if (!ejs) return;
  if (!ejs->vars) return;
  ejs->varCount=0;  
  ejs->vars=NULL;
  free(ejs);
}

JsVar *ejs_exec(struct ejs *ejs, const char *src, bool stringIsStatic) {
  ejs_set_instance(ejs);
  JsVar *v = jspEvaluate(src, stringIsStatic);
  // ^ if the string is static, we can let functions reference it directly
  ejs_process_exceptions(ejs);
  return v;
}

JsVar *ejs_execf(struct ejs *ejs, JsVar *func, JsVar *thisArg, int argCount, JsVar **argPtr) {
  ejs_set_instance(ejs);
  JsVar *v = jspExecuteFunction(func, thisArg, argCount, argPtr);
  ejs_process_exceptions(ejs);
  return v;
}
