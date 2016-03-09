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
  "type" : "object",
  "name" : "Error",
  "memberOf" : "global"
}
The base class for runtime errors
*/
/*JSON{
  "type" : "object",
  "name" : "SyntaxError",
  "memberOf" : "global"
}
The base class for syntax errors
*/
/*JSON{
  "type" : "object",
  "name" : "TypeError",
  "memberOf" : "global"
}
The base class for type errors
*/
/*JSON{
  "type" : "object",
  "name" : "InternalError",
  "memberOf" : "global"
}
The base class for internal errors
*/
/*JSON{
  "type" : "object",
  "name" : "ReferenceError",
  "memberOf" : "global"
}
The base class for reference errors - where a variable
which doesn't exist has been accessed.
*/

JsVar *_jswrap_error_constructor(JsVar *msg, char *type) {
  JsVar *d = jspNewObject(0,type);
  if (!d) return 0;

  if (msg) {
    msg = jsvAsString(msg, false);
    jsvObjectSetChildAndUnLock(d, "msg", msg);
  }
  jsvObjectSetChildAndUnLock(d, "type", jsvNewFromString(type));

  return d;
}

/*JSON{
  "type" : "constructor",
  "name" : "Error",
  "generate" : "jswrap_error_constructor",
  "params" : [
    ["message","JsVar","An optional message string"]
  ],
  "return" : ["JsVar","An Error object"]
}
Creates an Error object
*/
JsVar *jswrap_error_constructor(JsVar *msg) {
  return _jswrap_error_constructor(msg, "Error");
}
/*JSON{
  "type" : "constructor",
  "name" : "SyntaxError",
  "generate" : "jswrap_syntaxerror_constructor",
  "params" : [
    ["message","JsVar","An optional message string"]
  ],
  "return" : ["JsVar","A SyntaxError object"]
}
Creates a SyntaxError object
*/
JsVar *jswrap_syntaxerror_constructor(JsVar *msg) {
  return _jswrap_error_constructor(msg, "SyntaxError");
}
/*JSON{
  "type" : "constructor",
  "name" : "TypeError",
  "generate" : "jswrap_typeerror_constructor",
  "params" : [
    ["message","JsVar","An optional message string"]
  ],
  "return" : ["JsVar","A TypeError object"]
}
Creates a TypeError object
*/
JsVar *jswrap_typeerror_constructor(JsVar *msg) {
  return _jswrap_error_constructor(msg, "TypeError");
}
/*JSON{
  "type" : "constructor",
  "name" : "InternalError",
  "generate" : "jswrap_internalerror_constructor",
  "params" : [
    ["message","JsVar","An optional message string"]
  ],
  "return" : ["JsVar","An InternalError object"]
}
Creates an InternalError object
*/
JsVar *jswrap_internalerror_constructor(JsVar *msg) {
  return _jswrap_error_constructor(msg, "InternalError");
}

/*JSON{
  "type" : "constructor",
  "name" : "ReferenceError",
  "generate" : "jswrap_referenceerror_constructor",
  "params" : [
    ["message","JsVar","An optional message string"]
  ],
  "return" : ["JsVar","A ReferenceError object"]
}
Creates a ReferenceError object
*/
JsVar *jswrap_referenceerror_constructor(JsVar *msg) {
  return _jswrap_error_constructor(msg, "ReferenceError");
}

/*JSON{
  "type" : "function",
  "name" : "toString",
  "memberOf" : "Error.prototype",
  "thisParam" : true,
  "generate" : "jswrap_error_toString",
  "return" : ["JsVar","A String"]
}

*/
/*JSON{
  "type" : "function",
  "name" : "toString",
  "memberOf" : "SyntaxError.prototype",
  "thisParam" : true,
  "generate" : "jswrap_error_toString",
  "return" : ["JsVar","A String"]
}

*/
/*JSON{
  "type" : "function",
  "name" : "toString",
  "memberOf" : "TypeError.prototype",
  "thisParam" : true,
  "generate" : "jswrap_error_toString",
  "return" : ["JsVar","A String"]
}

*/
/*JSON{
  "type" : "function",
  "name" : "toString",
  "memberOf" : "InternalError.prototype",
  "thisParam" : true,
  "generate" : "jswrap_error_toString",
  "return" : ["JsVar","A String"]
}

*/
/*JSON{
  "type" : "function",
  "name" : "toString",
  "memberOf" : "ReferenceError.prototype",
  "thisParam" : true,
  "generate" : "jswrap_error_toString",
  "return" : ["JsVar","A String"]
}

*/
JsVar *jswrap_error_toString(JsVar *parent) {
  JsVar *str = jsvObjectGetChild(parent, "type", 0);
  if (!str) str = jsvNewFromString("Error");
  if (!str) return 0;

  JsVar *msg = jsvObjectGetChild(parent, "msg", 0);
  if (msg) {
    JsVar *newStr = jsvVarPrintf("%v: %v", str, msg);
    jsvUnLock2(msg, str);
    str = newStr;
  }

  return str;
}
