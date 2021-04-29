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
 * Step Counter
 * ----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "stepcount.h"

/*

==========================================================
FIR filter designed with http://t-filter.engineerjs.com/

AccelFilter_get modified to return 8 bits of fractional
data.
==========================================================

Source Code Tab:

Accel
Integer
10
int8_t
int

Actual Filter details:

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 12.5 Hz

fixed point precision: 10 bits

* 0 Hz - 1.3 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

* 1.6 Hz - 2.4 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 2.7 Hz - 6.25 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define ACCELFILTER_TAP_NUM 57

typedef struct {
  int8_t history[ACCELFILTER_TAP_NUM];
  unsigned int last_index;
} AccelFilter;

static const int filter_taps[ACCELFILTER_TAP_NUM] = {
  1,
  0,
  -2,
  -6,
  -4,
  4,
  11,
  8,
  -5,
  -16,
  -12,
  3,
  14,
  10,
  -1,
  -3,
  2,
  1,
  -14,
  -24,
  -7,
  32,
  51,
  19,
  -44,
  -74,
  -34,
  44,
  83,
  44,
  -34,
  -74,
  -44,
  19,
  51,
  32,
  -7,
  -24,
  -14,
  1,
  2,
  -3,
  -1,
  10,
  14,
  3,
  -12,
  -16,
  -5,
  8,
  11,
  4,
  -4,
  -6,
  -2,
  0,
  1
};

static void AccelFilter_init(AccelFilter* f) {
  int i;
  for(i = 0; i < ACCELFILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

static void AccelFilter_put(AccelFilter* f, int8_t input) {
  f->history[f->last_index++] = input;
  if(f->last_index == ACCELFILTER_TAP_NUM)
    f->last_index = 0;
}

static int AccelFilter_get(AccelFilter* f) {
  int acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < ACCELFILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : ACCELFILTER_TAP_NUM-1;
    acc += (int)f->history[index] * (int)filter_taps[i];
  };
  return acc >> 2; // MODIFIED - was 10. Now returns 8 bits of fractional data
}

AccelFilter accelFilter;

// ===============================================================

// These were calculated based on contributed data
#define stepCounterThresholdMin  800
#define stepCounterAvr 5

int stepCounterThreshold;
/// has filtered acceleration passed stepCounterThresholdLow?
bool stepWasLow;

// quick integer square root
// https://stackoverflow.com/questions/31117497/fastest-integer-square-root-in-the-least-amount-of-instructions
unsigned short int int_sqrt32(unsigned int x) {
  unsigned short int res=0;
  unsigned short int add= 0x8000;
  int i;
  for(i=0;i<16;i++) {
    unsigned short int temp=res | add;
    unsigned int g2=temp*temp;
    if (x>=g2)
      res=temp;
    add>>=1;
  }
  return res;
}

// Init step count
void stepcount_init() {
  stepWasLow = false;
  stepCounterThreshold = stepCounterThresholdMin;
  AccelFilter_init(&accelFilter);
}

// do calculations
bool stepcount_new(int accMagSquared) {
  // square root accelerometer data
  int accMag = int_sqrt32(accMagSquared);
  // scale to fit and clip
  int v = (accMag-8192)>>5;
  //printf("v %d\n",v);
  //if (v>127 || v<-128) printf("Out of range %d\n", v);
  if (v>127) v = 127;
  if (v<-128) v = -128;
  // do filtering
  AccelFilter_put(&accelFilter, v);
  int accFiltered = AccelFilter_get(&accelFilter);

  // check for step counter
  bool hadStep = false;
  if (accFiltered < -stepCounterThreshold)
    stepWasLow = true;
  else if ((accFiltered > stepCounterThreshold) && stepWasLow) {
    stepWasLow = false;

    hadStep = true;
  }

  int a = accFiltered;
  if (a<0) a=-a;
  stepCounterThreshold = (stepCounterThreshold*(32-stepCounterAvr) + a*stepCounterAvr) >> 5;
  if (stepCounterThreshold < stepCounterThresholdMin)
    stepCounterThreshold = stepCounterThresholdMin;

  return hadStep;
}
