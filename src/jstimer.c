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
#include "jsinteractive.h"

UtilTimerTask utilTimerTasks[UTILTIMERTASK_TASKS];
volatile unsigned char utilTimerTasksHead = 0;
volatile unsigned char utilTimerTasksTail = 0;


volatile bool utilTimerOn = false;
unsigned int utilTimerBit;
bool utilTimerInIRQ = false;
unsigned int utilTimerData;
uint16_t utilTimerReload0H, utilTimerReload0L, utilTimerReload1H, utilTimerReload1L;


#ifndef SAVE_ON_FLASH

static void jstUtilTimerSetupBuffer(UtilTimerTask *task) {
  task->data.buffer.var = _jsvGetAddressOf(task->data.buffer.currentBuffer);
  if (jsvIsFlatString(task->data.buffer.var)) {
    task->data.buffer.charIdx = sizeof(JsVar);
    task->data.buffer.endIdx =  (unsigned short)(sizeof(JsVar) + jsvGetCharactersInVar(task->data.buffer.var));
  } else {
    task->data.buffer.charIdx = 0;
    task->data.buffer.endIdx = (unsigned short)jsvGetCharactersInVar(task->data.buffer.var);
  }
}

static void jstUtilTimerInterruptHandlerNextByte(UtilTimerTask *task) {
  // move to next element in var
  task->data.buffer.charIdx++;
  if (task->data.buffer.charIdx >= task->data.buffer.endIdx) {
    task->data.buffer.charIdx = (unsigned short)(task->data.buffer.charIdx - task->data.buffer.endIdx);
    /* NOTE: We don't Lock/UnLock here. We assume that the string has already been
     * referenced elsewhere (in the Waveform class) so won't get freed. Why? Because
     * we can't lock easily. We could get an IRQ right as some other code was in the
     * middle of read/modify/write of the flags member, and then the locks would get
     * out of sync. */
    if (jsvGetLastChild(task->data.buffer.var)) {
      task->data.buffer.var = _jsvGetAddressOf(jsvGetLastChild(task->data.buffer.var));
      task->data.buffer.endIdx = (unsigned short)jsvGetCharactersInVar(task->data.buffer.var);
    } else { // else no more... move on to the next
      if (task->data.buffer.nextBuffer) {
        // flip buffers
        JsVarRef t = task->data.buffer.nextBuffer;
        task->data.buffer.nextBuffer = task->data.buffer.currentBuffer;
        task->data.buffer.currentBuffer = t;
        // Setup new buffer
        jstUtilTimerSetupBuffer(task);
      } else {
        task->data.buffer.var = 0;
        // No more data - make sure we don't repeat!
        task->repeatInterval = 0;
      }
    }
  }
}

static inline unsigned char *jstUtilTimerInterruptHandlerByte(UtilTimerTask *task) {
  return (unsigned char*)&task->data.buffer.var->varData.str[task->data.buffer.charIdx];
}
#endif

void jstUtilTimerInterruptHandler() {
  if (utilTimerOn) {
    utilTimerInIRQ = true;
    JsSysTime time = jshGetSystemTime();
    // execute any timers that are due
    while (utilTimerTasksTail!=utilTimerTasksHead && utilTimerTasks[utilTimerTasksTail].time <= time) {
      UtilTimerTask *task = &utilTimerTasks[utilTimerTasksTail];
      void (*executeFn)(JsSysTime time) = 0;

      // actually perform the task
      switch (task->type) {
        case UET_SET: {
          int j;
          for (j=0;j<UTILTIMERTASK_PIN_COUNT;j++) {
            if (task->data.set.pins[j] == PIN_UNDEFINED) break;
            jshPinSetValue(task->data.set.pins[j], (task->data.set.value >> j)&1);
          }
        } break;
#ifndef SAVE_ON_FLASH
        case UET_READ_SHORT: {
          if (!task->data.buffer.var) break;
          int v = jshPinAnalogFast(task->data.buffer.pin);
          *jstUtilTimerInterruptHandlerByte(task) = (unsigned char)v;  // LSB first
          jstUtilTimerInterruptHandlerNextByte(task);
          *jstUtilTimerInterruptHandlerByte(task) = (unsigned char)(v >> 8);
          jstUtilTimerInterruptHandlerNextByte(task);
          break;
        }
        case UET_READ_BYTE: {
          if (!task->data.buffer.var) break;
          *jstUtilTimerInterruptHandlerByte(task) = (unsigned char)(jshPinAnalogFast(task->data.buffer.pin) >> 8);
          jstUtilTimerInterruptHandlerNextByte(task);
          break;
        }
        case UET_WRITE_SHORT:
        case UET_WRITE_BYTE: {
          if (!task->data.buffer.var) break;
          // get data
          int sum;
          if (task->type == UET_WRITE_SHORT) {
             sum = *jstUtilTimerInterruptHandlerByte(task);  // LSB first
             jstUtilTimerInterruptHandlerNextByte(task);
          } else {
            sum = 0;
          }
          sum |=  (unsigned short)(*jstUtilTimerInterruptHandlerByte(task) << 8);
          jstUtilTimerInterruptHandlerNextByte(task);
          task->data.buffer.currentValue = (unsigned short)sum;
          // now search for other tasks writing to this pin... (polyphony)
          int t = (utilTimerTasksTail+1) & (UTILTIMERTASK_TASKS-1);
          while (t!=utilTimerTasksHead) {
            if (UET_IS_BUFFER_WRITE_EVENT(utilTimerTasks[t].type))
              sum += ((int)(unsigned int)utilTimerTasks[t].data.buffer.currentValue) - 32768;
            t = (t+1) & (UTILTIMERTASK_TASKS-1);
          }
          // saturate
          if (sum<0) sum = 0;
          if (sum>65535) sum = 65535;
          // and output...
          jshSetOutputValue(task->data.buffer.pinFunction, sum);
          break;
        }
        case UET_EXECUTE:
          executeFn = task->data.execute;
          break;
#endif
        case UET_WAKEUP: // we've already done our job by waking the device up
        default: break;
      }
      // If we need to repeat
      if (task->repeatInterval) {
        // update time (we know time > task->time) - what if we're being asked to do too fast? skip one (or 500 :)
        unsigned int t = ((unsigned int)(time+task->repeatInterval - task->time)) / task->repeatInterval;
        if (t<1) t=1;
        task->time = task->time + (JsSysTime)task->repeatInterval*t;
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

      // execute the function if we had one (we do this now, because if we did it earlier we'd have to cope with everything changing)
      if (executeFn) executeFn(time);
    }

    // re-schedule the timer if there is something left to do
    if (utilTimerTasksTail != utilTimerTasksHead) {
      jshUtilTimerReschedule(utilTimerTasks[utilTimerTasksTail].time - time);
    } else {
      utilTimerOn = false;
      jshUtilTimerDisable();
    }
    utilTimerInIRQ = false;
  } else {
    // Nothing left to do - disable the timer
    jshUtilTimerDisable();
  }
}

/// Return true if the utility timer is running
bool jstUtilTimerIsRunning() {
  return utilTimerOn;
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

#ifndef SAVE_ON_FLASH
/// Return true if a timer task for the given variable exists (and set 'task' to it)
bool jstGetLastBufferTimerTask(JsVar *var, UtilTimerTask *task) {
  JsVarRef ref = jsvGetRef(var);
  jshInterruptOff();
  unsigned char ptr = utilTimerTasksHead;
  if (ptr != utilTimerTasksTail) {
    ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    // now we're at the last timer task - work back until we've just gone back past utilTimerTasksTail
    while (ptr != ((utilTimerTasksTail+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1))) {
      if (UET_IS_BUFFER_EVENT(utilTimerTasks[ptr].type)) {
        if (utilTimerTasks[ptr].data.buffer.currentBuffer==ref || utilTimerTasks[ptr].data.buffer.nextBuffer==ref) {
          *task = utilTimerTasks[ptr];
          jshInterruptOn();
          return true;
        }
      }
      ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    }
  }
  jshInterruptOn();
  return false;
}
#endif

/// Is the timer full - can it accept any other signals?
static bool utilTimerIsFull() {
  unsigned char nextHead = (utilTimerTasksHead+1) & (UTILTIMERTASK_TASKS-1);
  return nextHead == utilTimerTasksTail;
}

// Queue a task up to be executed when a timer fires... return false on failure
bool utilTimerInsertTask(UtilTimerTask *task) {
  // check if queue is full or not
  if (utilTimerIsFull()) return false;


  if (!utilTimerInIRQ) jshInterruptOff();

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
  if (!utilTimerOn || haveChangedTimer) {
    utilTimerOn = true;
    jshUtilTimerStart(utilTimerTasks[utilTimerTasksTail].time - jshGetSystemTime());
  }

  if (!utilTimerInIRQ) jshInterruptOn();
  return true;
}

bool jstPinOutputAtTime(JsSysTime time, Pin *pins, int pinCount, uint8_t value) {
  assert(pinCount<=UTILTIMERTASK_PIN_COUNT);
  UtilTimerTask task;
  task.time = time;
  task.repeatInterval = 0;
  task.type = UET_SET;
  int i;
  for (i=0;i<UTILTIMERTASK_PIN_COUNT;i++)
    task.data.set.pins[i] = (Pin)((i<pinCount) ? pins[i] : PIN_UNDEFINED);
  task.data.set.value = value;

  WAIT_UNTIL(!utilTimerIsFull(), "Utility Timer");
  return utilTimerInsertTask(&task);
}

/// Set the utility timer so we're woken up in whatever time period
bool jstSetWakeUp(JsSysTime period) {
  UtilTimerTask task;
  task.time = jshGetSystemTime() + period;
  task.repeatInterval = 0;
  task.type = UET_WAKEUP;

  bool hasTimer = false;
  JsSysTime nextTime;

  // work out if we're waiting for a timer,
  // and if so, when it's going to be
  jshInterruptOff();
  if (utilTimerTasksTail!=utilTimerTasksHead) {
    hasTimer = true;
    nextTime = utilTimerTasks[utilTimerTasksTail].time;
  }
  jshInterruptOn();

  if (hasTimer && task.time >= nextTime) {
    // we already had a timer, and it's going to wake us up sooner.
    // don't create a WAKEUP timer task
    return true;
  }

  bool ok = utilTimerInsertTask(&task);
  // We wait until the timer is out of the reload event, because the reload event itself would wake us up
    return ok;
}

/** If the first timer task is a wakeup task, remove it. This stops
 * us filling the timer full of wakeup events if we wake up from sleep
 * before the wakeup event */
void jstClearWakeUp() {
  bool removedTimer = false;
  jshInterruptOff();
  // while the first item is a wakeup, remove it
  while (utilTimerTasksTail!=utilTimerTasksHead &&
      utilTimerTasks[utilTimerTasksTail].type == UET_WAKEUP) {
    utilTimerTasksTail = (utilTimerTasksTail+1) & (UTILTIMERTASK_TASKS-1);
    removedTimer = true;
  }
  // if the queue is now empty, and we stop the timer
  if (utilTimerTasksTail==utilTimerTasksHead && removedTimer)
    jshUtilTimerDisable();
  jshInterruptOn();
}

#ifndef SAVE_ON_FLASH
bool jstStartSignal(JsSysTime startTime, JsSysTime period, Pin pin, JsVar *currentData, JsVar *nextData, UtilTimerEventType type) {
  assert(jsvIsString(currentData));
  assert(jsvIsUndefined(nextData) || jsvIsString(nextData));
  if (!jshIsPinValid(pin)) return false;
  UtilTimerTask task;
  task.repeatInterval = (unsigned int)period;
  task.time = startTime + period;
  task.type = type;
  if (UET_IS_BUFFER_WRITE_EVENT(type)) {
    task.data.buffer.pinFunction = jshGetCurrentPinFunction(pin);
    if (!task.data.buffer.pinFunction) return false; // no pin function found...
  } else if (UET_IS_BUFFER_READ_EVENT(type)) {
#ifndef LINUX
    if (pinInfo[pin].analog == JSH_ANALOG_NONE) return false; // no analog...
#endif
    task.data.buffer.pin = pin;
  } else {
    assert(0);
    return false;
  }
  task.data.buffer.currentBuffer = jsvGetRef(currentData);
  if (nextData) {
    // then we're repeating!
    task.data.buffer.nextBuffer = jsvGetRef(nextData);
  } else {
    // then we're not repeating
    task.data.buffer.nextBuffer = 0;
  }
  jstUtilTimerSetupBuffer(&task);

  return utilTimerInsertTask(&task);
}

/// Return true if a timer task for the given variable exists (and set 'task' to it)
bool jstStopBufferTimerTask(JsVar *var) {
  JsVarRef ref = jsvGetRef(var);
  jshInterruptOff();
  unsigned char ptr = utilTimerTasksHead;
  if (ptr != utilTimerTasksTail) {
    unsigned char endPtr = ((utilTimerTasksTail+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1));
    ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    // now we're at the last timer task - work back until we've just gone back past utilTimerTasksTail
    while (ptr != endPtr) {
      if (UET_IS_BUFFER_EVENT(utilTimerTasks[ptr].type)) {
        if (utilTimerTasks[ptr].data.buffer.currentBuffer==ref || utilTimerTasks[ptr].data.buffer.nextBuffer==ref) {
          // shift tail back along
          unsigned char next = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
          while (next!=endPtr) {
            utilTimerTasks[ptr] = utilTimerTasks[next];
            ptr = next;
            next = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
          }
          // move 'end' pointer back
          utilTimerTasksTail = (utilTimerTasksTail+1) & (UTILTIMERTASK_TASKS-1);
          jshInterruptOn();
          return true;
        }
      }
      ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    }
  }
  jshInterruptOn();
  return false;
}
#endif

void jstReset() {
  jshUtilTimerDisable();
  utilTimerTasksTail = utilTimerTasksHead = 0;
}

void jstDumpUtilityTimers() {
  int i;
  UtilTimerTask uTimerTasks[UTILTIMERTASK_TASKS];
  jshInterruptOff();
  for (i=0;i<UTILTIMERTASK_TASKS;i++)
    uTimerTasks[i] = utilTimerTasks[i];
  unsigned char uTimerTasksHead = utilTimerTasksHead;
  unsigned char uTimerTasksTail = utilTimerTasksTail;
  jshInterruptOn();

  unsigned char t = uTimerTasksTail;
  while (t!=uTimerTasksHead) {
    UtilTimerTask task = uTimerTasks[t];
    jsiConsolePrintf("%08d us : ", (int)(1000*jshGetMillisecondsFromTime(task.time-jsiLastIdleTime)));

    switch (task.type) {
    case UET_WAKEUP : jsiConsolePrintf("WAKEUP\n"); break;
    case UET_SET : jsiConsolePrintf("SET ");
         for (i=0;i<UTILTIMERTASK_PIN_COUNT;i++)
           jsiConsolePrintf("%p=%d,", task.data.set.pins[i],  (task.data.set.value>>i)&1);
         jsiConsolePrintf("\n");
         break;
#ifndef SAVE_ON_FLASH
    case UET_WRITE_BYTE : jsiConsolePrintf("WRITE_BYTE\n"); break;
    case UET_READ_BYTE : jsiConsolePrintf("READ_BYTE\n"); break;
    case UET_WRITE_SHORT : jsiConsolePrintf("WRITE_SHORT\n"); break;
    case UET_READ_SHORT : jsiConsolePrintf("READ_SHORT\n"); break;
    case UET_EXECUTE : jsiConsolePrintf("EXECUTE %x\n", task.data.execute); break;
#endif
    default : jsiConsolePrintf("Unknown type %d\n", task.type); break;
    }

    t = (t+1) & (UTILTIMERTASK_TASKS-1);
  }

}
