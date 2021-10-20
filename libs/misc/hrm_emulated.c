/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2019 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * VC31 heart rate sensor
 * ----------------------------------------------------------------------------
 */

#include "hrm.h"
#include "jsutils.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "jstimer.h"


#define HRM_POLL_INTERVAL 20 // in msec - 50hz

HrmCallback hrmCallback;

void hrm_timer() {
  int v = (int)(100 * sin(jshGetMillisecondsFromTime(jshGetSystemTime())/1000));
  if (v<0) v=0;
  hrmCallback(v);
}

static void hrm_sensor_timer_start() {
  JsSysTime t = jshGetTimeFromMilliseconds(HRM_POLL_INTERVAL);
  jstExecuteFn(hrm_timer, NULL, jshGetSystemTime()+t, t);
}

static void hrm_sensor_timer_stop() {
  jstStopExecuteFn(hrm_timer, 0);
}

void hrm_sensor_on(HrmCallback callback) {
  hrmCallback = callback;
  hrm_sensor_timer_start();
}

void hrm_sensor_off() {
  hrm_sensor_timer_stop();
  hrmCallback = NULL;
}

JsVar *hrm_sensor_getJsVar() {
  JsVar *o = jsvNewObject();
  if (o) {
    // add custom info here
  }
  return o;
}

/// Called when JS engine torn down (disable timer/watch/etc)
void hrm_sensor_kill() {
  if (hrmCallback!=NULL) // if is running
    hrm_sensor_timer_stop();
}

/// Called when JS engine initialised
void hrm_sensor_init() {
  if (hrmCallback!=NULL) // if is running
    hrm_sensor_timer_start();
}
