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
 * JavaScript methods for Errors
 * ----------------------------------------------------------------------------
 */
#include "jswrap_error.h"
#include "jsparse.h"

/*JSON{ "type":"class",
        "class" : "Error",
        "description" : [ "The base class for runtime errors" ]
}*/

/*JSON{ "type":"constructor",
        "class" : "Error",
        "name" : "Error",
        "generate" : "jswrap_error_constructor",
        "description" : [ "Creates an Error object" ],
        "params" : [ [ "message", "JsVar", "An optional message string"] ],
        "return" : ["JsVar", "An Error object"]
}*/
JsVar *jswrap_error_constructor(JsVar *msg) {
  JsVar *d = jspNewObject(0,"Error");
  if (!d) return 0;

  if (msg) {
    msg = jsvAsString(msg, false);
    jsvUnLock(jsvObjectSetChild(d, "msg", msg));
  }

  return d;
}

/*JSON{ "type":"method",
        "class" : "Error", "name" : "toString",
        "generate" : "jswrap_error_toString",
        "return" : ["JsVar", "A String"]
}*/
JsVar *jswrap_error_toString(JsVar *parent) {
  JsVar *str = jsvNewFromString("Error");

  JsVar *msg = jsvObjectGetChild(parent, "msg", 0);
  if (msg) {
    jsvAppendPrintf(str, ": %v", msg);
    jsvUnLock(msg);
  }

  return str;
}
