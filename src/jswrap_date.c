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
#include "jshardware.h"

const int MSDAY = 24*60*60*1000;
const int YDAY = 365;
const int LDAY = 366;
const int FDAY = 4*365+1;
const int BASE_DOW = 4;
const short DAYS[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
const short LPDAYS[13] = {0,31,60,91,121,152,182,213,244,274,305,335,366};
const short YDAYS[4] = {0,365,365*2,365*3+1};

/* NOTE: we use / and % here because the compiler is smart enough to
 * condense them into one op. */

typedef struct {
  int daysSinceEpoch;
  int ms,sec,min,hour;
} TimeInDay;

typedef struct {
  int daysSinceEpoch;
  int day,month,year,dow;
} CalendarDate;

TimeInDay getTimeFromMilliSeconds(JsVarFloat ms_in) {
  TimeInDay t;
  t.daysSinceEpoch = (int)(ms_in / MSDAY);
  int ms = (int)(ms_in - ((JsVarFloat)t.daysSinceEpoch * MSDAY));
  int s = ms / 1000;
  t.ms = ms % 1000;
  t.hour = s / 3600;
  s = s % 3600;
  t.min = s/60;
  t.sec = s%60;
  return t;
}

JsVarFloat fromTimeInDay(TimeInDay *td) {
  return (JsVarFloat)(td->ms + (((td->hour*60+td->min)*60+td->sec)*1000) + (JsVarFloat)td->daysSinceEpoch*MSDAY);
}

// First calculate the number of four-year-interval, so calculation
// of leap year will be simple. Btw, because 2000 IS a leap year and
// 2100 is out of range, the formlua is simplified
CalendarDate getCalendarDate(int d) {
  int y,m;
  const short *mdays=DAYS;

  CalendarDate date;
  date.daysSinceEpoch = d;

  y=d / FDAY;
  d=d % FDAY;
  y=y*4 + 1970;

  if (d>=YDAY) {
    y=y+1;                  // Second year in four - 1971
    d=d-YDAY;
    if (d>=YDAY) {
      y=y+1;                // Could be third or fourth year
      d=d-YDAY;
      if (d>=LDAY) {
        y=y+1;              // Definitly fourth
        d=d-LDAY;
      } else {               // Third - leap year
        mdays=LPDAYS;
      }
    }
  }

  date.year=y;

  //Find the month

  m=0;
  while (mdays[m]<d+1) {
    m++;
  }
  date.month=m-1;
  date.day=d - mdays[date.month]+1;


  // Calculate day of week. Sunday is 0
  date.dow=(date.daysSinceEpoch+BASE_DOW)%7;
  return date;
};

int fromCalenderDate(CalendarDate *date) {
  int y=date->year - 1970;
  int f=y/4;
  int yf=y%4;
  const short *mdays;

  int ydays=yf*YDAY;

  if (yf==2) {
    mdays=LPDAYS;
  } else {
    mdays=DAYS;
  }

  if (yf>=2)
    ydays=ydays+1;

  return f*FDAY+YDAYS[yf]+mdays[date->month]+date->day-1;
};

/*JSON{ "type":"class",
        "class" : "Date",
        "description" : [ "The built-in class for handling Dates" ]
}*/

/*JSON{ "type":"staticmethod", "class": "Date", "name" : "now",
         "description" : "Get the number of milliseconds elapsed since 1970 (or on embedded platforms, since startup)",
         "generate" : "jswrap_date_now",
         "return" : ["float", ""]
}*/
JsVarFloat jswrap_date_now() {
  return (JsVarFloat)jshGetSystemTime() / (JsVarFloat)jshGetTimeFromMilliseconds(1);
}


/*JSON{ "type":"constructor",
        "class" : "Date",
        "name" : "Date",
        "generate" : "jswrap_date_constructor",
        "description" : [ "Creates a date object" ],
        "params" : [ [ "args", "JsVarArray", "Either nothing (current time), one numeric argument (milliseconds since 1970), a date string (unsupported), or [year, month, day, hour, minute, second, millisecond] "] ],
        "return" : ["JsVar", "A Date object"]
}*/
JsVar *jswrap_date_constructor(JsVar *args) {
  JsVar *d = jspNewObject(0,"Date");
  if (!d) return 0;

  JsVarFloat time = 0;

  if (jsvGetArrayLength(args)==0) {
    time = jswrap_date_now();
  } else if (jsvGetArrayLength(args)==0) {
    JsVar *arg = jsvGetArrayItem(args, 0);
    if (jsvIsNumeric(arg))
      time = jsvGetFloat(arg);
    else
      jsWarn("Strings in date constructor are unsupported");
    jsvUnLock(arg);
  } else {
    CalendarDate date;
    date.year = (int)jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 0));
    date.month = (int)jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 1));
    date.day = (int)jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 2));
    TimeInDay td;
    td.daysSinceEpoch = fromCalenderDate(&date);
    td.hour = (int)jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 3));
    td.min = (int)jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 4));
    td.sec = (int)jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 5));
    td.ms = (int)jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 6));
    time = fromTimeInDay(&td);
  }

  jsvUnLock(jsvObjectSetChild(d, "ms", jsvNewFromFloat(time)));

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


/*JSON{ "type":"method", "class": "Date", "name" : "getTime",
         "description" : "Return the number of milliseconds since 1970",
         "generate" : "jswrap_date_getTime",
         "return" : ["float", ""]
}*/
JsVarFloat jswrap_date_getTime(JsVar *date) {
  return jsvGetFloatAndUnLock(jsvObjectGetChild(date, "ms", 0));
}

static TimeInDay getTimeFromDateVar(JsVar *date) {
  return getTimeFromMilliSeconds(jswrap_date_getTime(date));
}

static CalendarDate getCalendarDateFromDateVar(JsVar *date) {
  return getCalendarDate(getTimeFromDateVar(date).daysSinceEpoch);
}


/*JSON{ "type":"method", "class": "Date", "name" : "getHours",
         "description" : "0..23",
         "generate" : "jswrap_date_getHours",
         "return" : ["int32", ""]
}*/
int jswrap_date_getHours(JsVar *parent) {
  return getTimeFromDateVar(parent).hour;
}

/*JSON{ "type":"method", "class": "Date", "name" : "getMinutes",
         "description" : "0..59",
         "generate" : "jswrap_date_getMinutes",
         "return" : ["int32", ""]
}*/
int jswrap_date_getMinutes(JsVar *parent) {
  return getTimeFromDateVar(parent).min;
}

/*JSON{ "type":"method", "class": "Date", "name" : "getMinutes",
         "description" : "0..59",
         "generate" : "jswrap_date_getSeconds",
         "return" : ["int32", ""]
}*/
int jswrap_date_getSeconds(JsVar *parent) {
  return getTimeFromDateVar(parent).sec;
}

/*JSON{ "type":"method", "class": "Date", "name" : "getMinutes",
         "description" : "0..999",
         "generate" : "jswrap_date_getMilliseconds",
         "return" : ["int32", ""]
}*/
int jswrap_date_getMilliseconds(JsVar *parent) {
  return getTimeFromDateVar(parent).ms;
}

/*JSON{ "type":"method", "class": "Date", "name" : "getDay",
         "description" : "Day of the week (0=sunday, 1=monday, etc)",
         "generate" : "jswrap_date_getDay",
         "return" : ["int32", ""]
}*/
int jswrap_date_getDay(JsVar *parent) {
  return getCalendarDateFromDateVar(parent).dow;
}

/*JSON{ "type":"method", "class": "Date", "name" : "getDate",
         "description" : "Day of the month 1..31",
         "generate" : "jswrap_date_getDate",
         "return" : ["int32", ""]
}*/
int jswrap_date_getDate(JsVar *parent) {
  return getCalendarDateFromDateVar(parent).day;
}


/*JSON{ "type":"method", "class": "Date", "name" : "getMonth",
         "description" : "Month of the year 1..12",
         "generate" : "jswrap_date_getMonth",
         "return" : ["int32", ""]
}*/
int jswrap_date_getMonth(JsVar *parent) {
  return getCalendarDateFromDateVar(parent).month;
}

/*JSON{ "type":"method", "class": "Date", "name" : "getFullYear",
         "description" : "The year, eg. 2014",
         "generate" : "jswrap_date_getFullYear",
         "return" : ["int32", ""]
}*/
int jswrap_date_getFullYear(JsVar *parent) {
  return getCalendarDateFromDateVar(parent).year;
}
