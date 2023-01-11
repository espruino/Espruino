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

/*JSON{
  "type" : "class",
  "class" : "Error"
}
The base class for runtime errors
 */
/*JSON{
  "type" : "class",
  "class" : "SyntaxError"
}
The base class for syntax errors
 */
/*JSON{
  "type" : "class",
  "class" : "TypeError"
}
The base class for type errors
 */
/*JSON{
  "type" : "class",
  "class" : "InternalError"
}
The base class for internal errors
 */
/*JSON{
  "type" : "class",
  "class" : "ReferenceError"
}
The base class for reference errors - where a variable which doesn't exist has
been accessed.
 */

JsVar *_jswrap_error_constructor(JsVar *msg, char *type) {
  JsVar *d = jspNewObject(0,type);
  if (!d) return 0;

  if (msg) {
    msg = jsvAsString(msg);
    jsvObjectSetChildAndUnLock(d, "message", msg);
  }
  jsvObjectSetChildAndUnLock(d, "type", jsvNewFromString(type));

  return d;
}

/*JSON{
  "type" : "constructor",
  "class" : "Error",
  "name" : "Error",
  "generate" : "jswrap_error_constructor",
  "params" : [
    ["message","JsVar","An optional message string"]
  ],
  "return" : ["JsVar","An Error object"],
  "typescript" : "new(message?: string): Error;"
}
Creates an Error object
 */
JsVar *jswrap_error_constructor(JsVar *msg) {
  return _jswrap_error_constructor(msg, "Error");
}
/*JSON{
  "type" : "constructor",
  "class" : "SyntaxError",
  "name" : "SyntaxError",
  "generate" : "jswrap_syntaxerror_constructor",
  "params" : [
    ["message","JsVar","An optional message string"]
  ],
  "return" : ["JsVar","A SyntaxError object"],
  "typescript" : "new(message?: string): SyntaxError;"
}
Creates a SyntaxError object
 */
JsVar *jswrap_syntaxerror_constructor(JsVar *msg) {
  return _jswrap_error_constructor(msg, "SyntaxError");
}
/*JSON{
  "type" : "constructor",
  "class" : "TypeError",
  "name" : "TypeError",
  "generate" : "jswrap_typeerror_constructor",
  "params" : [
    ["message","JsVar","An optional message string"]
  ],
  "return" : ["JsVar","A TypeError object"],
  "typescript" : "new(message?: string): TypeError;"
}
Creates a TypeError object
 */
JsVar *jswrap_typeerror_constructor(JsVar *msg) {
  return _jswrap_error_constructor(msg, "TypeError");
}
/*JSON{
  "type" : "constructor",
  "class" : "InternalError",
  "name" : "InternalError",
  "generate" : "jswrap_internalerror_constructor",
  "params" : [
    ["message","JsVar","An optional message string"]
  ],
  "return" : ["JsVar","An InternalError object"],
  "typescript" : "new(message?: string): InternalError;"
}
Creates an InternalError object
 */
JsVar *jswrap_internalerror_constructor(JsVar *msg) {
  return _jswrap_error_constructor(msg, "InternalError");
}

/*JSON{
  "type" : "constructor",
  "class" : "ReferenceError",
  "name" : "ReferenceError",
  "generate" : "jswrap_referenceerror_constructor",
  "params" : [
    ["message","JsVar","An optional message string"]
  ],
  "return" : ["JsVar","A ReferenceError object"],
  "typescript" : "new(message?: string): ReferenceError;"
}
Creates a ReferenceError object
 */
JsVar *jswrap_referenceerror_constructor(JsVar *msg) {
  return _jswrap_error_constructor(msg, "ReferenceError");
}

/*JSON{
  "type" : "method",
  "class" : "Error",
  "name" : "toString",
  "generate" : "jswrap_error_toString",
  "return" : ["JsVar","A String"],
  "typescript" : "toString(): string;"
}*/
/*JSON{
  "type" : "method",
  "class" : "SyntaxError",
  "name" : "toString",
  "generate" : "jswrap_error_toString",
  "return" : ["JsVar","A String"],
  "typescript" : "toString(): string;"
}*/
/*JSON{
  "type" : "method",
  "class" : "TypeError",
  "name" : "toString",
  "generate" : "jswrap_error_toString",
  "return" : ["JsVar","A String"],
  "typescript" : "toString(): string;"
}*/
/*JSON{
  "type" : "method",
  "class" : "InternalError",
  "name" : "toString",
  "generate" : "jswrap_error_toString",
  "return" : ["JsVar","A String"],
  "typescript" : "toString(): string;"
}*/
/*JSON{
  "type" : "method",
  "class" : "ReferenceError",
  "name" : "toString",
  "generate" : "jswrap_error_toString",
  "return" : ["JsVar","A String"],
  "typescript" : "toString(): string;"
}*/
JsVar *jswrap_error_toString(JsVar *parent) {
  JsVar *str = jsvObjectGetChild(parent, "type", 0);
  if (!str) str = jsvNewFromString("Error");
  if (!str) return 0;

  JsVar *msg = jsvObjectGetChild(parent, "message", 0);
  if (msg) {
    JsVar *newStr = jsvVarPrintf("%v: %v", str, msg);
    jsvUnLock2(msg, str);
    str = newStr;
  }

  return str;
}
