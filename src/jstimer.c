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
#include "jstimer.h"
#include "jsparse.h"

#define UTILTIMERTASK_TASKS (16) // MUST BE POWER OF 2
UtilTimerTask utilTimerTasks[UTILTIMERTASK_TASKS];
volatile unsigned char utilTimerTasksHead = 0;
volatile unsigned char utilTimerTasksTail = 0;


volatile UtilTimerType utilTimerType = UT_NONE;
unsigned int utilTimerBit;
bool utilTimerState;
unsigned int utilTimerData;
uint16_t utilTimerReload0H, utilTimerReload0L, utilTimerReload1H, utilTimerReload1L;


void jstUtilTimerInterruptHandler() {
  if (utilTimerType == UT_PIN_SET_RELOAD_EVENT) {
    // in order to set this timer, we must have set the arr register, fired the timer irq, and then waited for the next!
    utilTimerType = UT_PIN_SET;
  } else if (utilTimerType == UT_PIN_SET) {
    JsSysTime time = jshGetSystemTime();
    // execute any timers that are due
    while (utilTimerTasksTail!=utilTimerTasksHead && utilTimerTasks[utilTimerTasksTail].time<time) {
      UtilTimerTask *task = &utilTimerTasks[utilTimerTasksTail];
      // actually perform the task
      switch (task->type) {
        case UET_SET: {
          int j;
          for (j=0;j<UTILTIMERTASK_PIN_COUNT;j++) {
            if (task->data.set.pins[j] == PIN_UNDEFINED) break;
            jshPinSetValue(task->data.set.pins[j], (task->data.set.value >> j)&1);
          }
        } break;
        case UET_WRITE_BYTE: {
          // write data into var
          jshSetOutputValue(task->data.write.pinFunction, (unsigned char)task->data.write.var->varData.str[task->data.write.charIdx]);
          // move to next element in var
          task->data.write.charIdx++;
          unsigned char maxChars = (unsigned char)jsvGetCharactersInVar(task->data.write.var);
          if (task->data.write.charIdx >= maxChars) {
            task->data.write.charIdx = (unsigned char)(task->data.write.charIdx - maxChars);
            if (task->data.write.var->lastChild) {
              JsVar *next = jsvLock(task->data.write.var->lastChild);
              jsvUnLock(task->data.write.var);
              task->data.write.var = next;
            } else {
              jsvUnLock(task->data.write.var);
              task->data.write.var = 0;
              if (task->data.write.nextBuffer) {
                task->data.write.var = jsvLock(task->data.write.nextBuffer);
                // flip buffers
                JsVarRef t = task->data.write.nextBuffer;
                task->data.write.nextBuffer = task->data.write.currentBuffer;
                task->data.write.currentBuffer = t;
              } else {
                // No more data - make sure we don't repeat!
                task->repeatInterval = 0;
              }
            }
          }
        } break;
        case UET_WAKEUP: // we've already done our job by waking the device up
        default: break;
      }
      // If we need to repeat
      if (task->repeatInterval) {
        // update time
        task->time += task->repeatInterval;
        // do an in-place bubble sort to ensure that times are still in the right order
        unsigned char ta = utilTimerTasksTail;
        unsigned char tb = (ta+1) & (UTILTIMERTASK_TASKS-1);
        while (tb != utilTimerTasksHead) {
          if (utilTimerTasks[ta].time > utilTimerTasks[tb].time) {
            UtilTimerTask task = utilTimerTasks[ta];
            utilTimerTasks[ta] = utilTimerTasks[tb];
            utilTimerTasks[tb] = task;
          }
          ta = tb;
          tb = (tb+1) & (UTILTIMERTASK_TASKS-1);
        }
      } else {
        // Otherwise no repeat - just go straight to the next one!
        utilTimerTasksTail = (utilTimerTasksTail+1) & (UTILTIMERTASK_TASKS-1);
      }
    }

    // re-schedule the timer if there is something left to do
    if (utilTimerTasksTail != utilTimerTasksHead) {
      utilTimerType = UT_PIN_SET_RELOAD_EVENT;
      jshUtilTimerReschedule(utilTimerTasks[utilTimerTasksTail].time-time);
    } else {
      utilTimerType = UT_NONE;
      jshUtilTimerDisable();
    }
  } else {
    // Nothing left to do - disable the timer
    utilTimerType = UT_NONE;
    jshUtilTimerDisable();
  }

}

/// Return true if the utility timer is running
bool jstUtilTimerIsRunning() {
  return utilTimerType != UT_NONE;
}

void jstUtilTimerWaitEmpty() {
  WAIT_UNTIL(!jstUtilTimerIsRunning(), "Utility Timer");
}

/// Return the latest task for a pin (false if not found)
bool jstGetLastPinTimerTask(Pin pin, UtilTimerTask *task) {
  unsigned char ptr = utilTimerTasksHead;
  if (ptr == utilTimerTasksTail) return false; // nothing in here
  ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
  // now we're at the last timer task - work back until we've just gone back past utilTimerTasksTail
  while (ptr != ((utilTimerTasksTail+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1))) {
    if (utilTimerTasks[ptr].type == UET_SET) {
      int i;
      for (i=0;i<UTILTIMERTASK_PIN_COUNT;i++)
        if (utilTimerTasks[ptr].data.set.pins[i] == pin) {
          *task = utilTimerTasks[ptr];
          return true;
        } else if (utilTimerTasks[ptr].data.set.pins[i]==PIN_UNDEFINED)
          break;
    }
    ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
  }
  return false;
}

/// Is the timer full - can it accept any other signals?
static bool utilTimerIsFull() {
  unsigned char nextHead = (utilTimerTasksHead+1) & (UTILTIMERTASK_TASKS-1);
  return nextHead == utilTimerTasksTail;
}

// Queue a task up to be executed when a timer fires... return false on failure
static bool utilTimerInsertTask(UtilTimerTask *task) {
  // check if queue is full or not
  if (utilTimerIsFull()) return false;


  jshInterruptOff();

  // find out where to insert
  unsigned char insertPos = utilTimerTasksTail;
  while (insertPos != utilTimerTasksHead && utilTimerTasks[insertPos].time < task->time)
    insertPos = (insertPos+1) & (UTILTIMERTASK_TASKS-1);

  bool haveChangedTimer = insertPos==utilTimerTasksTail;
  //jsiConsolePrint("Insert at ");jsiConsolePrintInt(insertPos);jsiConsolePrint(", Tail is ");jsiConsolePrintInt(utilTimerTasksTail);jsiConsolePrint("\n");
  // shift items forward
  int i = utilTimerTasksHead;
  while (i != insertPos) {
    unsigned char next = (i+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    utilTimerTasks[i] = utilTimerTasks[next];
    i = next;
  }
  // add new item
  utilTimerTasks[insertPos] = *task;
  // increase task list size
  utilTimerTasksHead = (utilTimerTasksHead+1) & (UTILTIMERTASK_TASKS-1);

  //jsiConsolePrint("Head is ");jsiConsolePrintInt(utilTimerTasksHead);jsiConsolePrint("\n");
  // now set up timer if not already set up...
  if ((utilTimerType != UT_PIN_SET && utilTimerType != UT_PIN_SET_RELOAD_EVENT) || haveChangedTimer) {
    utilTimerType = UT_PIN_SET_RELOAD_EVENT;
    jshUtilTimerStart(utilTimerTasks[utilTimerTasksTail].time - jshGetSystemTime());
  }

  jshInterruptOn();
  return true;
}

bool jstPinOutputAtTime(JsSysTime time, Pin *pins, int pinCount, uint8_t value) {
  assert(pinCount<UTILTIMERTASK_PIN_COUNT);
  UtilTimerTask task;
  task.time = time;
  task.repeatInterval = 0;
  task.type = UET_SET;
  int i;
  for (i=0;i<UTILTIMERTASK_PIN_COUNT;i++)
    task.data.set.pins[i] = (Pin)((i<pinCount) ? pins[i] : PIN_UNDEFINED);
  task.data.set.value = value?0xFF:0;

  WAIT_UNTIL(!utilTimerIsFull(), "Utility Timer");
  return utilTimerInsertTask(&task);
}

/// Set the utility timer so we're woken up in whatever time period
bool jstSetWakeUp(JsSysTime period) {
  UtilTimerTask task;
  task.repeatInterval = 0;
  task.time = jshGetSystemTime() + period;
  task.type = UET_WAKEUP;
  bool ok = utilTimerInsertTask(&task);
  // We wait until the timer is out of the reload event, because the reload event itself would wake us up
  WAIT_UNTIL(utilTimerType != UT_PIN_SET_RELOAD_EVENT, "Utility Timer Init");
  return ok;
}

bool jstSignalWrite(JsSysTime period, Pin pin, JsVar *data) {
  UtilTimerTask task;
  task.repeatInterval = (unsigned int)period;
  task.time = jshGetSystemTime() + period;
  task.type = UET_WRITE_BYTE;
  task.data.write.pinFunction = jshGetCurrentPinFunction(pin);
  if (!task.data.write.pinFunction) return false; // no pin function found...
  task.data.write.charIdx = 0;
  task.data.write.var = jsvLockAgain(data);
  task.data.write.currentBuffer = 0;
  task.data.write.nextBuffer = 0;
  return utilTimerInsertTask(&task);
}
