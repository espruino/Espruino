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
 * JavaScript methods and functions for the interactive shell
 * ----------------------------------------------------------------------------
 */
#include "jswrap_interactive.h"
#include "jswrap_json.h" // for print/console.log
#include "jsvar.h"
#include "jsinteractive.h"

/*JSON{
  "type" : "class",
  "class" : "console"
}
An Object that contains functions for writing to the interactive console
 */

/*JSON{
  "type" : "variable",
  "name" : "global",
  "generate_full" : "jsvLockAgain(execInfo.root)",
  "return" : ["JsVar","The global scope"]
}
A reference to the global scope, where everything is defined.
 */

/*JSON{
  "type" : "function",
  "name" : "setBusyIndicator",
  "generate" : "jswrap_interface_setBusyIndicator",
  "params" : [
    ["pin","JsVar",""]
  ]
}
When Espruino is busy, set the pin specified here high. Set this to undefined to disable the feature.
 */
void jswrap_interface_setBusyIndicator(JsVar *pinVar) {
  Pin oldPin = pinBusyIndicator;
  pinBusyIndicator = jshGetPinFromVar(pinVar);
  // we should be busy right now anyway, so set stuff up right
  if (pinBusyIndicator!=oldPin) {
    if (oldPin!=PIN_UNDEFINED) jshPinOutput(oldPin, 0);
    if (pinBusyIndicator!=PIN_UNDEFINED) jshPinOutput(pinBusyIndicator, 1);
  }
}

/*JSON{
  "type" : "function",
  "name" : "setSleepIndicator",
  "generate" : "jswrap_interface_setSleepIndicator",
  "params" : [
    ["pin","JsVar",""]
  ]
}
When Espruino is asleep, set the pin specified here low (when it's awake, set it high). Set this to undefined to disable the feature.

Please see http://www.espruino.com/Power+Consumption for more details on this.
 */
void jswrap_interface_setSleepIndicator(JsVar *pinVar) {
  Pin oldPin = pinSleepIndicator;
  pinSleepIndicator = jshGetPinFromVar(pinVar);
  // we should be awake right now anyway, so set stuff up right
  if (pinSleepIndicator!=oldPin) {
    if (oldPin!=PIN_UNDEFINED) jshPinOutput(oldPin, 0);
    if (pinSleepIndicator!=PIN_UNDEFINED) jshPinOutput(pinSleepIndicator, 1);
  }
}

/*JSON{
  "type" : "function",
  "name" : "setDeepSleep",
  "generate" : "jswrap_interface_setDeepSleep",
  "params" : [
    ["sleep","bool",""]
  ]
}
Set whether we can enter deep sleep mode, which reduces power consumption to around 100uA. This only works on the Espruino Board.

Please see http://www.espruino.com/Power+Consumption for more details on this.
 */
void jswrap_interface_setDeepSleep(bool sleep) {
  if (sleep)
    jsiStatus |= JSIS_ALLOW_DEEP_SLEEP;
  else
    jsiStatus &= ~JSIS_ALLOW_DEEP_SLEEP;
}


/*JSON{
  "type" : "function",
  "name" : "trace",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_interface_trace",
  "params" : [
    ["root","JsVar","The symbol to output (optional). If nothing is specified, everything will be output"]
  ]
}
Output debugging information
 */
void jswrap_interface_trace(JsVar *root) {
  if (jsvIsUndefined(root)) {
    jsvTrace(execInfo.root, 0);
  } else {
    jsvTrace(root, 0);
  }
}


/*JSON{
  "type" : "function",
  "name" : "dump",
  "ifndef" : "SAVE_ON_FLASH",
  "generate_full" : "jsiDumpState((vcbprintf_callback)jsiConsolePrintString, 0)"
}
Output current interpreter state in a text form such that it can be copied to a new device

Note: 'Internal' functions are currently not handled correctly. You will need to recreate these in the `onInit` function.
 */
/*JSON{
  "type" : "function",
  "name" : "load",
  "generate_full" : "jsiStatus|=JSIS_TODO_FLASH_LOAD;"
}
Restart and load the program out of flash - this has an effect similar to
completely rebooting Espruino (power off/power on), but without actually
performing a full reset of the hardware.

This command only executes when the Interpreter returns to the Idle state - for
instance ```a=1;load();a=2;``` will still leave 'a' as undefined (or what it was
set to in the saved program).

Espruino will resume from where it was when you last typed `save()`.
If you want code to be executed right after loading (for instance to initialise
devices connected to Espruino), add an `init` event handler to `E` with
`E.on('init', function() { ... your_code ... });`. This will then be automatically
executed by Espruino every time it starts.
 */
/*JSON{
  "type" : "function",
  "name" : "save",
  "generate_full" : "jsiStatus|=JSIS_TODO_FLASH_SAVE;"
}
Save program memory into flash. It will then be loaded automatically every time
Espruino powers on or is hard-reset.

This command only executes when the Interpreter returns to the Idle state - for
instance ```a=1;save();a=2;``` will save 'a' as 2.

When Espruino powers on, it will resume from where it was when you typed `save()`.
If you want code to be executed right after loading (for instance to initialise
devices connected to Espruino), add an `init` event handler to `E` with
`E.on('init', function() { ... your_code ... });`. This will then be automatically
executed by Espruino every time it starts.

In order to stop the program saved with this command being loaded automatically,
hold down Button 1 while also pressing reset. On some boards, Button 1 enters
bootloader mode, so you will need to press Reset with Button 1 raised, and then
hold Button 1 down a fraction of a second later.
 */
/*JSON{
  "type" : "function",
  "name" : "reset",
  "generate_full" : "jsiStatus|=JSIS_TODO_RESET;"
}
Reset the interpreter - clear program memory, and do not load a saved program from flash. This does NOT reset the underlying hardware (which allows you to reset the device without it disconnecting from USB).

This command only executes when the Interpreter returns to the Idle state - for instance ```a=1;reset();a=2;``` will still leave 'a' as undefined.

The safest way to do a full reset is to hit the reset button.
 */
/*JSON{
  "type" : "function",
  "name" : "print",
  "generate" : "jswrap_interface_print",
  "params" : [
    ["text","JsVarArray",""]
  ]
}
Print the supplied string(s) to the console

 **Note:** If you're connected to a computer (not a wall adaptor) via USB but **you are not running a terminal app** then when you print data Espruino may pause execution and wait until the computer requests the data it is trying to print.
 */
/*JSON{
  "type" : "staticmethod",
  "class" : "console",
  "name" : "log",
  "generate" : "jswrap_interface_print",
  "params" : [
    ["text","JsVarArray","One or more arguments to print"]
  ]
}
Print the supplied string(s) to the console

 **Note:** If you're connected to a computer (not a wall adaptor) via USB but **you are not running a terminal app** then when you print data Espruino may pause execution and wait until the computer requests the data it is trying to print.
 */
void jswrap_interface_print(JsVar *v) {
  assert(jsvIsArray(v));

  jsiConsoleRemoveInputLine();
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, v);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *v = jsvObjectIteratorGetValue(&it);
    if (jsvIsString(v))
      jsiConsolePrintStringVar(v);
    else
      jsfPrintJSON(v, JSON_PRETTY | JSON_SOME_NEWLINES | JSON_SHOW_OBJECT_NAMES);
    jsvUnLock(v);
    jsvObjectIteratorNext(&it);
    if (jsvObjectIteratorHasValue(&it))
      jsiConsolePrint(" ");
  }
  jsvObjectIteratorFree(&it);
  jsiConsolePrint("\n");
}

/*JSON{
  "type" : "function",
  "name" : "edit",
  "generate" : "jswrap_interface_edit",
  "params" : [
    ["funcName","JsVar","The name of the function to edit (either a string or just the unquoted name)"]
  ]
}
Fill the console with the contents of the given function, so you can edit it.

NOTE: This is a convenience function - it will not edit 'inner functions'. For that, you must edit the 'outer function' and re-execute it.
 */
void jswrap_interface_edit(JsVar *funcName) {
  JsVar *func = 0;
  if (jsvIsString(funcName)) {
    funcName = jsvLockAgain(funcName);
    func = jsvSkipNameAndUnLock(jsvFindChildFromVar(execInfo.root, funcName, 0));
  } else {
    func = funcName;
    funcName = jsvGetPathTo(execInfo.root, func, 2, 0);
  }

  if (jsvIsString(funcName)) {
    if (jsvIsFunction(func)) {
      JsVar *scopeVar = jsvFindChildFromString(func, JSPARSE_FUNCTION_SCOPE_NAME, false);
      JsVar *inRoot = jsvGetIndexOf(execInfo.root, func, true);
      bool normalDecl = scopeVar==0 && inRoot!=0;
      jsvUnLock2(inRoot, scopeVar);
      JsVar *newLine = jsvNewFromEmptyString();
      if (newLine) { // could be out of memory
        /* normalDecl:
         *
         * function foo() { ... }
         *
         * NOT normalDecl:
         *
         * foo.replaceWith(function() { ... });
         *
         */
        JsVar *funcData = jsvAsString(func, false);

        if (normalDecl) {
          jsvAppendString(newLine, "function ");
          jsvAppendStringVarComplete(newLine, funcName);
          jsvAppendStringVar(newLine, funcData, 9, JSVAPPENDSTRINGVAR_MAXLENGTH);

        } else {
          jsvAppendStringVarComplete(newLine, funcName);
          jsvAppendString(newLine, ".replaceWith(");
          jsvAppendStringVarComplete(newLine, funcData);
          jsvAppendString(newLine, ");");
        }
        jsvUnLock(funcData);
        jsiReplaceInputLine(newLine);
        jsvUnLock(newLine);
      }
    } else {
      jsExceptionHere(JSET_ERROR, "Edit should be called with the name of a function");
    }
  } else {
    jsExceptionHere(JSET_ERROR, "Edit should be called with edit(funcName) or edit('funcName')");
  }
  jsvUnLock2(func, funcName);
}


/*JSON{
  "type" : "function",
  "name" : "echo",
  "generate" : "jswrap_interface_echo",
  "params" : [
    ["echoOn","bool",""]
  ]
}
Should TinyJS echo what you type back to you? true = yes (Default), false = no. When echo is off, the result of executing a command is not returned. Instead, you must use 'print' to send output.
 */
void jswrap_interface_echo(bool echoOn) {
  if (echoOn)
    jsiStatus &= ~JSIS_ECHO_OFF;
  else
    jsiStatus |= JSIS_ECHO_OFF;
}

/*JSON{
  "type" : "function",
  "name" : "getTime",
  "generate_full" : "(JsVarFloat)jshGetSystemTime() / (JsVarFloat)jshGetTimeFromMilliseconds(1000)",
  "return" : ["float",""]
}
Return the current system time in Seconds (as a floating point number)
 */

/*JSON{
  "type" : "function",
  "name" : "setTime",
  "generate" : "jswrap_interactive_setTime",
  "params" : [
    ["time","float",""]
  ]
}
Set the current system time in seconds (to the nearest second)
 */
void jswrap_interactive_setTime(JsVarFloat time) {
  JsSysTime stime = jshGetTimeFromMilliseconds(time*1000);
  jsiLastIdleTime = stime;
  jshSetSystemTime(stime);
}


/*JSON{
  "type" : "function",
  "name" : "getSerial",
  "generate" : "jswrap_interface_getSerial",
  "return" : ["JsVar","The board's serial number"]
}
Get the serial number of this board
 */
JsVar *jswrap_interface_getSerial() {
  char buf[8];
  unsigned char serial[32];
  int i, serialSize = jshGetSerialNumber(serial, sizeof(serial));

  JsVar *str = jsvNewFromEmptyString();
  if (!str) return 0;

  for (i=0;i<serialSize;i++) {
    if ((i&3)==0 && i) jsvAppendString(str, "-");
    itostr(serial[i] | 0x100, buf, 16);
    jsvAppendString(str, &buf[1]);
  }
  return str;
}

/*JSON{
  "type" : "function",
  "name" : "setInterval",
  "generate" : "jswrap_interface_setInterval",
  "params" : [
    ["function","JsVar","A Function or String to be executed"],
    ["timeout","float","The time between calls to the function"],
    ["args","JsVarArray","Optional arguments to pass to the function when executed"]
  ],
  "return" : ["JsVar","An ID that can be passed to clearInterval"]
}
Call the function (or evaluate the string) specified REPEATEDLY after the timeout in milliseconds.

For instance:

```
setInterval(function () {
  console.log("Hello World");
}, 1000);
// or
setInterval('console.log("Hello World");', 1000);
// both print 'Hello World' every second
```

You can also specify extra arguments that will be sent to the function when it is executed. For example:

```
setInterval(function (a,b) {
  console.log(a+" "+b);
}, 1000, "Hello", "World");
// prints 'Hello World' every second
```

If you want to stop your function from being called, pass the number that
was returned by `setInterval` into the `clearInterval` function.

 **Note:** If `setDeepSleep(true)` has been called and the interval is greater than 5 seconds, Espruino may execute the interval up to 1 second late. This is because Espruino can only wake from deep sleep every second - and waking early would cause Espruino to waste power while it waited for the correct time.
 */
/*JSON{
  "type" : "function",
  "name" : "setTimeout",
  "generate" : "jswrap_interface_setTimeout",
  "params" : [
    ["function","JsVar","A Function or String to be executed"],
    ["timeout","float","The time until the function will be executed"],
    ["args","JsVarArray","Optional arguments to pass to the function when executed"]
  ],
  "return" : ["JsVar","An ID that can be passed to clearTimeout"]
}
Call the function (or evaluate the string) specified ONCE after the timeout in milliseconds.

For instance:

```
setTimeout(function () {
  console.log("Hello World");
}, 1000);
// or
setTimeout('console.log("Hello World");', 1000);
// both print 'Hello World' after a second
```

You can also specify extra arguments that will be sent to the function when it is executed. For example:

```
setTimeout(function (a,b) {
  console.log(a+" "+b);
}, 1000, "Hello", "World");
// prints 'Hello World' after 1 second
```

If you want to stop the function from being called, pass the number that
was returned by `setTimeout` into the `clearInterval` function.

 **Note:** If `setDeepSleep(true)` has been called and the interval is greater than 5 seconds, Espruino may execute the interval up to 1 second late. This is because Espruino can only wake from deep sleep every second - and waking early would cause Espruino to waste power while it waited for the correct time.
 */
JsVar *_jswrap_interface_setTimeoutOrInterval(JsVar *func, JsVarFloat interval, JsVar *args, bool isTimeout) {
  // NOTE: The 5 sec delay mentioned in the description is handled by jshSleep
  JsVar *itemIndex = 0;
  if (!jsvIsFunction(func) && !jsvIsString(func)) {
    jsExceptionHere(JSET_ERROR, "Function or String not supplied!");
  } else {
    // Create a new timer
    JsVar *timerPtr = jsvNewObject();
    if (interval<TIMER_MIN_INTERVAL) interval=TIMER_MIN_INTERVAL;
    JsSysTime intervalInt = jshGetTimeFromMilliseconds(interval);
    jsvObjectSetChildAndUnLock(timerPtr, "time", jsvNewFromLongInteger((jshGetSystemTime() - jsiLastIdleTime) + intervalInt));
    if (!isTimeout) {
      jsvObjectSetChildAndUnLock(timerPtr, "interval", jsvNewFromLongInteger(intervalInt));
    }
    jsvObjectSetChild(timerPtr, "callback", func); // intentionally no unlock
    if (jsvGetArrayLength(args))
      jsvObjectSetChild(timerPtr, "args", args); // intentionally no unlock

    // Add to array
    itemIndex = jsvNewFromInteger(jsiTimerAdd(timerPtr));
    jsvUnLock(timerPtr);
    jsiTimersChanged(); // mark timers as changed
  }
  return itemIndex;
}
JsVar *jswrap_interface_setInterval(JsVar *func, JsVarFloat timeout, JsVar *args) {
  return _jswrap_interface_setTimeoutOrInterval(func, timeout, args, false);
}
JsVar *jswrap_interface_setTimeout(JsVar *func, JsVarFloat timeout, JsVar *args) {
  return _jswrap_interface_setTimeoutOrInterval(func, timeout, args, true);
}

/*JSON{
  "type" : "function",
  "name" : "clearInterval",
  "generate" : "jswrap_interface_clearInterval",
  "params" : [
    ["id","JsVar","The id returned by a previous call to setInterval"]
  ]
}
Clear the Interval that was created with setInterval, for example:

```var id = setInterval(function () { print('foo'); }, 1000);```

```clearInterval(id);```

If no argument is supplied, all timers and intervals are stopped
 */
/*JSON{
  "type" : "function",
  "name" : "clearTimeout",
  "generate" : "jswrap_interface_clearTimeout",
  "params" : [
    ["id","JsVar","The id returned by a previous call to setTimeout"]
  ]
}
Clear the Timeout that was created with setTimeout, for example:

```var id = setTimeout(function () { print('foo'); }, 1000);```

```clearTimeout(id);```

If no argument is supplied, all timers and intervals are stopped
 */
void _jswrap_interface_clearTimeoutOrInterval(JsVar *idVar, bool isTimeout) {
  JsVar *timerArrayPtr = jsvLock(timerArray);
  if (jsvIsUndefined(idVar)) {
    /* Delete all timers EXCEPT those with a 'watch' field,
     as those were generated by jsinteractive.c for debouncing watches */
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, timerArrayPtr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *timerPtr = jsvObjectIteratorGetValue(&it);
      JsVar *watchPtr = jsvObjectGetChild(timerPtr, "watch", 0);
      if (!watchPtr)
        jsvObjectIteratorRemoveAndGotoNext(&it, timerArrayPtr);
      else
        jsvObjectIteratorNext(&it); 
      jsvUnLock2(watchPtr, timerPtr);
    }
    jsvObjectIteratorFree(&it);
  } else {
    JsVar *child = jsvIsBasic(idVar) ? jsvFindChildFromVar(timerArrayPtr, idVar, false) : 0;
    if (child) {
      JsVar *timerArrayPtr = jsvLock(timerArray);
      jsvRemoveChild(timerArrayPtr, child);
      jsvUnLock2(child, timerArrayPtr);
    } else {
      if (isTimeout)
        jsExceptionHere(JSET_ERROR, "Unknown Timeout");
      else
        jsExceptionHere(JSET_ERROR, "Unknown Interval");
    }
  }
  jsvUnLock(timerArrayPtr);
  jsiTimersChanged(); // mark timers as changed
}
void jswrap_interface_clearInterval(JsVar *idVar) {
  _jswrap_interface_clearTimeoutOrInterval(idVar, false);
}
void jswrap_interface_clearTimeout(JsVar *idVar) {
  _jswrap_interface_clearTimeoutOrInterval(idVar, true);
}

/*JSON{
  "type" : "function",
  "name" : "changeInterval",
  "generate" : "jswrap_interface_changeInterval",
  "params" : [
    ["id","JsVar","The id returned by a previous call to setInterval"],
    ["time","float","The new time period in ms"]
  ]
}
Change the Interval on a callback created with setInterval, for example:

```var id = setInterval(function () { print('foo'); }, 1000); // every second```

```changeInterval(id, 1500); // now runs every 1.5 seconds```

This takes effect immediately and resets the timeout, so in the example above,
regardless of when you call `changeInterval`, the next interval will occur 1500ms
after it.
 */
void jswrap_interface_changeInterval(JsVar *idVar, JsVarFloat interval) {
  JsVar *timerArrayPtr = jsvLock(timerArray);
  if (interval<TIMER_MIN_INTERVAL) interval=TIMER_MIN_INTERVAL;
  JsVar *timerName = jsvIsBasic(idVar) ? jsvFindChildFromVar(timerArrayPtr, idVar, false) : 0;
  if (timerName) {
    JsVar *timer = jsvSkipNameAndUnLock(timerName);
    JsVar *v;
    JsVarInt intervalInt = (JsVarInt)jshGetTimeFromMilliseconds(interval);
    v = jsvNewFromInteger(intervalInt);
    jsvUnLock2(jsvSetNamedChild(timer, v, "interval"), v);
    v = jsvNewFromInteger((JsVarInt)(jshGetSystemTime()-jsiLastIdleTime) + intervalInt);
    jsvUnLock3(jsvSetNamedChild(timer, v, "time"), v, timer);
    // timerName already unlocked
    jsiTimersChanged(); // mark timers as changed
  } else {
    jsExceptionHere(JSET_ERROR, "Unknown Interval");
  }
  jsvUnLock(timerArrayPtr);
}
