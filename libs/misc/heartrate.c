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
 * Heart rate
 * ----------------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "heartrate.h"
#include "jshardware.h"

/*

==========================================================
FIR filter designed with http://t-filter.engineerjs.com/

AccelFilter_get modified to return 8 bits of fractional
data.
==========================================================

Source Code Tab:

HRM
Integer
11
int8_t
int

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 50 Hz

fixed point precision: 11 bits

* 0 Hz - 0.6 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

* 0.9 Hz - 3 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 3.6 Hz - 25 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define HRMFILTER_TAP_NUM 175

typedef struct {
  HrmValueType history[HRMFILTER_TAP_NUM];
  unsigned int last_index;
} HRMFilter;

static const int8_t filter_taps[HRMFILTER_TAP_NUM] = {
  7,
  5,
  6,
  7,
  7,
  7,
  5,
  3,
  0,
  -3,
  -7,
  -10,
  -13,
  -15,
  -16,
  -15,
  -14,
  -11,
  -8,
  -4,
  0,
  3,
  5,
  6,
  6,
  5,
  3,
  1,
  -1,
  -2,
  -3,
  -2,
  -1,
  2,
  5,
  8,
  10,
  12,
  13,
  13,
  11,
  9,
  6,
  3,
  1,
  -1,
  -1,
  0,
  2,
  4,
  7,
  10,
  12,
  12,
  10,
  7,
  3,
  -2,
  -8,
  -13,
  -17,
  -19,
  -18,
  -16,
  -12,
  -8,
  -3,
  0,
  1,
  -1,
  -5,
  -13,
  -23,
  -33,
  -43,
  -51,
  -56,
  -55,
  -49,
  -37,
  -19,
  2,
  26,
  49,
  71,
  88,
  99,
  103,
  99,
  88,
  71,
  49,
  26,
  2,
  -19,
  -37,
  -49,
  -55,
  -56,
  -51,
  -43,
  -33,
  -23,
  -13,
  -5,
  -1,
  1,
  0,
  -3,
  -8,
  -12,
  -16,
  -18,
  -19,
  -17,
  -13,
  -8,
  -2,
  3,
  7,
  10,
  12,
  12,
  10,
  7,
  4,
  2,
  0,
  -1,
  -1,
  1,
  3,
  6,
  9,
  11,
  13,
  13,
  12,
  10,
  8,
  5,
  2,
  -1,
  -2,
  -3,
  -2,
  -1,
  1,
  3,
  5,
  6,
  6,
  5,
  3,
  0,
  -4,
  -8,
  -11,
  -14,
  -15,
  -16,
  -15,
  -13,
  -10,
  -7,
  -3,
  0,
  3,
  5,
  7,
  7,
  7,
  6,
  5,
  7
};

void HRMFilter_init(HRMFilter* f) {
  int i;
  for(i = 0; i < HRMFILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void HRMFilter_put(HRMFilter* f, int input) {
  f->history[f->last_index++] = input;
  if(f->last_index == HRMFILTER_TAP_NUM)
    f->last_index = 0;
}

int HRMFilter_get(HRMFilter* f) {
  long long acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < HRMFILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : HRMFILTER_TAP_NUM-1;
    acc += (long long)f->history[index] * filter_taps[i];
  };
  return acc >> 4;
}

HRMFilter hrmFilter;

// =========================================================

HrmInfo hrmInfo;

/// Initialise heart rate monitoring
void hrm_init() {
  memset(&hrmInfo, 0, sizeof(hrmInfo));
  hrmInfo.wasLow = false;
  hrmInfo.lastBeatTime = jshGetSystemTime();
  HRMFilter_init(&hrmFilter);
}

uint16_t hrm_time_to_bpm10(uint8_t time) {
  return (10 * 60 * 100) / time; // 10x BPM
}

bool hrm_had_beat() {
  // Get time since last beat
  JsSysTime time = jshGetSystemTime();
  JsVarFloat beatTime = jshGetMillisecondsFromTime(time - hrmInfo.lastBeatTime) / 10; // in 1/100th sec
  hrmInfo.lastBeatTime = time;
  if (beatTime<20) return false; // 1/5th sec is too short
  if (beatTime>255) beatTime=255;

  // store HRM times in list (in 1/100th sec)
  hrmInfo.times[hrmInfo.timeIdx] = (uint8_t)beatTime;
  hrmInfo.timeIdx++;
  if (hrmInfo.timeIdx >= HRM_HIST_LEN)
    hrmInfo.timeIdx = 0;
  // copy times over
  uint8_t times[HRM_HIST_LEN];
  memcpy(times, hrmInfo.times, sizeof(hrmInfo.times));
  // bubble sort
  bool busy;
  do {
    busy = false;
    for (int i=0;i<HRM_HIST_LEN-1;i++) {
      if (times[i] > times[i+1]) {
        uint8_t t = times[i];
        times[i] = times[i+1];
        times[i+1] = t;
        busy = true;
      }
    }
  } while (busy);
  // calculate HRM from middle values
  int min = (HRM_HIST_LEN - HRM_MEDIAN_LEN)/2;
  int max = (HRM_HIST_LEN + HRM_MEDIAN_LEN)/2;
  int n = 0;
  int sumBPM = 0;
  for (int i=min;i<max;i++) {
    if (times[i]==0) continue;
    int BPM10 = hrm_time_to_bpm10(times[i]); // 10x BPM
    sumBPM += BPM10;
    n++;
  }
  if (n) {
    hrmInfo.bpm10 = sumBPM/n;
    if (n >= HRM_MEDIAN_LEN) { // not enough values to be confident
      // spread = difference between min+max BPM*10
      int spread = hrm_time_to_bpm10(times[min]) - hrm_time_to_bpm10(times[max]);
      if (spread > 100) spread -= 100; // 10bpm difference = 100% accuracy
      else spread = 0;
      if (spread > 400) spread=400; // 40bpm difference = low accuracy
      hrmInfo.confidence = 100 - spread/4;
      if (hrmInfo.bpm10 < 300)
        hrmInfo.confidence = 0; // not confident about BPM less than 30!
    } else
      hrmInfo.confidence = 0;
  } else {
    hrmInfo.confidence = 0;
  }
  return n!=0; // do we have a useful HRM value?
}

/// Add new heart rate value
bool hrm_new(int hrmValue) {
  if (hrmValue<HRMVALUE_MIN) hrmValue=HRMVALUE_MIN;
  if (hrmValue>HRMVALUE_MAX) hrmValue=HRMVALUE_MAX;
  hrmInfo.raw = hrmValue;
  HRMFilter_put(&hrmFilter, hrmValue);
  int h = HRMFilter_get(&hrmFilter);
  if (h<=-32768) h=-32768;
  if (h>32767) h=32767;
  hrmInfo.filtered2 = hrmInfo.filtered1;
  hrmInfo.filtered1 = hrmInfo.filtered;  
  hrmInfo.filtered = h;

  // check for step counter
  bool hadBeat = false;
  hrmInfo.isBeat = false;

  if (h < hrmInfo.avg)
    hrmInfo.wasLow = true;
  else if (hrmInfo.wasLow && (hrmInfo.filtered1 >= hrmInfo.filtered) && (hrmInfo.filtered1 >= hrmInfo.filtered2)) {
    hrmInfo.wasLow = false; // peak detected, and had previously gone below average
    hrmInfo.isBeat = true;
    hadBeat = hrm_had_beat();
  }

  if (hrmPollInterval > 30) // 40 = 25Hz, Bangle.js 2 default sample rate
    hrmInfo.avg = ((hrmInfo.avg*7) + h) >> 3;
  else // 20 = 50Hz, Bangle.js 1 default sample rate
    hrmInfo.avg = ((hrmInfo.avg*15) + h) >> 4;
  

  return hadBeat;
}
