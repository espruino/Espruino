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
  int8_t history[HRMFILTER_TAP_NUM];
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
  return acc >> 11;
}

HRMFilter hrmFilter;

// =========================================================

HrmInfo hrmInfo;

/// Initialise heart rate monitoring
void hrm_init() {
  hrmInfo.raw = 0;
  hrmInfo.filtered = 0;
  hrmInfo.thresh = HRM_THRESH_MIN;
  hrmInfo.wasLow = false;
  hrmInfo.timeSinceBeat = 0;
  hrmInfo.timeIdx = 0;
  hrmInfo.bpm10 = 0;
  hrmInfo.confidence = 0;
  memset(hrmInfo.times, 0, sizeof(hrmInfo.times));
  HRMFilter_init(&hrmFilter);
}

uint16_t hrm_time_to_bpm10(uint8_t time) {
  return 10 * 60 * HRM_SAMPLERATE / time; // 10x BPM
}

bool hrm_had_beat() {
  // store HRM times in list
  hrmInfo.times[hrmInfo.timeIdx] = hrmInfo.timeSinceBeat;
  hrmInfo.timeIdx++;
  if (hrmInfo.timeIdx >= HRM_HIST_LEN)
    hrmInfo.timeIdx = 0;
  hrmInfo.timeSinceBeat = 0;
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
    int spread = times[max]-times[min];
    if (spread > 4) spread -= 4;
    else spread = 0;
    if (spread>10) spread=10;
    hrmInfo.confidence = 100 - spread*10;
  } else {
    hrmInfo.confidence = 0;
  }
  return n!=0; // do we have a useful HRM value?
}

/// Add new heart rate value
bool hrm_new(int hrmValue) {
  if (hrmValue<-32768) hrmValue=-32768;
  if (hrmValue>32767) hrmValue=32767;
  hrmInfo.raw = hrmValue;
  HRMFilter_put(&hrmFilter, hrmValue);
  int h = HRMFilter_get(&hrmFilter);
  if (h<=-128) h=-128;
  if (h>127) h=127;
  hrmInfo.filtered = h;

  if (hrmInfo.timeSinceBeat<255)
    hrmInfo.timeSinceBeat++;
  // check for step counter
  bool hadBeat = false;
  int thresh = hrmInfo.thresh >> HRM_THRESH_SHIFT;
  if (hrmInfo.filtered < -thresh)
    hrmInfo.wasLow = true;
  else if ((hrmInfo.filtered > thresh) && hrmInfo.wasLow) {
    hrmInfo.wasLow = false;
    hadBeat = hrm_had_beat();
  }

  if (hrmInfo.thresh > HRM_THRESH_MIN<<HRM_THRESH_SHIFT)
    hrmInfo.thresh--;
  if (h<<HRM_THRESH_SHIFT < -hrmInfo.thresh)
    hrmInfo.thresh = -h<<HRM_THRESH_SHIFT;
  return hadBeat;
}
