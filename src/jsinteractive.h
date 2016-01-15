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
#define JSI_HISTORY_NAME "history"
#define JSI_INIT_CODE_NAME "init"
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

/// Queue a function, string, or array (of funcs/strings) to be executed next time around the idle loop
void jsiQueueEvents(JsVar *object, JsVar *callback, JsVar **args, int argCount);
/// Return true if the object has callbacks...
bool jsiObjectHasCallbacks(JsVar *object, const char *callbackName);
/// Queue up callbacks for other things (touchscreen? network?)
void jsiQueueObjectCallbacks(JsVar *object, const char *callbackName, JsVar **args, int argCount);
/// Execute callbacks straight away (like jsiQueueObjectCallbacks, but without queueing)
void jsiExecuteObjectCallbacks(JsVar *object, const char *callbackName, JsVar **args, int argCount);
/// Execute the given function/string/array of functions and return true on success, false on failure (break during execution)
bool jsiExecuteEventCallback(JsVar *thisVar, JsVar *callbackVar, unsigned int argCount, JsVar **argPtr);
/// Same as above, but with a JsVarArray (this calls jsiExecuteEventCallback, so use jsiExecuteEventCallback where possible)
bool jsiExecuteEventCallbackArgsArray(JsVar *thisVar, JsVar *callbackVar, JsVar *argsArray);


IOEventFlags jsiGetDeviceFromClass(JsVar *deviceClass);
JsVar *jsiGetClassNameFromDevice(IOEventFlags device);

/// Change the console to a new location
void jsiSetConsoleDevice(IOEventFlags device);
/// Get the device that the console is currently on
IOEventFlags jsiGetConsoleDevice();
/// Transmit a byte
void jsiConsolePrintChar(char data);
/// Transmit a string (may be any string)
void jsiConsolePrintString(const char *str);
#ifndef FLASH_STR
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
/// Transmit a position in the lexer (for reporting errors)
void jsiConsolePrintPosition(struct JsLex *lex, size_t tokenPos);
/// If the input line was shown in the console, remove it
void jsiConsoleRemoveInputLine();
/// Change what is in the inputline into something else (and update the console)
void jsiReplaceInputLine(JsVar *newLine);

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

typedef enum {
  JSIS_NONE,
  JSIS_ECHO_OFF = 1, ///< do we provide any user feedback? OFF=no
  JSIS_ECHO_OFF_FOR_LINE = 2,
  JSIS_ALLOW_DEEP_SLEEP = 4, // can we go into proper deep sleep?
  JSIS_TIMERS_CHANGED = 8,
#ifdef USE_DEBUGGER
  JSIS_IN_DEBUGGER = 16, // We're inside the debug loop
  JSIS_EXIT_DEBUGGER = 32, // we've been asked to exit the debug loop
#endif
  JSIS_TODO_FLASH_SAVE = 64, // save to flash
  JSIS_TODO_FLASH_LOAD = 128, // load from flash
  JSIS_TODO_RESET = 256, // reset the board, don't load anything
  JSIS_TODO_MASK = JSIS_TODO_FLASH_SAVE|JSIS_TODO_FLASH_LOAD|JSIS_TODO_RESET,


  JSIS_ECHO_OFF_MASK = JSIS_ECHO_OFF|JSIS_ECHO_OFF_FOR_LINE
} PACKED_FLAGS JsiStatus;

extern JsiStatus jsiStatus;
bool jsiEcho();

extern Pin pinBusyIndicator;
extern Pin pinSleepIndicator;
extern JsSysTime jsiLastIdleTime; ///< The last time we went around the idle loop - use this for timers

void jsiDumpState(vcbprintf_callback user_callback, void *user_data);
#define TIMER_MIN_INTERVAL 0.1 // in milliseconds
extern JsVarRef timerArray; // Linked List of timers to check and run
extern JsVarRef watchArray; // Linked List of input watches to check and run

extern JsVarInt jsiTimerAdd(JsVar *timerPtr);
extern void jsiTimersChanged(); // Flag timers changed so we can skip out of the loop if needed
// end for jswrap_interactive/io.c ------------------------------------------------

#ifdef USE_DEBUGGER
extern void jsiDebuggerLoop(); ///< Enter the debugger loop
#endif


#endif /* JSINTERACTIVE_H_ */
