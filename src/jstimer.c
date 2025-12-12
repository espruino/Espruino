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

/// Data for our tasks (eg when, what they are, etc)
UtilTimerTask utilTimerTaskInfo[UTILTIMERTASK_TASKS];
/// List of tasks in time order (implemented as a circular buffer)
uint8_t utilTimerTasks[UTILTIMERTASK_TASKS];
/// queue beginning (push tasks on here)
volatile unsigned char utilTimerTasksHead = 0;
/// queue end (tasks pop off here as they are complete)
volatile unsigned char utilTimerTasksTail = 0;

/// Is the utility timer actually running?
volatile bool utilTimerOn = false;
/// When we rescheduled the timer, how far in the future were we meant to get called (in system time)?
int utilTimerPeriod;
/// The system time at which the util timer's period was last set
JsSysTime utilTimerSetTime;
/// Incremented with utilTimerPeriod - used when we're adding multiple items and we want them all relative to each other
volatile int utilTimerOffset;


#ifndef SAVE_ON_FLASH

static void jstUtilTimerSetupBuffer(UtilTimerTask *task) {
  task->data.buffer.var = _jsvGetAddressOf(task->data.buffer.currentBuffer);
  if (jsvIsFlatString(task->data.buffer.var)) {
    task->data.buffer.charIdx = sizeof(JsVar); // point to the start of the flat string (after the current var)
    task->data.buffer.endIdx =  (unsigned short)(sizeof(JsVar) + jsvGetCharactersInVar(task->data.buffer.var));
  } else {
    task->data.buffer.charIdx = 0;
    task->data.buffer.endIdx = (unsigned short)jsvGetCharactersInVar(task->data.buffer.var);
  }
}

static void jstUtilTimerInterruptHandlerNextByte(UtilTimerTask *task, uint8_t timerId) {
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
        // notify rest of interpreter
        jshPushCustomEvent(EVC_TIMER_BUFFER_FLIP | (timerId<<EVC_DATA_SHIFT));
      } else {
        task->data.buffer.var = 0;
        // No more data - make sure we don't repeat!
        task->repeatInterval = 0;
      }
      jshHadEvent(); // make sure we properly trigger a run around the idle loop now
    }
  }
}

static inline unsigned char *jstUtilTimerInterruptHandlerByte(UtilTimerTask *task) {
  if (jsvIsNativeString(task->data.buffer.var))
    return &((unsigned char*)task->data.buffer.var->varData.nativeStr.ptr)[task->data.buffer.charIdx];
  return (unsigned char*)&task->data.buffer.var->varData.str[task->data.buffer.charIdx];
}
#endif

// Handle sending an event when the task is finished
static void jstUtilTimerTaskIsFinished(int timerId) {
  UtilTimerTask *task = &utilTimerTaskInfo[timerId];
  if (UET_EVENT_SEND_TIMER_FINISHED(task->type)) {
    task->type |= UET_FINISHED;
    jshPushCustomEvent(EVC_TIMER_FINISHED | (timerId<<EVC_DATA_SHIFT));
  } else {
    task->type = UET_NONE;
  }
}

void jstUtilTimerInterruptHandler() {
  /* Note: we're using 32 bit times here, even though the real time counter is 64 bit. We
   * just make sure nothing is scheduled that far in the future */
  if (utilTimerOn) {
    utilTimerSetTime = jshGetSystemTime();
    /* We keep UtilTimer running and then jshUtilTimerReschedule reschedules
    based on the time from when this IRQ was triggered, *not* from the
    current time. */
    int t = utilTimerTasksTail;
    while (t!=utilTimerTasksHead) {
      utilTimerTaskInfo[utilTimerTasks[t]].time -= utilTimerPeriod;
      t = (t+1) & (UTILTIMERTASK_TASKS-1);
    }
    utilTimerOffset += utilTimerPeriod;
    // Check timers and execute any timers that are due
    while (utilTimerTasksTail!=utilTimerTasksHead && utilTimerTaskInfo[utilTimerTasks[utilTimerTasksTail]].time <= 0) {
      uint8_t timerId = utilTimerTasks[utilTimerTasksTail];
      UtilTimerTask *task = &utilTimerTaskInfo[timerId];
      void (*executeFn)(JsSysTime time, void* userdata) = 0;
      void *executeData = 0;
      switch (task->type) {
      case UET_SET: {
        int j;
        for (j=0;j<UTILTIMERTASK_PIN_COUNT;j++) {
          if (task->data.set.pins[j] == PIN_UNDEFINED) break;
          jshPinSetValue(task->data.set.pins[j], (task->data.set.value >> j)&1);
        }
      } break;
      case UET_EXECUTE:
        executeFn = task->data.execute.fn;
        executeData = task->data.execute.userdata;
        break;
#ifndef SAVE_ON_FLASH
      case UET_READ_SHORT: {
        if (!task->data.buffer.var) break;
        int v = jshPinAnalogFast(task->data.buffer.pin);
        *jstUtilTimerInterruptHandlerByte(task) = (unsigned char)v;  // LSB first
        jstUtilTimerInterruptHandlerNextByte(task, timerId);
        *jstUtilTimerInterruptHandlerByte(task) = (unsigned char)(v >> 8);
        jstUtilTimerInterruptHandlerNextByte(task, timerId);
        break;
      }
      case UET_READ_BYTE: {
        if (!task->data.buffer.var) break;
        *jstUtilTimerInterruptHandlerByte(task) = (unsigned char)(jshPinAnalogFast(task->data.buffer.pin) >> 8);
        jstUtilTimerInterruptHandlerNextByte(task, timerId);
        break;
      }
      case UET_WRITE_SHORT:
      case UET_WRITE_BYTE: {
        if (!task->data.buffer.var) break;
        // get data
        int sum;
        if (task->type == UET_WRITE_SHORT) {
          sum = *jstUtilTimerInterruptHandlerByte(task);  // LSB first
          jstUtilTimerInterruptHandlerNextByte(task, timerId);
        } else {
          sum = 0;
        }
        sum |=  (unsigned short)(*jstUtilTimerInterruptHandlerByte(task) << 8);
        jstUtilTimerInterruptHandlerNextByte(task, timerId);
        task->data.buffer.currentValue = (unsigned short)sum;
        // now search for other tasks writing to this pin... (polyphony)
        int t = (utilTimerTasksTail+1) & (UTILTIMERTASK_TASKS-1);
        while (t!=utilTimerTasksHead) {
          if (UET_IS_BUFFER_WRITE_EVENT(utilTimerTaskInfo[utilTimerTasks[t]].type) &&
              utilTimerTaskInfo[utilTimerTasks[t]].data.buffer.pin == task->data.buffer.pin)
            sum += ((int)(unsigned int)utilTimerTaskInfo[utilTimerTasks[t]].data.buffer.currentValue) - 32768;
          t = (t+1) & (UTILTIMERTASK_TASKS-1);
        }
        // saturate
        if (sum<0) sum = 0;
        if (sum>65535) sum = 65535;
        // and output...
        if (task->data.buffer.npin == PIN_UNDEFINED) {
          jshSetOutputValue(jshGetCurrentPinFunction(task->data.buffer.pin), sum);
        } else {
          sum -= 32768;
          jshSetOutputValue(jshGetCurrentPinFunction(task->data.buffer.pin), (sum>0) ? sum*2 : 0);
          jshSetOutputValue(jshGetCurrentPinFunction(task->data.buffer.npin), (sum<0) ? -sum*2 : 0);
        }
        break;
      }
#endif // SAVE_ON_FLASH
#ifdef ESPR_USE_STEPPER_TIMER
      case UET_STEP: {
        if (task->data.step.steps > 0) {
          task->data.step.steps--;
          task->data.step.pIndex = (task->data.step.pIndex+1) & 7;
        } else if (task->data.step.steps < 0) {
          task->data.step.steps++;
          task->data.step.pIndex = (task->data.step.pIndex+7) & 7;
        }
        uint8_t step = task->data.step.pattern[task->data.step.pIndex>>1] >> (4*(task->data.step.pIndex&1));
        jshPinSetValue(task->data.step.pins[0], (step&1)!=0);
        jshPinSetValue(task->data.step.pins[1], (step&2)!=0);
        jshPinSetValue(task->data.step.pins[2], (step&4)!=0);
        jshPinSetValue(task->data.step.pins[3], (step&8)!=0);
        if (task->data.step.steps == 0) {
          task->repeatInterval = 0; // remove task
          jshHadEvent(); // ensure idle loop runs so Stepper class can see if anything needs doing
        }
        break;
      }
#endif // ESPR_USE_STEPPER_TIMER
      case UET_WAKEUP: // we've already done our job by waking the device up
      default: break;
      }
      // If we need to repeat
      if (task->repeatInterval) {
        // update time (we know time > task->time)
        task->time += task->repeatInterval;
        // do an in-place bubble sort to ensure that times are still in the right order
        unsigned char ta = utilTimerTasksTail;
        unsigned char tb = (ta+1) & (UTILTIMERTASK_TASKS-1);
        while (tb != utilTimerTasksHead) {
          if (utilTimerTaskInfo[utilTimerTasks[ta]].time > utilTimerTaskInfo[utilTimerTasks[tb]].time) {
            uint8_t idx = utilTimerTasks[ta];
            utilTimerTasks[ta] = utilTimerTasks[tb];
            utilTimerTasks[tb] = idx;
          }
          ta = tb;
          tb = (tb+1) & (UTILTIMERTASK_TASKS-1);
        }
      } else {
        // Otherwise no repeat - mark this as done and go straight to the next one!
        jstUtilTimerTaskIsFinished(timerId);
        utilTimerTasksTail = (utilTimerTasksTail+1) & (UTILTIMERTASK_TASKS-1);
      }
      // execute the function if we had one (we do this now, because if we did it earlier we'd have to cope with everything changing)
      if (executeFn) executeFn(jshGetSystemTime(), executeData);
    }

    // re-schedule the timer if there is something left to do
    if (utilTimerTasksTail != utilTimerTasksHead) {
      utilTimerPeriod = utilTimerTaskInfo[utilTimerTasks[utilTimerTasksTail]].time;
      if (utilTimerPeriod<0) utilTimerPeriod=0;
      jshUtilTimerReschedule(utilTimerPeriod);
    } else {
      utilTimerOn = false;
      jshUtilTimerDisable();
    }
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

/// Get the current timer offset - supply this when adding >1 timer task to ensure they are all executed at the same time relative to each other
uint32_t jstGetUtilTimerOffset() {
  return utilTimerOffset;
}

/// Is the timer full - can it accept any other signals?
static bool utilTimerIsFull() {
  unsigned char nextHead = (utilTimerTasksHead+1) & (UTILTIMERTASK_TASKS-1);
  return nextHead == utilTimerTasksTail;
}

/* Restart the utility timer with the right period. This should not normally
need to be called by anything outside jstimer.c */
void jstRestartUtilTimer() {
  utilTimerPeriod = utilTimerTaskInfo[utilTimerTasks[utilTimerTasksTail]].time;
  utilTimerSetTime = jshGetSystemTime();
  if (utilTimerPeriod<0) utilTimerPeriod=0;
  jshUtilTimerStart(utilTimerPeriod);
}

/// Get the index of next free task slot, or -1 if none free
int utilTimerGetUnusedIndex(bool wait) {
  if (wait)
    WAIT_UNTIL(!utilTimerIsFull(), "Utility Timer");
  for (int i=0;i<UTILTIMERTASK_TASKS;i++)
    if (utilTimerTaskInfo[i].type == UET_NONE)
      return i;
  return -1;
}

/** Queue a task up to be executed when a timer fires... return false on failure.
 * task.time is the delay at which to execute the task. If timerOffset!==NULL then
 * task.time is relative to the time at which timerOffset=jstGetUtilTimerOffset().
 * This allows pulse trains/etc to be scheduled in sync.
 * If firstOnly=true, only insert the task if it'll be the first in the queue (otherwise set task type to UET_NONE and exit)
 */
bool utilTimerInsertTask(uint8_t taskIdx, uint32_t *timerOffset, bool firstOnly) {
  assert(!utilTimerIsFull()); // queue should not be full since we had to allocate a task to get the index
  UtilTimerTask *task = &utilTimerTaskInfo[taskIdx];

  jshInterruptOff();
  // See above - keep times in sync
  if (timerOffset)
    task->time += (int)*timerOffset - (int)utilTimerOffset;
  // How long was it since the timer was last scheduled? Update existing tasks #2575
  int timePassed = utilTimerOn ? (jshGetSystemTime() - utilTimerSetTime) : 0;
  // find out where to insert
  unsigned char insertPos = utilTimerTasksTail;
  while (insertPos != utilTimerTasksHead && utilTimerTaskInfo[utilTimerTasks[insertPos]].time < (task->time+timePassed)) {
    if (firstOnly) { // if we're only allowed to schedule as the first item, we must bail out now
      jshInterruptOn();
      utilTimerTaskInfo[taskIdx].type = UET_NONE;
      return false;
    }
    insertPos = (insertPos+1) & (UTILTIMERTASK_TASKS-1);
  }
  bool haveChangedTimer = insertPos==utilTimerTasksTail;
  //jsiConsolePrintf("Insert at %d, Tail is %d\n",insertPos,utilTimerTasksTail);
  // shift items forward
  int i = utilTimerTasksHead;
  while (i != insertPos) {
    unsigned char next = (i+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    utilTimerTasks[i] = utilTimerTasks[next];
    i = next;
  }
  // increase task list size
  utilTimerTasksHead = (utilTimerTasksHead+1) & (UTILTIMERTASK_TASKS-1);
  // update timings (#2575)
  if (haveChangedTimer) { // timer will change - update all tasks
    i = utilTimerTasksTail;
    while (i != utilTimerTasksHead) {
      if (utilTimerTaskInfo[utilTimerTasks[i]].time > timePassed)
        utilTimerTaskInfo[utilTimerTasks[i]].time -= timePassed;
      else
        utilTimerTaskInfo[utilTimerTasks[i]].time = 0;
      i = (i+1) & (UTILTIMERTASK_TASKS-1);
    }
  } else // timer hasn't changed, we have to update our task's time
    task->time += timePassed;
  // add new item
  utilTimerTasks[insertPos] = taskIdx;
  //jsiConsolePrint("Head is %d\n", utilTimerTasksHead);
  // now set up timer if not already set up...
  if (!utilTimerOn || haveChangedTimer) {
    utilTimerOn = true;
    jstRestartUtilTimer();
  }
  jshInterruptOn();
  return true;
}

/// Find a task that 'checkCallback' returns true for. Returns -1 if none found
int utilTimerFindTask(bool (checkCallback)(UtilTimerTask *task, void* data), void *checkCallbackData) {
  for (int i=0;i<UTILTIMERTASK_TASKS;i++)
    if (checkCallback(&utilTimerTaskInfo[i], checkCallbackData))
      return i;
  return -1;
}

/// Remove the task with the given ID. Also sets type to UET_NONE. Returns false if none found
bool utilTimerRemoveTask(int id) {
  jshInterruptOff();
  jstUtilTimerTaskIsFinished(id);  // queue events for task finished
  unsigned char ptr = utilTimerTasksHead;
  if (ptr != utilTimerTasksTail) {
    unsigned char endPtr = ((utilTimerTasksTail+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1));
    ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    // now we're at the last timer task - work back until we've just gone back past utilTimerTasksTail
    while (ptr != endPtr) {
      if (utilTimerTasks[ptr]==id) {
        utilTimerTaskInfo[utilTimerTasks[ptr]].type |= UET_FINISHED;
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
      ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    }
  }
  jshInterruptOn();
  return false;
}

/// If 'checkCallback' returns true for a task, set 'task' to it and return true. Returns false if none found
bool utilTimerGetLastTask(bool (checkCallback)(UtilTimerTask *task, void* data), void *checkCallbackData, UtilTimerTask *task) {
  // FIXME: we should return the task index here, and just look over utilTimerTaskInfo outside of an IRQ
  jshInterruptOff();
  unsigned char ptr = utilTimerTasksHead;
  if (ptr != utilTimerTasksTail) {
    ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    // now we're at the last timer task - work back until we've just gone back past utilTimerTasksTail
    while (ptr != ((utilTimerTasksTail+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1))) {
      if (checkCallback(&utilTimerTaskInfo[utilTimerTasks[ptr]], checkCallbackData)) {
        *task = utilTimerTaskInfo[utilTimerTasks[ptr]];
        jshInterruptOn();
        return true;
      }
      ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    }
  }
  jshInterruptOn();
  return false;
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

#ifndef SAVE_ON_FLASH
// data = *JsVarRef
static bool jstBufferTaskChecker(UtilTimerTask *task, void *data) {
  if (!UET_IS_BUFFER_EVENT(task->type)) return false;
  JsVarRef ref = *(JsVarRef*)data;
  return task->data.buffer.currentBuffer==ref || task->data.buffer.nextBuffer==ref;
}
#endif

// data = *Pin
static bool jstPinTaskChecker(UtilTimerTask *task, void *data) {
  if (task->type != UET_SET
#ifdef ESPR_USE_STEPPER_TIMER
    && task->type != UET_STEP
#endif
     ) return false;
  Pin pin = *(Pin*)data;
  for (int i=0;i<UTILTIMERTASK_PIN_COUNT;i++)
    if (task->data.set.pins[i] == pin) {
      return true;
    } else if (task->data.set.pins[i]==PIN_UNDEFINED)
      return false;
  return false;
}

// data = *fn
static bool jstExecuteTaskChecker(UtilTimerTask *task, void *data) {
  if (task->type != UET_EXECUTE) return false;
  return memcmp(&task->data.execute, (UtilTimerTaskExec*)data, sizeof(UtilTimerTaskExec))==0;
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

/// Return the latest task for a pin (false if not found)
bool jstGetLastPinTimerTask(Pin pin, UtilTimerTask *task) {
  return utilTimerGetLastTask(jstPinTaskChecker, (void*)&pin, task);
}

#ifndef SAVE_ON_FLASH
/// Return true if a timer task for the given variable exists (and set 'task' to it)
bool jstGetLastBufferTimerTask(JsVar *var, UtilTimerTask *task) {
  JsVarRef ref = jsvGetRef(var);
  return utilTimerGetLastTask(jstBufferTaskChecker, (void*)&ref, task);
}
#endif

bool jstPinOutputAtTime(JsSysTime time, uint32_t *timerOffset, Pin *pins, int pinCount, uint8_t value) {
  assert(pinCount<=UTILTIMERTASK_PIN_COUNT);
  int idx = utilTimerGetUnusedIndex(true/*wait*/);
  if (idx<0) return false; // no free tasks even after waiting
  UtilTimerTask *task = &utilTimerTaskInfo[idx];
  task->type = UET_SET;
  task->time = (int)time;
  task->repeatInterval = 0;
  int i;
  for (i=0;i<UTILTIMERTASK_PIN_COUNT;i++)
    task->data.set.pins[i] = (Pin)((i<pinCount) ? pins[i] : PIN_UNDEFINED);
  task->data.set.value = value;
  utilTimerInsertTask(idx, timerOffset, false/*doesn't have to be first*/);
  return true;
}

// Do software PWM on the given pin, using the timer IRQs
bool jstPinPWM(JsVarFloat freq, JsVarFloat dutyCycle, Pin pin) {
  // if anything is wrong, exit now
  if (dutyCycle<=0 || dutyCycle>=1 || freq<=0) {
    // remove any timer tasks
    int taskID;
    while ((taskID = utilTimerFindTask(jstPinTaskChecker, (void*)&pin)) >= 0)
      utilTimerRemoveTask(taskID);
    // Now set pin to the correct state and exit
    jshPinSetValue(pin, dutyCycle >= 0.5);
    return false;
  }
  // do calculations...
  JsSysTime pulseLength = jshGetTimeFromMilliseconds(dutyCycle * 1000.0 / freq);
  JsSysTime period = jshGetTimeFromMilliseconds(1000.0 / freq);
  if (period > 0xFFFFFFFF) {
    jsWarn("Frequency of %f Hz is too slow", freq);
    period = 0xFFFFFFFF;
  }

  // First, search for existing PWM tasks
  UtilTimerTask *ptaskon=0, *ptaskoff=0;
  jshInterruptOff();
  unsigned char ptr = utilTimerTasksHead;
  if (ptr != utilTimerTasksTail) {
    ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    // now we're at the last timer task - work back until we've just gone back past utilTimerTasksTail
    while (ptr != ((utilTimerTasksTail+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1))) {
      if (jstPinTaskChecker(&utilTimerTaskInfo[utilTimerTasks[ptr]], (void*)&pin)) {
        if (utilTimerTaskInfo[utilTimerTasks[ptr]].data.set.value)
          ptaskon = &utilTimerTaskInfo[utilTimerTasks[ptr]];
        else
          ptaskoff = &utilTimerTaskInfo[utilTimerTasks[ptr]];
      }
      ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    }
  }
  if (ptaskon && ptaskoff) {
    // Great! We have PWM... Just update it
    if (ptaskoff->time > ptaskon->time)
      ptaskoff->time = ptaskon->time + pulseLength;
    else
      ptaskoff->time = ptaskon->time + pulseLength - (unsigned int)period;
    ptaskon->repeatInterval = (unsigned int)period;
    ptaskoff->repeatInterval = (unsigned int)period;
    /* don't bother rescheduling - everything will work out next time
     * the timer fires anyway. */
    // All done - just return!
    jshInterruptOn();
    return true;
  }
  jshInterruptOn();

  /// Remove any tasks using the given pin (if they existed)
  if (ptaskon || ptaskoff) {
    while (jstStopPinTimerTask(pin));
  }

  int onidx = utilTimerGetUnusedIndex(true/*wait*/);
  if (onidx<0) return false; // no free tasks
  UtilTimerTask *taskon = &utilTimerTaskInfo[onidx];
  taskon->type = UET_SET;
  int offidx = utilTimerGetUnusedIndex(true/*wait*/);
  if (offidx<0) {
    taskon->type = UET_NONE; // free last task
    return false; // no free tasks
  }
  UtilTimerTask *taskoff = &utilTimerTaskInfo[offidx];
  taskoff->type = UET_SET;
  taskon->data.set.value = 1;
  taskoff->data.set.value = 0;
  taskon->time = (int)period;
  taskoff->time = (int)pulseLength;
  taskon->repeatInterval = (unsigned int)period;
  taskoff->repeatInterval = (unsigned int)period;
  taskon->data.set.pins[0] = pin;
  taskoff->data.set.pins[0] = pin;
  int i;
  for (i=1;i<UTILTIMERTASK_PIN_COUNT;i++) {
    taskon->data.set.pins[i] = PIN_UNDEFINED;
    taskoff->data.set.pins[i] = PIN_UNDEFINED;
  }

  // first task is to turn on
  jshPinSetValue(pin, 1);
  uint32_t timerOffset = jstGetUtilTimerOffset();
  // now start the 2 PWM tasks
  utilTimerInsertTask(onidx, &timerOffset, false/*doesn't have to be first*/);
  utilTimerInsertTask(offidx, &timerOffset, false/*doesn't have to be first*/);
  return true;
}

/** Execute the given function repeatedly after the given time period. If period=0, don't repeat. True on success or false on failure to schedule
 * See utilTimerInsertTask for notes on timerOffset
 */
bool jstExecuteFn(UtilTimerTaskExecFn fn, void *userdata, JsSysTime startTime, uint32_t period, uint32_t *timerOffset) {
  int idx = utilTimerGetUnusedIndex(true/*wait*/);
  if (idx<0) return false; // no free tasks even after waiting
  UtilTimerTask *task = &utilTimerTaskInfo[idx];
  task->time = (int)startTime;
  task->repeatInterval = period;
  task->type = UET_EXECUTE;
  task->data.execute.fn = fn;
  task->data.execute.userdata = userdata;
  utilTimerInsertTask(idx, timerOffset, false/*doesn't have to be first*/);
  return true;
}

/// Stop executing the given function
bool jstStopExecuteFn(UtilTimerTaskExecFn fn, void *userdata) {
  UtilTimerTaskExec e;
  e.fn = fn;
  e.userdata = userdata;
  return utilTimerRemoveTask(utilTimerFindTask(jstExecuteTaskChecker, (void*)&e));
}

/// Set the utility timer so we're woken up in whatever time period
void jstSetWakeUp(JsSysTime period) {
  bool hasTimer = false;
  int wakeupTime = (int)period;
  int nextTime;

  int idx = utilTimerGetUnusedIndex(false/*don't wait*/);
  if (idx<0) return; // no free tasks!
  UtilTimerTask *task = &utilTimerTaskInfo[idx];
  task->type = UET_WAKEUP;
  task->time = wakeupTime;
  task->repeatInterval = 0;
  utilTimerInsertTask(idx, NULL, true/* must be first! If not something else will wake us first so don't bother. */);
}

/** If the first timer task is a wakeup task, remove it. This stops
 * us filling the timer full of wakeup events if we wake up from sleep
 * before the wakeup event */
void jstClearWakeUp() {
  bool removedTimer = false;
  jshInterruptOff();
  // while the first item is a wakeup, remove it
  while (utilTimerTasksTail!=utilTimerTasksHead &&
      utilTimerTaskInfo[utilTimerTasks[utilTimerTasksTail]].type == UET_WAKEUP) {
    utilTimerTaskInfo[utilTimerTasks[utilTimerTasksTail]].type = UET_NONE;
    utilTimerTasksTail = (utilTimerTasksTail+1) & (UTILTIMERTASK_TASKS-1);
    removedTimer = true;
  }
  // if the queue is now empty, and we stop the timer
  if (utilTimerTasksTail==utilTimerTasksHead && removedTimer)
    jshUtilTimerDisable();
  jshInterruptOn();
}

#ifndef SAVE_ON_FLASH

int jstStartSignal(JsSysTime startTime, JsSysTime period, Pin pin, Pin npin, JsVar *currentData, JsVar *nextData, UtilTimerEventType type) {
  assert(jsvIsString(currentData));
  assert(jsvIsUndefined(nextData) || jsvIsString(nextData));
  if (!jshIsPinValid(pin)) return -1;
  int idx = utilTimerGetUnusedIndex(true/*wait*/);
  if (idx<0) return -1; // no free tasks!
  UtilTimerTask *task = &utilTimerTaskInfo[idx];
  task->type = type;
  task->repeatInterval = (unsigned int)period;
  task->time = (int)(startTime + period);
  if (UET_IS_BUFFER_WRITE_EVENT(type)) {
    task->data.buffer.pin = pin;
    task->data.buffer.npin = npin;
  } else if (UET_IS_BUFFER_READ_EVENT(type)) {
#ifndef LINUX
    if (pinInfo[pin].analog == JSH_ANALOG_NONE) return -1; // no analog...
#endif
    task->data.buffer.pin = pin;
  } else {
    assert(0);
    return -1;
  }
  task->data.buffer.currentBuffer = jsvGetRef(currentData);
  if (nextData) {
    // then we're repeating!
    task->data.buffer.nextBuffer = jsvGetRef(nextData);
  } else {
    // then we're not repeating
    task->data.buffer.nextBuffer = 0;
  }
  jstUtilTimerSetupBuffer(task);
  utilTimerInsertTask(idx, NULL, false/*doesn't have to be first*/);
  return idx;
}



/// Remove the task that uses the buffer 'var'
bool jstStopBufferTimerTask(JsVar *var) {
  JsVarRef ref = jsvGetRef(var);
  return utilTimerRemoveTask(utilTimerFindTask(jstBufferTaskChecker, (void*)&ref));
}
#endif
/// Remove the task that uses the given pin
bool jstStopPinTimerTask(Pin pin) {
  return utilTimerRemoveTask(utilTimerFindTask(jstPinTaskChecker, (void*)&pin));
}

static void jstOnTaskFinished(int id) {
  if ((utilTimerTaskInfo[id].type&~UET_FINISHED) == UET_EXECUTE) { // if EXEC with a JS function, free the function
    JsVar *timerFns = jsvObjectGetChildIfExists(execInfo.hiddenRoot, JSI_TIMER_RUN_JS_NAME);
    if (timerFns) {
      jsvRemoveArrayItem(timerFns, id);
      jsvUnLock(timerFns);
    }
  }
  utilTimerTaskInfo[id].type = UET_NONE;
}


void jstReset() {
  jshUtilTimerDisable();
  utilTimerOn = false;
  utilTimerTasksTail = utilTimerTasksHead = 0;
  for (int i=0;i<UTILTIMERTASK_TASKS;i++) {
    if (utilTimerTaskInfo[i].type != UET_NONE)
      jstOnTaskFinished(i);
  }
  utilTimerOffset = 0;
  utilTimerPeriod = 0;
  utilTimerSetTime = jshGetSystemTime();
}

/** when system time is changed, also change the time in the timers.
This should be done with interrupts off */
void jstSystemTimeChanged(JsSysTime diff) {
  // we don't actually care since timers should hopefully just run based on delays
}

/// Called from the idle loop if a custom event is received (could be for the timer)
void jstOnCustomEvent(IOEventFlags eventFlags, uint8_t *data, int dataLen) {
  IOCustomEventFlags customFlags = *(IOCustomEventFlags*)data;
  if ((customFlags&EVC_TYPE_MASK)==EVC_TIMER_FINISHED) {
    int id = customFlags >> EVC_DATA_SHIFT;
    if (utilTimerTaskInfo[id].type & UET_FINISHED) {
      jstOnTaskFinished(id);

    }
  }
}
