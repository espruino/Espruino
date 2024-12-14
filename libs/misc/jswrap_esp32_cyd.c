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
#include "jsspi.h"
#include "jsinteractive.h"

spi_sender_data touchSPI;
bool wasTouched;
int touchx, touchy;

/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py */

/*JSON{
  "type" : "event",
  "class" : "E",
  "name" : "touch",
  "params" : [["xy","JsVar","An object `{x,y,b}`"]]
}
*/


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
  "type" : "variable",
  "name" : "SPK",
  "generate_full" : "(Pin)26",
  "return" : ["pin","the pin for the speaker"]
}
*/

/*JSON{
  "type" : "init",
  "generate" : "jswrap_cyd_init"
}*/
void jswrap_cyd_init() {
  jshPinOutput(LED1_PININDEX,0); // LED1 is glowing by default?
  jshPinOutput(LED3_PININDEX,0); // LED3 is on by default?

  jshPinSetState(TOUCH_PIN_SCK,  JSHPINSTATE_GPIO_OUT);
  jshPinSetState(TOUCH_PIN_MISO,  JSHPINSTATE_GPIO_IN);
  jshPinSetState(TOUCH_PIN_MOSI,  JSHPINSTATE_GPIO_OUT);
  jshPinSetState(TOUCH_PIN_CS,  JSHPINSTATE_GPIO_OUT);
  jshPinSetState(TOUCH_PIN_IRQ,  JSHPINSTATE_GPIO_IN);

  jshDelayMicroseconds(1000);

  jshSPIInitInfo(&touchSPI);
  touchSPI.pinMISO = TOUCH_PIN_MISO;
  touchSPI.pinMOSI = TOUCH_PIN_MOSI;
  touchSPI.pinSCK = TOUCH_PIN_SCK;

  unsigned char d[2] = {0x90,0};
  jsspiSoftwareFunc(d, NULL, 2, &touchSPI); // turn on
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_cyd_idle"
}*/
bool jswrap_cyd_idle() {
  bool sendEvent = false;
  if (!jshPinGetValue(TOUCH_PIN_IRQ)) {
    unsigned char d[5] = {0x90,0,0xD0,0,0};
    jsspiSoftwareFunc(d, d, 5, &touchSPI); // get data
    int x = d[1]<<8 | d[2];
    int y = d[3]<<8 | d[4];
    x = x * LCD_WIDTH / 31000;
    y = y * LCD_HEIGHT / 31000;
    if (!wasTouched || x!=touchx || y!=touchy) {
      touchx = x;
      touchy = y;
      sendEvent = true;
    }
    wasTouched = true;
  } else {
    if (wasTouched) sendEvent = true;
    wasTouched = false;
  }
  if (sendEvent) {
    JsVar *E = jsvObjectGetChildIfExists(execInfo.root, "E");
    if (E) {
      JsVar *o = jsvNewObject();
      jsvObjectSetChildAndUnLock(o,"x", jsvNewFromInteger(touchx));
      jsvObjectSetChildAndUnLock(o,"y", jsvNewFromInteger(touchy));
      jsvObjectSetChildAndUnLock(o,"b", jsvNewFromInteger(wasTouched?1:0));
      jsiQueueObjectCallbacks(E, JS_EVENT_PREFIX"touch", &o, 1);
      jsvUnLock2(E,o);
    }
  }
  return false;
}