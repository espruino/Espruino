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
#include "jslex.h"

const int MSDAY = 24*60*60*1000;
const int BASE_DOW = 4;
const char *MONTHNAMES = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec";
const char *DAYNAMES = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";

// Convert y,m,d into a number of days since 1970, where 0<=m<=11
// https://github.com/deirdreobyrne/CalendarAndDST
int getDayNumberFromDate(int y, int m, int d) {
  int ans;

  if (m < 2) {
    y--;
    m+=12;
  }
  ans = (y/100);
  return 365*y + (y>>2) - ans + (ans>>2) + 30*m + ((3*m+6)/5) + d - 719531;
}

// Convert a number of days since 1970 into y,m,d. 0<=m<=11
// https://github.com/deirdreobyrne/CalendarAndDST
void getDateFromDayNumber(int day, int *y, int *m, int *date) {
  int a = day + 135081;
  int b,c,d;
  a = (a-(a/146097)+146095)/36524;
  a = day + a - (a>>2);
  b = ((a<<2)+2877911)/1461;
  c = a + 719600 - 365*b - (b>>2);
  d = (5*c-1)/153;
  if (date) *date=c-30*d-((3*d)/5);
  if (m) {
    if (d<14)
      *m=d-2;
    else
      *m=d-14;
  }
  if (y) {
    if (d>13)
      *y=b+1;
    else
      *y=b;
  }
}

#ifndef ESPR_NO_DAYLIGHT_SAVING
// Given a set of DST change settings, calculate the time (in GMT seconds since 1970) that the change happens in year y
// If as_local_time is true, then returns the number of seconds in the timezone in effect, as opposed to GMT
// https://github.com/deirdreobyrne/CalendarAndDST
JsVarFloat getDstChangeTime(int y, int dow_number, int dow, int month, int day_offset, int timeOfDay, bool is_start, int dst_offset, int timezone, bool as_local_time) {
  int ans;
  if (dow_number == 4) { // last X of this month? Work backwards from 1st of next month.
    if (++month > 11) {
      y++;
      month-=12;
    }
  }
  ans = getDayNumberFromDate(y, month, 1); // ans % 7 is 0 for THU; (ans + 4) % 7 is 0 for SUN
  // ((14 - ((ans + 4) % 7) + dow) % 7) is zero if 1st is our dow, 1 if 1st is the day before our dow etc
  if (dow_number == 4) {
    ans += ((14 - ((ans + 4) % 7) + dow) % 7) - 7;
  } else {
    ans += 7 * dow_number + (14 - ((ans + 4) % 7) + dow) % 7;
  }
  ans = (ans + day_offset) * 1440 + timeOfDay;
  if (!as_local_time) {
    ans -= timezone;
    if (!is_start) ans -= dst_offset;
  }
  return 60.0*ans;
}
#endif

// Returns the effective timezone in minutes east
// is_local_time is true if ms is referenced to local time, false if it's referenced to GMT
// if is_dst is not zero, then it will be set to true if DST is in effect
// https://github.com/deirdreobyrne/CalendarAndDST
int jsdGetEffectiveTimeZone(JsVarFloat ms, bool is_local_time, bool *is_dst) {
#ifndef ESPR_NO_DAYLIGHT_SAVING
  JsVar *dst = jsvObjectGetChild(execInfo.hiddenRoot, JS_DST_SETTINGS_VAR, 0);
  if ((dst) && (jsvIsArrayBuffer(dst)) && (jsvGetLength(dst) == 12) && (dst->varData.arraybuffer.type == ARRAYBUFFERVIEW_INT16)) {
    int y;
    JsVarInt dstSetting[12];
    JsvArrayBufferIterator it;
  
    jsvArrayBufferIteratorNew(&it, dst, 0);
    y = 0;
    while (y < 12) {
      dstSetting[y++]=jsvArrayBufferIteratorGetIntegerValue(&it);
      jsvArrayBufferIteratorNext(&it);
    }
    jsvArrayBufferIteratorFree(&it);
    jsvUnLock(dst);
    if (dstSetting[0]) {
      JsVarFloat sec = ms/1000;
      JsVarFloat dstStart,dstEnd;
      bool dstActive;
      
      getDateFromDayNumber((int)(sec/86400),&y,0,0);
      dstStart = getDstChangeTime(y, dstSetting[2], dstSetting[3], dstSetting[4], dstSetting[5], dstSetting[6], 1, dstSetting[0], dstSetting[1], is_local_time);
      dstEnd = getDstChangeTime(y, dstSetting[7], dstSetting[8], dstSetting[9], dstSetting[10], dstSetting[11], 0, dstSetting[0], dstSetting[1], is_local_time);
      if (dstStart < dstEnd) { // Northern hemisphere
        dstActive = (sec >= dstStart) && (sec < dstEnd);
      } else { // Southern hemisphere
        dstActive = (sec < dstEnd) || (sec >= dstStart);
      }
      if (is_dst) *is_dst = dstActive;
      return dstActive ? dstSetting[0]+dstSetting[1] : dstSetting[1];
    }
  } else {
    jsvUnLock(dst);
  }
#endif
  if (is_dst) *is_dst = false;
  return jsvGetIntegerAndUnLock(jsvObjectGetChild(execInfo.hiddenRoot, JS_TIMEZONE_VAR, 0));
}

// this needs to be called just before a TimeInDay is used -- unless the TimeInDay timezone has been determined by other means.
void setCorrectTimeZone(TimeInDay *td) {
  td->zone = 0;
  td->zone = jsdGetEffectiveTimeZone(fromTimeInDay(td),true,&(td->is_dst));
}

/* NOTE: we use / and % here because the compiler is smart enough to
 * condense them into one op. */
TimeInDay getTimeFromMilliSeconds(JsVarFloat ms_in, bool forceGMT) {
  TimeInDay t;
  t.zone = forceGMT ? 0 : jsdGetEffectiveTimeZone(ms_in, false, &(t.is_dst));
  ms_in += t.zone*60000;
  t.daysSinceEpoch = (int)(ms_in / MSDAY);

  if (forceGMT) t.is_dst = false;
  int ms = (int)(ms_in - ((JsVarFloat)t.daysSinceEpoch * MSDAY));
  if (ms<0) {
    ms += MSDAY;
    t.daysSinceEpoch--;
  }
  int s = ms / 1000;
  t.ms = ms % 1000;
  t.hour = s / 3600;
  s = s % 3600;
  t.min = s/60;
  t.sec = s%60;

  return t;
}

JsVarFloat fromTimeInDay(TimeInDay *td) {
  return (JsVarFloat)(td->ms + (((td->hour*60+td->min - td->zone)*60+td->sec)*1000) + (JsVarFloat)td->daysSinceEpoch*MSDAY);
}

CalendarDate getCalendarDate(int d) {
  CalendarDate date;

  getDateFromDayNumber(d, &date.year, &date.month, &date.day);
  date.daysSinceEpoch = d;
  // Calculate day of week. Sunday is 0
  date.dow=(date.daysSinceEpoch+BASE_DOW)%7;
  if (date.dow<0) date.dow+=7;
  return date;
};

int fromCalenderDate(CalendarDate *date) {
  while (date->month < 0) {
    date->year--;
    date->month += 12;
  }
  while (date->month > 11) {
    date->year++;
    date->month -= 12;
  }
  return getDayNumberFromDate(date->year, date->month, date->day);
};


static int getMonth(const char *s) {
  int i;
  for (i=0;i<12;i++)
    if (s[0]==MONTHNAMES[i*4] &&
        s[1]==MONTHNAMES[i*4+1] &&
        s[2]==MONTHNAMES[i*4+2])
      return i;
  return -1;
}

static int getDay(const char *s) {
  int i;
  for (i=0;i<7;i++)
    if (strcmp(s, &DAYNAMES[i*4])==0)
      return i;
  return -1;
}

static TimeInDay getTimeFromDateVar(JsVar *date, bool forceGMT) {
  return getTimeFromMilliSeconds(jswrap_date_getTime(date), forceGMT);
}

static CalendarDate getCalendarDateFromDateVar(JsVar *date, bool forceGMT) {
  return getCalendarDate(getTimeFromDateVar(date, forceGMT).daysSinceEpoch);
}

/*JSON{
  "type" : "class",
  "class" : "Date"
}
The built-in class for handling Dates.

**Note:** By default the time zone is GMT+0, however you can change the timezone
using the `E.setTimeZone(...)` function.

For example `E.setTimeZone(1)` will be GMT+0100

*However* if you have daylight savings time set with `E.setDST(...)` then the
timezone set by `E.setTimeZone(...)` will be _ignored_.

 */

/*JSON{
  "type" : "staticmethod",
  "class" : "Date",
  "name" : "now",
  "generate" : "jswrap_date_now",
  "return" : ["float",""]
}
Get the number of milliseconds elapsed since 1970 (or on embedded platforms,
since startup)
 */
JsVarFloat jswrap_date_now() {
  // Not quite sure why we need this, but (JsVarFloat)jshGetSystemTime() / (JsVarFloat)jshGetTimeFromMilliseconds(1) in inaccurate on STM32
  return ((JsVarFloat)jshGetSystemTime() / (JsVarFloat)jshGetTimeFromMilliseconds(1000)) * 1000;
}


JsVar *jswrap_date_from_milliseconds(JsVarFloat time) {
  JsVar *d = jspNewObject(0,"Date");
  jswrap_date_setTime(d, time);
  return d;
}


/*JSON{
  "type" : "constructor",
  "class" : "Date",
  "name" : "Date",
  "generate" : "jswrap_date_constructor",
  "params" : [
    ["args","JsVarArray","Either nothing (current time), one numeric argument (milliseconds since 1970), a date string (see `Date.parse`), or [year, month, day, hour, minute, second, millisecond] "]
  ],
  "return" : ["JsVar","A Date object"],
  "return_object" : "Date",
  "typescript" : [
    "new(): Date;",
    "new(value: number | string): Date;",
    "new(year: number, month: number, date?: number, hours?: number, minutes?: number, seconds?: number, ms?: number): Date;"
  ]
}
Creates a date object
 */
JsVar *jswrap_date_constructor(JsVar *args) {
  JsVarFloat time = 0;

  if (jsvGetArrayLength(args)==0) {
    time = jswrap_date_now();
  } else if (jsvGetArrayLength(args)==1) {
    JsVar *arg = jsvGetArrayItem(args, 0);
    if (jsvIsNumeric(arg))
      time = jsvGetFloat(arg);
    else if (jsvIsString(arg))
      time = jswrap_date_parse(arg);
    else
      jsExceptionHere(JSET_TYPEERROR, "Variables of type %t are not supported in date constructor", arg);
    jsvUnLock(arg);
  } else {
    CalendarDate date;
    date.year = (int)jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 0));
    date.month = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 1)));
    date.day = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 2)));
    TimeInDay td;
    td.daysSinceEpoch = fromCalenderDate(&date);
    td.hour = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 3)));
    td.min = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 4)));
    td.sec = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 5)));
    td.ms = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 6)));
    setCorrectTimeZone(&td);
    time = fromTimeInDay(&td);
  }

  return jswrap_date_from_milliseconds(time);
}

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Date",
  "name" : "getTimezoneOffset",
  "generate" : "jswrap_date_getTimezoneOffset",
  "return" : ["int32","The difference, in minutes, between UTC and local time"]
}
This returns the time-zone offset from UTC, in minutes.

 */
int jswrap_date_getTimezoneOffset(JsVar *parent) {
  return -getTimeFromDateVar(parent, false/*system timezone*/).zone;
}

// I'm assuming SAVE_ON_FLASH always goes with ESPR_NO_DAYLIGHT_SAVING
/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Date",
  "name" : "getIsDST",
  "generate" : "jswrap_date_getIsDST",
  "return" : ["int32","true if daylight savings time is in effect"],
  "typescript" : "getIsDST(): boolean"
}
This returns a boolean indicating whether daylight savings time is in effect.

 */
int jswrap_date_getIsDST(JsVar *parent) {
  return getTimeFromDateVar(parent, false/*system timezone*/).is_dst ? 1 : 0;
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getTime",
  "generate" : "jswrap_date_getTime",
  "return" : ["float",""]
}
Return the number of milliseconds since 1970
 */
/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "valueOf",
  "generate" : "jswrap_date_getTime",
  "return" : ["float",""]
}
Return the number of milliseconds since 1970
 */
JsVarFloat jswrap_date_getTime(JsVar *date) {
  return jsvGetFloatAndUnLock(jsvObjectGetChild(date, "ms", 0));
}
/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "setTime",
  "generate" : "jswrap_date_setTime",
  "params" : [
    ["timeValue","float","the number of milliseconds since 1970"]
  ],
  "return" : ["float","the number of milliseconds since 1970"]
}
Set the time/date of this Date class
 */
JsVarFloat jswrap_date_setTime(JsVar *date, JsVarFloat timeValue) {
  if (date)
    jsvObjectSetChildAndUnLock(date, "ms", jsvNewFromFloat(timeValue));
  return timeValue;
}


/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getHours",
  "generate" : "jswrap_date_getHours",
  "return" : ["int32",""]
}
0..23
 */
int jswrap_date_getHours(JsVar *parent) {
  return getTimeFromDateVar(parent, false/*system timezone*/).hour;
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getMinutes",
  "generate" : "jswrap_date_getMinutes",
  "return" : ["int32",""]
}
0..59
 */
int jswrap_date_getMinutes(JsVar *parent) {
  return getTimeFromDateVar(parent, false/*system timezone*/).min;
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getSeconds",
  "generate" : "jswrap_date_getSeconds",
  "return" : ["int32",""]
}
0..59
 */
int jswrap_date_getSeconds(JsVar *parent) {
  return getTimeFromDateVar(parent, false/*system timezone*/).sec;
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getMilliseconds",
  "generate" : "jswrap_date_getMilliseconds",
  "return" : ["int32",""]
}
0..999
 */
int jswrap_date_getMilliseconds(JsVar *parent) {
  return getTimeFromDateVar(parent, false/*system timezone*/).ms;
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getDay",
  "generate" : "jswrap_date_getDay",
  "return" : ["int32",""]
}
Day of the week (0=sunday, 1=monday, etc)
 */
int jswrap_date_getDay(JsVar *parent) {
  return getCalendarDateFromDateVar(parent, false/*system timezone*/).dow;
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getDate",
  "generate" : "jswrap_date_getDate",
  "return" : ["int32",""]
}
Day of the month 1..31
 */
int jswrap_date_getDate(JsVar *parent) {
  return getCalendarDateFromDateVar(parent, false/*system timezone*/).day;
}


/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getMonth",
  "generate" : "jswrap_date_getMonth",
  "return" : ["int32",""]
}
Month of the year 0..11
 */
int jswrap_date_getMonth(JsVar *parent) {
  return getCalendarDateFromDateVar(parent, false/*system timezone*/).month;
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getFullYear",
  "generate" : "jswrap_date_getFullYear",
  "return" : ["int32",""]
}
The year, e.g. 2014
 */
int jswrap_date_getFullYear(JsVar *parent) {
  return getCalendarDateFromDateVar(parent, false/*system timezone*/).year;
}


/// -------------------------------------------------------

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Date",
  "name" : "setHours",
  "generate" : "jswrap_date_setHours",
  "params" : [
    ["hoursValue","int","number of hours, 0..23"],
    ["minutesValue","JsVar","number of minutes, 0..59"],
    ["secondsValue","JsVar","optional - number of seconds, 0..59"],
    ["millisecondsValue","JsVar","optional - number of milliseconds, 0..999"]
  ],
  "return" : ["float","The number of milliseconds since 1970"],
  "typescript" : "setHours(hoursValue: number, minutesValue?: number, secondsValue?: number, millisecondsValue?: number): number;"
}
0..23
 */
JsVarFloat jswrap_date_setHours(JsVar *parent, int hoursValue, JsVar *minutesValue, JsVar *secondsValue, JsVar *millisecondsValue) {
  TimeInDay td = getTimeFromDateVar(parent, false/*system timezone*/);
  td.hour = hoursValue;
  if (jsvIsNumeric(minutesValue))
    td.min = jsvGetInteger(minutesValue);
  if (jsvIsNumeric(secondsValue))
    td.sec = jsvGetInteger(secondsValue);
  if (jsvIsNumeric(millisecondsValue))
    td.ms = jsvGetInteger(millisecondsValue);
  setCorrectTimeZone(&td);
  return jswrap_date_setTime(parent, fromTimeInDay(&td));
}

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Date",
  "name" : "setMinutes",
  "generate" : "jswrap_date_setMinutes",
  "params" : [
    ["minutesValue","int","number of minutes, 0..59"],
    ["secondsValue","JsVar","optional - number of seconds, 0..59"],
    ["millisecondsValue","JsVar","optional - number of milliseconds, 0..999"]
  ],
  "return" : ["float","The number of milliseconds since 1970"],
  "typescript" : "setMinutes(minutesValue: number, secondsValue?: number, millisecondsValue?: number): number;"
}
0..59
 */
JsVarFloat jswrap_date_setMinutes(JsVar *parent, int minutesValue, JsVar *secondsValue, JsVar *millisecondsValue) {
  TimeInDay td = getTimeFromDateVar(parent, false/*system timezone*/);
  td.min = minutesValue;
  if (jsvIsNumeric(secondsValue))
    td.sec = jsvGetInteger(secondsValue);
  if (jsvIsNumeric(millisecondsValue))
    td.ms = jsvGetInteger(millisecondsValue);
  setCorrectTimeZone(&td);
  return jswrap_date_setTime(parent, fromTimeInDay(&td));
}

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Date",
  "name" : "setSeconds",
  "generate" : "jswrap_date_setSeconds",
  "params" : [
    ["secondsValue","int","number of seconds, 0..59"],
    ["millisecondsValue","JsVar","optional - number of milliseconds, 0..999"]
  ],
  "return" : ["float","The number of milliseconds since 1970"],
  "typescript" : "setSeconds(secondsValue: number, millisecondsValue?: number): number;"
}
0..59
 */
JsVarFloat jswrap_date_setSeconds(JsVar *parent, int secondsValue, JsVar *millisecondsValue) {
  TimeInDay td = getTimeFromDateVar(parent, false/*system timezone*/);
  td.sec = secondsValue;
  if (jsvIsNumeric(millisecondsValue))
    td.ms = jsvGetInteger(millisecondsValue);
  setCorrectTimeZone(&td);
  return jswrap_date_setTime(parent, fromTimeInDay(&td));
}

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Date",
  "name" : "setMilliseconds",
  "generate" : "jswrap_date_setMilliseconds",
  "params" : [
    ["millisecondsValue","int","number of milliseconds, 0..999"]
  ],
  "return" : ["float","The number of milliseconds since 1970"]
}
 */
JsVarFloat jswrap_date_setMilliseconds(JsVar *parent, int millisecondsValue) {
  TimeInDay td = getTimeFromDateVar(parent, false/*system timezone*/);
  td.ms = millisecondsValue;
  setCorrectTimeZone(&td);
  return jswrap_date_setTime(parent, fromTimeInDay(&td));
}

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Date",
  "name" : "setDate",
  "generate" : "jswrap_date_setDate",
  "params" : [
    ["dayValue","int","the day of the month, between 0 and 31"]
  ],
  "return" : ["float","The number of milliseconds since 1970"]
}
Day of the month 1..31
 */
JsVarFloat jswrap_date_setDate(JsVar *parent, int dayValue) {
  TimeInDay td = getTimeFromDateVar(parent, false/*system timezone*/);
  CalendarDate d = getCalendarDate(td.daysSinceEpoch);
  d.day = dayValue;
  td.daysSinceEpoch = fromCalenderDate(&d);
  setCorrectTimeZone(&td);
  return jswrap_date_setTime(parent, fromTimeInDay(&td));
}


/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Date",
  "name" : "setMonth",
  "generate" : "jswrap_date_setMonth",
  "params" : [
    ["yearValue","int","The month, between 0 and 11"],
    ["dayValue","JsVar","optional - the day, between 0 and 31"]
  ],
  "return" : ["float","The number of milliseconds since 1970"],
  "typescript" : "setMonth(yearValue: number, dayValue?: number): number;"
}
Month of the year 0..11
 */
JsVarFloat jswrap_date_setMonth(JsVar *parent, int monthValue, JsVar *dayValue) {
  TimeInDay td = getTimeFromDateVar(parent, false/*system timezone*/);
  CalendarDate d = getCalendarDate(td.daysSinceEpoch);
  d.month = monthValue;
  if (jsvIsNumeric(dayValue))
    d.day = jsvGetInteger(dayValue);
  td.daysSinceEpoch = fromCalenderDate(&d);
  setCorrectTimeZone(&td);
  return jswrap_date_setTime(parent, fromTimeInDay(&td));
}

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Date",
  "name" : "setFullYear",
  "generate" : "jswrap_date_setFullYear",
  "params" : [
    ["yearValue","int","The full year - eg. 1989"],
    ["monthValue","JsVar","optional - the month, between 0 and 11"],
    ["dayValue","JsVar","optional - the day, between 0 and 31"]
  ],
  "return" : ["float","The number of milliseconds since 1970"],
  "typescript" : "setFullYear(yearValue: number, monthValue?: number, dayValue?: number): number;"
}
 */
JsVarFloat jswrap_date_setFullYear(JsVar *parent, int yearValue, JsVar *monthValue, JsVar *dayValue) {
  TimeInDay td = getTimeFromDateVar(parent, false/*system timezone*/);
  CalendarDate d = getCalendarDate(td.daysSinceEpoch);
  d.year = yearValue;
  if (jsvIsNumeric(monthValue))
    d.month = jsvGetInteger(monthValue);
  if (jsvIsNumeric(dayValue))
    d.day = jsvGetInteger(dayValue);
  td.daysSinceEpoch = fromCalenderDate(&d);
  setCorrectTimeZone(&td);
  return jswrap_date_setTime(parent, fromTimeInDay(&td));
}


/// -------------------------------------------------------


/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "toString",
  "generate" : "jswrap_date_toString",
  "return" : ["JsVar","A String"],
  "typescript" : "toString(): string;"
}
Converts to a String, e.g: `Fri Jun 20 2014 14:52:20 GMT+0000`

 **Note:** This uses whatever timezone was set with `E.setTimeZone()` or
 `E.setDST()`
*/
JsVar *jswrap_date_toString(JsVar *parent) {
  TimeInDay time = getTimeFromDateVar(parent, false/*system timezone*/);
  CalendarDate date = getCalendarDate(time.daysSinceEpoch);
  char zonesign;
  int zone;
  if (time.zone<0) {
    zone = -time.zone;
    zonesign = '-';
  } else {
    zone = +time.zone;
    zonesign = '+';
  }
  return jsvVarPrintf("%s %s %d %d %02d:%02d:%02d GMT%c%04d",
      &DAYNAMES[date.dow*4], &MONTHNAMES[date.month*4], date.day, date.year,
      time.hour, time.min, time.sec,
      zonesign, ((zone/60)*100)+(zone%60));
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "toUTCString",
  "generate" : "jswrap_date_toUTCString",
  "return" : ["JsVar","A String"],
  "typescript" : "toUTCString(): string;"
}
Converts to a String, e.g: `Fri, 20 Jun 2014 14:52:20 GMT`

 **Note:** This always assumes a timezone of GMT
 */
JsVar *jswrap_date_toUTCString(JsVar *parent) {
  TimeInDay time = getTimeFromDateVar(parent, true/*GMT*/);
  CalendarDate date = getCalendarDate(time.daysSinceEpoch);

  return jsvVarPrintf("%s, %d %s %d %02d:%02d:%02d GMT", &DAYNAMES[date.dow*4], date.day, &MONTHNAMES[date.month*4], date.year, time.hour, time.min, time.sec);
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "toISOString",
  "generate" : "jswrap_date_toISOString",
  "return" : ["JsVar","A String"],
  "typescript" : "toISOString(): string;"
}
Converts to a ISO 8601 String, e.g: `2014-06-20T14:52:20.123Z`

 **Note:** This always assumes a timezone of GMT
 */
/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "toJSON",
  "generate" : "jswrap_date_toISOString",
  "return" : ["JsVar","A String"],
  "typescript" : "toJSON(): string;"
}
Calls `Date.toISOString` to output this date to JSON
*/
JsVar *jswrap_date_toISOString(JsVar *parent) {
  TimeInDay time = getTimeFromDateVar(parent, true/*GMT*/);
  CalendarDate date = getCalendarDate(time.daysSinceEpoch);

  return jsvVarPrintf("%d-%02d-%02dT%02d:%02d:%02d.%03dZ", date.year, date.month+1, date.day, time.hour, time.min, time.sec, time.ms);
}
/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Date",
  "name" : "toLocalISOString",
  "generate" : "jswrap_date_toLocalISOString",
  "return" : ["JsVar","A String"],
  "typescript" : "toLocalISOString(): string;"
}
Converts to a ISO 8601 String (with timezone information), e.g:
`2014-06-20T14:52:20.123-0500`
 */
JsVar *jswrap_date_toLocalISOString(JsVar *parent) {
  TimeInDay time = getTimeFromDateVar(parent, false/*system timezone*/);
  CalendarDate date = getCalendarDate(time.daysSinceEpoch);
  char zonesign;
  int zone;
  if (time.zone<0) {
    zone = -time.zone;
    zonesign = '-';
  } else {
    zone = +time.zone;
    zonesign = '+';
  }
  zone = 100*(zone/60) + (zone%60);
  return jsvVarPrintf("%d-%02d-%02dT%02d:%02d:%02d.%03d%c%04d", date.year, date.month+1, date.day, time.hour, time.min, time.sec, time.ms, zonesign, zone);
}

static JsVarInt _parse_int() {
  return (int)stringToIntWithRadix(jslGetTokenValueAsString(), 10, NULL, NULL);
}

static bool _parse_time(TimeInDay *time, int initialChars) {
  time->hour = (int)stringToIntWithRadix(&jslGetTokenValueAsString()[initialChars], 10, NULL, NULL);
  jslGetNextToken();
  if (lex->tk==':') {
    jslGetNextToken();
    if (lex->tk == LEX_INT) {
      time->min = _parse_int();
      jslGetNextToken();
      if (lex->tk==':') {
        jslGetNextToken();
        if (lex->tk == LEX_INT || lex->tk == LEX_FLOAT) {
          JsVarFloat f = stringToFloat(jslGetTokenValueAsString());
          time->sec = (int)f;
          time->ms = (int)(f*1000) % 1000;
          jslGetNextToken();
          if (lex->tk == LEX_ID) {
            const char *tkstr = jslGetTokenValueAsString();
            if (strcmp(tkstr,"GMT")==0 || strcmp(tkstr,"Z")==0) {
              time->zone = 0;
              jslGetNextToken();
              if (lex->tk == LEX_EOF) return true;
            } else {
              setCorrectTimeZone(time);
            }
          }
          if (lex->tk == '+' || lex->tk == '-') {
            int sign = lex->tk == '+' ? 1 : -1;
            jslGetNextToken();
            if (lex->tk == LEX_INT) {
              int i = _parse_int();
              // correct the fact that it's HHMM and turn it into just minutes
              i = (i%100) + ((i/100)*60);
              time->zone = i*sign;
              jslGetNextToken();
            } else {
              setCorrectTimeZone(time);
            }
          } else {
            setCorrectTimeZone(time);
          }
          return true;
        }
      }
    }
  }
  setCorrectTimeZone(time);
  return false;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Date",
  "name" : "parse",
  "generate" : "jswrap_date_parse",
  "params" : [
    ["str","JsVar","A String"]
  ],
  "return" : ["float","The number of milliseconds since 1970"],
  "typescript" : "parse(str: string): number;"
}
Parse a date string and return milliseconds since 1970. Data can be either
'2011-10-20T14:48:00', '2011-10-20' or 'Mon, 25 Dec 1995 13:30:00 +0430'
 */
JsVarFloat jswrap_date_parse(JsVar *str) {
  if (!jsvIsString(str)) return 0;
  TimeInDay time;
  time.daysSinceEpoch = 0;
  time.hour = 0;
  time.min = 0;
  time.sec = 0;
  time.ms = 0;
  time.zone = 0;
  time.is_dst = false;
  CalendarDate date = getCalendarDate(0);
  bool timezoneSet = false;

  JsLex lex;
  JsLex *oldLex = jslSetLex(&lex);
  jslInit(str);

  if (lex.tk == LEX_ID) {
    date.month = getMonth(jslGetTokenValueAsString());
    date.dow = getDay(jslGetTokenValueAsString());
    if (date.month>=0) {
      // Aug 9, 1995
      jslGetNextToken();
      if (lex.tk == LEX_INT) {
        date.day = _parse_int();
        jslGetNextToken();
        if (lex.tk==',') {
          jslGetNextToken();
          if (lex.tk == LEX_INT) {
            date.year = _parse_int();
            time.daysSinceEpoch = fromCalenderDate(&date);
            jslGetNextToken();
            if (lex.tk == LEX_INT) {
              _parse_time(&time, 0);
              timezoneSet = true;
            }
          }
        }
      }
    } else if (date.dow>=0) {
      // Mon, 25 Dec 1995
      date.month = 0;
      jslGetNextToken();
      if (lex.tk==',') {
        jslGetNextToken();
        if (lex.tk == LEX_INT) {
          date.day = _parse_int();
          jslGetNextToken();
          if (lex.tk == LEX_ID && getMonth(jslGetTokenValueAsString())>=0) {
            date.month = getMonth(jslGetTokenValueAsString());
            jslGetNextToken();
            if (lex.tk == LEX_INT) {
              date.year = _parse_int();
              time.daysSinceEpoch = fromCalenderDate(&date);
              jslGetNextToken();
              if (lex.tk == LEX_INT) {
                _parse_time(&time, 0);
                timezoneSet = true;
              }
            }
          }
        }
      }
    } else {
      date.dow = 0;
      date.month = 0;
    }
  } else if (lex.tk == LEX_INT) {
    // assume 2011-10-10T14:48:00 format
    date.year = _parse_int();
    jslGetNextToken();
    if (lex.tk=='-') {
      jslGetNextToken();
      if (lex.tk == LEX_INT) {
        date.month = _parse_int() - 1;
        jslGetNextToken();
        if (lex.tk=='-') {
          jslGetNextToken();
          if (lex.tk == LEX_INT) {
            date.day = _parse_int();
            time.daysSinceEpoch = fromCalenderDate(&date);
            jslGetNextToken();
            if (lex.tk == LEX_ID && jslGetTokenValueAsString()[0]=='T') {
              _parse_time(&time, 1);
              timezoneSet = true;
            }
          }
        }
      }
    }
  }

  if (!timezoneSet) setCorrectTimeZone(&time);
  jslKill();
  jslSetLex(oldLex);
  return fromTimeInDay(&time);
}
