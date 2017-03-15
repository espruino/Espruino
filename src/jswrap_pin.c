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
 * JavaScript Pin Object Functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_pin.h"
#include "jswrap_io.h"
#include "jstimer.h"

/*JSON{
  "type"  : "class",
  "class" : "Pin",
  "check" : "jsvIsPin(var)"
}
This is the built-in class for Pins, such as D0,D1,LED1, or BTN

You can call the methods on Pin, or you can use Wiring-style functions such as digitalWrite
*/

/*JSON{
  "type"     : "constructor",
  "class"    : "Pin",
  "name"     : "Pin",
  "generate" : "jswrap_pin_constructor",
  "params"   : [
    ["value", "JsVar", "A value to be converted to a pin. Can be a number, pin, or String."]
  ],
  "return"   : ["JsVar","A Pin object"]
}
Creates a pin from the given argument (or returns undefined if no argument)
*/
JsVar *jswrap_pin_constructor(JsVar *val) {
  Pin pin = jshGetPinFromVar(val);
  if (!jshIsPinValid(pin)) return 0;
#ifdef ESP8266
  if (jsvIsInt(val) && !jsvIsPin(val))
    jsWarn("The Pin() constructor is deprecated. Please use `D%d`, or NodeMCU.Dx instead", pin);
#endif
  return jsvNewFromPin(pin);
}


/*JSON{
  "type"     : "method",
  "class"    : "Pin",
  "name"     : "read",
  "generate" : "jswrap_pin_read",
  "return"   : ["bool","Whether pin is a logical 1 or 0"]
}
Returns the input state of the pin as a boolean.

 **Note:** if you didn't call `pinMode` beforehand then this function will also reset the pin's state to `"input"`
*/
bool jswrap_pin_read(JsVar *parent) {
  Pin pin = jshGetPinFromVar(parent);
  return jshPinInput(pin);
}

/*JSON{
  "type"     : "method",
  "class"    : "Pin",
  "name"     : "set",
  "generate" : "jswrap_pin_set"
}
Sets the output state of the pin to a 1

 **Note:** if you didn't call `pinMode` beforehand then this function will also reset the pin's state to `"output"`
 */
void jswrap_pin_set(JsVar *parent) {
  Pin pin = jshGetPinFromVar(parent);
  jshPinOutput(pin, 1);
}

/*JSON{
  "type"     : "method",
  "class"    : "Pin",
  "name"     : "reset",
  "generate" : "jswrap_pin_reset"
}
Sets the output state of the pin to a 0

 **Note:** if you didn't call `pinMode` beforehand then this function will also reset the pin's state to `"output"`
 */
void jswrap_pin_reset(JsVar *parent) {
  Pin pin = jshGetPinFromVar(parent);
  jshPinOutput(pin, 0);
}

/*JSON{
  "type"     : "method",
  "class"    : "Pin",
  "name"     : "write",
  "generate" : "jswrap_pin_write",
  "params"   : [
    ["value", "bool", "Whether to set output high (true/1) or low (false/0)"]
  ]
}
Sets the output state of the pin to the parameter given

 **Note:** if you didn't call `pinMode` beforehand then this function will also reset the pin's state to `"output"`
 */
void jswrap_pin_write(
    JsVar *parent, //!< The class instance representing the Pin.
    bool value     //!< The value to set the pin.
  ) {
  Pin pin = jshGetPinFromVar(parent);
  jshPinOutput(pin, value);
}

/*JSON{
  "type"     : "method",
  "class"    : "Pin",
  "name"     : "writeAtTime",
  "ifndef"   : "SAVE_ON_FLASH",
  "generate" : "jswrap_pin_writeAtTime",
  "params" : [
    ["value", "bool", "Whether to set output high (true/1) or low (false/0)"],
    ["time", "float", "Time at which to write"]
  ]
}
Sets the output state of the pin to the parameter given at the specified time.

 **Note:** this **doesn't** change the mode of the pin to an output. To do that, you need to use `pin.write(0)` or `pinMode(pin, 'output')` first.
 */
void jswrap_pin_writeAtTime(JsVar *parent, bool value, JsVarFloat time) {
  Pin pin = jshGetPinFromVar(parent);
  JsSysTime sTime = jshGetTimeFromMilliseconds(time*1000);
  jstPinOutputAtTime(sTime, &pin, 1, value);
}


/*JSON{
  "type"     : "method",
  "class"    : "Pin",
  "name"     : "getMode",
  "generate" : "jswrap_pin_getMode",
  "return"   : ["JsVar", "The pin mode, as a string"]
}
Return the current mode of the given pin. See `pinMode` for more information.
 */
JsVar *jswrap_pin_getMode(JsVar *parent) {
  return jswrap_io_getPinMode(jshGetPinFromVar(parent));  
}

/*JSON{
  "type"     : "method",
  "class"    : "Pin",
  "name"     : "mode",
  "generate" : "jswrap_pin_mode",
  "params" : [
    ["mode", "JsVar", "The mode - a string that is either 'analog', 'input', 'input_pullup', 'input_pulldown', 'output', 'opendrain', 'af_output' or 'af_opendrain'. Do not include this argument if you want to revert to automatic pin mode setting."]
  ]
}
Set the mode of the given pin. See [`pinMode`](#l__global_pinMode) for more information on pin modes.
 */
void jswrap_pin_mode(JsVar *parent, JsVar *mode) {
  jswrap_io_pinMode(jshGetPinFromVar(parent), mode, false);
}

/*JSON{
  "type"     : "method",
  "class"    : "Pin",
  "name"     : "toggle",
  "generate" : "jswrap_pin_toggle",
  "return"   : ["bool", "True if the pin is high after calling the function"]
}
Toggles the state of the pin from off to on, or from on to off.

**Note:** This method doesn't currently work on the ESP8266 port of Espruino.

**Note:** if you didn't call `pinMode` beforehand then this function will also reset the pin's state to `"output"`
*/
bool jswrap_pin_toggle(JsVar *parent) {
  Pin pin = jshGetPinFromVar(parent);
  if (!jshIsPinValid(pin)) return;
  bool on = !(jshPinGetState(pin)&JSHPINSTATE_PIN_IS_ON);
  jshPinOutput(pin, on);
  return on;
}


/*JSON{
  "type"     : "method",
  "class"    : "Pin",
  "name"     : "getInfo",
  "ifndef"   : "SAVE_ON_FLASH",
  "generate" : "jswrap_pin_getInfo",
  "return"   : ["JsVar","An object containing information about this pins"]
}
Get information about this pin and its capabilities. Of the form:

```
{
  "port"      : "A", // the Pin's port on the chip
  "num"       : 12, // the Pin's number
  "in_addr"   : 0x..., // (if available) the address of the pin's input address in bit-banded memory (can be used with peek)
  "out_addr"  : 0x..., // (if available) the address of the pin's output address in bit-banded memory (can be used with poke)
  "analog"    : { ADCs : [1], channel : 12 }, // If analog input is available
  "functions" : {
    "TIM1":{type:"CH1, af:0},
    "I2C3":{type:"SCL", af:1}
  }
}
```
Will return undefined if pin is not valid.
*/
JsVar *jswrap_pin_getInfo(
    JsVar *parent //!< The class instance representing the pin.
  ) {
  Pin pin = jshGetPinFromVar(parent);
  if (!jshIsPinValid(pin)) return 0;
  const JshPinInfo *inf = &pinInfo[pin];
  JsVar *obj = jsvNewObject();
  if (!obj) return 0;

  char buf[2];
  buf[0] = (char)('A'+(inf->port-JSH_PORTA));
  buf[1] = 0;
  jsvObjectSetChildAndUnLock(obj, "port", jsvNewFromString(buf));
  jsvObjectSetChildAndUnLock(obj, "num", jsvNewFromInteger(inf->pin-JSH_PIN0));
#ifdef STM32
  volatile uint32_t *addr;
  addr = jshGetPinAddress(pin, JSGPAF_INPUT);
  if (addr) jsvObjectSetChildAndUnLock(obj, "in_addr", jsvNewFromInteger((JsVarInt)addr));
  addr = jshGetPinAddress(pin, JSGPAF_OUTPUT);
  if (addr) jsvObjectSetChildAndUnLock(obj, "out_addr", jsvNewFromInteger((JsVarInt)addr));
#endif
  // ADC
  if (inf->analog) {
    JsVar *an = jsvNewObject();
    if (an) {
      JsVar *arr = jsvNewEmptyArray();
      if (arr) {
        int i;
        for (i=0;i<ADC_COUNT;i++)
          if (inf->analog&(JSH_ANALOG1<<i))
            jsvArrayPushAndUnLock(arr, jsvNewFromInteger(1+i));
        jsvObjectSetChildAndUnLock(an, "ADCs", arr);
      }
      jsvObjectSetChildAndUnLock(obj, "channel", jsvNewFromInteger(inf->analog & JSH_MASK_ANALOG_CH));
    }
  }
  JsVar *funcs = jsvNewObject();
  if (funcs) {
    int i;
    for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
      if (inf->functions[i]) {
        JsVar *func = jsvNewObject();
        if (func) {
          char buf[16];
          jshPinFunctionToString(inf->functions[i], JSPFTS_TYPE, buf, sizeof(buf));
          jsvObjectSetChildAndUnLock(func, "type", jsvNewFromString(buf));
          jsvObjectSetChildAndUnLock(func, "af", jsvNewFromInteger(inf->functions[i] & JSH_MASK_AF));

          jshPinFunctionToString(inf->functions[i], JSPFTS_DEVICE|JSPFTS_DEVICE_NUMBER, buf, sizeof(buf));
          jsvObjectSetChildAndUnLock(funcs, buf, func);
        }
      }
    }
    jsvObjectSetChildAndUnLock(obj, "functions", funcs);
  }

  return obj;
}

