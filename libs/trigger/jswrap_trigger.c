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
 * Contains JavaScript interface for trigger wheel functionality
 * ----------------------------------------------------------------------------
 */
#include "trigger.h"
#include "jswrap_trigger.h"

/*JSON{ "type":"class",
        "class" : "Trig",
        "description" : ["This class exists in order to interface Espruino with fast-moving trigger wheels. Trigger wheels are physical discs with evenly spaced teeth cut into them, and often with one or two teeth next to each other missing. A sensor sends a signal whenever a tooth passed by, and this allows a device to measure not only RPM, but absolute position.",
                         "This class is currently in testing - it is NOT AVAILABLE on normal boards." ]
}*/

/*JSON{ "type":"staticmethod", "class": "Trig", "name" : "getPosAtTime",
         "description" : "Get the position of the trigger wheel at the given time (from getTime)",
         "generate" : "jswrap_trig_getPosAtTime",
         "params" : [ [ "time", "float", "The time at which to find the position" ] ],
         "return" : [ "float", "The position of the trigger wheel in teeth - as a floating point number" ]
}*/
JsVarFloat jswrap_trig_getPosAtTime(JsVarFloat time) {
  JsSysTime sTime = (JsSysTime)(time * (JsVarFloat)jshGetTimeFromMilliseconds(1000));
  TriggerStruct *trig = &mainTrigger;
  JsVarFloat position = trigGetToothAtTime(trig, sTime);
  return wrapAround((position * 360 / trig->teethTotal) + trig->keyPosition, 360);
}

/*JSON{ "type":"staticmethod", "class": "Trig", "name" : "setup",
         "description" : "Initialise the trigger class",
         "generate" : "jswrap_trig_setup",
         "params" : [ [ "pin", "pin", "The pin to use for triggering" ],
                      ["options", "JsVar", "Additional options as an object. defaults are: ```{teethTotal:60,teethMissing:2,minRPM:30,keyPosition:0}```" ]]
}*/
void jswrap_trig_setup(Pin pin, JsVar *options) {
  if (!jshIsPinValid(pin)) {
    jsError("Invalid pin supplied as an argument to Trig.setup");
    return;
  }

  TriggerStruct *trig = &mainTrigger;
  // static info
  trig->teethMissing = 2;
  trig->teethTotal = 60;
  trig->keyPosition = 0;
  JsVarFloat minRPM = 30;
  if (jsvIsObject(options)) {
    JsVar *v;
    v = jsvObjectGetChild(options, "teethMissing", 0);
    if (!jsvIsUndefined(v)) trig->teethMissing = (unsigned char)jsvGetInteger(v);
    jsvUnLock(v);
    v = jsvObjectGetChild(options, "teethTotal", 0);
    if (!jsvIsUndefined(v)) trig->teethTotal = (unsigned char)jsvGetInteger(v);
    jsvUnLock(v);
    v = jsvObjectGetChild(options, "minRPM", 0);
    if (!jsvIsUndefined(v)) minRPM = jsvGetFloat(v);
    jsvUnLock(v);
    v = jsvObjectGetChild(options, "keyPosition", 0);
    if (!jsvIsUndefined(v)) trig->keyPosition = jsvGetFloat(v);
    jsvUnLock(v);
  }
  trig->maxTooth = (unsigned int)jshGetTimeFromMilliseconds(60000 / (JsVarFloat)(trig->teethTotal * minRPM));

  // semi-static info
  int i;
  for (i=0;i<TRIGGER_TRIGGERS_COUNT;i++) {
    trig->triggers[i].tooth = TRIGGERPOINT_TOOTH_DISABLE;
    trig->triggers[i].newTooth = TRIGGERPOINT_TOOTH_DISABLE;
  }
  // dynamic info
  trig->lastTime = jshGetSystemTime();
  trig->avrTrigger = (unsigned int)jshGetTimeFromMilliseconds(10); // average time for a trigger pulse
  trig->avrTooth = (unsigned int)jshGetTimeFromMilliseconds(10); // average time for a tooth
  trig->currTooth = 0;
  trig->teethSinceStart = 0;
  trig->wrongTriggerTeeth = 0;
  // finally set up the watch!
  if (jshIsPinValid(trig->sensorPin))
    jshPinWatch(trig->sensorPin, false);
  trig->sensorPin = pin;
  jshPinWatch(trig->sensorPin, true);
}

/*JSON{ "type":"staticmethod", "class": "Trig", "name" : "setTrigger",
         "description" : "Set a trigger for a certain point in the cycle",
         "generate" : "jswrap_trig_setTrigger",
         "params" : [ ["num", "int", "The trigger number (0..7)"],
                      ["pos", "float", "The position (in degrees) to fire the trigger at" ],
                      ["pins", "JsVar", "An array of pins to pulse (max 4)" ],
                      ["pulseLength", "float", "The time (in msec) to pulse for" ]]
}*/
void jswrap_trig_setTrigger(JsVarInt num, JsVarFloat position, JsVar *pins, JsVarFloat pulseLength) {
  TriggerStruct *trig = &mainTrigger;
  if (num<0 || num>=TRIGGER_TRIGGERS_COUNT) {
     jsWarn("Invalid trigger number\n");
     return;
   }
  if (!jsvIsArray(pins)) {
    jsWarn("Second argument must be an array of pins\n");
    return;
  }
  if (jsvGetArrayLength(pins) > TRIGGERPOINT_TRIGGERS_COUNT) {
    jsWarn("Too many pins in array\n");
    return;
  }

  // convert from degrees to teeth
  position = wrapAround(((position - trig->keyPosition) * trig->teethTotal / 360), trig->teethTotal);

  TriggerPointStruct *tp = &trig->triggers[num];
  tp->newTooth = (unsigned char)position;
  tp->newToothFraction = (unsigned char)((position - tp->tooth)*256);
  tp->pulseLength = jshGetTimeFromMilliseconds(pulseLength);
  int i;
  for (i=0;i<TRIGGERPOINT_TRIGGERS_COUNT;i++) {
    tp->pins[i] = jshGetPinFromVarAndUnLock(jsvGetArrayItem(pins, i));
  }
  // now copy over data if we need to do it immediately
  if (tp->tooth==TRIGGERPOINT_TOOTH_DISABLE || tp->newTooth==TRIGGERPOINT_TOOTH_DISABLE) {
    tp->tooth = tp->newTooth;
    tp->toothFraction = tp->newToothFraction;
  }
  // all done!
}

/*JSON{ "type":"staticmethod", "class": "Trig", "name" : "killTrigger",
         "description" : "Disable a trigger",
         "generate" : "jswrap_trig_killTrigger",
         "params" : [ ["num", "int", "The trigger number (0..7)"] ]
}*/
void jswrap_trig_killTrigger(JsVarInt num) {
  TriggerStruct *trig = &mainTrigger;
  if (num<0 || num>=TRIGGER_TRIGGERS_COUNT) {
     jsWarn("Invalid trigger number\n");
     return;
   }

  TriggerPointStruct *tp = &trig->triggers[num];
  tp->tooth = TRIGGERPOINT_TOOTH_DISABLE;
  tp->newTooth = TRIGGERPOINT_TOOTH_DISABLE;
}

/*JSON{ "type":"staticmethod", "class": "Trig", "name" : "getTrigger",
         "description" : "Get the current state of a trigger",
         "generate" : "jswrap_trig_getTrigger",
         "params" : [ ["num", "int", "The trigger number (0..7)"] ],
         "return" : [ "JsVar", "A structure containing all information about the trigger" ]
}*/
JsVar *jswrap_trig_getTrigger(JsVarInt num) {
  TriggerStruct *trig = &mainTrigger;
  if (num<0 || num>=TRIGGER_TRIGGERS_COUNT) {
     jsWarn("Invalid trigger number\n");
     return 0;
   }
  TriggerPointStruct *tp = &trig->triggers[num];

  // get offset in teeth
  JsVarFloat position = tp->tooth + (tp->toothFraction/256.0);
  // convert from teeth to degrees
  position = wrapAround((position * 360 / trig->teethTotal) + trig->keyPosition, 360);


  JsVar *obj = jsvNewWithFlags(JSV_OBJECT);
  if (!obj) return 0;
  JsVar *v;
  v = jsvNewFromFloat(position);
  jsvUnLock(jsvAddNamedChild(obj, v, "pos"));
  jsvUnLock(v);
  v = jsvNewFromFloat(jshGetMillisecondsFromTime(tp->pulseLength));
  jsvUnLock(jsvAddNamedChild(obj, v, "pulseLength"));
  jsvUnLock(v);
  v = jsvNewWithFlags(JSV_ARRAY);
  int i;
  if (v) {
    for (i=0;i<TRIGGERPOINT_TRIGGERS_COUNT;i++)
      if (tp->pins[i]>=0) {
        jsvArrayPushAndUnLock(v, jsvNewFromPin(tp->pins[i]));
      }
  }
  jsvUnLock(jsvAddNamedChild(obj, v, "pins"));
  jsvUnLock(v);
  return obj;
}

/*JSON{ "type":"staticmethod", "class": "Trig", "name" : "getRPM",
         "description" : "Get the RPM of the trigger wheel",
         "generate" : "jswrap_trig_getRPM",
         "return" : [ "float", "The current RPM of the trigger wheel" ]
}*/
JsVarFloat jswrap_trig_getRPM() {
  TriggerStruct *trig = &mainTrigger;

  if (jshGetSystemTime() > (trig->lastTime + trig->maxTooth)) return 0;
  return jshGetTimeFromMilliseconds(60000) / (JsVarFloat)(trig->avrTooth * trig->teethTotal);
}

/*JSON{ "type":"staticmethod", "class": "Trig", "name" : "getErrors",
         "description" : "Get the current error flags from the trigger wheel - and zero them",
         "generate" : "jswrap_trig_getErrors",
         "return" : [ "int", "The error flags" ]
}*/
JsVarInt jswrap_trig_getErrors() {
  TriggerStruct *trig = &mainTrigger;
  TriggerError errors = trig->errors;
  trig->errors = 0;
  return (JsVarInt)errors;
}

/*JSON{ "type":"staticmethod", "class": "Trig", "name" : "getErrorArray",
         "description" : "Get the current error flags from the trigger wheel - and zero them",
         "generate" : "jswrap_trig_getErrorArray",
         "return" : [ "JsVar", "An array of error strings" ]
}*/
JsVar* jswrap_trig_getErrorArray() {
  TriggerStruct *trig = &mainTrigger;
  TriggerError errors = trig->errors;
  trig->errors = 0;

  JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
  if (arr) {
    int i;
    for (i=1;i<=errors;i<<=1) {
      if (errors & i) {
        const char *s = trigGetErrorString(i);
        if (s) {
          jsvArrayPushAndUnLock(arr, jsvNewFromString(s));
        }
      }
    }
  }
  return arr;
}
