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
  jsVars = (JsVar*)ejs->vars;
  execInfo.hiddenRoot = (JsVar*)ejs->hiddenRoot;
  execInfo.root = (JsVar*)ejs->root; 
}
void ejs_unset_instance() {
 /* FIXME - we need these but if they are in, js* functions (eg to print/get values)
   can't be called from outside of ejs_exec */
 /* jsVarsSize = 0;
  jsVars = NULL; 
  execInfo.hiddenRoot = NULL;
  execInfo.root = NULL; */
}

/* Create an instance */
struct ejs *ejs_create(unsigned int varCount) {
  struct ejs *ejs = (struct ejs*)malloc(sizeof(ejs));
  if (!ejs) return 0;
  ejs->varCount = varCount;
 
  jswHWInit();
  jsvInit(varCount);
  jspInit();
  
  ejs->vars = (struct JsVar*)jsVars;
  ejs->hiddenRoot = (struct JsVar*)execInfo.hiddenRoot;
  ejs->root = (struct JsVar*)execInfo.root; 
  ejs_unset_instance();
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

struct JsVar *ejs_exec(struct ejs *ejs, const char *src) {
  ejs_set_instance(ejs);
  JsVar *v = jspEvaluate(src, false/* string is assumed to not be static */);  
  // ^ if the string is static, we can let functions reference it directly
  JsVar *exception = jspGetException();
  if (exception) {
    jsiConsolePrintf("Uncaught %v\n", exception);
    if (jsvIsObject(exception)) {
      JsVar *stackTrace = jsvObjectGetChild(exception, "stack", 0);
      if (stackTrace) {
        jsiConsolePrintf("%v\n", stackTrace);
        jsvUnLock(stackTrace);
      }
    }
    jsvUnLock(exception);
  }
  ejs_unset_instance();
  return (struct JsVar*)v;
}

