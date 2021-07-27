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

#include "nrf_saadc.h"

#define HRM_POLL_INTERVAL 20 // in msec - 50hz

HrmCallback hrmCallback;

void hrm_timer() {
  extern nrf_saadc_value_t nrf_analog_read();
  extern bool nrf_analog_read_start();
  extern void nrf_analog_read_end(bool adcInUse);
  extern bool nrf_analog_read_interrupted;

  nrf_saadc_input_t ain = 1 + (pinInfo[HEARTRATE_PIN_ANALOG].analog & JSH_MASK_ANALOG_CH);

  nrf_saadc_channel_config_t config;
  config.acq_time = NRF_SAADC_ACQTIME_10US;
  config.gain = NRF_SAADC_GAIN1;
  config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
  config.pin_p = ain;
  config.pin_n = ain;
  config.reference = NRF_SAADC_REFERENCE_INTERNAL;
  config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
  config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
  bool adcInUse = nrf_analog_read_start();

  // make reading
  int v;
  do {
    nrf_analog_read_interrupted = false;
    nrf_saadc_enable();
    nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_8BIT);
    nrf_saadc_channel_init(0, &config);

    v = nrf_analog_read();
  } while (nrf_analog_read_interrupted);

  nrf_analog_read_end(adcInUse);

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
  jshPinAnalog(HEARTRATE_PIN_ANALOG);
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
