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
 * JavaScript methods for Date Handling
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

typedef struct {
  int daysSinceEpoch;
  int ms,sec,min,hour;
  int zone; // timezone in minutes
} TimeInDay;

typedef struct {
  int daysSinceEpoch;
  int day,month,year,dow;
} CalendarDate;

TimeInDay getTimeFromMilliSeconds(JsVarFloat ms_in, bool forceGMT);
JsVarFloat fromTimeInDay(TimeInDay *td);
CalendarDate getCalendarDate(int d);
int fromCalenderDate(CalendarDate *date);

JsVarFloat jswrap_date_now();
JsVar *jswrap_date_from_milliseconds(JsVarFloat time);
JsVar *jswrap_date_constructor(JsVar *args);

JsVarFloat jswrap_date_getTimezoneOffset(JsVar *parent);
JsVarFloat jswrap_date_getTime(JsVar *parent);
JsVarFloat jswrap_date_setTime(JsVar *date, JsVarFloat timeValue);
int jswrap_date_getHours(JsVar *parent);
int jswrap_date_getMinutes(JsVar *parent);
int jswrap_date_getSeconds(JsVar *parent);
int jswrap_date_getMilliseconds(JsVar *parent);
int jswrap_date_getDay(JsVar *parent);
int jswrap_date_getDate(JsVar *parent);
int jswrap_date_getMonth(JsVar *parent);
int jswrap_date_getFullYear(JsVar *parent);

JsVarFloat jswrap_date_setHours(JsVar *parent, int hoursValue, JsVar *minutesValue, JsVar *secondsValue, JsVar *millisecondsValue);
JsVarFloat jswrap_date_setMinutes(JsVar *parent, int minutesValue, JsVar *secondsValue, JsVar *millisecondsValue);
JsVarFloat jswrap_date_setSeconds(JsVar *parent, int secondsValue, JsVar *millisecondsValue);
JsVarFloat jswrap_date_setMilliseconds(JsVar *parent, int millisecondsValue);
JsVarFloat jswrap_date_setDate(JsVar *parent, int dayValue);
JsVarFloat jswrap_date_setMonth(JsVar *parent, int monthValue, JsVar *dayValue);
JsVarFloat jswrap_date_setFullYear(JsVar *parent, int yearValue, JsVar *monthValue, JsVar *dayValue);

JsVar *jswrap_date_toString(JsVar *parent);
JsVar *jswrap_date_toUTCString(JsVar *parent);
JsVar *jswrap_date_toISOString(JsVar *parent);
JsVarFloat jswrap_date_parse(JsVar *str);

