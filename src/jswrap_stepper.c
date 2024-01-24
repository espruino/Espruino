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
 * JavaScript methods for Stepper Motor control using builtin timer
 * ----------------------------------------------------------------------------
 */
#include "jswrap_stepper.h"
#include "jswrap_arraybuffer.h"
#include "jswrap_promise.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jstimer.h"

#define JSI_STEPPER_NAME "step"

#ifdef ESPR_USE_STEPPER_TIMER

/*JSON{
  "type" : "class",
  "ifdef" : "ESPR_USE_STEPPER_TIMER",
  "class" : "Stepper"
}
(2v21+ only) This class allows Espruino to control stepper motors.

On Espruino before 2v20 you can still use the Stepper Motor module at https://www.espruino.com/StepperMotor - it just isn't quite as fast.
 */

/// Get pins (4 element array) from stepper object
static bool jswrap_stepper_getPins(JsVar *stepper, Pin *pins) {
  JsVar *pinsVar = jsvObjectGetChildIfExists(stepper, "pins");
  bool ok = false;
  if (jsvIsArray(pinsVar) && jsvGetArrayLength(pinsVar)==4) {
    ok = true;
    for (int i=0;i<4;i++) {
      pins[i] = jsvGetIntegerAndUnLock(jsvGetArrayItem(pinsVar, i));
      if (!jshIsPinValid(pins[i]))
        ok = false;
    }
  }
  jsvUnLock(pinsVar);
  return ok;
}

/// Get pattern (a 4 element uint8 array) from stepper object
static bool jswrap_stepper_getPattern(JsVar *stepper, uint8_t *pattern) {
  JsVar *patternVar = jsvObjectGetChildIfExists(stepper, "pattern");
  bool ok = false;
  if (jsvIsArray(patternVar) && (jsvGetArrayLength(patternVar)==4 || jsvGetArrayLength(patternVar)==8)) {
    ok = true;
    int l = jsvGetArrayLength(patternVar);
    memset(pattern,0,4);
    for (int i=0;i<l;i++) {
      int v = jsvGetIntegerAndUnLock(jsvGetArrayItem(patternVar, i));
      if (v<0 || v>15) ok = false;
      if (i&1) v<<=4;
      pattern[i>>1] |= v;
    }
    if (l==4) { // for length of 4 just repeat
      pattern[2] = pattern[0];
      pattern[3] = pattern[1];
    }
  }
  jsvUnLock(patternVar);
  return ok;
}


/*JSON{
  "type" : "idle",
  "generate" : "jswrap_stepper_idle",
  "ifdef" : "ESPR_USE_STEPPER_TIMER"
}*/
bool jswrap_stepper_idle() {
  JsVar *steppers = jsvObjectGetChildIfExists(execInfo.hiddenRoot, JSI_STEPPER_NAME);
  if (steppers) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, steppers);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *stepper = jsvObjectIteratorGetValue(&it);
      bool running = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(stepper, "running"));
      Pin pins[4];
      if (running) {
        UtilTimerTask task;
        // Search for a timer task
        if (!jswrap_stepper_getPins(stepper, pins) || !jstGetLastPinTimerTask(pins[0], &task)) {
          // if the task is now gone...
          jsiQueueObjectCallbacks(stepper, JS_EVENT_PREFIX"finish", NULL, 0);
          // Update current position
          jsvObjectSetChildAndUnLock(stepper, "pos", jsvNewFromInteger(
            jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(stepper, "pos"))+
            jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(stepper, "_direction"))));
          jsvObjectRemoveChild(stepper, "_direction");
          if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(stepper, "_turnOff"))) {
            Pin pins[4];
            int offpattern = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(stepper, "offpattern"));
            if (jswrap_stepper_getPins(stepper, pins)) {
              for (int i=0;i<4;i++)
                jshPinSetValue(pins[i], (offpattern>>i)&1);
            }
          }
          jsvObjectRemoveChild(stepper, "_turnOff");
          // set running flag
          running = false;
          jsvObjectSetChildAndUnLock(stepper, "running", jsvNewFromBool(running));
          JsVar *promise = jsvObjectGetChildIfExists(stepper, "promise");
          if (promise) {
            jsvObjectRemoveChild(stepper, "promise");
            jspromise_resolve(promise, 0);
            jsvUnLock(promise);
          }
        } else {
          // If the timer task is still there, don't do anything
          // note... we could fire off a 'stepping' event?
        }
      }
      jsvUnLock(stepper);
      // if not running, remove stepper from this list
      if (!running)
        jsvObjectIteratorRemoveAndGotoNext(&it, steppers);
      else
        jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    jsvUnLock(steppers);
  }
  return false; // no need to stay awake - an IRQ will wake us
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_stepper_kill",
  "ifdef" : "ESPR_USE_STEPPER_TIMER"
}*/
void jswrap_stepper_kill() { // be sure to remove all stepper instances...
  JsVar *steppers = jsvObjectGetChildIfExists(execInfo.hiddenRoot, JSI_STEPPER_NAME);
  if (steppers) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, steppers);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *stepper = jsvObjectIteratorGetValue(&it);
      bool running = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(stepper, "running"));
      if (running) {
        Pin pins[4];
        if (!jswrap_stepper_getPins(stepper, pins) || !jstStopPinTimerTask(pins[0]))
          jsExceptionHere(JSET_ERROR, "Stepper couldn't be stopped");
      }
      jsvUnLock(stepper);
      // if not running, remove stepper from this list
      jsvObjectIteratorRemoveAndGotoNext(&it, steppers);
    }
    jsvObjectIteratorFree(&it);
    jsvUnLock(steppers);
  }
}


/*JSON{
  "type" : "constructor",
  "class" : "Stepper",
  "name" : "Stepper",
  "ifdef" : "ESPR_USE_STEPPER_TIMER",
  "generate" : "jswrap_stepper_constructor",
  "params" : [
      ["options","JsVar","options struct `{pins:[1,2,3,4]}`"]
  ],
  "return" : ["JsVar","An Stepper object"]
}
Create a `Stepper` class. `options` can contain:

```
{
  pins : [...], // required - 4 element array of pins
  pattern : [...], // optional - a 4/8 element array of step patterns
  offpattern : 0, // optional (default 0) - the pattern to output to stop driving the stepper motor
  freq : 500,   // optional (default 500) steps per second
}
```

`pins` must be supplied as a 4 element array of pins. When created,
if pin state has not been set manually on each pin, the pins will
be set to outputs.

If `pattern` isn't specified, a default pattern of `[0b0001,0b0010,0b0100,0b1000]` will be used. You
can specify different patterns, for example `[0b1100,0b1000,0b1001,0b0001,0b0011,0b0010,0b0110,0b0100]`.
 */
JsVar *jswrap_stepper_constructor(JsVar *options) {
  if (!jsvIsObject(options)) {
    jsExceptionHere(JSET_ERROR, "Expecting options to be an Object, not %t", options);
    return 0;
  }

  JsVar *pinsVar = 0;
  JsVar *patternVar = 0;
  JsVarFloat freq = 500;
  int offpattern = -1;
  jsvConfigObject configs[] = {
      {"pins", JSV_ARRAY, &pinsVar},
      {"pattern", JSV_ARRAY, &patternVar},
      {"freq", JSV_FLOAT, &freq},
      {"offpattern", JSV_INTEGER, &offpattern}
  };
  if (!jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    return 0;
  }
  if (!jsvIsArray(pinsVar) || jsvGetArrayLength(pinsVar)!=4) {
    jsvUnLock2(pinsVar, patternVar);
    jsExceptionHere(JSET_ERROR, "'pins' must be a 4 element array");
    return 0;
  }

  JsVar *stepper = jspNewObject(0, "Stepper");
  if (!stepper) // out of memory
    return 0;
  jsvObjectSetChildAndUnLock(stepper, "pins", pinsVar);
  jsvObjectSetChildAndUnLock(stepper, "pos", jsvNewFromInteger(0));
  jsvObjectSetChildAndUnLock(stepper, "freq", jsvNewFromFloat(freq));
  if (offpattern) jsvObjectSetChildAndUnLock(stepper, "offpattern", jsvNewFromInteger(offpattern));
  if (patternVar) {
    jsvObjectSetChildAndUnLock(stepper, "pattern", patternVar);
    uint8_t pattern[4];
    if (!jswrap_stepper_getPattern(stepper, pattern)) {
      jsExceptionHere(JSET_ERROR, "'pattern' isn't valid 4/8 element array");
    }
  }

  Pin pins[4];
  if (jswrap_stepper_getPins(stepper, pins)) {
    for (int i=0;i<4;i++)
      if (!jshGetPinStateIsManual(pins[i]))
        jshPinSetState(pins[i], JSHPINSTATE_GPIO_OUT);
  } else {
    jsExceptionHere(JSET_ERROR, "Not all pins are valid");
  }
  return stepper;
}

/*JSON{
  "type" : "method",
  "class" : "Stepper",
  "name" : "moveTo",
  "ifdef" : "ESPR_USE_STEPPER_TIMER",
  "generate" : "jswrap_stepper_moveTo",
  "params" : [
    ["direction","int","The amount of steps to move in either direction"],
    ["options","JsVar","Optional options struct"]
  ],
  "return" : ["JsVar","A Promise that resolves when the stepper has finished moving"],
  "return_object": "Promise"
}

Move a certain number of steps in either direction, `options` can be:

```
{
  freq : 100, // optional (frequency in Hz) step frequency
  turnOff : true, // optional (default false) turn off stepper after this movement?
}
```

 */
JsVar *jswrap_stepper_moveTo(JsVar *stepper, int direction, JsVar *options) {
  bool running = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(stepper, "running"));
  if (running) {
    jsExceptionHere(JSET_ERROR, "Stepper is already running");
    return 0;
  }
  if (direction==0) // resolve immediately if not moving
    return jswrap_promise_resolve(NULL);

  JsVarFloat freq = jsvGetFloatAndUnLock(jsvObjectGetChildIfExists(stepper, "freq"));;
  if (jsvIsObject(options)) {
    JsVarFloat t = jsvGetFloatAndUnLock(jsvObjectGetChildIfExists(options, "freq"));
    if (isfinite(t)) freq = t;
  } else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_ERROR, "Expecting options to be undefined or an Object, not %t", options);
  }
  if (freq<0.001) {
    jsExceptionHere(JSET_ERROR, "Frequency must be above 0.001Hz");
    return 0;
  }
  bool turnOff = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(options, "turnOff"));


  JsVar *promise = jspromise_create();
  jsvObjectSetChild(stepper, "promise", promise);

  UtilTimerTask task;
  task.time = 0;
  task.repeatInterval = jshGetTimeFromMilliseconds(1000.0 / freq);
  task.type = UET_STEP;
  jswrap_stepper_getPins(stepper, task.data.step.pins);
  task.data.step.steps = direction;
  task.data.step.pIndex = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(stepper, "pos"))&7;
  task.data.step.pattern[0] = 0b00010010;
  task.data.step.pattern[1] = 0b01001000;
  task.data.step.pattern[2] = 0b00010010;
  task.data.step.pattern[3] = 0b01001000;
  jswrap_stepper_getPattern(stepper, task.data.step.pattern);

  if (!utilTimerInsertTask(&task, NULL)) {
    jsExceptionHere(JSET_ERROR, "Failed to add timer task");
    jsvUnLock(promise);
    return 0;
  }
  // And finally set it up
  jsvObjectSetChildAndUnLock(stepper, "running", jsvNewFromBool(true));
  jsvObjectSetChildAndUnLock(stepper, "_direction", jsvNewFromInteger(direction));
  jsvObjectSetChildAndUnLock(stepper, "_turnOff", jsvNewFromBool(turnOff));
  // Add to our list of active steppers
  JsVar *steppers = jsvObjectGetChild(execInfo.hiddenRoot, JSI_STEPPER_NAME, JSV_ARRAY);
  if (steppers) {
    jsvArrayPush(steppers, stepper);
    jsvUnLock(steppers);
  }
  return promise;
}

/*JSON{
  "type" : "method",
  "class" : "Stepper",
  "name" : "stop",
  "ifdef" : "ESPR_USE_STEPPER_TIMER",
  "generate" : "jswrap_stepper_stop",
  "params" : [
    ["options","JsVar","Optional options struct"]
  ]
}
Stop a stepper motor that is currently running.

You can specify `.stop({turnOff:true})` to force the stepper motor to turn off.
 */
void jswrap_stepper_stop(JsVar *stepper, JsVar *options) {
  bool running = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(stepper, "running"));
  if (!running) {
    jsExceptionHere(JSET_ERROR, "Stepper is not running");
    return;
  }
  if (jsvIsObject(options)) {
    bool turnOff = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(options, "turnOff"));
    if (turnOff)
      jsvObjectSetChildAndUnLock(stepper, "_turnOff", jsvNewFromBool(turnOff));
    // the _idle handler will see _turnOff and will turn off the stepper
  }

  Pin pins[4];
  bool ok = false;
  if (jswrap_stepper_getPins(stepper, pins)) {
    UtilTimerTask task;
    if (jstGetLastPinTimerTask(pins[0], &task)) {
      // update step count to where the timer was when we stopped
      jsvObjectSetChildAndUnLock(stepper, "_direction", jsvNewFromInteger(
            jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(stepper, "_direction")) -
            task.data.step.steps));
    }
    ok = jstStopPinTimerTask(pins[0]);
  }
  if (!ok)
    jsExceptionHere(JSET_ERROR, "Stepper couldn't be stopped");
  // now run idle loop as this will issue the finish event and will clean up
  jswrap_stepper_idle();
}
#endif

