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
 * heart rate monitoring using VC31 proprietary binary blob
 * ----------------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "heartrate.h"
#include "hrm_vc31.h"
#include "hrm.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "vc31_binary/algo.h"

HrmInfo hrmInfo;

/// Initialise heart rate monitoring
void hrm_init() {
  memset(&hrmInfo, 0, sizeof(hrmInfo));
  hrmInfo.isWorn = false;
  hrmInfo.lastPPGTime = jshGetSystemTime();  
}

/// Add new heart rate value
bool hrm_new(int ppgValue, Vector3 *acc) {
  // work out time passed since last sample (it might not be exactly hrmUpdateInterval)
  JsSysTime time = jshGetSystemTime();
  int timeDiff = (int)(jshGetMillisecondsFromTime(time-hrmInfo.lastPPGTime)+0.5);
  hrmInfo.lastPPGTime = time;
  // if we've just started wearing again, reset the algorith
  if (vcInfo.isWearing && !hrmInfo.isWorn) {
    hrmInfo.isWorn = true;
    // initialise VC31 algorithm (should do when going from wearing to not wearing)
    Algo_Init();
    hrmInfo.lastHRM = 0;
    hrmInfo.lastConfidence = 0; 
    hrmInfo.msSinceLastHRM = 0;
    hrmInfo.avg = ppgValue;
  } else {
    hrmInfo.isWorn = vcInfo.isWearing;
    if (!hrmInfo.isWorn) return false;
  }
  int f = (ppgValue-hrmInfo.avg)*256;
  if (f < HRMVALUE_MIN) f = HRMVALUE_MIN;
  if (f > HRMVALUE_MAX) f = HRMVALUE_MAX;
  hrmInfo.filtered = f;
  hrmInfo.avg = ((int)hrmInfo.avg*7 + ppgValue) >> 3;  
  // Feed data into algorithm
  AlgoInputData_t inputData;
  inputData.axes.x = acc->y >> 5;  // perpendicular to the direction of the arm
  inputData.axes.y = acc->x >> 5;  // along the direction of the arm
  inputData.axes.z = acc->z >> 5;
  inputData.ppgSample = vcInfo.ppgValue | (vcInfo.wasAdjusted ? 0x1000 : 0);
  inputData.envSample = vcInfo.envValue;
  hrmInfo.msSinceLastHRM += timeDiff;
  Algo_Input(&inputData, timeDiff, SPORT_TYPE_NORMAL,0/*surfaceRecogMode*/,0/*opticalAidMode*/);
  AlgoOutputData_t outputData;
  Algo_Output(&outputData);
  //jsiConsolePrintf("HRM %d %d %d\n", outputData.hrData, outputData.reliability, hrmInfo.msSinceLastHRM);
  if (outputData.hrData!=hrmInfo.lastHRM ||
      outputData.reliability!=hrmInfo.lastConfidence ||
      ((hrmInfo.msSinceLastHRM > 2000) && outputData.hrData && outputData.reliability)) {
    // update when figures change OR when 2 secs have passed (readings are usually every 1 sec)
    hrmInfo.lastConfidence = outputData.reliability;
    hrmInfo.lastHRM = outputData.hrData;
    hrmInfo.bpm10 = 10 * outputData.hrData;
    hrmInfo.confidence = outputData.reliability;
    hrmInfo.msSinceLastHRM = 0;
    return true;
  }
  return false;
}

// Append extra information to an existing HRM event object
void hrm_get_hrm_info(JsVar *o) {
  NOT_USED(o);
}

// Append extra information to an existing HRM-raw event object
void hrm_get_hrm_raw_info(JsVar *o) {
}