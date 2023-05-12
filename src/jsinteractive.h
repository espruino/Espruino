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
 * Interactive Shell implementation
 * ----------------------------------------------------------------------------
 */
#ifndef JSINTERACTIVE_H_
#define JSINTERACTIVE_H_

#include "jsparse.h"
#include "jshardware.h"

#define JSI_WATCHES_NAME "watches"
#define JSI_TIMERS_NAME "timers"
#define JSI_DEBUG_HISTORY_NAME "dbghist"
#define JSI_HISTORY_NAME "history"
#define JSI_INIT_CODE_NAME "init" ///< used to temporarily store initialisation JS code for state in save()
#define JSI_LOAD_CODE_NAME "load" ///< used to temporarily store the name of a file to load from Storage when load(xyz) is used
#define JSI_JSFLAGS_NAME "flags"
#define JSI_ONINIT_NAME "onInit"

/// autoLoad = do we load the current state if it exists?
void jsiInit(bool autoLoad);
void jsiKill();

#ifndef LINUX
// This should get called from jshardware.c one second after startup,
// it does initialisation tasks like setting the right console device
void jsiOneSecondAfterStartup();
#endif

/// do main loop stuff, return true if it was busy this iteration
bool jsiLoop();

/// Tries to get rid of some memory (by clearing command history). Returns true if it got rid of something, false if it didn't.
bool jsiFreeMoreMemory();

bool jsiHasTimers(); // are there timers still left to run?
bool jsiIsWatchingPin(Pin pin); // are there any watches for the given pin?

void jsiCtrlC(); // Ctrl-C - force interrupt of execution

/// Queue a function, string, or array (of funcs/strings) to be executed next time around the idle loop
void jsiQueueEvents(JsVar *object, JsVar *callback, JsVar **args, int argCount);
/// Return true if the object has callbacks...
bool jsiObjectHasCallbacks(JsVar *object, const char *callbackName);
/// Queue up callbacks for other things (touchscreen? network?)
void jsiQueueObjectCallbacks(JsVar *object, const char *callbackName, JsVar **args, int argCount);
/// Execute the given function/string/array of functions and return true on success, false on failure (break during execution)
bool jsiExecuteEventCallback(JsVar *thisVar, JsVar *callbackVar, unsigned int argCount, JsVar **argPtr);
/// Same as above, but with a JsVarArray (this calls jsiExecuteEventCallback, so use jsiExecuteEventCallback where possible)
bool jsiExecuteEventCallbackArgsArray(JsVar *thisVar, JsVar *callbackVar, JsVar *argsArray);
// Execute the named Event callback on object, and return true if it exists
bool jsiExecuteEventCallbackName(JsVar *obj, const char *cbName, unsigned int argCount, JsVar **argPtr);
/// Utility version of jsiExecuteEventCallback for calling events on global variables
bool jsiExecuteEventCallbackOn(const char *objectName, const char *cbName, unsigned int argCount, JsVar **argPtr);

/// Create a timeout in JS to execute the given native function (outside of an IRQ). Returns the index
JsVar *jsiSetTimeout(void (*functionPtr)(void), JsVarFloat milliseconds);

IOEventFlags jsiGetDeviceFromClass(JsVar *deviceClass);
JsVar *jsiGetClassNameFromDevice(IOEventFlags device);

/// If Espruino could choose right now, what would be the best console device to use?
IOEventFlags jsiGetPreferredConsoleDevice();
/** Change the console to a new location - if force is set, this console
 * device will be 'sticky' - it will not change when the device changes
 * connection state */
void jsiSetConsoleDevice(IOEventFlags device, bool force);
/// Get the device that the console is currently on
IOEventFlags jsiGetConsoleDevice();
/// is the console forced into a given place (See jsiSetConsoleDevice)
bool jsiIsConsoleDeviceForced();
/// Transmit a byte
void jsiConsolePrintChar(char data);
/// Transmit a string (may be any string)
void jsiConsolePrintString(const char *str);
void vcbprintf_callback_jsiConsolePrintString(const char *str, void* user_data);
#ifndef USE_FLASH_MEMORY
#define jsiConsolePrint jsiConsolePrintString
/// Write the formatted string to the console (see vcbprintf)
void jsiConsolePrintf(const char *fmt, ...);
#else
/// Write the formatted string to the console (see vcbprintf), but place the format string into
// into flash
#define jsiConsolePrintf(fmt, ...) do { \
    FLASH_STR(flash_str, fmt); \
    jsiConsolePrintf_int(flash_str, ##__VA_ARGS__); \
  } while(0)
void jsiConsolePrintf_int(const char *fmt, ...);
/// Transmit a string (must be a literal string)
#define jsiConsolePrint(str) do { \
    FLASH_STR(flash_str, str); \
    jsiConsolePrintString_int(flash_str); \
} while(0)
void jsiConsolePrintString_int(const char *str);
#endif
/// Print the contents of a string var - directly
void jsiConsolePrintStringVar(JsVar *v);
/// If the input line was shown in the console, remove it
void jsiConsoleRemoveInputLine();
/// Change what is in the inputline into something else (and update the console)
void jsiReplaceInputLine(JsVar *newLine);
/** Clear the input line of data. If updateConsole is set, it
 * sends VT100 characters to physically remove the line from
 * the user's terminal. */
void jsiClearInputLine(bool updateConsole);

/// Flags for jsiSetBusy - THESE SHOULD BE 2^N
typedef enum {
  BUSY_INTERACTIVE = 1,
  BUSY_TRANSMIT    = 2,
  // ???           = 4
} JsiBusyDevice;
/// Shows a busy indicator, if one is set up
void jsiSetBusy(JsiBusyDevice device, bool isBusy);

/// Flags for jsiSetSleep
typedef enum {
  JSI_SLEEP_AWAKE  = 0,
  JSI_SLEEP_ASLEEP = 1,
  JSI_SLEEP_DEEP   = 2,
} JsiSleepType;

/// Shows a sleep indicator, if one is set up
void jsiSetSleep(JsiSleepType isSleep);


// for jswrap_interactive/io.c ----------------------------------------------------
#define USART_CALLBACK_NAME JS_EVENT_PREFIX"data"
#define USART_BAUDRATE_NAME "_baudrate"
#define DEVICE_OPTIONS_NAME "_options"
#define INIT_CALLBACK_NAME JS_EVENT_PREFIX"init" ///< Callback for `E.on('init'`
#define KILL_CALLBACK_NAME JS_EVENT_PREFIX"kill" ///< Callback for `E.on('kill'`
#define PASSWORD_VARIABLE_NAME "pwd"

typedef enum {
  JSIS_NONE,
  JSIS_ECHO_OFF           = 1<<0, ///< do we provide any user feedback? OFF=no
  JSIS_ECHO_OFF_FOR_LINE  = 1<<1, ///< Echo is off just for one line, then back on
  JSIS_TIMERS_CHANGED     = 1<<2,
#ifdef USE_DEBUGGER
  JSIS_IN_DEBUGGER        = 1<<3, ///< We're inside the debug loop
  JSIS_EXIT_DEBUGGER      = 1<<4, ///< we've been asked to exit the debug loop
#endif
  JSIS_TODO_FLASH_SAVE    = 1<<5, ///< save to flash
  JSIS_TODO_FLASH_LOAD    = 1<<6, ///< load from flash
  JSIS_TODO_RESET         = 1<<7, ///< reset the board, don't load anything
  JSIS_TODO_MASK = JSIS_TODO_FLASH_SAVE|JSIS_TODO_FLASH_LOAD|JSIS_TODO_RESET,
  JSIS_CONSOLE_FORCED     = 1<<8, ///< see jsiSetConsoleDevice
  JSIS_WATCHDOG_AUTO      = 1<<9, ///< Automatically kick the watchdog timer on idle
  JSIS_PASSWORD_PROTECTED = 1<<10, ///< Password protected
  JSIS_COMPLETELY_RESET   = 1<<11, ///< Has the board powered on *having not loaded anything from flash*
  JSIS_FIRST_BOOT         = 1<<12, ///< Is this the first time we started, or has load/reset/etc been called?

  JSIS_EVENTEMITTER_PROCESSING = 1<<13, ///< Are we currently executing events with jsiExecuteEvent*
  JSIS_EVENTEMITTER_STOP = 1<<14,       ///< Has E.stopEventPropagation() been called during event processing?
  JSIS_EVENTEMITTER_INTERRUPTED = 1<<15,///< Has there been an error during jsiExecuteEvent* execution?

  JSIS_ECHO_OFF_MASK = JSIS_ECHO_OFF|JSIS_ECHO_OFF_FOR_LINE,
  JSIS_SOFTINIT_MASK = JSIS_PASSWORD_PROTECTED|JSIS_WATCHDOG_AUTO|JSIS_TODO_MASK|JSIS_FIRST_BOOT|JSIS_COMPLETELY_RESET // stuff that DOESN'T get reset on softinit
    // watchdog can't be reset without a reboot so if it's set to auto we must keep it as auto
} PACKED_FLAGS JsiStatus;

extern JsiStatus jsiStatus;
bool jsiEcho();

#ifndef SAVE_ON_FLASH
extern Pin pinBusyIndicator;
extern Pin pinSleepIndicator;
#endif
extern JsSysTime jsiLastIdleTime; ///< The last time we went around the idle loop - use this for timers

void jsiDumpJSON(vcbprintf_callback user_callback, void *user_data, JsVar *data, JsVar *existing);
void jsiDumpState(vcbprintf_callback user_callback, void *user_data);
#define TIMER_MIN_INTERVAL 0.1 // in milliseconds
#define TIMER_MAX_INTERVAL 31536000001000ULL // in milliseconds
extern JsVarRef timerArray; // Linked List of timers to check and run
extern JsVarRef watchArray; // Linked List of input watches to check and run

extern JsVarInt jsiTimerAdd(JsVar *timerPtr);
extern void jsiTimersChanged(); // Flag timers changed so we can skip out of the loop if needed
// end for jswrap_interactive/io.c ------------------------------------------------

#ifdef USE_DEBUGGER
extern void jsiDebuggerLoop(); ///< Enter the debugger loop
#endif


#endif /* JSINTERACTIVE_H_ */
