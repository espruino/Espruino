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
 * JavaScript methods for Numbers
 * ----------------------------------------------------------------------------
 */
#include "jswrap_number.h"

/*JSON{
  "type" : "object",
  "name" : "Number",
  "memberOf" : "global",
  "check" : "jsvIsNumeric(var)"
}
This is the built-in JavaScript class for numbers.
*/

/*JSON{
  "type" : "constructor",
  "name" : "Number",
  "generate" : "jswrap_number_constructor",
  "params" : [
    ["value","JsVarArray","A single value to be converted to a number"]
  ],
  "return" : ["JsVar","A Number object"]
}
Creates a number
*/
JsVar *jswrap_number_constructor(JsVar *args) {
  if (jsvGetArrayLength(args)==0) return jsvNewFromInteger(0);
  JsVar *val = jsvGetArrayItem(args, 0);
  JsVar *result = 0;

  if (jsvIsArray(val)) {
    JsVarInt l = jsvGetArrayLength(val);
    if (l==0) result = jsvNewFromInteger(0);
    else if (l==1) {
      JsVar *n = jsvGetArrayItem(val, 0);
      if (jsvIsString(n) && jsvIsEmptyString(n)) result = jsvNewFromInteger(0);
      else if (!jsvIsBoolean(n)) result=jsvAsNumber(n);
      jsvUnLock(n);
    } // else NaN
  } else if (jsvIsUndefined(val) || jsvIsObject(val))
    result = 0;
  else {
    if (jsvIsString(val) && jsvIsEmptyString(val)) {
      result = jsvNewFromInteger(0);
    } else
      result = jsvAsNumber(val);
  }
  jsvUnLock(val);
  if (result) return result;
  return jsvNewFromFloat(NAN);
}


/*JSON{
  "type" : "variable",
  "name" : "NaN",
  "memberOf" : "global",
  "thisParam" : false,
  "generate_full" : "NAN",
  "return" : ["float","Not a  Number"]
}

*/

/*JSON{
  "type" : "variable",
  "name" : "Infinity",
  "memberOf" : "global",
  "thisParam" : false,
  "generate_full" : "INFINITY",
  "return" : ["float","Positive Infinity (1/0)"]
}

*/

/*JSON{
  "type" : "variable",
  "name" : "NaN",
  "memberOf" : "Number",
  "thisParam" : false,
  "generate_full" : "NAN",
  "return" : ["float","Not a  Number"]
}

*/

/*JSON{
  "type" : "variable",
  "name" : "MAX_VALUE",
  "memberOf" : "Number",
  "thisParam" : false,
  "generate_full" : "DBL_MAX",
  "return" : ["float","Maximum representable value"]
}

*/

/*JSON{
  "type" : "variable",
  "name" : "MIN_VALUE",
  "memberOf" : "Number",
  "thisParam" : false,
  "generate_full" : "DBL_MIN",
  "return" : ["float","Smallest representable value"]
}

*/

/*JSON{
  "type" : "variable",
  "name" : "NEGATIVE_INFINITY",
  "memberOf" : "Number",
  "thisParam" : false,
  "generate_full" : "-INFINITY",
  "return" : ["float","Negative Infinity (-1/0)"]
}

*/

/*JSON{
  "type" : "variable",
  "name" : "POSITIVE_INFINITY",
  "memberOf" : "Number",
  "thisParam" : false,
  "generate_full" : "INFINITY",
  "return" : ["float","Positive Infinity (1/0)"]
}

*/

/*JSON{
  "type" : "function",
  "name" : "toFixed",
  "memberOf" : "Number.prototype",
  "thisParam" : true,
  "generate" : "jswrap_number_toFixed",
  "params" : [
    ["decimalPlaces","int32","A number between 0 and 20 specifying the number of decimal digits after the decimal point"]
  ],
  "return" : ["JsVar","A string"]
}
Format the number as a fixed point number
*/
JsVar *jswrap_number_toFixed(JsVar *parent, int decimals) {
  if (decimals<0) decimals=0;
  if (decimals>20) decimals=20;
  char buf[JS_NUMBER_BUFFER_SIZE];
  ftoa_bounded_extra(jsvGetFloat(parent), buf, sizeof(buf), 10, decimals);
  return jsvNewFromString(buf);
}

/*JSON{
  "type" : "variable",
  "name" : "HIGH",
  "memberOf" : "global",
  "thisParam" : false,
  "generate_full" : "1",
  "return" : ["int32","Logic 1 for Arduino compatibility - this is the same as just typing `1`"]
}

*/

/*JSON{
  "type" : "variable",
  "name" : "LOW",
  "memberOf" : "global",
  "thisParam" : false,
  "generate_full" : "0",
  "return" : ["int32","Logic 0 for Arduino compatibility - this is the same as just typing `0`"]
}

*/
