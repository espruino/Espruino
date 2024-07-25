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
  "type" : "variable",
  "name" : "IRLED",
  "generate_full" : "CURIO_IR_LED",
  "return" : ["pin","The pin for the IR LED"]
}
*/
/*JSON{
  "type" : "variable",
  "name" : "IRL",
  "generate_full" : "CURIO_IR_L",
  "return" : ["pin","The pin for the left IR sensor"]
}
*/

/*JSON{
  "type" : "variable",
  "name" : "ML1",
  "generate_full" : "CURIO_MOTORL1",
  "return" : ["pin","The pin for the left motor"]
}
*/
/*JSON{
  "type" : "variable",
  "name" : "ML2",
  "generate_full" : "CURIO_MOTORL2",
  "return" : ["pin","The pin for the left motor"]
}
*/
/*JSON{
  "type" : "variable",
  "name" : "IRR",
  "generate_full" : "CURIO_IR_R",
  "return" : ["pin","The pin for the right IR sensor"]
}
*/
/*JSON{
  "type" : "variable",
  "name" : "MR1",
  "generate_full" : "CURIO_MOTORR1",
  "return" : ["pin","The pin for the right motor"]
}
*/
/*JSON{
  "type" : "variable",
  "name" : "MR2",
  "generate_full" : "CURIO_MOTORR2",
  "return" : ["pin","The pin for the right motor"]
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
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_curio_idle"
}*/
bool jswrap_curio_idle() {
  return false;
}