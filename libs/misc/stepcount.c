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

* 0 Hz - 1.4 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

* 1.5 Hz - 2.5 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 2.6 Hz - 6.25 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define ACCELFILTER_TAP_NUM 64

typedef struct {
  int8_t history[ACCELFILTER_TAP_NUM];
  unsigned int last_index;
} AccelFilter;

static int8_t filter_taps[ACCELFILTER_TAP_NUM] = {
    1,
    1,
    -1,
    10,
    9,
    -1,
    -16,
    -19,
    -2,
    23,
    29,
    6,
    -25,
    -34,
    -11,
    20,
    28,
    10,
    -9,
    -9,
    0,
    -4,
    -20,
    -19,
    13,
    50,
    45,
    -13,
    -72,
    -67,
    5,
    78,
    78,
    5,
    -67,
    -72,
    -13,
    45,
    50,
    13,
    -19,
    -20,
    -4,
    0,
    -9,
    -9,
    10,
    28,
    20,
    -11,
    -34,
    -25,
    6,
    29,
    23,
    -2,
    -19,
    -16,
    -1,
    9,
    10,
    -1,
    1,
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
