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
#include "jswrap_process.h"

/*JSON{ "type":"class",
        "class" : "process",
        "description" : "This class contains information about Espruino itself"
}*/

/*JSON{ "type":"staticproperty",
         "class" : "process", "name" : "version",
         "description" : "Returns the version of Espruino as a String",
         "generate_full" : "jsvNewFromString(JS_VERSION)",
         "return" : ["JsVar", "The version of Espruino"]
}*/

/*JSON{ "type":"staticproperty",
         "class" : "process", "name" : "env",
         "description" : "Returns an Object containing various pre-defined variables. standard ones are BOARD, VERSION",
         "generate" : "jswrap_process_env",
         "return" : ["JsVar", "An object"]
}*/
JsVar *jswrap_process_env() {
  JsVar *obj = jsvNewWithFlags(JSV_OBJECT);
  jsvUnLock(jsvObjectSetChild(obj, "VERSION", jsvNewFromString(JS_VERSION)));
  jsvUnLock(jsvObjectSetChild(obj, "BUILD_DATE", jsvNewFromString(__DATE__)));
  jsvUnLock(jsvObjectSetChild(obj, "BUILD_TIME", jsvNewFromString(__TIME__)));
  jsvUnLock(jsvObjectSetChild(obj, "BOARD", jsvNewFromString(PC_BOARD_ID)));
  jsvUnLock(jsvObjectSetChild(obj, "CHIP", jsvNewFromString(PC_BOARD_CHIP)));
  jsvUnLock(jsvObjectSetChild(obj, "CHIP_FAMILY", jsvNewFromString(PC_BOARD_CHIP_FAMILY)));
  jsvUnLock(jsvObjectSetChild(obj, "FLASH", jsvNewFromInteger(FLASH_TOTAL)));
  jsvUnLock(jsvObjectSetChild(obj, "RAM", jsvNewFromInteger(RAM_TOTAL)));

  return obj;
}
