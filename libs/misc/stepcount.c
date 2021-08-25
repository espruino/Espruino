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

// a1bc34f9a9f5c54b9d68c3c26e973dba195e2105   HughB-walk-1834.csv  1446
// oxford filter                                                   1584
// peak detection                                                  1752

// STEPCOUNT_CONFIGURABLE is for use with https://github.com/gfwilliams/step-count
// to test/configure the step counter offline

/*

==========================================================
FIR filter designed with http://t-filter.engineerjs.com/

AccelFilter_get modified to return 8 bits of fractional
data.
==========================================================

*/

#define ACCELFILTER_TAP_NUM 7

typedef struct {
  int8_t history[ACCELFILTER_TAP_NUM];
  unsigned int last_index;
} AccelFilter;

const static int8_t filter_taps[ACCELFILTER_TAP_NUM] = {
    -11, -15, 44, 68, 44, -15, -11
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
  return acc >> 2;
}

AccelFilter accelFilter;

// ===============================================================

// These were calculated based on contributed data
// using https://github.com/gfwilliams/step-count
#define STEPCOUNTERTHRESHOLD_DEFAULT  300
#define STEPCOUNTERHISTORY_DEFAULT      3
#define STEPCOUNTERHISTORY_TIME_DEFAULT 90

// These are the ranges of values we test over
// when calculating the best data offline
#define STEPCOUNTERTHRESHOLD_MIN 0
#define STEPCOUNTERTHRESHOLD_MAX 1000
#define STEPCOUNTERTHRESHOLD_STEP 100

#define STEPCOUNTERHISTORY_MIN 1
#define STEPCOUNTERHISTORY_MAX 8
#define STEPCOUNTERHISTORY_STEP 1

#define STEPCOUNTERHISTORY_TIME_MIN 20
#define STEPCOUNTERHISTORY_TIME_MAX 100
#define STEPCOUNTERHISTORY_TIME_STEP 10

// This is a bit of a hack to allow us to configure
// variables which would otherwise have been compiler defines
#ifdef STEPCOUNT_CONFIGURABLE
int stepCounterThreshold = STEPCOUNTERTHRESHOLD_DEFAULT;
int STEPCOUNTERHISTORY = STEPCOUNTERHISTORY_DEFAULT;
int STEPCOUNTERHISTORY_TIME = STEPCOUNTERHISTORY_TIME_DEFAULT;
// These are handy values used for graphing
int accScaled;
#else
#define stepCounterThreshold STEPCOUNTERTHRESHOLD_DEFAULT
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

int16_t accFiltered; // last accel reading, after running through filter
int16_t accFilteredHist[2]; // last 2 accel readings, 1=newest

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
  accFiltered = 0;
  accFilteredHist[0] = 0;
  accFilteredHist[1] = 0;
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
#endif

  // do filtering
  AccelFilter_put(&accelFilter, v);
  accFilteredHist[0] = accFilteredHist[1];
  accFilteredHist[1] = accFiltered;
  int a = AccelFilter_get(&accelFilter);
  if (a>32767) a=32767;
  if (a<-32768) a=32768;
  accFiltered = a;

  // increment step count history counters
  for (int i=0;i<STEPCOUNTERHISTORY;i++)
    if (stepHistory[i]<255)
      stepHistory[i]++;

  bool hadStep = false;
  // check for a peak in the last sample - in which case report a step
  if (accFilteredHist[1] > accFilteredHist[0] &&
      accFilteredHist[1] > accFiltered &&
      accFiltered > stepCounterThreshold) {
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
