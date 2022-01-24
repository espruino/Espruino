/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2021 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * heart rate monitoring
 * ----------------------------------------------------------------------------
 */


#include "jsutils.h"

#define HRM_HIST_LEN 16 // how many BPM values do we keep a history of
#define HRM_MEDIAN_LEN 8 // how many BPM values do we average in our median filter to get a BPM reading

// Do we use 8 or 16 bits for data storage?
#ifdef HEARTRATE_DEVICE_VC31
#define HrmValueType int16_t
#define HRMVALUE_MIN -32768
#define HRMVALUE_MAX 32767
#else
#define HrmValueType int8_t
#define HRMVALUE_MIN -128
#define HRMVALUE_MAX 127
#endif

typedef struct {
  HrmValueType raw;
  int16_t filtered;
  int16_t filtered1; // before filtered
  int16_t filtered2; // before filtered1
  int16_t avg; // average signal value, moving average

  bool wasLow; // has the signal gone below the average? set =false when a beat detected
  bool isBeat; // was this sample classified as a detected beat?
  JsSysTime lastBeatTime; // timestamp of last heartbeat
  uint8_t times[HRM_HIST_LEN]; // times of previous beats, in 1/100th secs
  uint8_t timeIdx; // index in times
  uint16_t bpm10; // 10x BPM
  uint8_t confidence; // 0..100%
} HrmInfo;

extern HrmInfo hrmInfo;

uint16_t hrm_time_to_bpm10(uint8_t time);

/// Initialise heart rate monitoring
void hrm_init();

/// Add new heart rate value, return true if there was a heart beat
bool hrm_new(int hrmValue);

void hrm_sensor_on();
void hrm_sensor_off();
