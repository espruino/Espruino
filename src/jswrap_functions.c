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
 * JavaScript methods and functions in the global namespace
 * ----------------------------------------------------------------------------
 */
#include "jswrap_functions.h"
#include "jslex.h"
#include "jsparse.h"
#include "jsinteractive.h"

/*JSON{ "type":"function", "name" : "eval",
         "description" : "Evaluate a string containing JavaScript code",
         "generate" : "jswrap_eval",
         "params" : [ [ "code", "JsVar", ""] ],
         "return" : ["JsVar", "The result of evaluating the string"]
}*/
JsVar *jswrap_eval(JsVar *v) {
  if (!v) return 0;
  JsVar *s = jsvAsString(v, false); // get as a string
  JsVar *result = jspEvaluateVar(jsiGetParser(), s, 0);
  jsvUnLock(s);
  return result;
}

/*JSON{ "type":"function", "name" : "parseInt",
         "description" : "Convert a string representing a number into an integer",
         "generate" : "jswrap_parseInt",
         "params" :  [ [ "string", "JsVar", ""],
                       [ "radix", "int", "The Radix of the string (optional)"] ],
         "return" : ["int", "The value of the string"]
}*/
JsVarInt jswrap_parseInt(JsVar *v, JsVarInt radix) {
  char buffer[JS_NUMBER_BUFFER_SIZE];
  jsvGetString(v, buffer, JS_NUMBER_BUFFER_SIZE);
  return stringToIntWithRadix(buffer, (int)radix);
}

/*JSON{ "type":"function", "name" : "parseFloat",
         "description" : "Convert a string representing a number into an float",
         "generate" : "jswrap_parseFloat",
         "params" :  [ [ "string", "JsVar", ""] ],
         "return" : ["float", "The value of the string"]
}*/
JsVarFloat jswrap_parseFloat(JsVar *v) {
  char buffer[JS_NUMBER_BUFFER_SIZE];
  jsvGetString(v, buffer, JS_NUMBER_BUFFER_SIZE);
  return stringToFloat(buffer);
}
