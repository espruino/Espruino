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
 * VC31 heart rate sensor - additional info
 * ----------------------------------------------------------------------------
 */

#ifndef EMULATED

// Hack to fix Eclipse syntax lint
#ifndef PACKED_FLAGS
#define PACKED_FLAGS
#endif
// ---

// VC31 info shared between VC31A/B
typedef struct {
  bool isWearing;
  int8_t isWearCnt, unWearCnt; // counters for switching worn/unworn state
  uint16_t ppgValue; // current PPG value
  uint16_t ppgLastValue; // last PPG value
  int16_t ppgOffset; // PPG 'offset' value. When PPG adjusts we change the offset so it matches the last value, then slowly adjust 'ppgOffset' back down to 0
  uint8_t wasAdjusted; // true if LED/etc adjusted since the last reading
  uint16_t envValue; // env value (but VC31B has 3 value slots so we just use the one we know is ok here)
  // the meaning of these is device-dependent but it's nice to have them in one place
  uint8_t irqStatus;
  uint8_t raw[12];
  // settings
  bool allowGreenAdjust; // allow automatic adjustment of LED power
  bool allowWearDetect; // allow automatic check for whether HRM is worn
  bool pushEnvData; // call jsbangle_push_event to push data for every received ENV sample
} PACKED_FLAGS VC31Info;

extern VC31Info vcInfo;

#endif
