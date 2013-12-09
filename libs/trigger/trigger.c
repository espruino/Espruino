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
#include "trigger.h"
#include "jsinteractive.h"
#include "jshardware.h"
#include "jshardware_pininfo.h"

const int TRIGGER_LOOKAHEAD = 4;

typedef struct {
  TriggerError flag;
  const char *str;
} TriggerErrorStr;

TriggerErrorStr triggerErrorStrings[] = {
  { TRIGERR_TRIG_IN_FUTURE         , "TRIG_IN_FUTURE"             },
  { TRIGERR_TRIG_IN_PAST           , "TRIG_IN_PAST"               },
  { TRIGERR_MISSED_TOOTH           , "MISSED_TOOTH"               },
  { TRIGERR_MISSED_TRIG_TOOTH      , "MISSED_TRIG_TOOTH"          },
  { TRIGERR_WHEEL_MISSED_TOOTH     , "WHEEL_MISSED_TOOTH"         },
  { TRIGERR_WHEEL_GAINED_TOOTH     , "WHEEL_GAINED_TOOTH"         },
  { TRIGERR_WHEEL_MISSED_TRIG_TOOTH, "WHEEL_MISSED_TRIG_TOOTH"    },
  { TRIGERR_SHORT_TOOTH            , "SHORT_TOOTH"                },
  { TRIGERR_TRIG_TOOTH_CHANGED     , "TRIG_TOOTH_CHANGED"         },
  { TRIGERR_INVALID_ARG            , "INVALID_ARG"                },
  { TRIGERR_WRONG_TIME             , "WRONG_TIME"                 },
  { TRIGERR_TIMER_FULL             , "TIMER_FULL"                 }
};

const char *trigGetErrorString(TriggerError flag) {
  int i, l = sizeof(triggerErrorStrings) / sizeof(TriggerErrorStr);
  for (i=0;i<l;i++)
    if (triggerErrorStrings[i].flag == flag)
      return triggerErrorStrings[i].str;
  return 0;
}


/** TODO:

Log errors (error flag?) when timing is wrong
Issue using with save()
Needs to have a minRPM to detect when wheel is stationary
*/

TriggerStruct mainTrigger = { (Pin)-1/*pin*/};

void trigOnTimingPulse(TriggerStruct *data, JsSysTime pulseTime) {
  JsSysTime currentTime = jshGetSystemTime();
  int timeDiff = (int)(pulseTime - data->lastTime);
  if (timeDiff < 0) {
    data->errors |= TRIGERR_WRONG_TIME;
    timeDiff = 0;
//    jsiConsolePrintf("0x%Lx 0x%Lx 0x%Lx\n",data->lastTime2, data->lastTime, pulseTime);
    pulseTime = data->lastTime + data->avrTrigger; // just make it up and hope!
  }

  data->lastTime2 = data->lastTime;
  data->lastTime = pulseTime;
  unsigned char teeth = (unsigned char)((((timeDiff<<1) / data->avrTrigger) + 1) >> 1); // round to find out # of teeth
  if (teeth<1) {
    data->errors |= TRIGERR_SHORT_TOOTH;
    teeth=1;
  }
  // running average
  if (data->teethSinceStart<16)
    data->avrTrigger = (data->avrTrigger + timeDiff) >> 1; 
  else
    data->avrTrigger = (data->avrTrigger*7 + timeDiff) >> 3; 
  data->avrTooth = (data->avrTooth*7 + timeDiff/teeth) >> 3; 
  if (data->teethSinceStart<0xFFFFFFFF) data->teethSinceStart++;
  // move tooth count
  unsigned char lastTooth = data->currTooth;
  data->currTooth = (unsigned char)(data->currTooth + teeth);
  // handle trigger tooth
  if (teeth > data->teethMissing) {
    if (teeth != data->teethMissing+1) data->errors |= TRIGERR_MISSED_TRIG_TOOTH;

    if (data->currTooth == data->teethTotal) {
      /* just what we expect - set back to 0.
       * Also reset wrongTriggerTeeth as we know all is good*/
      data->wrongTriggerTeeth = 0;
      data->currTooth = 0;
    } else {
      // Something has gone wrong - we got a trigger tooth when we didn't expect one
      if (data->currTooth < data->teethTotal) {
        data->errors |= TRIGERR_WHEEL_MISSED_TOOTH;
      } else { // data->currTooth > expectedTooth
        data->errors |= TRIGERR_WHEEL_GAINED_TOOTH;
      }
      // increment counter
      data->wrongTriggerTeeth++;
      /* if we've had the wrong trigger tooth for two consecutive revs,
       * change over. */
      if (data->wrongTriggerTeeth > 1) {
        data->wrongTriggerTeeth = 0;
        data->currTooth = 0;
        data->errors |= TRIGERR_TRIG_TOOTH_CHANGED;
      }
    }
  } else {
    // just a normal tooth event
    if (teeth!=1) data->errors |= TRIGERR_MISSED_TOOTH;
  }
  // handle roll-over
  if (data->teethTotal>0) { // sanity check to stop endless loop if misconfigured
    while (data->currTooth >= data->teethTotal) {
      data->errors |= TRIGERR_WHEEL_MISSED_TRIG_TOOTH;
      data->currTooth = (unsigned char)(data->currTooth + data->teethTotal);
    }
  }


  // handle sending events
  if (data->teethSinceStart > data->teethTotal) {
    // TODO: teethSinceStart>10 && hadTrigger?
    // don't start firing events until we actually know where we are!!

    unsigned char currTooth = data->currTooth;
    if (currTooth < lastTooth) currTooth = (unsigned char)(data->currTooth + data->teethTotal);
    unsigned int tooth;

    for (tooth=lastTooth+TRIGGER_LOOKAHEAD;tooth<currTooth+TRIGGER_LOOKAHEAD;tooth++) {
      // actually we want to check a few teeth into the future to give us time
      int i;
      for (i=0;i<TRIGGER_TRIGGERS_COUNT;i++) {
        TriggerPointStruct *trig = &data->triggers[i];
        if (trig->tooth != TRIGGERPOINT_TOOTH_DISABLE) {
          if ((trig->tooth == tooth) || ((trig->tooth+data->teethTotal) == tooth)) { // because we wrap
           // doAtTime(data->triggers[i].toothFraction * data->avrTooth >> 8);
            JsSysTime trigTime = pulseTime +
                                 ((trig->toothFraction * data->avrTooth) >> 8) +
                                 (((int)tooth-(int)currTooth) * (int)data->avrTooth);
            if (trigTime > pulseTime + jshGetTimeFromMilliseconds(500)) {
              trigTime = jshGetTimeFromMilliseconds(500);
              data->errors |= TRIGERR_TRIG_IN_FUTURE;
            }
            if (trigTime < currentTime) {
              data->errors |= TRIGERR_TRIG_IN_PAST;
              //jsiConsolePrint("Trigger already passed\n");
            }
            int j;
            for (j=0;j<TRIGGERPOINT_TRIGGERS_COUNT;j++) {
              if (trig->pins[j]>=0) {
                //jsiConsolePrintf("%d,%d,%d\n", tooth,tooth-currTooth,j);
                if (!jshPinOutputAtTime(trigTime, trig->pins[j], 1)) data->errors |= TRIGERR_TIMER_FULL;
                if (trig->pulseLength>0) {
                  if (!jshPinOutputAtTime(trigTime+trig->pulseLength, trig->pins[j], 0))
                    data->errors |= TRIGERR_TIMER_FULL;
                }
              }
            }
            // trigger fired, so update it
            trig->tooth  = trig->newTooth;
            trig->toothFraction = trig->newToothFraction;
          }
        }
      }
    }
  }
}

/** Actually handle a trigger event, and do stuff if it is for us */
bool trigHandleEXTI(IOEventFlags channel, JsSysTime time) {
  IOEvent event;
  event.flags = channel;

  if (mainTrigger.sensorPin>=0 && jshIsEventForPin(&event, mainTrigger.sensorPin)) {
  //  jshPinOutput(JSH_PORTB_OFFSET + 4, event.flags & EV_EXTI_IS_HIGH);

    if (!(event.flags & EV_EXTI_IS_HIGH)) // we only care about falling edges
      trigOnTimingPulse(&mainTrigger, time);
    return true; // return true anyway, so stop this being added to our event queue
  }
  return false;
}

/** At a certain time, get which tooth number we're on */
JsVarFloat trigGetToothAtTime(TriggerStruct *data, JsSysTime time) {
  JsVarFloat tooth = data->currTooth + ((JsVarFloat)(time - data->lastTime) / ((data->avrTooth>1)?data->avrTooth:0));
  return wrapAround(tooth, data->teethTotal);
}


