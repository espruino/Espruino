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

TimeInDay getTimeFromMilliSeconds(JsVarFloat ms_in);
JsVarFloat fromTimeInDay(TimeInDay *td);
CalendarDate getCalendarDate(int d);
int fromCalenderDate(CalendarDate *date);

JsVarFloat jswrap_date_now();
JsVar *jswrap_date_from_milliseconds(JsVarFloat time);
JsVar *jswrap_date_constructor(JsVar *args);

JsVarFloat jswrap_date_getTimezoneOffset(JsVar *parent);
JsVarFloat jswrap_date_getTime(JsVar *parent);
int jswrap_date_getHours(JsVar *parent);
int jswrap_date_getMinutes(JsVar *parent);
int jswrap_date_getSeconds(JsVar *parent);
int jswrap_date_getMilliseconds(JsVar *parent);
int jswrap_date_getDay(JsVar *parent);
int jswrap_date_getDate(JsVar *parent);
int jswrap_date_getMonth(JsVar *parent);
int jswrap_date_getFullYear(JsVar *parent);
JsVar *jswrap_date_toString(JsVar *parent);
JsVar *jswrap_date_toUTCString(JsVar *parent);
JsVarFloat jswrap_date_parse(JsVar *str);
