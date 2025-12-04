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
 * JavaScript methods for accessing the built-in timer
 * ----------------------------------------------------------------------------
 */
#include "jswrap_timer.h"
#include "jsvar.h"
#include "jstimer.h"
#include "jsinteractive.h"

/*JSON{
  "type" : "library",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "timer"
}
(2v29+ only) This class allows Espruino to control stepper motors.

```
require("timer").list()
// [ { id: 0, type: 'SET', pins: [ D2, D3, D4, D5 ], value: 0, time: 10 }, ... ]
```

This replaces `E.dumpTimers()` and `Pin.writeAtTime`
*/
volatile bool runningInterruptingJS = false;

static void jswrap_timer_queue_interrupt_js(JsSysTime time, void* userdata) {
  uint8_t timerIdx = (uint8_t)(size_t)userdata;
  // if we were already running interrupting JS, don't interrupt it to run more!
  if (!runningInterruptingJS) {
    jshPushIOCharEvents(EV_RUN_INTERRUPT_JS, (char*)&timerIdx, 1);
    execInfo.execute |= EXEC_RUN_INTERRUPT_JS;
    jshHadEvent();
  }
}

/** This is called if a EV_RUN_INTERRUPT_JS is received, or when a EXEC_RUN_INTERRUPT_JS is set.
It executes JavaScript code that was pushed to the queue by a require("timer").add({type:"EXEC", fn:myFunction... */
void jstOnRunInterruptJSEvent(const uint8_t *eventData, unsigned int eventLen) {
  runningInterruptingJS = true;
  execInfo.execute &= ~EXEC_RUN_INTERRUPT_JS;
  for (unsigned int i=0;i<eventLen;i++) {
    uint8_t timerIdx = eventData[i];
    JsVar *timerFns = jsvObjectGetChildIfExists(execInfo.hiddenRoot, JSI_TIMER_RUN_JS_NAME);
    if (timerFns) {
      JsVar *fn = jsvGetArrayItem(timerFns, timerIdx);
      if (jsvIsFunction(fn)) {
        jsvUnLock(jspExecuteFunction(fn, execInfo.root, 0, NULL));
        jsiCheckErrors(false); // check for any errors and report them
      }
      jsvUnLock2(timerFns, fn);
    }
  }
  runningInterruptingJS = false;
}

/** This is called from the parser if EXEC_RUN_INTERRUPT_JS is set.
It executes JavaScript code that was pushed to the queue by require("timer").add({type:"EXEC", fn:myFunction... */
void jstRunInterruptingJS() {
  uint8_t data[IOEVENT_MAX_LEN];
  unsigned int len = 0;
  if (jshPopIOEventOfType(EV_RUN_INTERRUPT_JS, data, &len))
    jstOnRunInterruptJSEvent(data, len);
  /*if (jshIsTopEvent(EV_RUN_INTERRUPT_JS)) {
    jshPopIOEvent(data, &len);
    jstOnRunInterruptJSEvent(data, len);
  } else/
    execInfo.execute &= ~EXEC_RUN_INTERRUPT_JS; // stop until idle
  */
}


/*JSON{
  "type" : "staticmethod",
  "class" : "timer",
  "name" : "list",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_timer_list",
  "return" : ["JsVar","Return a list of objects representing active timers"],
  "return_object" : "Array"
}
See `require("timer").get` for details of the fields in each timer.
*/
JsVar *jswrap_timer_list() {
  JsVar *arr = jsvNewEmptyArray();
  for (int idx=0;idx<UTILTIMERTASK_TASKS;idx++) {
    JsVar *obj = jswrap_timer_get(idx);
    if (obj) {
      jsvSetArrayItem(arr, idx, obj);
      jsvUnLock(obj);
    }
  }
  return arr;
}


/*JSON{
  "type" : "staticmethod",
  "class" : "timer",
  "name" : "get",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_timer_get",
  "params" : [
    ["timerID","int","The ID of the timer to get"]
  ],
  "return" : ["JsVar","An object describing the added timer, with an `id` field added."]
}
Returns:

```
{
  id : int, // timer ID (corresponds to array index)
  type : string, // type of timer (eg 'SET/EXEC/STEP/WR8/WR16/RD8/RD16')
  time : float, // time (in milliseconds) when the timer will next fire
  interval : float, // (optional) if the timer repeats, the interval (in milliseconds
  // the following fields are only present on devices with enough flash memory)
  pins : [ Pin, ... ], // (for SET/STEP) the pins used
  value : int, // (for SET) the value being set
  ptr : int, // (for EXEC) pointer to the function being executed
  userdata : int, // (for EXEC) userdata pointer
  buffer : JsVar, // (for WR8/WR16/RD8/RD16) the buffer being used
  buffer2 : JsVar // (for WR8/WR16/RD8/RD16) the second buffer being used (if any)
}
```

**Note:** `time` is set when the timer was last serviced, so if you set a 1 second timer
and then look at it after 500ms, it will still show as 1000ms (unless another timer
as been serviced before).
*/
JsVar *jswrap_timer_get(int id) {
  if (id<0 || id>=UTILTIMERTASK_TASKS) {
    return 0;
  }
  jshInterruptOff();
  UtilTimerTask task = utilTimerTaskInfo[id];
  jshInterruptOn();
  if (task.type == UET_NONE) return 0;
  JsVar *obj = jsvNewObject();
  jsvObjectSetChildAndUnLock(obj, "id", jsvNewFromInteger(id));
  const char *typeStr = NULL;
  switch (task.type) {
  case UET_NONE: assert(0); break;
  case UET_WAKEUP : typeStr="WKUP"; break;
  case UET_SET :
    typeStr="SET";
#ifndef SAVE_ON_FLASH
    jsvObjectSetChildAndUnLock(obj, "value", jsvNewFromInteger(task.data.set.value));
#endif
    break;
#ifndef SAVE_ON_FLASH
  case UET_WRITE_BYTE : typeStr="WR8"; break;
  case UET_READ_BYTE : typeStr="RD8"; break;
  case UET_WRITE_SHORT : typeStr="WR16"; break;
  case UET_READ_SHORT : typeStr="RD16"; break;
#endif
#ifdef ESPR_USE_STEPPER_TIMER
  case UET_STEP : typeStr="STEP"; break;
#endif
  case UET_EXECUTE :
    typeStr="EXEC";
#ifndef SAVE_ON_FLASH
    if (task.data.execute.fn == jswrap_timer_queue_interrupt_js) {
      int timerIdx = (int)(size_t)task.data.execute.userdata;
      JsVar *timerFns = jsvObjectGetChildIfExists(execInfo.hiddenRoot, JSI_TIMER_RUN_JS_NAME);
      if (timerFns) {
        jsvObjectSetChildAndUnLock(obj, "fn", jsvGetArrayItem(timerFns, timerIdx));
        jsvUnLock(timerFns);
      }
    } else {
      jsvObjectSetChildAndUnLock(obj, "ptr", jsvNewFromInteger((size_t)task.data.execute.fn));
      jsvObjectSetChildAndUnLock(obj, "userdata", jsvNewFromInteger((size_t)task.data.execute.userdata));
    }
#endif
    break;
  default: break; // could be other things if ORDed with UET_FINISHED - ignore them (fixed warning)
  }
#ifndef SAVE_ON_FLASH
  if (UET_EVENT_HAS_PINS(task.type)) {
    JsVar *pinsArr = jsvNewEmptyArray();
    int pinCount = UET_PIN_COUNT(task.type);
    for (int i=0;i<pinCount;i++)
      if (task.data.set.pins[i] != PIN_UNDEFINED)
        jsvArrayPushAndUnLock(pinsArr, jsvNewFromPin(task.data.set.pins[i]));
    jsvObjectSetChildAndUnLock(obj, "pins", pinsArr);
  }
  if (UET_IS_BUFFER_EVENT(task.type)) {
    jsvObjectSetChildAndUnLock(obj, "buffer", jsvLock(task.data.buffer.currentBuffer));
    if (task.data.buffer.nextBuffer)
      jsvObjectSetChildAndUnLock(obj, "buffer2", jsvLock(task.data.buffer.nextBuffer));
  }
#endif
#ifdef ESPR_USE_STEPPER_TIMER
  if (task.type == UET_STEP) {
    jsvObjectSetChildAndUnLock(obj, "steps", jsvNewFromInteger(task.data.step.steps));
    jsvObjectSetChildAndUnLock(obj, "stepIdx", jsvNewFromInteger(task.data.step.pIndex));
  }
#endif
  jsvObjectSetChildAndUnLock(obj, "type", typeStr ? jsvNewFromString(typeStr) : jsvNewFromInteger(task.type));
  jsvObjectSetChildAndUnLock(obj, "time", jsvNewFromFloat(jshGetMillisecondsFromTime(task.time)));
  if (task.repeatInterval)
    jsvObjectSetChildAndUnLock(obj, "interval", jsvNewFromFloat(jshGetMillisecondsFromTime(task.repeatInterval)));
  return obj;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "timer",
  "name" : "add",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_timer_add",
  "params" : [
    ["timer","JsVar","An object describing the timer to add. See below."]
  ],
  "return" : ["int","Return the ID of the added timer"]
}

To set one or more pins at a specific point in the future:

```
require("timer").add({
  type : "SET",
  pin : Pin, // required
  pin2 : Pin, // optional
  pin3 : Pin, // optional
  pin4 : Pin, // optional
  value : int, // required
  time : float, // time (in milliseconds) when the timer will first fire
  interval : float // (optional) if the timer repeats, the interval (in milliseconds)
})
// eg. set LED2 in 1 second (note: LED2 should already be an output)
require("timer").add({
  type: "SET",
  pin: LED2, value: 1,
  time: 1000
});
```

To execute some code at a specific point in the future:

```
require("timer").add({
  type : "EXEC",
  ptr : int, userdata : int, // required - pointer to native function(time:uint64, userdate:int) to call, and uint32 userdata to pass to function
  fn : JsVar, // alternative to ptr/userdata - a JS function to call (note: this function must be referenced elsewhere)
  time : float, // time (in milliseconds) when the timer will first fire
  interval : float // (optional) if the timer repeats, the interval (in milliseconds)
})
// eg. execute myFunction in 100ms, then 200ms thereafter
require("timer").add({
  type:"EXEC", fn: () => LED.toggle(),
  time:100,
  interval:200,
});
```

**Note:** `require("timer").add({type:"EXEC",fn:...})` differs from `setInterval`/`setTimeout` in that
it is scheduled using a hardware timer. When the timer fires, JavaScript that's executing will be
paused at the next statement and the JS will be executed right away. This can be great for things
like scanning out screens where you don't want your execution to be paused even if you're executing
JavaScript code.
*/
int jswrap_timer_add(JsVar *timer) {
  JsVarFloat time=0, interval=0;
  JsVar *type = 0, *fn = 0;
  int value=0,ptr=0,userdata=0;
  Pin pins[UTILTIMERTASK_PIN_COUNT] = {PIN_UNDEFINED, PIN_UNDEFINED, PIN_UNDEFINED, PIN_UNDEFINED};
  jsvConfigObject configs[] = {
      {"type", JSV_STRING_0, &type},
      {"time", JSV_FLOAT, &time},
      {"interval", JSV_FLOAT, &interval},
      {"value", JSV_INTEGER, &value},
      {"fn", JSV_OBJECT, &fn},
      {"ptr", JSV_INTEGER, &ptr},
      {"userdata", JSV_INTEGER, &userdata},
      {"pin", JSV_PIN, &pins[0]},
      {"pin2", JSV_PIN, &pins[1]},
      {"pin3", JSV_PIN, &pins[2]},
      {"pin4", JSV_PIN, &pins[3]}
  };
  if (!jsvReadConfigObject(timer, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    return -1;
  }
  UtilTimerEventType evtType = UET_NONE;
  if (jsvIsStringIEqual(type, "SET")) {
    if (pins[0] != PIN_UNDEFINED) evtType = UET_SET;
    else jsExceptionHere(JSET_ERROR, "`pin` required for SET timer");
  } else if (jsvIsStringIEqual(type, "EXEC")) {
    if (ptr || jsvIsFunction(fn)) evtType = UET_EXECUTE;
    else jsExceptionHere(JSET_ERROR, "`ptr` or `fn` required for EXEC timer");
  } else jsExceptionHere(JSET_ERROR, "Unsupported timer type %q", type);
  jsvUnLock(type);
  if (evtType == UET_NONE) {
    jsvUnLock(fn);
    return -1;
  }

  int idx = utilTimerGetUnusedIndex(true/*wait*/);
  if (idx<0) return -1; // no free tasks!
  UtilTimerTask *task = &utilTimerTaskInfo[idx];
  task->time = (int)jshGetTimeFromMilliseconds(time);
  task->repeatInterval = (unsigned int)jshGetTimeFromMilliseconds(interval);
  if (evtType == UET_SET) {
    for (int i=0;i<UTILTIMERTASK_PIN_COUNT;i++)
      task->data.set.pins[i] = pins[i];
    task->data.set.value = value;
  } else if (evtType == UET_EXECUTE) {
    if (jsvIsFunction(fn)) { // if a function is passed we use EXEC_RUN_INTERRUPT_JS
      JsVar *timerFns = jsvObjectGetChild(execInfo.hiddenRoot, JSI_TIMER_RUN_JS_NAME, JSV_ARRAY); // set the timer function in the hidden scope so we can look it up later
      if (timerFns) jsvSetArrayItem(timerFns, idx, fn);
      jsvUnLock(timerFns);
      ptr = (size_t)jswrap_timer_queue_interrupt_js;
      userdata = idx;
    }
    task->data.execute.fn = (UtilTimerTaskExecFn)(size_t)ptr;
    task->data.execute.userdata = (void*)(size_t)userdata;
  }
  task->type = evtType; // set type here, as we may have returned with error earlier
  jsvUnLock(fn);
  utilTimerInsertTask(idx, NULL, false/*doesn't have to be first*/);
  return idx;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "timer",
  "name" : "remove",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_timer_remove",
  "params" : [
    ["timerID","int","The ID of the timer to remove"]
  ],
  "return" : ["bool","`true` on success or `false` if there was no timer with that ID"]
}
*/
bool jswrap_timer_remove(int id) {
  return utilTimerRemoveTask(id);
}
