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
 * JavaScript 'process' object - for information about the Espruino board
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"
#include "jsparse.h"
#include "jswrap_process.h"
#include "jswrap_interactive.h"
#include "jswrapper.h"
#include "jsinteractive.h"

/*JSON{
  "type" : "class",
  "class" : "process"
}
This class contains information about Espruino itself
 */

/*JSON{
  "type" : "event",
  "class" : "process",
  "name" : "uncaughtException"
}
This event is called when an exception gets thrown and isn't caught (eg. it gets all the way back to the event loop).

You can use this for logging potential problems that might occur during execution.

**Note:** When this is used, exceptions will cease to be reported on the console - which
may make debugging difficult!
*/

/*JSON{
  "type" : "staticproperty",
  "class" : "process",
  "name" : "version",
  "generate_full" : "jsvNewFromString(JS_VERSION)",
  "return" : ["JsVar","The version of Espruino"]
}
Returns the version of Espruino as a String
 */

#ifndef SAVE_ON_FLASH
/* NOTE: The order of these is very important, as 
the online compiler has its own copy of this table */
const void *exportPtrs[] = {
    jsvLockAgainSafe,
    jsvUnLock,
    jsvSkipName,
    jsvMathsOp,
    jsvNewWithFlags,
    jsvNewFromFloat,
    jsvNewFromInteger,
    jsvNewFromString,
    jsvNewFromBool,
    jsvGetFloat,
    jsvGetInteger,
    jsvGetBool,
    jspReplaceWith,
    jspeFunctionCall,
    jspGetNamedVariable,
    jspGetNamedField,
    jspGetVarNamedField,
    0
};
const char *exportNames = 
    "jsvLockAgainSafe\0jsvUnLock\0jsvSkipName\0jsvMathsOp\0"
    "jsvNewWithFlags\0jsvNewFromFloat\0jsvNewFromInteger\0jsvNewFromString\0jsvNewFromBool\0"
    "jsvGetFloat\0jsvGetInteger\0jsvGetBool\0"
    "jspReplaceWith\0jspeFunctionCall\0"
    "jspGetNamedVariable\0jspGetNamedField\0jspGetVarNamedField\0"
    "\0\0";
#endif

/*JSON{
  "type" : "staticproperty",
  "class" : "process",
  "name" : "env",
  "generate" : "jswrap_process_env",
  "return" : ["JsVar","An object"]
}
Returns an Object containing various pre-defined variables. standard ones are BOARD, VERSION
 */
JsVar *jswrap_process_env() {
  JsVar *obj = jsvNewObject();
  jsvObjectSetChildAndUnLock(obj, "VERSION", jsvNewFromString(JS_VERSION));
#ifdef GIT_COMMIT
  jsvObjectSetChildAndUnLock(obj, "GIT_COMMIT", jsvNewFromString(STRINGIFY(GIT_COMMIT)));
#endif
  jsvObjectSetChildAndUnLock(obj, "BOARD", jsvNewFromString(PC_BOARD_ID));
  jsvObjectSetChildAndUnLock(obj, "FLASH", jsvNewFromInteger(FLASH_TOTAL));
  jsvObjectSetChildAndUnLock(obj, "RAM", jsvNewFromInteger(RAM_TOTAL));
  jsvObjectSetChildAndUnLock(obj, "SERIAL", jswrap_interface_getSerial());
  jsvObjectSetChildAndUnLock(obj, "CONSOLE", jsvNewFromString(jshGetDeviceString(jsiGetConsoleDevice())));
  jsvObjectSetChildAndUnLock(obj, "MODULES", jsvNewFromString(jswGetBuiltInLibraryNames()));
#if !defined(SAVE_ON_FLASH) && !defined(BLUETOOTH)
  // It takes too long to send this information over BLE...
  JsVar *arr = jsvNewObject();
  if (arr) {
    const char *s = exportNames;
    void **p = (void**)exportPtrs;
    while (*s) {
      jsvObjectSetChildAndUnLock(arr, s, jsvNewFromInteger((JsVarInt)(size_t)*p));
      p++;
      while (*s) s++; // skip until 0
      s++; // skip over 0
    }
    jsvObjectSetChildAndUnLock(obj, "EXPORTS", arr);
  }  
#endif
#ifndef SAVE_ON_FLASH
  // Pointer to a list of predefined exports - eventually we'll get rid of the array above
  jsvObjectSetChildAndUnLock(obj, "EXPTR", jsvNewFromInteger((JsVarInt)(size_t)exportPtrs));
#endif
  return obj;
}


/*JSON{
  "type" : "staticmethod",
  "class" : "process",
  "name" : "memory",
  "generate" : "jswrap_process_memory",
  "return" : ["JsVar","Information about memory usage"]
}
Run a Garbage Collection pass, and return an object containing information on memory usage.

* `free`  : Memory that is available to be used (in blocks)
* `usage` : Memory that has been used (in blocks)
* `total` : Total memory (in blocks)
* `history` : Memory used for command history - that is freed if memory is low. Note that this is INCLUDED in the figure for 'free'
* `gc`      : Memory freed during the GC pass
* `gctime`  : Time taken for GC pass (in milliseconds)
* `stackEndAddress` : (on ARM) the address (that can be used with peek/poke/etc) of the END of the stack. The stack grows down, so unless you do a lot of recursion the bytes above this can be used.
* `flash_start`      : (on ARM) the address of the start of flash memory (usually `0x8000000`)
* `flash_binary_end` : (on ARM) the address in flash memory of the end of Espruino's firmware.
* `flash_code_start` : (on ARM) the address in flash memory of pages that store any code that you save with `save()`.
* `flash_length` : (on ARM) the amount of flash memory this firmware was built for (in bytes). **Note:** Some STM32 chips actually have more memory than is advertised.

Memory units are specified in 'blocks', which are around 16 bytes each (depending on your device). See http://www.espruino.com/Performance for more information.

**Note:** To find free areas of flash memory, see `require('Flash').getFree()`
 */
JsVar *jswrap_process_memory() {
  JsSysTime time1 = jshGetSystemTime();
  int gc = jsvGarbageCollect();
  JsSysTime time2 = jshGetSystemTime();
  JsVar *obj = jsvNewObject();
  if (obj) {
    unsigned int history = 0;
    JsVar *historyVar = jsvObjectGetChild(execInfo.hiddenRoot, JSI_HISTORY_NAME, 0);
    if (historyVar) {
      history = (unsigned int)jsvCountJsVarsUsed(historyVar); // vars used to store history
      jsvUnLock(historyVar);
    }
    unsigned int usage = jsvGetMemoryUsage() - history;
    unsigned int total = jsvGetMemoryTotal();
    jsvObjectSetChildAndUnLock(obj, "free", jsvNewFromInteger((JsVarInt)(total-usage)));
    jsvObjectSetChildAndUnLock(obj, "usage", jsvNewFromInteger((JsVarInt)usage));
    jsvObjectSetChildAndUnLock(obj, "total", jsvNewFromInteger((JsVarInt)total));
    jsvObjectSetChildAndUnLock(obj, "history", jsvNewFromInteger((JsVarInt)history));
    jsvObjectSetChildAndUnLock(obj, "gc", jsvNewFromInteger((JsVarInt)gc));
    jsvObjectSetChildAndUnLock(obj, "gctime", jsvNewFromFloat(jshGetMillisecondsFromTime(time2-time1)));

#ifdef ARM
    extern int LINKER_END_VAR; // end of ram used (variables) - should be 'void', but 'int' avoids warnings
    extern int LINKER_ETEXT_VAR; // end of flash text (binary) section - should be 'void', but 'int' avoids warnings
    jsvObjectSetChildAndUnLock(obj, "stackEndAddress", jsvNewFromInteger((JsVarInt)(unsigned int)&LINKER_END_VAR));
    jsvObjectSetChildAndUnLock(obj, "flash_start", jsvNewFromInteger((JsVarInt)FLASH_START));
    jsvObjectSetChildAndUnLock(obj, "flash_binary_end", jsvNewFromInteger((JsVarInt)(unsigned int)&LINKER_ETEXT_VAR));
    jsvObjectSetChildAndUnLock(obj, "flash_code_start", jsvNewFromInteger((JsVarInt)FLASH_SAVED_CODE_START));
    jsvObjectSetChildAndUnLock(obj, "flash_length", jsvNewFromInteger((JsVarInt)FLASH_TOTAL));
#endif
  }
  return obj;
}
