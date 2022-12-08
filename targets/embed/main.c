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
void jsiConsolePrint(const char *str) {
  printf("%s",str);
}
void jsiConsolePrintf(const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsiConsolePrint,0, fmt, argp);
  va_end(argp);
}
void jsiConsolePrintStringVar(JsVar *v) {
  // the jsinteractive one converts \n -> \r\n but we don't care here
  jsiConsolePrintf("%v", v);
}
bool jsiFreeMoreMemory() { return false; } // no extra memory is allocated
void jshKickWatchDog() { }
void jsiConsoleRemoveInputLine() {}

JsSysTime jshGetSystemTime() {
  struct timeval tm;
  gettimeofday(&tm, 0);
  return (JsSysTime)(tm.tv_sec)*1000000L + tm.tv_usec;
}
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime)(ms*1000);
}
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return ((JsVarFloat)time)/1000;
}

// 
int main() {

  jswHWInit();

  bool buttonState = false;
  jsvInit(0);
  jspInit();
//  jsiInit(!buttonState /* load from flash by default */); // pressing USER button skips autoload
//  while (1) {
//    jsiLoop();
//  }
//  jsiKill();
  printf("Embedded Espruino test. Type JS and hit enter:\n>");

  char buf[1000];
  while (true) {
    fgets(buf, sizeof(buf), stdin);
    JsVar *v = jspEvaluate(buf, false);
    jsiConsolePrintf("=%v\n>",v);
    jsvUnLock(v);
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
    if (jspIsInterrupted()) {
      jsiConsolePrint("Execution Interrupted\n");
      jspSetInterrupted(false);
    }
  }  
  jspKill();
  jsvKill();
}


// V=1 BOARD=EMBED DEBUG=1 make sourcecode
// gcc sourcecode.c -lm
