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
 * JavaScript Hardware IO Functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_io.h"
#include "jsvar.h"

/*JSON{ "type":"function", "name" : "peek8",
         "description" : [ "Read 8 bits of memory at the given location - DANGEROUS!" ],
         "generate_full" : "(JsVarInt)*(unsigned char*)(size_t)addr",
         "params" : [ [ "addr", "int", "The address in memory to read"] ],
         "return" : ["int", "The value of memory at the given location"]
}*/
/*JSON{ "type":"function", "name" : "poke8",
         "description" : [ "Write 8 bits of memory at the given location - VERY DANGEROUS!" ],
         "generate_full" : "(*(unsigned char*)(size_t)addr) = (unsigned char)value",
         "params" : [ [ "addr", "int", "The address in memory to write"],
                      [ "value", "int", "The value to write"] ]
}*/
/*JSON{ "type":"function", "name" : "peek16",
         "description" : [ "Read 16 bits of memory at the given location - DANGEROUS!" ],
         "generate_full" : "(JsVarInt)*(unsigned short*)(size_t)addr",
         "params" : [ [ "addr", "int", "The address in memory to read"] ],
         "return" : ["int", "The value of memory at the given location"]
}*/
/*JSON{ "type":"function", "name" : "poke16",
         "description" : [ "Write 16 bits of memory at the given location - VERY DANGEROUS!" ],
         "generate_full" : "(*(unsigned short*)(size_t)addr) = (unsigned short)value",
         "params" : [ [ "addr", "int", "The address in memory to write"],
                      [ "value", "int", "The value to write"] ]
}*/
/*JSON{ "type":"function", "name" : "peek32",
         "description" : [ "Read 32 bits of memory at the given location - DANGEROUS!" ],
         "generate_full" : "(JsVarInt)*(unsigned int*)(size_t)addr",
         "params" : [ [ "addr", "int", "The address in memory to read"] ],
         "return" : ["int", "The value of memory at the given location"]
}*/
/*JSON{ "type":"function", "name" : "poke32",
         "description" : [ "Write 32 bits of memory at the given location - VERY DANGEROUS!" ],
         "generate_full" : "(*(unsigned int*)(size_t)addr) = (unsigned int)value",
         "params" : [ [ "addr", "int", "The address in memory to write"],
                      [ "value", "int", "The value to write"] ]
}*/

/*JSON{ "type":"function", "name" : "analogRead",
         "description" : ["Get the analog value of the given pin",
                          "This is different to Arduino which only returns an integer between 0 and 1023",
                          "However only pins connected to an ADC will work (see the datasheet)"],
         "generate" : "jshPinAnalog",
         "params" : [ [ "pin", "pin", [ "The pin to use", "You can find out which pins to use by looking at [your board's reference page](#boards) and searching for pins with the `ADC` markers." ] ] ],
         "return" : ["float", "The analog Value of the Pin between 0 and 1"]
}*/
/*JSON{ "type":"function", "name" : "analogWrite",
         "description" : "Set the analog Value of a pin. It will be output using PWM",
         "generate" : "jswrap_io_analogWrite",
         "params" : [ [ "pin", "pin", ["The pin to use", "You can find out which pins to use by looking at [your board's reference page](#boards) and searching for pins with the `PWM` or `DAC` markers." ]],
                      [ "value", "float", "A value between 0 and 1"],
                      [ "options", "JsVar", ["An object containing options.",
                                            "Currently only freq (pulse frequency in Hz) is available: ```analogWrite(A0,0.5,{ freq : 10 });``` ",
                                            "Note that specifying a frequency will force PWM output, even if the pin has a DAC"] ]  ]
}*/
void jswrap_io_analogWrite(Pin pin, JsVarFloat value, JsVar *options) {
  JsVarFloat freq = 0;
  if (jsvIsObject(options)) {
    freq = jsvGetFloatAndUnLock(jsvObjectGetChild(options, "freq", 0));
  }

  jshPinAnalogOutput(pin, value, freq);
}

/*JSON{ "type":"function", "name" : "digitalPulse",
         "description" : ["Pulse the pin with the value for the given time in milliseconds. It uses a hardware timer to produce accurate pulses, and returns immediately (before the pulse has finished). Use `digitalPulse(A0,1,0)` to wait until a previous pulse has finished.",
                          "eg. `digitalPulse(A0,1,5);` pulses A0 high for 5ms",
                          "digitalPulse is for SHORT pulses that need to be very accurate. If you're doing anything over a few milliseconds, use setTimeout instead." ],
         "generate" : "jswrap_io_digitalPulse",
         "params" : [ [ "pin", "pin", "The pin to use"],
                      [ "value", "bool", "Whether to pulse high (true) or low (false)"],
                      [ "time", "float", "A time in milliseconds"] ]
}*/
void jswrap_io_digitalPulse(Pin pin, bool value, JsVarFloat time) {
  if (time<0 || isnan(time)) {
    jsWarn("Pulse Time given for digitalPulse is less than 0, or not a number");
  } else {
    //jsPrintInt((JsVarInt)(time*1000));
    jshPinPulse(pin, value, time);
  }
}

/*JSON{ "type":"function", "name" : "digitalWrite",
         "description" : ["Set the digital value of the given pin",
                          "If pin is an array of pins, eg. ```[A2,A1,A0]``` the value will be treated as an integer where the first array element is the MSB.",
                          "In the case of an array of pins, pin values are set LSB first (from the right-hand side of the array of pins)." ],
         "generate" : "jswrap_io_digitalWrite",
         "params" : [ [ "pin", "JsVar", "The pin to use"],
                      [ "value", "int", "Whether to pulse high (true) or low (false)"] ]
}*/
void jswrap_io_digitalWrite(JsVar *pinVar, JsVarInt value) {
  if (jsvIsArray(pinVar)) {
    JsVarRef pinName = pinVar->lastChild; // NOTE: start at end and work back!
    while (pinName) {
      JsVar *pinNamePtr = jsvLock(pinName);
      JsVar *pinPtr = jsvSkipName(pinNamePtr);
      jshPinOutput(jshGetPinFromVar(pinPtr), value&1);
      jsvUnLock(pinPtr);
      pinName = pinNamePtr->prevSibling;
      jsvUnLock(pinNamePtr);
      value = value>>1; // next bit down
    }
  } else {
    Pin pin = jshGetPinFromVar(pinVar);
    jshPinOutput(pin, value!=0);
  }
}


/*JSON{ "type":"function", "name" : "digitalRead",
         "description" : ["Get the digital value of the given pin",
                          "If pin is an array of pins, eg. ```[A2,A1,A0]``` the value will be treated as an integer where the first array element is the MSB" ],
         "generate" : "jswrap_io_digitalRead",
         "params" : [ [ "pin", "JsVar", "The pin to use"] ],
         "return" : ["int", "The digital Value of the Pin"]
}*/
JsVarInt jswrap_io_digitalRead(JsVar *pinVar) {
  if (jsvIsArray(pinVar)) {
    int pins = 0;
    JsVarInt value = 0;
    JsVarRef pinName = pinVar->firstChild;
    while (pinName) {
      JsVar *pinNamePtr = jsvLock(pinName);
      JsVar *pinPtr = jsvSkipName(pinNamePtr);
      value = (value<<1) | jshPinInput(jshGetPinFromVar(pinPtr));
      jsvUnLock(pinPtr);
      pinName = pinNamePtr->nextSibling;
      jsvUnLock(pinNamePtr);
      pins++;
    }
    if (pins==0) return 0; // return undefined if array empty
    return value;
  } else {
    Pin pin = jshGetPinFromVar(pinVar);
    return jshPinInput(pin);
  }
}

/*JSON{ "type":"function", "name" : "pinMode",
         "description" : ["Set the mode of the given pin - note that digitalRead/digitalWrite/etc set this automatically unless pinMode has been called first. If you want digitalRead/etc to set the pin mode automatically after you have called pinMode, simply call it again with no mode argument: ```pinMode(pin)```" ],
         "generate" : "jswrap_io_pinMode",
         "params" : [ [ "pin", "pin", "The pin to set pin mode for"], [ "mode", "JsVar", "The mode - a string that is either 'input', 'input_pullup', 'input_pulldown', 'output', 'opendrain', 'af_output' or 'af_opendrain'. Do not include this argument if you want to revert to automatic pin mode setting."] ]
}*/
void jswrap_io_pinMode(Pin pin, JsVar *mode) {
  if (!jshIsPinValid(pin)) {
    jsError("Invalid pin");
    return;
  }
  JshPinState m = JSHPINSTATE_UNDEFINED;
  if (jsvIsString(mode)) {
    if (jsvIsStringEqual(mode, "input")) m = JSHPINSTATE_GPIO_IN;
    if (jsvIsStringEqual(mode, "input_pullup")) m = JSHPINSTATE_GPIO_IN_PULLUP;
    if (jsvIsStringEqual(mode, "input_pulldown")) m = JSHPINSTATE_GPIO_IN_PULLDOWN;
    if (jsvIsStringEqual(mode, "output")) m = JSHPINSTATE_GPIO_OUT;
    if (jsvIsStringEqual(mode, "opendrain")) m = JSHPINSTATE_GPIO_OUT_OPENDRAIN;
    if (jsvIsStringEqual(mode, "af_output")) m = JSHPINSTATE_AF_OUT;
    if (jsvIsStringEqual(mode, "af_opendrain")) m = JSHPINSTATE_AF_OUT_OPENDRAIN;
  }
  if (m != JSHPINSTATE_UNDEFINED) {
    jshSetPinStateIsManual(pin, true);
    jshPinSetState(pin, m);
  } else {
    jshSetPinStateIsManual(pin, false);
    if (!jsvIsUndefined(mode)) {
      jsError("Unknown pin mode");
    }
  }
}

/*JSON{ "type":"function", "name" : "getPinMode",
         "description" : ["Return the current mode of the given pin. See `pinMode`" ],
         "generate" : "jswrap_io_getPinMode",
         "params" : [ [ "pin", "pin", "The pin to check"] ],
         "return" : ["JsVar", "The pin mode, as a string"]
}*/
JsVar *jswrap_io_getPinMode(Pin pin) {
  if (!jshIsPinValid(pin)) {
    jsError("Invalid pin");
    return 0;
  }
  JshPinState m = jshPinGetState(pin)&JSHPINSTATE_MASK;
  const char *text = 0;
  switch (m) {
    case JSHPINSTATE_GPIO_IN : text = "input"; break;
    case JSHPINSTATE_GPIO_IN_PULLUP : text = "input_pullup"; break;
    case JSHPINSTATE_GPIO_IN_PULLDOWN : text = "input_pulldown"; break;
    case JSHPINSTATE_GPIO_OUT : text = "output"; break;
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN : text = "opendrain"; break;
    case JSHPINSTATE_AF_OUT : text = "af_output"; break;
    case JSHPINSTATE_AF_OUT_OPENDRAIN : text = "af_opendrain"; break;
    default: break;
  }
  if (text) return jsvNewFromString(text);
  return 0;
}

/*JSON{ "type":"function", "name" : "setWatch",
         "description" : ["Call the function specified when the pin changes",
                          "The function may also take an argument, which is an object of type `{time:float, lastTime:float, state:bool}`.",
                          "`time` is the time in seconds at which the pin changed state, `lastTime` is the time in seconds at which the pin last changed state, and `state` is the current state of the pin.",
                          "For instance, if you want to measure the length of a positive pusle you could use: ```setWatch(function(e) { console.log(e.time-e.lastTime); }, BTN, { repeat:true, edge:'falling' });```",
                          "This can also be removed using clearWatch" ],
         "generate" : "jswrap_interface_setWatch",
         "params" : [ [ "function", "JsVar", "A Function or String to be executed"],
                      [ "pin", "pin", "The pin to watch" ],
                      [ "options", "JsVar", ["If this is a boolean or integer, it determines whether to call this once (false = default) or every time a change occurs (true)",
                                             "If this is an object, it can contain the following information: ```{ repeat: true/false(default), edge:'rising'/'falling'/'both'(default), debounce:10}```. `debounce` is the time in ms to wait for bounces to subside, or 0." ] ]  ],
         "return" : ["JsVar", "An ID that can be passed to clearWatch"]
}*/
JsVar *jswrap_interface_setWatch(JsVar *func, Pin pin, JsVar *repeatOrObject) {

  if (!jsiIsWatchingPin(pin) && !jshCanWatch(pin)) {
    jsWarn("Unable to set watch on pin. You may already have a watch on a pin with the same number.");
    return 0;
  }

  bool repeat = false;
  JsVarFloat debounce = 0;
  int edge = 0;
  if (jsvIsObject(repeatOrObject)) {
    JsVar *v;
    repeat = jsvGetBoolAndUnLock(jsvObjectGetChild(repeatOrObject, "repeat", 0));
    debounce = jsvGetFloatAndUnLock(jsvObjectGetChild(repeatOrObject, "debounce", 0));
    if (isnan(debounce) || debounce<0) debounce=0;
    v = jsvObjectGetChild(repeatOrObject, "edge", 0);
    if (jsvIsString(v)) {
      if (jsvIsStringEqual(v, "rising")) edge=1;
      else if (jsvIsStringEqual(v, "falling")) edge=-1;
      else if (jsvIsStringEqual(v, "both")) edge=0;
      else jsWarn("'edge' in setWatch should be a string - either 'rising', 'falling' or 'both'");
    } else if (!jsvIsUndefined(v))
      jsWarn("'edge' in setWatch should be a string - either 'rising', 'falling' or 'both'");
    jsvUnLock(v);
  } else
    repeat = jsvGetBool(repeatOrObject);

  JsVarInt itemIndex = -1;
  if (!jsvIsFunction(func) && !jsvIsString(func)) {
    jsError("Function or String not supplied!");
  } else {
    // Create a new watch
    JsVar *watchPtr = jsvNewWithFlags(JSV_OBJECT);
    if (watchPtr) {
      jsvUnLock(jsvObjectSetChild(watchPtr, "pin", jsvNewFromPin(pin)));
      if (repeat) jsvUnLock(jsvObjectSetChild(watchPtr, "recur", jsvNewFromBool(repeat)));
      if (debounce>0) jsvUnLock(jsvObjectSetChild(watchPtr, "debounce", jsvNewFromInteger(jshGetTimeFromMilliseconds(debounce))));
      if (edge) jsvUnLock(jsvObjectSetChild(watchPtr, "edge", jsvNewFromInteger(edge)));
      jsvObjectSetChild(watchPtr, "callback", func); // no unlock intentionally
    }

    // If nothing already watching the pin, set up a watch
    if (!jsiIsWatchingPin(pin))
      jshPinWatch(pin, true);

    JsVar *watchArrayPtr = jsvLock(watchArray);
    itemIndex = jsvArrayPushWithInitialSize(watchArrayPtr, watchPtr, 1) - 1;
    jsvUnLock(watchArrayPtr);
    jsvUnLock(watchPtr);
  }
  return (itemIndex>=0) ? jsvNewFromInteger(itemIndex) : 0/*undefined*/;
}

/*JSON{ "type":"function", "name" : "clearInterval",
         "description" : ["Clear the Interval that was created with setInterval, for example:",
                          "```var id = setInterval(function () { print('foo'); }, 1000);```",
                          "```clearInterval(id);```",
                          "If no argument is supplied, all timers and intervals are stopped" ],
         "generate" : "jswrap_interface_clearInterval",
         "params" : [ [ "id", "JsVar", "The id returned by a previous call to setInterval"] ]
}*/
/*JSON{ "type":"function", "name" : "clearTimeout",
         "description" : ["Clear the Timeout that was created with setTimeout, for example:",
                          "```var id = setTimeout(function () { print('foo'); }, 1000);```",
                          "```clearTimeout(id);```",
                          "If no argument is supplied, all timers and intervals are stopped" ],
         "generate" : "jswrap_interface_clearTimeout",
         "params" : [ [ "id", "JsVar", "The id returned by a previous call to setTimeout"] ]
}*/
void _jswrap_interface_clearTimeoutOrInterval(JsVar *idVar, bool isTimeout) {
  JsVar *timerArrayPtr = jsvLock(timerArray);
  if (jsvIsUndefined(idVar)) {
    jsvRemoveAllChildren(timerArrayPtr);
  } else {
    JsVar *child = jsvIsBasic(idVar) ? jsvFindChildFromVar(timerArrayPtr, idVar, false) : 0;
    if (child) {
      JsVar *timerArrayPtr = jsvLock(timerArray);
      jsvRemoveChild(timerArrayPtr, child);
      jsvUnLock(child);
      jsvUnLock(timerArrayPtr);
    } else {
      jsError(isTimeout ? "Unknown Timeout" : "Unknown Interval");
    }
  }
  jsvUnLock(timerArrayPtr);
}
void jswrap_interface_clearInterval(JsVar *idVar) {
  _jswrap_interface_clearTimeoutOrInterval(idVar, false);
}
void jswrap_interface_clearTimeout(JsVar *idVar) {
  _jswrap_interface_clearTimeoutOrInterval(idVar, true);
}

/*JSON{ "type":"function", "name" : "changeInterval",
         "description" : ["Change the Interval on a callback created with setInterval, for example:",
                          "```var id = setInterval(function () { print('foo'); }, 1000); // every second```",
                          "```changeInterval(id, 1500); // now runs every 1.5 seconds```",
                          "This takes effect the text time the callback is called (so it is not immediate)."],
         "generate" : "jswrap_interface_changeInterval",
         "params" : [ [ "id", "JsVar", "The id returned by a previous call to setInterval"],
                      [ "time","float","The new time period in ms" ] ]
}*/
void jswrap_interface_changeInterval(JsVar *idVar, JsVarFloat interval) {
  JsVar *timerArrayPtr = jsvLock(timerArray);
  if (interval<TIMER_MIN_INTERVAL) interval=TIMER_MIN_INTERVAL;
  JsVar *timerName = jsvIsBasic(idVar) ? jsvFindChildFromVar(timerArrayPtr, idVar, false) : 0;

  if (timerName) {
    JsVar *timer = jsvSkipNameAndUnLock(timerName);
    JsVar *v;
    v = jsvNewFromInteger(jshGetTimeFromMilliseconds(interval));
    jsvUnLock(jsvSetNamedChild(timer, v, "interval"));
    jsvUnLock(v);
    v = jsvNewFromInteger(jshGetSystemTime() + jshGetTimeFromMilliseconds(interval));
    jsvUnLock(jsvSetNamedChild(timer, v, "time"));
    jsvUnLock(v);
    jsvUnLock(timer);
    // timerName already unlocked
  } else {
    jsError("Unknown Interval");
  }
  jsvUnLock(timerArrayPtr);
}

/*JSON{ "type":"function", "name" : "clearWatch",
         "description" : [ "Clear the Watch that was created with setWatch. If no parameter is supplied, all watches will be removed." ],
         "generate" : "jswrap_interface_clearWatch",
         "params" : [ [ "id", "JsVar", "The id returned by a previous call to setWatch"] ]
}*/
void jswrap_interface_clearWatch(JsVar *idVar) {

  if (jsvIsUndefined(idVar)) {
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, watchArrayPtr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *watchPtr = jsvArrayIteratorGetElement(&it);
      JsVar *watchPin = jsvObjectGetChild(watchPtr, "pin", 0);
      jshPinWatch(jshGetPinFromVar(watchPin), false);
      jsvUnLock(watchPin);
      jsvUnLock(watchPtr);
      jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    // remove all items
    jsvRemoveAllChildren(watchArrayPtr);
    jsvUnLock(watchArrayPtr);
  } else {
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsVar *watchNamePtr = jsvFindChildFromVar(watchArrayPtr, idVar, false);
    jsvUnLock(watchArrayPtr);
    if (watchNamePtr) { // child is a 'name'
      JsVar *watchPtr = jsvSkipName(watchNamePtr);
      Pin pin = jshGetPinFromVarAndUnLock(jsvObjectGetChild(watchPtr, "pin", 0));
      jsvUnLock(watchPtr);

      JsVar *watchArrayPtr = jsvLock(watchArray);
      jsvRemoveChild(watchArrayPtr, watchNamePtr);
      jsvUnLock(watchNamePtr);
      jsvUnLock(watchArrayPtr);

      // Now check if this pin is still being watched
      if (!jsiIsWatchingPin(pin))
        jshPinWatch(pin, false); // 'unwatch' pin
    } else {
      jsError("Unknown Watch");
    }
  }
}


