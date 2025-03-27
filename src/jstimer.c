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
UtilTimerTask utilTimerTasks[UTILTIMERTASK_TASKS];
/// queue beginning (push tasks on here)
volatile unsigned char utilTimerTasksHead = 0;
/// queue end (tasks pop off here as they are complete)
volatile unsigned char utilTimerTasksTail = 0;

/// Is the utility timer actually running?
volatile bool utilTimerOn = false;
/// When we rescheduled the timer, how far in the future were we meant to get called (in system time)?
int utilTimerPeriod;
/// The system time from which the timer was last scheduled - add utilTimerPeriod to this to get the time when the timer should be called next
JsSysTime utilTimerStartTime = 0;
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

void jstUtilTimerInterruptHandler() {
  /* Note: we're using 32 bit times here, even though the real time counter is 64 bit. We
   * just make sure nothing is scheduled that far in the future */
  if (utilTimerOn) {
    // TODO: Keep UtilTimer running and then use the value from it
    // to estimate how long utilTimerPeriod really was
    // Subtract utilTimerPeriod from all timers' time
    int t = utilTimerTasksTail;
    while (t!=utilTimerTasksHead) {
      utilTimerTasks[t].time -= utilTimerPeriod;
      t = (t+1) & (UTILTIMERTASK_TASKS-1);
    }
    utilTimerOffset += utilTimerPeriod;
    // Check timers and execute any timers that are due
    while (utilTimerTasksTail!=utilTimerTasksHead && utilTimerTasks[utilTimerTasksTail].time <= 0) {
      UtilTimerTask *task = &utilTimerTasks[utilTimerTasksTail];
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
          if (UET_IS_BUFFER_WRITE_EVENT(utilTimerTasks[t].type) &&
              utilTimerTasks[t].data.buffer.pin == task->data.buffer.pin)
            sum += ((int)(unsigned int)utilTimerTasks[t].data.buffer.currentValue) - 32768;
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
      if (executeFn) executeFn(jshGetSystemTime(), executeData);
    }

    // re-schedule the timer if there is something left to do
    if (utilTimerTasksTail != utilTimerTasksHead) {
      // advance by last timer period
      utilTimerStartTime += utilTimerPeriod;
      utilTimerPeriod = utilTimerTasks[utilTimerTasksTail].time;
      if (utilTimerPeriod<0) utilTimerPeriod=0;
      // how long until we want to be called again?
      JsSysTime waitTime = utilTimerStartTime + (JsSysTime)utilTimerPeriod - jshGetSystemTime();
      if (waitTime<0) waitTime=0;
      jshUtilTimerReschedule(waitTime);
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
void  jstRestartUtilTimer() {
  utilTimerPeriod = utilTimerTasks[utilTimerTasksTail].time;
  JsSysTime currentTime = jshGetSystemTime();
  // just to be on the safe side; setting the start time to the current time should not normally be necessary here
  if (!utilTimerOn) utilTimerStartTime = currentTime;
  if (utilTimerPeriod<0) utilTimerPeriod=0;
  // how long until we want to be called again?
  JsSysTime waitTime = utilTimerStartTime + (JsSysTime)utilTimerPeriod - currentTime;
  if (waitTime<0) waitTime=0;
  jshUtilTimerStart(waitTime);
}

/** Queue a task up to be executed when a timer fires... return false on failure.
 * task.time is the delay at which to execute the task. If timerOffset!==NULL then
 * task.time is relative to the time at which timerOffset=jstGetUtilTimerOffset().
 * This allows pulse trains/etc to be scheduled in sync.
 */
bool utilTimerInsertTask(UtilTimerTask *task, uint32_t *timerOffset) {
  // check if queue is full or not
  if (utilTimerIsFull()) return false;
  jshInterruptOff();

  // See above - keep times in sync
  if (timerOffset)
    task->time += (int)*timerOffset - (int)utilTimerOffset;

  JsSysTime currentTime = jshGetSystemTime();
  if (!utilTimerOn) {
    // when enabling the timer, set the start time to the current time
    // otherwise (when rescheduling the timer) we only advance by the last timer period
    utilTimerStartTime = currentTime;
  }

  // How long was it since the timer was last scheduled? Update existing tasks #2575
  uint32_t timePassed = currentTime - utilTimerStartTime;
  // find out where to insert
  unsigned char insertPos = utilTimerTasksTail;
  while (insertPos != utilTimerTasksHead && utilTimerTasks[insertPos].time < (task->time+timePassed))
    insertPos = (insertPos+1) & (UTILTIMERTASK_TASKS-1);
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
      if (utilTimerTasks[i].time > timePassed)
        utilTimerTasks[i].time -= timePassed;
      else
        utilTimerTasks[i].time = 0;
      i = (i+1) & (UTILTIMERTASK_TASKS-1);
    }
    utilTimerStartTime += timePassed;
  } else // timer hasn't changed, we have to update our task's time
    task->time += timePassed;
  // add new item
  utilTimerTasks[insertPos] = *task;

  //jsiConsolePrint("Head is %d\n", utilTimerTasksHead);
  // now set up timer if not already set up...
  if (!utilTimerOn || haveChangedTimer) {
    utilTimerOn = true;
    jstRestartUtilTimer();
  }
  jshInterruptOn();
  return true;
}

/// Remove the task that that 'checkCallback' returns true for. Returns false if none found
bool utilTimerRemoveTask(bool (checkCallback)(UtilTimerTask *task, void* data), void *checkCallbackData) {
  jshInterruptOff();
  unsigned char ptr = utilTimerTasksHead;
  if (ptr != utilTimerTasksTail) {
    unsigned char endPtr = ((utilTimerTasksTail+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1));
    ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    // now we're at the last timer task - work back until we've just gone back past utilTimerTasksTail
    while (ptr != endPtr) {
      if (checkCallback(&utilTimerTasks[ptr], checkCallbackData)) {
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
  jshInterruptOff();
  unsigned char ptr = utilTimerTasksHead;
  if (ptr != utilTimerTasksTail) {
    ptr = (ptr+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
    // now we're at the last timer task - work back until we've just gone back past utilTimerTasksTail
    while (ptr != ((utilTimerTasksTail+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1))) {
      if (checkCallback(&utilTimerTasks[ptr], checkCallbackData)) {
        *task = utilTimerTasks[ptr];
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

#ifdef ESPR_USE_STEPPER_TIMER
// data = *Pin[4]
static bool jstStepTaskChecker(UtilTimerTask *task, void *data) {
  if (task->type != UET_STEP) return false;
  Pin *pins = (Pin*)data;
  for (int i=0;i<4;i++)
    if (task->data.step.pins[i] != pins[i])
      return false;
  return true;
}
#endif

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
  UtilTimerTask task;
  task.time = (int)time;
  task.repeatInterval = 0;
  task.type = UET_SET;
  int i;
  for (i=0;i<UTILTIMERTASK_PIN_COUNT;i++)
    task.data.set.pins[i] = (Pin)((i<pinCount) ? pins[i] : PIN_UNDEFINED);
  task.data.set.value = value;

  WAIT_UNTIL(!utilTimerIsFull(), "Utility Timer");
  return utilTimerInsertTask(&task, timerOffset);
}

// Do software PWM on the given pin, using the timer IRQs
bool jstPinPWM(JsVarFloat freq, JsVarFloat dutyCycle, Pin pin) {
  // if anything is wrong, exit now
  if (dutyCycle<=0 || dutyCycle>=1 || freq<=0) {
    // remove any timer tasks
    while (utilTimerRemoveTask(jstPinTaskChecker, (void*)&pin));
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
      if (jstPinTaskChecker(&utilTimerTasks[ptr], (void*)&pin)) {
        if (utilTimerTasks[ptr].data.set.value)
          ptaskon = &utilTimerTasks[ptr];
        else
          ptaskoff = &utilTimerTasks[ptr];
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
    while (utilTimerRemoveTask(jstPinTaskChecker, (void*)&pin));
  }
  UtilTimerTask taskon, taskoff;
  taskon.data.set.value = 1;
  taskoff.data.set.value = 0;
  taskon.time = (int)period;
  taskoff.time = (int)pulseLength;
  taskon.repeatInterval = (unsigned int)period;
  taskoff.repeatInterval = (unsigned int)period;
  taskon.type = UET_SET;
  taskoff.type = UET_SET;
  taskon.data.set.pins[0] = pin;
  taskoff.data.set.pins[0] = pin;
  int i;
  for (i=1;i<UTILTIMERTASK_PIN_COUNT;i++) {
    taskon.data.set.pins[i] = PIN_UNDEFINED;
    taskoff.data.set.pins[i] = PIN_UNDEFINED;
  }

  // first task is to turn on
  jshPinSetValue(pin, 1);
  uint32_t timerOffset = jstGetUtilTimerOffset();
  // now start the 2 PWM tasks
  WAIT_UNTIL(!utilTimerIsFull(), "Utility Timer");
  if (!utilTimerInsertTask(&taskon, &timerOffset)) return false;
  WAIT_UNTIL(!utilTimerIsFull(), "Utility Timer");
  return utilTimerInsertTask(&taskoff, &timerOffset);
}

/** Execute the given function repeatedly after the given time period. If period=0, don't repeat. True on success or false on failure to schedule
 * See utilTimerInsertTask for notes on timerOffset
 */
bool jstExecuteFn(UtilTimerTaskExecFn fn, void *userdata, JsSysTime startTime, uint32_t period, uint32_t *timerOffset) {
  UtilTimerTask task;
  task.time = (int)startTime;
  task.repeatInterval = period;
  task.type = UET_EXECUTE;
  task.data.execute.fn = fn;
  task.data.execute.userdata = userdata;

  WAIT_UNTIL(!utilTimerIsFull(), "Utility Timer");
  return utilTimerInsertTask(&task, timerOffset);
}

/// Stop executing the given function
bool jstStopExecuteFn(UtilTimerTaskExecFn fn, void *userdata) {
  UtilTimerTaskExec e;
  e.fn = fn;
  e.userdata = userdata;
  return utilTimerRemoveTask(jstExecuteTaskChecker, (void*)&e);
}

/// Set the utility timer so we're woken up in whatever time period
bool jstSetWakeUp(JsSysTime period) {
  UtilTimerTask task;
  task.time = (int)period;
  task.repeatInterval = 0;
  task.type = UET_WAKEUP;

  bool hasTimer = false;
  int nextTime;

  // work out if we're waiting for a timer,
  // and if so, when it's going to be
  jshInterruptOff();
  if (utilTimerOn) {
    hasTimer = true;
    nextTime = (int)(utilTimerStartTime + (JsSysTime)utilTimerPeriod - jshGetSystemTime());
  }
  jshInterruptOn();

  if (hasTimer && task.time >= nextTime) {
    // we already had a timer, and it's going to wake us up sooner.
    // don't create a WAKEUP timer task
    return true;
  }

  bool ok = utilTimerInsertTask(&task, NULL);
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

bool jstStartSignal(JsSysTime startTime, JsSysTime period, Pin pin, Pin npin, JsVar *currentData, JsVar *nextData, UtilTimerEventType type) {
  assert(jsvIsString(currentData));
  assert(jsvIsUndefined(nextData) || jsvIsString(nextData));
  if (!jshIsPinValid(pin)) return false;
  UtilTimerTask task;
  task.repeatInterval = (unsigned int)period;
  task.time = (int)(startTime + period);
  task.type = type;
  if (UET_IS_BUFFER_WRITE_EVENT(type)) {
    task.data.buffer.pin = pin;
    task.data.buffer.npin = npin;
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

  return utilTimerInsertTask(&task, NULL);
}



/// Remove the task that uses the buffer 'var'
bool jstStopBufferTimerTask(JsVar *var) {
  JsVarRef ref = jsvGetRef(var);
  return utilTimerRemoveTask(jstBufferTaskChecker, (void*)&ref);
}

/// Remove the task that uses the given pin
bool jstStopPinTimerTask(Pin pin) {
  return utilTimerRemoveTask(jstPinTaskChecker, (void*)&pin);
}

#endif

void jstReset() {
  jshUtilTimerDisable();
  utilTimerOn = false;
  utilTimerTasksTail = utilTimerTasksHead = 0;
  utilTimerOffset = 0;
  utilTimerPeriod = 0;
  utilTimerStartTime = jshGetSystemTime();
}

/** when system time is changed, also change the time in the timers.
This should be done with interrupts off */
void jstSystemTimeChanged(JsSysTime diff) {
  // we don't actually care since timers should hopefully just run based on delays
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

  jsiConsolePrintf("Util Timer %s\n", utilTimerOn?"on":"off");
  unsigned char t = uTimerTasksTail;
  bool hadTimers = false;
  while (t!=uTimerTasksHead) {
    hadTimers = true;

    UtilTimerTask task = uTimerTasks[t];
    jsiConsolePrintf("%08d us, repeat %08d us : ", (int)(1000*jshGetMillisecondsFromTime(task.time)), (int)(1000*jshGetMillisecondsFromTime(task.repeatInterval)));

    switch (task.type) {
    case UET_WAKEUP : jsiConsolePrintf("WKUP\n"); break;
    case UET_SET : jsiConsolePrintf("SET ");
    for (i=0;i<UTILTIMERTASK_PIN_COUNT;i++)
      if (task.data.set.pins[i] != PIN_UNDEFINED)
        jsiConsolePrintf("%p=%d,", task.data.set.pins[i],  (task.data.set.value>>i)&1);
    jsiConsolePrintf("\n");
    break;
#ifndef SAVE_ON_FLASH
    case UET_WRITE_BYTE : jsiConsolePrintf("WR8\n"); break;
    case UET_READ_BYTE : jsiConsolePrintf("RD8\n"); break;
    case UET_WRITE_SHORT : jsiConsolePrintf("WR16\n"); break;
    case UET_READ_SHORT : jsiConsolePrintf("RD16\n"); break;
#endif
    case UET_EXECUTE : jsiConsolePrintf("EXEC %x(%x)\n", task.data.execute.fn, task.data.execute.userdata); break;
    default : jsiConsolePrintf("?[%d]\n", task.type); break;
    }

    t = (t+1) & (UTILTIMERTASK_TASKS-1);
  }
  if (!hadTimers)
      jsiConsolePrintf("No Timers found.\n");

}
