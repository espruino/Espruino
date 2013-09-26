/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Trigger wheel functionality
 * ----------------------------------------------------------------------------
 */
#include "jsutils.h"
#include "jshardware.h"

typedef enum {
  TRIGERR_NONE,
  TRIGERR_TRIG_IN_FUTURE           = 1<<0,
  TRIGERR_TRIG_IN_PAST             = 1<<1,
  TRIGERR_MISSED_TOOTH             = 1<<2,
  TRIGERR_MISSED_TRIG_TOOTH        = 1<<3,  //< missed tooth on trigger pulse
  TRIGERR_WHEEL_MISSED_TOOTH       = 1<<4,  //< too few teeth when trigger pulse arrived
  TRIGERR_WHEEL_GAINED_TOOTH       = 1<<5,  //< too many teeth when trigger pulse arrived
  TRIGERR_WHEEL_MISSED_TRIG_TOOTH  = 1<<6,  //< Missed trigger tooth and overflowed back to 0
  TRIGERR_SHORT_TOOTH              = 1<<7,  //< A pulse was received that looked too small to be a real tooth. Was treated as one anyway
  TRIGERR_TRIG_TOOTH_CHANGED       = 1<<8,  //< The trigger tooth changed position!
  TRIGERR_INVALID_ARG              = 1<<9,  //< An invalid argument was given to a function
  TRIGERR_WRONG_TIME               = 1<<10, //< Time is in the past
  TRIGERR_TIMER_FULL               = 1<<11, //< The timer queue is full
} TriggerError;

/** Given a single flag, return a string for it */
const char *trigGetErrorString(TriggerError flag);

#define TRIGGERPOINT_TOOTH_DISABLE ((unsigned char)(-1))

#define TRIGGERPOINT_TRIGGERS_COUNT (4)
typedef struct TriggerPointStruct {
  unsigned char tooth; //< Tooth at which trigger is fired
  unsigned char toothFraction; //< Fraction of a tooth at which trigger is fired
  unsigned char newTooth; //< After an trigger point is reached, tooth is updated to this
  unsigned char newToothFraction;
  Pin pins[TRIGGERPOINT_TRIGGERS_COUNT];
  JsSysTime pulseLength; 
} PACKED_FLAGS TriggerPointStruct;

#define TRIGGER_TRIGGERS_COUNT (8)
typedef struct TriggerStruct {
  Pin sensorPin;
  // static info
  unsigned char teethMissing; // Number of teeth missing to make up a key
  unsigned char teethTotal; // Number of teeth on wheel INCLUDING missing teeth
  unsigned int maxTooth; // maximum tooth time before we consider ourselves stopped
  JsVarFloat keyPosition; // actual position (in degrees) of the first tooth after the missing teeth
  // semi-static info
  TriggerPointStruct triggers[TRIGGER_TRIGGERS_COUNT];
  // dynamic info
  JsSysTime lastTime, lastTime2;
  unsigned int avrTrigger; // average time for a trigger pulse
  unsigned int avrTooth; // average time for a tooth
  unsigned char currTooth;
  unsigned int teethSinceStart;
  unsigned char wrongTriggerTeeth;
  TriggerError errors;
} PACKED_FLAGS TriggerStruct;

/** This is a bit hacky - ideally we'd link to this via each object, so we can have >1 trigger */
extern TriggerStruct mainTrigger;


/** Actually handle a trigger event, and do stuff if it is for us */
bool trigHandleEXTI(IOEventFlags channel, JsSysTime time);
/** At a certain time, get which tooth number we're on */
JsVarFloat trigGetToothAtTime(TriggerStruct *data, JsSysTime time);
