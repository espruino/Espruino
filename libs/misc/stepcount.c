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
// state machine                                                   1751

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

// These are the ranges of values we test over
// when calculating the best data offline
#define STEPCOUNTERTHRESHOLD_MIN 0
#define STEPCOUNTERTHRESHOLD_MAX 1000
#define STEPCOUNTERTHRESHOLD_STEP 100

// This is a bit of a hack to allow us to configure
// variables which would otherwise have been compiler defines
#ifdef STEPCOUNT_CONFIGURABLE
int stepCounterThreshold = STEPCOUNTERTHRESHOLD_DEFAULT;
// These are handy values used for graphing
int accScaled;
#else
#define stepCounterThreshold STEPCOUNTERTHRESHOLD_DEFAULT
#endif

// ===============================================================

/** stepHistory allows us to check for repeated step counts.
Rather than registering each instantaneous step, we now only
measure steps if there were at least 3 steps (including the
current one) in 3 seconds

For each step it contains the number of iterations ago it occurred. 255 is the maximum
*/

int16_t accFiltered; // last accel reading, after running through filter
int16_t accFilteredHist[2]; // last 2 accel readings, 1=newest

// ===============================================================

/**
 * (4) State Machine
 *
 * The state machine ensure all steps are checked that they fall
 * between T_MIN_STEP and T_MAX_STEP. The 2v9.90 firmare uses X steps
 * in Y seconds but this just enforces that the step X steps ago was
 * within 6 seconds (75 samples).  It is possible to have 4 steps
 * within 1 second and then not get the 5th until T5 seconds.  This
 * could mean that the F/W would would be letting through 2 batches
 * of steps that actually would not meet the threshold as the step at
 * T5 could be the last.  The F/W version also does not give back the
 * X steps detected whilst it is waiting for X steps in Y seconds.
 * After 100 cycles of the algorithm this would amount to 500 steps
 * which is a 5% error over 10K steps.  In practice the number of
 * passes through the step machine from STEP_1 state to STEPPING
 * state can be as high as 500 events.  So using the state machine
 * approach avoids this source of error.
 *
 */

typedef enum {
  S_STILL = 0,       // just created state m/c no steps yet
  S_STEP_1 = 1,      // first step recorded
  S_STEP_22N = 2,    // counting 2-X steps
  S_STEPPING = 3,    // we've had X steps in X seconds
} StepState;

// In periods of 12.5Hz
#define T_MIN_STEP 4 // ~333ms
#define T_MAX_STEP 16 // ~1300ms
#define X_STEPS 5 // steps in a row needed

StepState stepState;
unsigned char holdSteps; // how many steps are we holding back?
unsigned char stepLength; // how many poll intervals since the last step?
// ===============================================================

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
  AccelFilter_init(&accelFilter);
  accFiltered = 0;
  accFilteredHist[0] = 0;
  accFilteredHist[1] = 0;
  stepState = S_STILL;
  holdSteps = 0;
  stepLength = 0;
}

int stepcount_had_step() {
  StepState st = stepState;

  switch (st) {
  case S_STILL:
    stepState = S_STEP_1;
    holdSteps = 1;
    return 0;

  case S_STEP_1:
    holdSteps = 1;
    // we got a step within expected period
    if (stepLength <= T_MAX_STEP && stepLength >= T_MIN_STEP) {
      //stepDebug("  S_STEP_1 -> S_STEP_22N");
      stepState = S_STEP_22N;
      holdSteps = 2;
    } else {
      // we stay in STEP_1 state
      //stepDebug("  S_STEP_1 -> S_STEP_1");
      //reject_count++;
    }
    return 0;

  case S_STEP_22N:
    // we got a step within expected time range
    if (stepLength <= T_MAX_STEP && stepLength >= T_MIN_STEP) {
      holdSteps++;

      if (holdSteps >= X_STEPS) {
        stepState = S_STEPPING;
        //pass_count++;  // we are going to STEPPING STATE
        //stepDebug("  S_STEP_22N -> S_STEPPING");
        return X_STEPS;
      }

      //stepDebug("  S_STEP_22N -> S_STEP_22N");
    } else {
      // we did not get the step in time, back to STEP_1
      stepState = S_STEP_1;
      //stepDebug("  S_STEP_22N -> S_STEP_1");
      //reject_count++;
    }
    return 0;

  case S_STEPPING:
    // we got a step within the expected window
    if (stepLength <= T_MAX_STEP && stepLength >= T_MIN_STEP) {
      stepState = S_STEPPING;
      //stepDebug("  S_STEPPING -> S_STEPPING");
      return 1;
    } else {
      // we did not get the step in time, back to STEP_1
      stepState = S_STEP_1;
      //stepDebug("  S_STEPPING -> S_STEP_1");
      //reject_count++;
    }
    return 0;
  }

  // should never get here
  //stepDebug("  ERROR");
  return 0;
}

// do calculations
int stepcount_new(int accMagSquared) {
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
  if (stepLength<255)
    stepLength++;

  int stepsCounted = 0;
  // check for a peak in the last sample - in which case report a step
  if (accFilteredHist[1] > accFilteredHist[0] &&
      accFilteredHist[1] > accFiltered &&
      accFiltered > stepCounterThreshold) {
    // We now have something resembling a step!
    stepsCounted = stepcount_had_step();
    stepLength = 0;
  }

  return stepsCounted;
}
