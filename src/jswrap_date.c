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
const int YDAY = 365;
const int LDAY = 366;
const int FDAY = 4*365+1;
const int BASE_DOW = 4;
const short DAYS[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
const short LPDAYS[13] = {0,31,60,91,121,152,182,213,244,274,305,335,366};
const short YDAYS[4] = {0,365,365*2,365*3+1};
const char *MONTHNAMES = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec";
const char *DAYNAMES = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";

/* NOTE: we use / and % here because the compiler is smart enough to
 * condense them into one op. */

TimeInDay getTimeFromMilliSeconds(JsVarFloat ms_in) {
  TimeInDay t;
  t.daysSinceEpoch = (int)(ms_in / MSDAY);
  
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
  t.zone = 0;
  return t;
}

JsVarFloat fromTimeInDay(TimeInDay *td) {
  return (JsVarFloat)(td->ms + (((td->hour*60+td->min - td->zone)*60+td->sec)*1000) + (JsVarFloat)td->daysSinceEpoch*MSDAY);
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
  d=d - (y * FDAY);
  if (d<0) {
    d += FDAY;
    y--;
  }
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
  while (mdays[m]<d+1 && m<12) {
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

  return f*FDAY+YDAYS[yf]+mdays[date->month%12]+date->day-1;
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

/*JSON{
  "type" : "class",
  "class" : "Date"
}
The built-in class for handling Dates
 */

/*JSON{
  "type" : "staticmethod",
  "class" : "Date",
  "name" : "now",
  "generate" : "jswrap_date_now",
  "return" : ["float",""]
}
Get the number of milliseconds elapsed since 1970 (or on embedded platforms, since startup)
 */
JsVarFloat jswrap_date_now() {
  // Not quite sure why we need this, but (JsVarFloat)jshGetSystemTime() / (JsVarFloat)jshGetTimeFromMilliseconds(1) in inaccurate on STM32
  return ((JsVarFloat)jshGetSystemTime() / (JsVarFloat)jshGetTimeFromMilliseconds(1000)) * 1000;
}


JsVar *jswrap_date_from_milliseconds(JsVarFloat time) {
  JsVar *d = jspNewObject(0,"Date");
  if (!d) return 0;
  jsvObjectSetChildAndUnLock(d, "ms", jsvNewFromFloat(time));
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
  "return_object" : "Date"
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
    date.month = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 1)) % 12);
    date.day = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 2)) % 31);
    TimeInDay td;
    td.daysSinceEpoch = fromCalenderDate(&date);
    td.hour = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 3)) % 24);
    td.min = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 4)) % 60);
    td.sec = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 5)) % 60);
    td.ms = (int)(jsvGetIntegerAndUnLock(jsvGetArrayItem(args, 6)) % 1000);
    td.zone = 0;
    time = fromTimeInDay(&td);
  }

  return jswrap_date_from_milliseconds(time);
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getTimezoneOffset",
  "generate" : "jswrap_date_getTimezoneOffset",
  "return" : ["float","The difference, in minutes, between UTC and local time"]
}
The getTimezoneOffset() method returns the time-zone offset from UTC, in minutes, for the current locale.
 */
JsVarFloat jswrap_date_getTimezoneOffset(JsVar *parent) {
  NOT_USED(parent);
  return 0;
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

static TimeInDay getTimeFromDateVar(JsVar *date) {
  return getTimeFromMilliSeconds(jswrap_date_getTime(date));
}

static CalendarDate getCalendarDateFromDateVar(JsVar *date) {
  return getCalendarDate(getTimeFromDateVar(date).daysSinceEpoch);
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
  return getTimeFromDateVar(parent).hour;
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
  return getTimeFromDateVar(parent).min;
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
  return getTimeFromDateVar(parent).sec;
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
  return getTimeFromDateVar(parent).ms;
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
  return getCalendarDateFromDateVar(parent).dow;
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
  return getCalendarDateFromDateVar(parent).day;
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
  return getCalendarDateFromDateVar(parent).month;
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "getFullYear",
  "generate" : "jswrap_date_getFullYear",
  "return" : ["int32",""]
}
The year, eg. 2014
 */
int jswrap_date_getFullYear(JsVar *parent) {
  return getCalendarDateFromDateVar(parent).year;
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "toString",
  "generate" : "jswrap_date_toString",
  "return" : ["JsVar","A String"]
}
Converts to a String, eg: `Fri Jun 20 2014 14:52:20 GMT+0000`

 **Note:** This always assumes a timezone of GMT+0000
 */
JsVar *jswrap_date_toString(JsVar *parent) {
  TimeInDay time = getTimeFromDateVar(parent);
  CalendarDate date = getCalendarDate(time.daysSinceEpoch);

  return jsvVarPrintf("%s %s %d %d %02d:%02d:%02d GMT+0000", &DAYNAMES[date.dow*4], &MONTHNAMES[date.month*4], date.day, date.year, time.hour, time.min, time.sec);
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "toUTCString",
  "generate" : "jswrap_date_toUTCString",
  "return" : ["JsVar","A String"]
}
Converts to a String, eg: `Fri, 20 Jun 2014 14:52:20 GMT`

 **Note:** This always assumes a timezone of GMT
 */
JsVar *jswrap_date_toUTCString(JsVar *parent) {
  TimeInDay time = getTimeFromDateVar(parent);
  CalendarDate date = getCalendarDate(time.daysSinceEpoch);

  return jsvVarPrintf("%s, %d %s %d %02d:%02d:%02d GMT", &DAYNAMES[date.dow*4], date.day, &MONTHNAMES[date.month*4], date.year, time.hour, time.min, time.sec);
}

/*JSON{
  "type" : "method",
  "class" : "Date",
  "name" : "toISOString",
  "generate" : "jswrap_date_toISOString",
  "return" : ["JsVar","A String"]
}
Converts to a ISO 8601 String, eg: `2014-06-20T14:52:20.123Z`

 **Note:** This always assumes a timezone of GMT
 */
JsVar *jswrap_date_toISOString(JsVar *parent) {
  TimeInDay time = getTimeFromDateVar(parent);
  CalendarDate date = getCalendarDate(time.daysSinceEpoch);

  return jsvVarPrintf("%d-%02d-%02dT%02d:%02d:%02d.%03dZ", date.year, date.month+1, date.day, time.hour, time.min, time.sec, time.ms);
}

static JsVarInt _parse_int() {
  return (int)stringToIntWithRadix(jslGetTokenValueAsString(), 10, 0);
}

static bool _parse_time(TimeInDay *time, int initialChars) {
  time->hour = (int)stringToIntWithRadix(&jslGetTokenValueAsString()[initialChars], 10, 0);
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
          if (lex->tk == LEX_ID && strcmp(jslGetTokenValueAsString(),"GMT")==0) {
            jslGetNextToken();
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
            }
          }

          return true;
        }
      }
    }
  }
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
  "return" : ["float","The number of milliseconds since 1970"]
}
Parse a date string and return milliseconds since 1970. Data can be either '2011-10-20T14:48:00', '2011-10-20' or 'Mon, 25 Dec 1995 13:30:00 +0430' 
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
  CalendarDate date = getCalendarDate(0);

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
            jslGetNextToken();
            if (lex.tk == LEX_INT) {
              _parse_time(&time, 0);
            }
          }
        }
      }
    } else if (date.dow>=0) {
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
              jslGetNextToken();
              if (lex.tk == LEX_INT) {
                _parse_time(&time, 0);
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
            jslGetNextToken();
            if (lex.tk == LEX_ID && jslGetTokenValueAsString()[0]=='T') {
              _parse_time(&time, 1);
            }
          }
        }
      }
    }
  }

  jslKill();
  jslSetLex(oldLex);
  time.daysSinceEpoch = fromCalenderDate(&date);
  return fromTimeInDay(&time);
}
