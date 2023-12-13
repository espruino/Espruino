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
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript interface for Jolt.js Qwiic object
 * ----------------------------------------------------------------------------


 */

#include "jsvar.h"
#include "jsparse.h"
#include "jswrap_qwiic.h"
#include "jswrap_spi_i2c.h"
#include "jshardware.h"
#include "jsinteractive.h"

/*JSON{
  "type": "class",
  "class" : "Qwiic",
  "ifdef" : "JOLTJS"
}
Class containing utility functions for the Qwiic connectors
on the [Jolt.js Smart Bluetooth driver](http://www.espruino.com/Jolt.js).

Each class (available from `Jolt.Q1`/`Jolt.Q2`/`Jolt.Q3`/`Jolt.Q4`)
has `sda` and `scl` fields with the pins for SDA and SCL on them.

On Jolt.js, the four Qwiic connectors can be individually powered:

* Q1/Q2 - GND is switched with a 500mA FET. The `fet` field contains the pin that controls the FET
* Q3/Q4 - all 4 pins are connected to GPIO. `gnd` and `vcc` fields contain the pins for GND and VCC

To control the power, use `Qwiic.setPower`, for example: `Jolt.Q1.setPower(true)`

*/

/*JSON{
  "type" : "method",
  "class" : "Qwiic",
  "name" : "setPower",
  "generate" : "jswrap_jqwiic_setPower",
  "params" : [
    ["isOn","bool","Whether the Qwiic connector is to be on or not"]
  ],
  "return" : ["JsVar","The same Qwiic object (for call chaining)"]
}
This turns power for the given Qwiic connector on or off. See `Qwiic` for more information.
*/
JsVar *jswrap_jqwiic_setPower(JsVar *parent, bool isOn) {
  JsVar *o = jsvNewObject();
  if (!o) return 0;
  JsVar *fet = jsvObjectGetChildIfExists(parent, "fet");
  if (fet) {
    jshPinOutput(jsvGetIntegerAndUnLock(fet), isOn);
  }
  JsVar *gnd = jsvObjectGetChildIfExists(parent, "gnd");
  if (gnd) {
    jshPinOutput(jsvGetIntegerAndUnLock(gnd), 0);
  }
  JsVar *vcc = jsvObjectGetChildIfExists(parent, "vcc");
  if (vcc) {
    jshPinOutput(jsvGetIntegerAndUnLock(vcc), isOn);
  }
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "property",
  "class" : "Qwiic",
  "name" : "i2c",
  "generate" : "jswrap_jqwiic_i2c",
  "return" : ["JsVar","An I2C object using this Qwiic connector, already set up"]
}
*/
JsVar *jswrap_jqwiic_i2c(JsVar *parent) {
  // set options
  JsVar *o = jsvNewObject();
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o, "sda", jsvObjectGetChildIfExists(parent, "sda"));
  jsvObjectSetChildAndUnLock(o, "scl", jsvObjectGetChildIfExists(parent, "scl"));
  JsVar *i2c = jspNewObject(0, "I2C");
  if (!i2c) {
    jsvUnLock(o);
    return 0;
  }
  jswrap_i2c_setup(i2c, o);
  jsvUnLock(o);
  jsvObjectSetChild(parent, "i2c", i2c); // set the child so we don't get called again
  return i2c;
}
