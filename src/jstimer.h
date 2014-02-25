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
 * Common utility timer handling functions
 * ----------------------------------------------------------------------------
 */
#ifndef JSTIMER_H_
#define JSTIMER_H_

#include "jsutils.h"
#include "jshardware.h"

typedef enum {
  UT_NONE,
  UT_PIN_SET_RELOAD_EVENT,
  UT_PIN_SET,
} PACKED_FLAGS UtilTimerType;

typedef enum {
  UET_SET, ///< Set a pin to a value
  UET_WRITE_BYTE, ///< Write a byte to an address
} PACKED_FLAGS UtilTimerEventType;

#define UTILTIMERTASK_PIN_COUNT (4)

typedef struct UtilTimerTaskSet {
  Pin pins[UTILTIMERTASK_PIN_COUNT]; ///< pins to set
  uint8_t value; ///< value to set pins to
} PACKED_FLAGS UtilTimerTaskSet;


typedef struct UtilTimerTaskWrite {
  void *addr; ///< Address to write to (some peripheral?)
  JsVar *var; ///< variable to get data from
  unsigned char charIdx; ///< Index of character in variable
} PACKED_FLAGS UtilTimerTaskWrite;

typedef union UtilTimerTaskData {
  UtilTimerTaskSet set;
  UtilTimerTaskWrite write;
} UtilTimerTaskData;

typedef struct UtilTimerTask {
  JsSysTime time; // time at which to set pins
  unsigned int repeatInterval; // if nonzero, repeat the timer
  UtilTimerEventType type;
  UtilTimerTaskData data; // data used when timer is hit
} PACKED_FLAGS UtilTimerTask;

void jstUtilTimerInterruptHandler();

/// Wait until the utility timer is totally empty (use with care as timers can repeat)
void jstUtilTimerWaitEmpty();

/// Return true if a timer task for the given pin exists (and set 'task' to it)
bool jstGetLastPinTimerTask(Pin pin, UtilTimerTask *task);

/// returns false if timer queue was full... Changes the state of one or more pins at a certain time (using a timer)
bool jstPinOutputAtTime(JsSysTime time, Pin *pins, int pinCount, uint8_t value);

/// Start writing a string out at the given period between samples
bool jstSignalWrite(JsSysTime period, Pin pin, JsVar *data);

#endif /* JSTIMER_H_ */

