/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2021 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Platform Specific entry point
 * ----------------------------------------------------------------------------
 */
#include <stdio.h> 
#include <sys/time.h>

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
bool jsiFreeMoreMemory() { return false; }
void jshKickWatchDog() {}
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
//  jsiInit(!buttonState /* load from flash by default */); // pressing USER button skips autoload
//  while (1) {
//    jsiLoop();
//  }
//  jsiKill();
  JsVar *v = jspEvaluate("1+2", true);
  jsvTrace(v,0);
  jsvUnLock(v);
  jsvKill();
}


