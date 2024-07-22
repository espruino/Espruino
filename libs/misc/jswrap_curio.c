/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2024 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 *  Curio v2 implementation (https://trycurio.com/)
 * ----------------------------------------------------------------------------
 */
#include "jswrap_curio.h"
#include "jshardware.h"
#include "jsparse.h"
#include "jswrap_neopixel.h"

/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py */

volatile int stepCountL = 0;
volatile int stepCountR = 0;
volatile int motorDirL = 0;
volatile int motorDirR = 0;

const int CURIO_PWMFREQ = 1000;

/*JSON{
  "type" : "variable",
  "name" : "Q",
  "generate" : "jswrap_curio_q",
  "return" : ["JsVar","An object containing the pins for the Qwiic connector on Curio `{sda,scl}`"],
  "return_object" : "Qwiic"
}
*/
JsVar *jswrap_curio_q() {
  JsVar *o = jspNewObject(0, "Qwiic");
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o, "sda", jsvNewFromPin(CURIO_QWIIC_D));
  jsvObjectSetChildAndUnLock(o, "scl", jsvNewFromPin(CURIO_QWIIC_C));
  return o;
}

/*JSON{
  "type" : "variable",
  "name" : "SERVO",
  "generate_full" : "CURIO_SERVO",
  "return" : ["pin","The pin for the servo motor"]
}
*/

/*JSON{
    "type" : "function",
    "name" : "led",
    "generate" : "jswrap_curio_led",
    "params" : [
      ["col","JsVar","The colours to use, a 24 element array (8 x RGB)"]
    ]
}*/
void jswrap_curio_led(JsVar *col) {
  JsVar *allocated = 0;
  if (!jsvIsArray(col)) {
    unsigned char rgb[24];
    int c = jsvGetInteger(col);
    if (jsvIsBoolean(col)) c = c?255:0;
    if (c<0) c=0;
    if (c>255) c=255;
    for (int i=0;i<24;i++)
      rgb[i] = c;
    allocated = jsvNewArrayBufferWithData(24, rgb);
    col = allocated;
  }
  jswrap_neopixel_write(CURIO_NEOPIXEL, col);
  jsvUnLock(allocated);
}

JsVarFloat curio_getMotorPWM(int steps) {
  JsVarFloat f = 0.6 + (steps*0.4/30);
  if (f>1) f=1;
  if (f<0) f=0;
  return f;
}

void curio_setMotorStates() {
  JsVarFloat s;

  s = curio_getMotorPWM(stepCountL);
  if (motorDirL<0) {
    jshPinOutput(CURIO_MOTORL1,0);
    jshPinAnalogOutput(CURIO_MOTORL2,s,CURIO_PWMFREQ,0);
  } else if (motorDirL>0) {
    jshPinAnalogOutput(CURIO_MOTORL1,s,CURIO_PWMFREQ,0);
    jshPinOutput(CURIO_MOTORL2,0);
  } else {
    jshPinOutput(CURIO_MOTORL1,0);
    jshPinOutput(CURIO_MOTORL2,0);
  }

  s = curio_getMotorPWM(stepCountR);
  if (motorDirR<0) {
    jshPinOutput(CURIO_MOTORR1,0);
    jshPinAnalogOutput(CURIO_MOTORR2,s,CURIO_PWMFREQ,0);
  } else if (motorDirR>0) {
    jshPinAnalogOutput(CURIO_MOTORR1,s,CURIO_PWMFREQ,0);
    jshPinOutput(CURIO_MOTORR2,0);
  } else {
    jshPinOutput(CURIO_MOTORR1,0);
    jshPinOutput(CURIO_MOTORR2,0);
  }
}

/*JSON{
    "type" : "function",
    "name" : "go",
    "generate" : "jswrap_curio_go",
    "params" : [
      ["l","int","Steps to move left motor"],
      ["r","int","Steps to move right motor"],
      ["sps","int","Steps per second (0/undefined) is default"],
      ["callback","JsVar","Callback when complete"]
    ]
}*/
void jswrap_curio_go(int l, int r, int sps, JsVar *callback) {
  if (l>0) motorDirL=1;
  else if (l<0) motorDirL=-1;
  else motorDirL=0;
  if (r>0) motorDirR=1;
  else if (r<0) motorDirR=-1;
  else motorDirR=0;

  stepCountL = (motorDirL>0) ? l : -l;
  stepCountR = (motorDirR>0) ? r : -r;

  curio_setMotorStates();
}

void motorLHandler(bool state, IOEventFlags flags) {
  stepCountL--;

}

void motorRHandler(bool state, IOEventFlags flags) {
  stepCountR--;
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_curio_init"
}*/
void jswrap_curio_init() {
  jshPinOutput(CURIO_MOTORL1,0);
  jshPinOutput(CURIO_MOTORL2,0);
  jshPinOutput(CURIO_MOTORR1,0);
  jshPinOutput(CURIO_MOTORR2,0);

  jshPinOutput(CURIO_IR_LED,0); // LEDs on

  IOEventFlags channel;
  jshSetPinShouldStayWatched(CURIO_IR_L,true);
  channel = jshPinWatch(CURIO_IR_L, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, motorLHandler);
  jshSetPinShouldStayWatched(CURIO_IR_R,true);
  channel = jshPinWatch(CURIO_IR_R, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, motorRHandler);
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_curio_idle"
}*/
bool jswrap_curio_idle() {
  bool busy = false;
  if (motorDirL || motorDirR) {
    curio_setMotorStates();
    if (stepCountL<0) {
      stepCountL=0;
      motorDirL=0;
      jshPinOutput(CURIO_MOTORL1,0);
      jshPinOutput(CURIO_MOTORL2,0);
    }
    if (stepCountR<0) {
      stepCountR=0;
      motorDirR=0;
      jshPinOutput(CURIO_MOTORR1,0);
      jshPinOutput(CURIO_MOTORR2,0);
    }
    busy = true;
  }
  jsvObjectSetChildAndUnLock(execInfo.root, "CL", jsvNewFromInteger(stepCountL)); // debug
  jsvObjectSetChildAndUnLock(execInfo.root, "CR", jsvNewFromInteger(stepCountR)); // debug
  return false;
}