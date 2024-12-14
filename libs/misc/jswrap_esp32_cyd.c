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
 *  ESP32 CYD (cheap yellow display) implementation
 * ----------------------------------------------------------------------------
 */
#include "jswrap_curio.h"
#include "jshardware.h"
#include "jsparse.h"

/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py */

/*JSON{
  "type" : "variable",
  "name" : "CN1",
  "generate" : "jswrap_cyd_cn1",
  "return" : ["JsVar","An object containing the pins for the CN1 connector"],
  "return_object" : "Qwiic"
}
*/
JsVar *jswrap_cyd_cn1() {
  JsVar *o = jspNewObject(0, "Qwiic");
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o, "sda", jsvNewFromPin(QWIIC0_PIN_SDA));
  jsvObjectSetChildAndUnLock(o, "scl", jsvNewFromPin(QWIIC0_PIN_SCL));
  jsvObjectSetChild(execInfo.root, "CN1", o);
  return o;
}
/*JSON{
  "type" : "variable",
  "name" : "P1",
  "generate" : "jswrap_cyd_p1",
  "return" : ["JsVar","An object containing the pins for the P1 connector"],
  "return_object" : "Qwiic"
}
*/
JsVar *jswrap_cyd_p1() {
  JsVar *o = jspNewObject(0, "Qwiic");
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o, "sda", jsvNewFromPin(QWIIC1_PIN_SDA));
  jsvObjectSetChildAndUnLock(o, "scl", jsvNewFromPin(QWIIC1_PIN_SCL));
  jsvObjectSetChild(execInfo.root, "P1", o);
  return o;
}
/*JSON{
  "type" : "variable",
  "name" : "P3",
  "generate" : "jswrap_cyd_p3",
  "return" : ["JsVar","An object containing the pins for the P3 connector"],
  "return_object" : "Qwiic"
}
*/
JsVar *jswrap_cyd_p3() {
  JsVar *o = jspNewObject(0, "Qwiic");
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o, "sda", jsvNewFromPin(QWIIC3_PIN_SDA));
  jsvObjectSetChildAndUnLock(o, "scl", jsvNewFromPin(QWIIC3_PIN_SCL));
  jsvObjectSetChildAndUnLock(o, "vcc", jsvNewFromPin(QWIIC3_PIN_VCC));
  jsvObjectSetChild(execInfo.root, "P3", o);
  return o;
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_cyd_init"
}*/
void jswrap_cyd_init() {
  jshPinOutput(LED1_PININDEX,0); // LED1 is glowing by default?
  jshPinOutput(LED3_PININDEX,0); // LED3 is on by default?
}
