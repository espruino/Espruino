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
 *  This file is designed to be parsed during the build process
 *
 * JavaScript methods for Date Handling
 * ----------------------------------------------------------------------------
 */
#include "jswrap_date.h"
#include "jsparse.h"

/*JSON{ "type":"class",
        "class" : "Date",
        "description" : [ "The built-in class for handling Dates" ]
}*/

/*JSON{ "type":"constructor",
        "class" : "Date",
        "name" : "Date",
        "generate" : "jswrap_date_constructor",
        "description" : [ "Creates a date object" ],
        "return" : ["JsVar", "A Date object"]
}*/
JsVar *jswrap_date_constructor() {
  JsVar *d = jspNewObject(0,"Date");
  if (!d) return 0;

  return d;
}

/*JSON{ "type":"method", "class": "Date", "name" : "getTimezoneOffset",
         "description" : "The getTimezoneOffset() method returns the time-zone offset from UTC, in minutes, for the current locale.",
         "generate" : "jswrap_date_getTimezoneOffset",
         "return" : ["float", "The difference, in minutes, between UTC and local time"]
}*/
JsVarFloat jswrap_date_getTimezoneOffset(JsVar *parent) {
  NOT_USED(parent);
  return 0;
}

/*JSON{ "type":"method", "class": "Date", "name" : "getHours",
         "description" : "0..23",
         "generate" : "jswrap_date_getHours",
         "return" : ["int32", ""]
}*/
int jswrap_date_getHours(JsVar *parent) {
  return (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(parent, "hours", 0));
}

/*JSON{ "type":"method", "class": "Date", "name" : "getMinutes",
         "description" : "0..59",
         "generate" : "jswrap_date_getMinutes",
         "return" : ["int32", ""]
}*/
int jswrap_date_getMinutes(JsVar *parent) {
  return (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(parent, "minutes", 0));
}

/*JSON{ "type":"method", "class": "Date", "name" : "getDay",
         "description" : "Day of the week (0=sunday, 1=monday, etc)",
         "generate" : "jswrap_date_getDay",
         "return" : ["int32", ""]
}*/
int jswrap_date_getDay(JsVar *parent) {
  return (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(parent, "day", 0));
}

/*JSON{ "type":"method", "class": "Date", "name" : "getDate",
         "description" : "Day of the month 1..31",
         "generate" : "jswrap_date_getDate",
         "return" : ["int32", ""]
}*/
int jswrap_date_getDate(JsVar *parent) {
  return (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(parent, "date", 0));
}


/*JSON{ "type":"method", "class": "Date", "name" : "getMonth",
         "description" : "Month of the year 1..12",
         "generate" : "jswrap_date_getMonth",
         "return" : ["int32", ""]
}*/
int jswrap_date_getMonth(JsVar *parent) {
  return (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(parent, "month", 0));
}
