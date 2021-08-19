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

// STEPCOUNT_CONFIGURABLE is for use with https://github.com/gfwilliams/step-count
// to test/configure the step counter offline

/*

==========================================================
FIR filter designed with http://t-filter.engineerjs.com/

AccelFilter_get modified to return 8 bits of fractional
data.
==========================================================

Source Code Tab:

Accel
Integer
9
int8_t
int

*/
/*

FIR filter designed with
 http://t-filter.engineerjs.com/

sampling frequency: 12.5 Hz

fixed point precision: 9 bits

* 0 Hz - 2.7 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 3.3 Hz - 6.25 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define ACCELFILTER_TAP_NUM 23

typedef struct {
  int8_t history[ACCELFILTER_TAP_NUM];
  unsigned int last_index;
} AccelFilter;

const static int8_t filter_taps[ACCELFILTER_TAP_NUM] = {
  5,
  6,
  -4,
  -17,
  -15,
  4,
  11,
  -10,
  -27,
  8,
  80,
  118,
  80,
  8,
  -27,
  -10,
  11,
  4,
  -15,
  -17,
  -4,
  6,
  5
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
  return acc >> 1; // MODIFIED - was 9. Now returns 8 bits of fractional data
}

AccelFilter accelFilter;

// ===============================================================

// These were calculated based on contributed data
// using https://github.com/gfwilliams/step-count
#define STEPCOUNTERTHRESHOLDLO_DEFAULT  1500
#define STEPCOUNTERTHRESHOLDHI_DEFAULT  2000
#define STEPCOUNTERHISTORY_DEFAULT      3
#define STEPCOUNTERHISTORY_TIME_DEFAULT 90

// These are the ranges of values we test over
// when calculating the best data offline
#define STEPCOUNTERTHRESHOLD_MIN 0
#define STEPCOUNTERTHRESHOLD_MAX 6000
#define STEPCOUNTERTHRESHOLD_STEP 500

#define STEPCOUNTERHISTORY_MIN 1
#define STEPCOUNTERHISTORY_MAX 8
#define STEPCOUNTERHISTORY_STEP 1

#define STEPCOUNTERHISTORY_TIME_MIN 20
#define STEPCOUNTERHISTORY_TIME_MAX 100
#define STEPCOUNTERHISTORY_TIME_STEP 10

// This is a bit of a hack to allow us to configure
// variables which would otherwise have been compiler defines
#ifdef STEPCOUNT_CONFIGURABLE
int stepCounterThresholdLo = STEPCOUNTERTHRESHOLDLO_DEFAULT;
int stepCounterThresholdHi = STEPCOUNTERTHRESHOLDHI_DEFAULT;
int STEPCOUNTERHISTORY = STEPCOUNTERHISTORY_DEFAULT;
int STEPCOUNTERHISTORY_TIME = STEPCOUNTERHISTORY_TIME_DEFAULT;
// These are handy values used for graphing
int accScaled;
int accFiltered;
#else
#define stepCounterThresholdLo STEPCOUNTERTHRESHOLDLO_DEFAULT
#define stepCounterThresholdHi STEPCOUNTERTHRESHOLDHI_DEFAULT
#define STEPCOUNTERHISTORY       STEPCOUNTERHISTORY_DEFAULT
#undef STEPCOUNTERHISTORY_MAX
#define STEPCOUNTERHISTORY_MAX   STEPCOUNTERHISTORY_DEFAULT
#define STEPCOUNTERHISTORY_TIME   STEPCOUNTERHISTORY_TIME_DEFAULT
#endif

// ===============================================================

/** stepHistory allows us to check for repeated step counts.
Rather than registering each instantaneous step, we now only
measure steps if there were at least 3 steps (including the
current one) in 3 seconds

For each step it contains the number of iterations ago it occurred. 255 is the maximum
*/

uint8_t stepHistory[STEPCOUNTERHISTORY_MAX];

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
  for (int i=0;i<STEPCOUNTERHISTORY;i++)
    stepHistory[i]=255;
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
#ifdef STEPCOUNT_CONFIGURABLE
  accScaled = v;
#else
  int accFiltered;
#endif

  // do filtering
  AccelFilter_put(&accelFilter, v);
  accFiltered = AccelFilter_get(&accelFilter);

  // increment step count history counters
  for (int i=0;i<STEPCOUNTERHISTORY;i++)
    if (stepHistory[i]<255)
      stepHistory[i]++;

  // check for step counter
  bool hadStep = false;
  if (accFiltered < stepCounterThresholdLo)
    stepWasLow = true;
  else if ((accFiltered > stepCounterThresholdHi) && stepWasLow) {
    stepWasLow = false;
    // We now have something resembling a step!
    // Don't register it unless we've already had X steps within Y time period
    if (stepHistory[0] < STEPCOUNTERHISTORY_TIME)
      hadStep = true;
    // Add it to our history anyway so we can keep track of how many steps we have
    for (int i=0;i<STEPCOUNTERHISTORY-1;i++)
      stepHistory[i] = stepHistory[i+1];
    stepHistory[STEPCOUNTERHISTORY-1] = 0;
  }

  return hadStep;
}
