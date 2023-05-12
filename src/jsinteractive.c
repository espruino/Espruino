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
#include "jsutils.h"
#include "jsinteractive.h"
#include "jshardware.h"
#include "jstimer.h"
#include "jspin.h"
#include "jsflags.h"
#include "jswrapper.h"
#include "jswrap_json.h"
#include "jswrap_io.h"
#include "jswrap_stream.h"
#include "jswrap_espruino.h" // jswrap_espruino_getErrorFlagArray
#include "jsflash.h" // load and save to flash
#include "jswrap_interactive.h" // jswrap_interactive_setTimeout
#include "jswrap_object.h" // jswrap_object_keys_or_property_names
#include "jsnative.h" // jsnSanityTest
#ifdef BLUETOOTH
#include "bluetooth.h"
#include "jswrap_bluetooth.h"
#endif

#ifdef ARM
#define CHAR_DELETE_SEND 0x08
#else
#define CHAR_DELETE_SEND '\b'
#endif

#ifdef ESP8266
extern void jshPrintBanner(void); // prints a debugging banner while we're in beta
extern void jshSoftInit(void);    // re-inits wifi after a soft-reset
#endif

#ifdef ESP32
extern void jshSoftInit(void);
#endif

// ----------------------------------------------------------------------------
typedef enum {
  IS_NONE,
  IS_HAD_R,
  IS_HAD_27,
  IS_HAD_27_79,
  IS_HAD_27_91,
  IS_HAD_27_91_NUMBER, ///< Esc [ then 0-9
} PACKED_FLAGS InputState;

JsVar *events = 0; // Array of events to execute
JsVarRef timerArray = 0; // Linked List of timers to check and run
JsVarRef watchArray = 0; // Linked List of input watches to check and run
// ----------------------------------------------------------------------------
IOEventFlags consoleDevice = DEFAULT_CONSOLE_DEVICE; ///< The console device for user interaction
#ifndef SAVE_ON_FLASH
Pin pinBusyIndicator = DEFAULT_BUSY_PIN_INDICATOR;
Pin pinSleepIndicator = DEFAULT_SLEEP_PIN_INDICATOR;
#endif
JsiStatus jsiStatus = 0;
JsSysTime jsiLastIdleTime;  ///< The last time we went around the idle loop - use this for timers
#ifndef EMBEDDED
uint32_t jsiTimeSinceCtrlC; ///< When was Ctrl-C last pressed. We use this so we quit on desktop when we do Ctrl-C + Ctrl-C
#endif
// ----------------------------------------------------------------------------
JsVar *inputLine = 0; ///< The current input line
JsvStringIterator inputLineIterator; ///< Iterator that points to the end of the input line
int inputLineLength = -1;
bool inputLineRemoved = false;
size_t inputCursorPos = 0; ///< The position of the cursor in the input line
InputState inputState = 0; ///< state for dealing with cursor keys
uint16_t inputStateNumber; ///< Number from when `Esc [ 1234` is sent - for storing line number
uint16_t jsiLineNumberOffset; ///< When we execute code, this is the 'offset' we apply to line numbers in error/debug
bool hasUsedHistory = false; ///< Used to speed up - if we were cycling through history and then edit, we need to copy the string
unsigned char loopsIdling = 0; ///< How many times around the loop have we been entirely idle?
JsErrorFlags lastJsErrorFlags = 0; ///< Compare with jsErrorFlags in order to report errors
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void jsiDebuggerLine(JsVar *line);
#endif
void jsiCheckErrors();
// ----------------------------------------------------------------------------

/**
 * Get the device from the class variable.
 */
IOEventFlags jsiGetDeviceFromClass(JsVar *class) {
  // Devices have their Object data set up to something special
  // See jspNewObject
  if (class &&
      class->varData.str[0]=='D' &&
      class->varData.str[1]=='E' &&
      class->varData.str[2]=='V')
    return (IOEventFlags)class->varData.str[3];

  return EV_NONE;
}


JsVar *jsiGetClassNameFromDevice(IOEventFlags device) {
  const char *deviceName = jshGetDeviceString(device);
  if (!deviceName[0]) return 0; // could be empty string
  return jsvFindChildFromString(execInfo.root, deviceName, false);
}

NO_INLINE bool jsiEcho() {
  return ((jsiStatus&JSIS_ECHO_OFF_MASK)==0);
}

NO_INLINE bool jsiPasswordProtected() {
  return ((jsiStatus&JSIS_PASSWORD_PROTECTED)!=0);
}

static bool jsiShowInputLine() {
  return jsiEcho() && !inputLineRemoved && !jsiPasswordProtected();
}

/** Called when the input line/cursor is modified *and its iterator should be reset
 * Because JsvStringIterator doesn't lock the string, it's REALLY IMPORTANT
 * that we call this BEFORE we do jsvUnLock(inputLine) */
static NO_INLINE void jsiInputLineCursorMoved() {
  // free string iterator
  if (inputLineIterator.var) {
    jsvStringIteratorFree(&inputLineIterator);
    inputLineIterator.var = 0;
  }
  inputLineLength = -1;
}

/// Called to append to the input line
static NO_INLINE void jsiAppendToInputLine(const char *str) {
  // recreate string iterator if needed
  if (!inputLineIterator.var) {
    jsvStringIteratorNew(&inputLineIterator, inputLine, 0);
    jsvStringIteratorGotoEnd(&inputLineIterator);
  }
  while (*str) {
    jsvStringIteratorAppend(&inputLineIterator, *(str++));
    inputLineLength++;
  }
}

/// If Espruino could choose right now, what would be the best console device to use?
IOEventFlags jsiGetPreferredConsoleDevice() {
  IOEventFlags dev = DEFAULT_CONSOLE_DEVICE;
#ifdef USE_TERMINAL
  if (!jshIsDeviceInitialised(dev))
    dev = EV_TERMINAL;
#endif
#ifdef USB
  if (jshIsUSBSERIALConnected())
    dev = EV_USBSERIAL;
#endif
#ifdef BLUETOOTH
  if (jsble_has_peripheral_connection(dev))
    dev = EV_BLUETOOTH;
#endif
  return dev;
}

void jsiSetConsoleDevice(IOEventFlags device, bool force) {
  if (force)
    jsiStatus |= JSIS_CONSOLE_FORCED;
  else
    jsiStatus &= ~JSIS_CONSOLE_FORCED;

  if (device == consoleDevice) return;

  if (DEVICE_IS_USART(device) && !jshIsDeviceInitialised(device)) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    jshUSARTSetup(device, &inf);
  }

  bool echo = jsiEcho();
  // If we're still in 'limbo', move any contents over
  if (consoleDevice == EV_LIMBO) {
    echo = false;
    jshTransmitMove(EV_LIMBO, device);
    jshUSARTKick(device);
  }

  // Log to the old console that we are moving consoles and then, once we have moved
  // the console, log to the new console that we have moved consoles.
  if (echo) { // intentionally not using jsiShowInputLine()
    jsiConsoleRemoveInputLine();
    jsiConsolePrintf("-> %s\n", jshGetDeviceString(device));
  }
  IOEventFlags oldDevice = consoleDevice;
  consoleDevice = device;
  if (echo) { // intentionally not using jsiShowInputLine()
    jsiConsolePrintf("<- %s\n", jshGetDeviceString(oldDevice));
  }
}

IOEventFlags jsiGetConsoleDevice() {
  // The `consoleDevice` is the global used to hold the current console.  This function
  // encapsulates access.
  return consoleDevice;
}

bool jsiIsConsoleDeviceForced() {
  return (jsiStatus & JSIS_CONSOLE_FORCED)!=0;
}

/**
 * Send a character to the console.
 */
NO_INLINE void jsiConsolePrintChar(char data) {
  jshTransmit(consoleDevice, (unsigned char)data);
}

/**
 * \breif Send a NULL terminated string to the console.
 */
NO_INLINE void jsiConsolePrintString(const char *str) {
  while (*str) {
    if (*str == '\n') jsiConsolePrintChar('\r');
    jsiConsolePrintChar(*(str++));
  }
}

void vcbprintf_callback_jsiConsolePrintString(const char *str, void* user_data) {
  NOT_USED(user_data);
  jsiConsolePrintString(str);
}

#ifdef USE_FLASH_MEMORY
// internal version that copies str from flash to an internal buffer
NO_INLINE void jsiConsolePrintString_int(const char *str) {
  size_t len = flash_strlen(str);
  char buff[len+1];
  flash_strncpy(buff, str, len+1);
  jsiConsolePrintString(buff);
}
#endif

/**
 * Perform a printf to the console.
 * Execute a printf command to the current JS console.
 */
#ifndef USE_FLASH_MEMORY
void jsiConsolePrintf(const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsiConsolePrint,0, fmt, argp);
  va_end(argp);
}
#else
void jsiConsolePrintf_int(const char *fmt, ...) {
  // fmt is in flash and requires special aligned accesses
  size_t len = flash_strlen(fmt);
  char buff[len+1];
  flash_strncpy(buff, fmt, len+1);
  va_list argp;
  va_start(argp, fmt);
  vcbprintf(vcbprintf_callback_jsiConsolePrintString, 0, buff, argp);
  va_end(argp);
}
#endif

/// Print the contents of a string var from a character position until end of line (adding an extra ' ' to delete a character if there was one)
void jsiConsolePrintStringVarUntilEOL(JsVar *v, size_t fromCharacter, size_t maxChars, bool andBackup) {
  size_t chars = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, fromCharacter);
  while (jsvStringIteratorHasChar(&it) && chars<maxChars) {
    char ch = jsvStringIteratorGetCharAndNext(&it);
    if (ch == '\n') break;
    jsiConsolePrintChar(ch);
    chars++;
  }
  jsvStringIteratorFree(&it);
  if (andBackup) {
    jsiConsolePrintChar(' ');chars++;
    while (chars--) jsiConsolePrintChar(0x08); //delete
  }
}

/** Print the contents of a string var - directly - starting from the given character, and
 * using newLineCh to prefix new lines (if it is not 0). */
void jsiConsolePrintStringVarWithNewLineChar(JsVar *v, size_t fromCharacter, char newLineCh) {
  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, fromCharacter);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetCharAndNext(&it);
    if (ch == '\n') jsiConsolePrintChar('\r');
    jsiConsolePrintChar(ch);
    if (ch == '\n' && newLineCh) jsiConsolePrintChar(newLineCh);
  }
  jsvStringIteratorFree(&it);
}

/**
 * Print the contents of a string var - directly.
 */
void jsiConsolePrintStringVar(JsVar *v) {
  jsiConsolePrintStringVarWithNewLineChar(v,0,0);
}

/** Erase everything from the cursor position onwards */
void jsiConsoleEraseAfterCursor() {
  jsiConsolePrint("\x1B[J"); // 27,91,74 - delete all to right and down
}

void jsiMoveCursor(size_t oldX, size_t oldY, size_t newX, size_t newY) {
  // see http://www.termsys.demon.co.uk/vtansi.htm - we could do this better
  // move cursor
  while (oldX < newX) {
    jsiConsolePrint("\x1B[C"); // 27,91,67 - right
    oldX++;
  }
  while (oldX > newX) {
    jsiConsolePrint("\x1B[D"); // 27,91,68 - left
    oldX--;
  }
  while (oldY < newY) {
    jsiConsolePrint("\x1B[B"); // 27,91,66 - down
    oldY++;
  }
  while (oldY > newY) {
    jsiConsolePrint("\x1B[A"); // 27,91,65 - up
    oldY--;
  }
}

void jsiMoveCursorChar(JsVar *v, size_t fromCharacter, size_t toCharacter) {
  if (fromCharacter==toCharacter) return;
  size_t oldX, oldY;
  jsvGetLineAndCol(v, fromCharacter, &oldY, &oldX);
  size_t newX, newY;
  jsvGetLineAndCol(v, toCharacter, &newY, &newX);
  jsiMoveCursor(oldX, oldY, newX, newY);
}

/// If the input line was shown in the console, remove it
void jsiConsoleRemoveInputLine() {
  if (!inputLineRemoved) {
    inputLineRemoved = true;
    if (jsiEcho() && inputLine) { // intentionally not using jsiShowInputLine()
      jsiMoveCursorChar(inputLine, inputCursorPos, 0); // move cursor to start of line
      jsiConsolePrintChar('\r'); // go propery to start of line - past '>'
      jsiConsoleEraseAfterCursor(); // delete all to right and down
#ifdef USE_DEBUGGER
      if (jsiStatus & JSIS_IN_DEBUGGER) {
        jsiConsolePrintChar(0x08); // d
        jsiConsolePrintChar(0x08); // e
        jsiConsolePrintChar(0x08); // b
        jsiConsolePrintChar(0x08); // u
        jsiConsolePrintChar(0x08); // g
      }
#endif
    }
  }
}

/// If the input line has been removed, return it
void jsiConsoleReturnInputLine() {
  if (inputLineRemoved) {
    inputLineRemoved = false;
    if (jsiEcho()) { // intentionally not using jsiShowInputLine()
#ifdef USE_DEBUGGER
      if (jsiStatus & JSIS_IN_DEBUGGER)
        jsiConsolePrint("debug");
#endif
      if (jsiPasswordProtected())
        jsiConsolePrint("password");
      jsiConsolePrintChar('>'); // show the prompt
      jsiConsolePrintStringVarWithNewLineChar(inputLine, 0, ':');
      jsiMoveCursorChar(inputLine, jsvGetStringLength(inputLine), inputCursorPos);
    }
  }
}

/**
 * Clear the input line of data. If updateConsole is set, it
 * sends VT100 characters to physically remove the line from
 * the user's terminal.
 */
void jsiClearInputLine(bool updateConsole) {
  // input line already empty - don't do anything
  if (jsvIsEmptyString(inputLine))
    return;
  // otherwise...
  if (updateConsole)
    jsiConsoleRemoveInputLine();
  // clear input line
  jsiInputLineCursorMoved();
  jsvUnLock(inputLine);
  inputLine = jsvNewFromEmptyString();
  inputCursorPos = 0;
}

/* Sets 'busy state' - this is used for lighting up a busy indicator LED, which can be used for debugging power usage */
void jsiSetBusy(
    JsiBusyDevice device, //!< ???
    bool isBusy           //!< ???
  ) {
#ifndef SAVE_ON_FLASH
  static JsiBusyDevice business = 0;

  if (isBusy)
    business |= device;
  else
    business &= (JsiBusyDevice)~device;

  if (pinBusyIndicator != PIN_UNDEFINED)
    jshPinOutput(pinBusyIndicator, business!=0);
#endif
}

/**
 * Set the status of a pin as a function of whether we are asleep.
 * When called, if a pin is set for a sleep indicator, we set the pin to be true
 * if the sleep type is awake and false otherwise.
 */
void jsiSetSleep(JsiSleepType isSleep) {
#ifndef SAVE_ON_FLASH
  if (pinSleepIndicator != PIN_UNDEFINED)
    jshPinOutput(pinSleepIndicator, isSleep == JSI_SLEEP_AWAKE);
#endif
}

static JsVarRef _jsiInitNamedArray(const char *name) {
  JsVar *array = jsvObjectGetChild(execInfo.hiddenRoot, name, JSV_ARRAY);
  JsVarRef arrayRef = 0;
  if (array) arrayRef = jsvGetRef(jsvRef(array));
  jsvUnLock(array);
  return arrayRef;
}

// Used when recovering after being flashed
// 'claim' anything we are using
void jsiSoftInit(bool hasBeenReset) {
  jsErrorFlags = 0;
  lastJsErrorFlags = 0;
  events = jsvNewEmptyArray();
  inputLine = jsvNewFromEmptyString();
  inputCursorPos = 0;
  jsiLineNumberOffset = 0;
  jsiInputLineCursorMoved();
  inputLineIterator.var = 0;

  jsfSetFlag(JSF_DEEP_SLEEP, 0);
#ifndef SAVE_ON_FLASH
  pinBusyIndicator = DEFAULT_BUSY_PIN_INDICATOR;
  pinSleepIndicator = DEFAULT_SLEEP_PIN_INDICATOR;
#endif

  // Load timer/watch arrays
  timerArray = _jsiInitNamedArray(JSI_TIMERS_NAME);
  watchArray = _jsiInitNamedArray(JSI_WATCHES_NAME);

  // Make sure we set up lastIdleTime, as this could be used
  // when adding an interval from onInit (called below)
  jsiLastIdleTime = jshGetSystemTime();
#ifndef EMBEDDED
  jsiTimeSinceCtrlC = 0xFFFFFFFF;
#endif

  // Set up interpreter flags and remove
  JsVar *flags = jsvObjectGetChildIfExists(execInfo.hiddenRoot, JSI_JSFLAGS_NAME);
  if (flags) {
    jsFlags = jsvGetIntegerAndUnLock(flags);
    jsvObjectRemoveChild(execInfo.hiddenRoot, JSI_JSFLAGS_NAME);
  }

  // Run wrapper initialisation stuff
  jswInit();

  // Run 'boot code' - textual JS in flash
  jsfLoadBootCodeFromFlash(hasBeenReset);

  // Now run initialisation code
  JsVar *initCode = jsvObjectGetChildIfExists(execInfo.hiddenRoot, JSI_INIT_CODE_NAME);
  if (initCode) {
    jsvUnLock2(jspEvaluateVar(initCode, 0, 0), initCode);
    jsvObjectRemoveChild(execInfo.hiddenRoot, JSI_INIT_CODE_NAME);
  }

  // Check any existing watches and set up interrupts for them
  if (watchArray) {
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, watchArrayPtr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *watch = jsvObjectIteratorGetValue(&it);
      JsVar *watchPin = jsvObjectGetChildIfExists(watch, "pin");
      bool highAcc = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(watch, "hispeed"));
      jshPinWatch(jshGetPinFromVar(watchPin), true, highAcc?JSPW_HIGH_SPEED:JSPW_NONE);
      jsvUnLock2(watchPin, watch);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    jsvUnLock(watchArrayPtr);
  }

  // Timers are stored by time in the future now, so no need
  // to fiddle with them.

  // Execute `init` events on `E`
  jsiExecuteEventCallbackOn("E", INIT_CALLBACK_NAME, 0, 0);
  // Execute the `onInit` function
  JsVar *onInit = jsvObjectGetChildIfExists(execInfo.root, JSI_ONINIT_NAME);
  if (onInit) {
    if (jsiEcho()) jsiConsolePrint("Running onInit()...\n");
    jsiExecuteEventCallback(0, onInit, 0, 0);
    jsvUnLock(onInit);
  }
}

/** Output the given variable as JSON, or if it exists
 * in the root scope (and it's not 'existing') then just
 * the name is dumped.  */
void jsiDumpJSON(vcbprintf_callback user_callback, void *user_data, JsVar *data, JsVar *existing) {
  // Check if it exists in the root scope
  JsVar *name = jsvGetIndexOf(execInfo.root,  data, true);
  if (name && jsvIsString(name) && name!=existing) {
    // if it does, print the name
    cbprintf(user_callback, user_data, "%v", name);
  } else {
    // if it doesn't, print JSON
    jsfGetJSONWithCallback(data, NULL, JSON_SOME_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES, 0, user_callback, user_data);
  }
}

NO_INLINE static void jsiDumpEvent(vcbprintf_callback user_callback, void *user_data, JsVar *parentName, JsVar *eventKeyName, JsVar *eventFn) {
  JsVar *eventName = jsvNewFromStringVar(eventKeyName, strlen(JS_EVENT_PREFIX), JSVAPPENDSTRINGVAR_MAXLENGTH);
  cbprintf(user_callback, user_data, "%v.on(%q, ", parentName, eventName);
  jsvUnLock(eventName);
  jsiDumpJSON(user_callback, user_data, eventFn, 0);
  user_callback(");\n", user_data);
}

/** Output extra functions defined in an object such that they can be copied to a new device */
NO_INLINE void jsiDumpObjectState(vcbprintf_callback user_callback, void *user_data, JsVar *parentName, JsVar *parent) {
  JsvIsInternalChecker checker = jsvGetInternalFunctionCheckerFor(parent);

  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, parent);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *child = jsvObjectIteratorGetKey(&it);
    JsVar *data = jsvObjectIteratorGetValue(&it);

    if (!checker || !checker(child)) {
      if (jsvIsStringEqual(child, JSPARSE_PROTOTYPE_VAR)) {
        // recurse to print prototypes
        JsVar *name = jsvNewFromStringVar(parentName,0,JSVAPPENDSTRINGVAR_MAXLENGTH);
        if (name) {
          jsvAppendString(name, ".prototype");
          jsiDumpObjectState(user_callback, user_data, name, data);
          jsvUnLock(name);
        }
      } else if (jsvIsStringEqualOrStartsWith(child, JS_EVENT_PREFIX, true)) {
        // Handle the case that this is an event
        if (jsvIsArray(data)) {
          JsvObjectIterator ait;
          jsvObjectIteratorNew(&ait, data);
          while (jsvObjectIteratorHasValue(&ait)) {
            JsVar *v = jsvObjectIteratorGetValue(&ait);
            jsiDumpEvent(user_callback, user_data, parentName, child, v);
            jsvUnLock(v);
            jsvObjectIteratorNext(&ait);
          }
          jsvObjectIteratorFree(&ait);
        } else {
          jsiDumpEvent(user_callback, user_data, parentName, child, data);
        }
      } else {
        // It's a normal function
        if (!jsvIsNativeFunction(data)) {
          cbprintf(user_callback, user_data, "%v.%v = ", parentName, child);
          jsiDumpJSON(user_callback, user_data, data, 0);
          user_callback(";\n", user_data);
        }
      }
    }
    jsvUnLock2(data, child);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
}

/** Dump the code required to initialise a serial port to this string */
void jsiDumpSerialInitialisation(vcbprintf_callback user_callback, void *user_data, const char *serialName, bool humanReadableDump) {
  JsVar *serialVarName = jsvFindChildFromString(execInfo.root, serialName, false);
  JsVar *serialVar = jsvSkipName(serialVarName);

  if (serialVar) {
    if (humanReadableDump)
      jsiDumpObjectState(user_callback, user_data, serialVarName, serialVar);

    JsVar *baud = jsvObjectGetChildIfExists(serialVar, USART_BAUDRATE_NAME);
    JsVar *options = jsvObjectGetChildIfExists(serialVar, DEVICE_OPTIONS_NAME);
    if (baud || options) {
      int baudrate = (int)jsvGetInteger(baud);
      if (baudrate <= 0) baudrate = DEFAULT_BAUD_RATE;
      cbprintf(user_callback, user_data, "%s.setup(%d", serialName, baudrate);
      if (jsvIsObject(options)) {
        user_callback(", ", user_data);
        jsfGetJSONWithCallback(options, NULL, JSON_SHOW_DEVICES, 0, user_callback, user_data);
      }
      user_callback(");\n", user_data);
    }
    jsvUnLock3(baud, options, serialVar);
  }
  jsvUnLock(serialVarName);
}

/** Dump the code required to initialise a SPI port to this string */
void jsiDumpDeviceInitialisation(vcbprintf_callback user_callback, void *user_data, const char *deviceName) {
  JsVar *deviceVar = jsvObjectGetChildIfExists(execInfo.root, deviceName);
  if (deviceVar) {
    JsVar *options = jsvObjectGetChildIfExists(deviceVar, DEVICE_OPTIONS_NAME);
    if (options) {
      cbprintf(user_callback, user_data, "%s.setup(", deviceName);
      if (jsvIsObject(options))
        jsfGetJSONWithCallback(options, NULL, JSON_SHOW_DEVICES, 0, user_callback, user_data);
      user_callback(");\n", user_data);
    }
    jsvUnLock2(options, deviceVar);
  }
}

/** Dump all the code required to initialise hardware to this string */
void jsiDumpHardwareInitialisation(vcbprintf_callback user_callback, void *user_data, bool humanReadableDump) {
#ifndef NO_DUMP_HARDWARE_INITIALISATION // eg. Banglejs doesn't need to dump hardware initialisation
  if (jsiStatus&JSIS_ECHO_OFF) user_callback("echo(0);", user_data);
#ifndef SAVE_ON_FLASH
  if (pinBusyIndicator != DEFAULT_BUSY_PIN_INDICATOR) {
    cbprintf(user_callback, user_data, "setBusyIndicator(%p);\n", pinBusyIndicator);
  }
  if (pinSleepIndicator != DEFAULT_SLEEP_PIN_INDICATOR) {
    cbprintf(user_callback, user_data, "setSleepIndicator(%p);\n", pinSleepIndicator);
  }
#endif
  if (humanReadableDump && jsFlags/* non-standard flags */) {
    JsVar *v = jsfGetFlags();
    cbprintf(user_callback, user_data, "E.setFlags(%j);\n", v);
    jsvUnLock(v);
  }

#ifdef USB
  jsiDumpSerialInitialisation(user_callback, user_data, "USB", humanReadableDump);
#endif
  int i;
  for (i=0;i<USART_COUNT;i++)
    jsiDumpSerialInitialisation(user_callback, user_data, jshGetDeviceString(EV_SERIAL1+i), humanReadableDump);
  for (i=0;i<SPI_COUNT;i++)
    jsiDumpDeviceInitialisation(user_callback, user_data, jshGetDeviceString(EV_SPI1+i));
  for (i=0;i<I2C_COUNT;i++)
    jsiDumpDeviceInitialisation(user_callback, user_data, jshGetDeviceString(EV_I2C1+i));
  // pins
  Pin pin;

  for (pin=0;jshIsPinValid(pin) && pin<JSH_PIN_COUNT;pin++) {
    if (IS_PIN_USED_INTERNALLY(pin)) continue;
    JshPinState state = jshPinGetState(pin);
    JshPinState statem = state&JSHPINSTATE_MASK;

    if (statem == JSHPINSTATE_GPIO_OUT && !jshGetPinStateIsManual(pin)) {
      bool isOn = (state&JSHPINSTATE_PIN_IS_ON)!=0;
      if (!isOn && IS_PIN_A_LED(pin)) continue;
      cbprintf(user_callback, user_data, "digitalWrite(%p, %d);\n",pin,isOn?1:0);
    } else {
#ifdef DEFAULT_CONSOLE_RX_PIN
      // the console input pin is always a pullup now - which is expected
      if (pin == DEFAULT_CONSOLE_RX_PIN &&
          (statem == JSHPINSTATE_GPIO_IN_PULLUP ||
           statem == JSHPINSTATE_AF_OUT)) continue;
#endif
#ifdef DEFAULT_CONSOLE_TX_PIN
      // the console input pin is always a pullup now - which is expected
      if (pin == DEFAULT_CONSOLE_TX_PIN &&
          (statem == JSHPINSTATE_AF_OUT)) continue;
#endif
#if defined(BTN1_PININDEX) && defined(BTN1_PINSTATE)
      if (pin == BTN1_PININDEX &&
          statem == BTN1_PINSTATE) continue;
#endif
#if defined(BTN2_PININDEX) && defined(BTN2_PINSTATE)
      if (pin == BTN2_PININDEX &&
          statem == BTN2_PINSTATE) continue;
#endif
#if defined(BTN3_PININDEX) && defined(BTN3_PINSTATE)
      if (pin == BTN3_PININDEX &&
          statem == BTN3_PINSTATE) continue;
#endif
#if defined(BTN4_PININDEX) && defined(BTN4_PINSTATE)
      if (pin == BTN4_PININDEX &&
          statem == BTN4_PINSTATE) continue;
#endif

      // don't bother with normal inputs, as they come up in this state (ish) anyway
      if (!jshIsPinStateDefault(pin, statem)) {
        // use getPinMode to get the correct string (remove some duplication)
        JsVar *s = jswrap_io_getPinMode(pin);
        if (s) cbprintf(user_callback, user_data, "pinMode(%p, %q%s);\n",pin,s,jshGetPinStateIsManual(pin)?"":", true");
        jsvUnLock(s);
      }
    }
  }
#ifdef BLUETOOTH
  if (humanReadableDump)
    jswrap_ble_dumpBluetoothInitialisation(user_callback, user_data);
#endif
#endif
}

// Used when shutting down before flashing
// 'release' anything we are using, but ensure that it doesn't get freed
void jsiSoftKill() {
  // Execute `kill` events on `E`
  jsiExecuteEventCallbackOn("E", KILL_CALLBACK_NAME, 0, 0);
  jsiCheckErrors();
  // Clear input line...
  inputCursorPos = 0;
  jsiInputLineCursorMoved();
  jsvUnLock(inputLine);
  inputLine=0;
  // kill any wrapped stuff
  jswKill();
  // Stop all active timer tasks
  jstReset();
  // Unref Watches/etc
  if (events) {
    jsvUnLock(events);
    events=0;
  }
  if (timerArray) {
    jsvUnRefRef(timerArray);
    timerArray=0;
  }
  if (watchArray) {
    // Check any existing watches and disable interrupts for them
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, watchArrayPtr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *watchPtr = jsvObjectIteratorGetValue(&it);
      JsVar *watchPin = jsvObjectGetChildIfExists(watchPtr, "pin");
      jshPinWatch(jshGetPinFromVar(watchPin), false, JSPW_NONE);
      jsvUnLock2(watchPin, watchPtr);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    jsvUnRef(watchArrayPtr);
    jsvUnLock(watchArrayPtr);
    watchArray=0;
  }
  // Save flags if required
  if (jsFlags)
    jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, JSI_JSFLAGS_NAME, jsvNewFromInteger(jsFlags));

  // Save initialisation information
  JsVar *initCode = jsvNewFromEmptyString();
  if (initCode) { // out of memory
    JsvStringIterator it;
    jsvStringIteratorNew(&it, initCode, 0);
    jsiDumpHardwareInitialisation((vcbprintf_callback)&jsvStringIteratorPrintfCallback, &it, false/*human readable*/);
    jsvStringIteratorFree(&it);
    jsvObjectSetChild(execInfo.hiddenRoot, JSI_INIT_CODE_NAME, initCode);
    jsvUnLock(initCode);
  }
  // If we're here we're loading, saving or resetting - board is no longer at power-on state
  jsiStatus &= ~JSIS_COMPLETELY_RESET; // loading code, remove this flag
  jsiStatus &= ~JSIS_FIRST_BOOT; // this is no longer the first boot!
}

/** Called as part of initialisation - loads boot code.
 *
 * loadedFilename is set if we're loading a file, and we can use that for setting the __FILE__ variable
 */
void jsiSemiInit(bool autoLoad, JsfFileName *loadedFilename) {
  // Set up execInfo.root/etc
  jspInit();
  // Set defaults
  jsiStatus &= JSIS_SOFTINIT_MASK;
#ifndef SAVE_ON_FLASH
  pinBusyIndicator = DEFAULT_BUSY_PIN_INDICATOR;
#endif
  // Set __FILE__ if we have a filename available
  if (loadedFilename)
    jsvObjectSetChildAndUnLock(execInfo.root, "__FILE__", jsfVarFromName(*loadedFilename));

  // Search for invalid storage and erase do this only on first boot.
  // We need to do it before we check storage for any files!
#if !defined(EMSCRIPTEN) && !defined(SAVE_ON_FLASH)
  bool fullTest = jsiStatus & JSIS_FIRST_BOOT;
  if (fullTest) {
#ifdef BANGLEJS
    jsiConsolePrintf("Checking storage...\n");
#endif
    if (!jsfIsStorageValid(JSFSTT_NORMAL | JSFSTT_FIND_FILENAME_TABLE)) {
      jsiConsolePrintf("Storage is corrupt.\n");
      jsfResetStorage();
    } else {
#ifdef BANGLEJS
      jsiConsolePrintf("Storage Ok.\n");
#endif
    }
  }
#endif

  /* If flash contains any code, then we should
     Try and load from it... */
  bool loadFlash = autoLoad && jsfFlashContainsCode();
  if (loadFlash) {
    jsiStatus &= ~JSIS_COMPLETELY_RESET; // loading code, remove this flag
    jspSoftKill();
    jsvSoftKill();
    jsfLoadStateFromFlash();
    jsvSoftInit();
    jspSoftInit();
  }

  // If a password was set, apply the lock
  JsVar *pwd = jsvObjectGetChildIfExists(execInfo.hiddenRoot, PASSWORD_VARIABLE_NAME);
  if (pwd)
    jsiStatus |= JSIS_PASSWORD_PROTECTED;
  jsvUnLock(pwd);

  // Softinit may run initialisation code that will overwrite defaults
  jsiSoftInit(!autoLoad);

#ifdef ESP8266
  jshSoftInit();
#endif
#ifdef ESP32
  jshSoftInit();
#endif

  if (jsiEcho()) { // intentionally not using jsiShowInputLine()
    if (!loadFlash) {
#ifdef USE_TERMINAL
      if (consoleDevice != EV_TERMINAL) // don't spam the terminal
#endif
      jsiConsolePrint(
#ifndef LINUX
          // set up terminal to avoid word wrap
          "\e[?7l"
#endif
          // rectangles @ http://www.network-science.de/ascii/
          "\n"
          " ____                 _ \n"
          "|  __|___ ___ ___ _ _|_|___ ___ \n"
          "|  __|_ -| . |  _| | | |   | . |\n"
          "|____|___|  _|_| |___|_|_|_|___|\n"
          "         |_| espruino.com\n"
          " "JS_VERSION" (c) 2021 G.Williams\n"
        // Point out about donations - but don't bug people
        // who bought boards that helped Espruino
#if !defined(PICO) && !defined(ESPRUINOBOARD) && !defined(ESPRUINOWIFI) && !defined(PUCKJS) && !defined(PIXLJS) && !defined(BANGLEJS) && !defined(EMSCRIPTEN)
          "\n"
          "Espruino is Open Source. Our work is supported\n"
          "only by sales of official boards and donations:\n"
          "http://espruino.com/Donate\n"
#endif
        );
#ifdef ESP8266
      jshPrintBanner();
#endif
    }
#ifdef USE_TERMINAL
    if (consoleDevice != EV_TERMINAL) // don't spam the terminal
#endif
      jsiConsolePrint("\n"); // output new line
    inputLineRemoved = true; // we need to put the input line back...
  }
}

// The 'proper' init function - this should be called only once at bootup
void jsiInit(bool autoLoad) {
  jsiStatus = JSIS_COMPLETELY_RESET | JSIS_FIRST_BOOT;

#if defined(LINUX) || !defined(USB)
  consoleDevice = jsiGetPreferredConsoleDevice();
#else
  consoleDevice = EV_LIMBO;
#endif

#ifndef RELEASE
  jsnSanityTest();
#endif

  jsiSemiInit(autoLoad, NULL/* no filename */);
  // just in case, update the busy indicator
  jsiSetBusy(BUSY_INTERACTIVE, false);
}

#ifndef LINUX
// This should get jsiOneSecondAfterStartupcalled from jshardware.c one second after startup,
// it does initialisation tasks like setting the right console device
void jsiOneSecondAfterStartup() {
  /* When we start up, we put all console output into 'Limbo' (EV_LIMBO),
     because we want to get started immediately, but we don't know where
     to send any console output (USB takes a while to initialise). Not only
     that but if we start transmitting on Serial right away, the first
     char or two can get corrupted.
   */
#ifdef USB

  if (consoleDevice == EV_LIMBO) {
    consoleDevice = jsiGetPreferredConsoleDevice();
    // now move any output that was made to Limbo to the given device
    jshTransmitMove(EV_LIMBO, consoleDevice);
    // finally, kick output - just in case
    jshUSARTKick(consoleDevice);
  } else {
    // the console has already been moved
    jshTransmitClearDevice(EV_LIMBO);
  }
#endif
}
#endif

void jsiKill() {
  jsiSoftKill();
  jspKill();
}

int jsiCountBracketsInInput() {
  int brackets = 0;

  JsLex lex;
  JsLex *oldLex = jslSetLex(&lex);
  jslInit(inputLine);
  while (lex.tk!=LEX_EOF &&
         lex.tk!=LEX_UNFINISHED_COMMENT &&
         lex.tk!=LEX_UNFINISHED_STR &&
         lex.tk!=LEX_UNFINISHED_TEMPLATE_LITERAL) {
    if (lex.tk=='{' || lex.tk=='[' || lex.tk=='(') brackets++;
    if (lex.tk=='}' || lex.tk==']' || lex.tk==')') brackets--;
    if (brackets<0) break; // closing bracket before opening!
    jslGetNextToken();
  }
  if (lex.tk==LEX_UNFINISHED_STR)
    brackets=0; // execute immediately so it can error
  if (lex.tk==LEX_UNFINISHED_COMMENT || lex.tk==LEX_UNFINISHED_TEMPLATE_LITERAL)
    brackets=1000; // if there's an unfinished comment, we're in the middle of something
  jslKill();
  jslSetLex(oldLex);

  return brackets;
}

/// Tries to get rid of some memory (by clearing command history). Returns true if it got rid of something, false if it didn't.
bool jsiFreeMoreMemory() {
#ifdef USE_DEBUGGER
  // remove debug history first
  jsvObjectRemoveChild(execInfo.hiddenRoot, JSI_DEBUG_HISTORY_NAME);
#endif
  // delete history one item at a time
  JsVar *history = jsvObjectGetChildIfExists(execInfo.hiddenRoot, JSI_HISTORY_NAME);
  if (!history) return 0;
  JsVar *item = jsvArrayPopFirst(history);
  bool freed = item!=0;
  jsvUnLock2(item, history);
  // TODO: could also free the array structure?
  // TODO: could look at all streams (Serial1/HTTP/etc) and see if their buffers contain data that could be removed

  return freed;
}

// Return the history array
static JsVar *jsiGetHistory() {
  return jsvObjectGetChild(
      execInfo.hiddenRoot,
#ifdef USE_DEBUGGER
      (jsiStatus & JSIS_IN_DEBUGGER) ? JSI_DEBUG_HISTORY_NAME : JSI_HISTORY_NAME,
#else
      JSI_HISTORY_NAME,
#endif
      JSV_ARRAY);
}

// Add a new line to the command history
void jsiHistoryAddLine(JsVar *newLine) {
  if (!newLine) return;
  size_t len = jsvGetStringLength(newLine);
  if (len==0 || len>500) return; // don't store history for lines of text over 500 chars
  JsVar *history = jsiGetHistory();
  if (!history) return; // out of memory
  // if it was already in history, remove it - we'll put it back in front
  JsVar *alreadyInHistory = jsvGetIndexOf(history, newLine, false/*not exact*/);
  if (alreadyInHistory) {
    jsvRemoveChild(history, alreadyInHistory);
    jsvUnLock(alreadyInHistory);
  }
  // put it back in front
  jsvArrayPush(history, newLine);
  jsvUnLock(history);
}

JsVar *jsiGetHistoryLine(bool previous /* next if false */) {
  JsVar *history = jsiGetHistory();
  if (!history) return 0; // out of memory
  JsVar *historyLine = 0;
  JsVar *idx = jsvGetIndexOf(history, inputLine, true/*exact*/); // get index of current line
  if (idx) {
    if (previous && jsvGetPrevSibling(idx)) {
      historyLine = jsvSkipNameAndUnLock(jsvLock(jsvGetPrevSibling(idx)));
    } else if (!previous && jsvGetNextSibling(idx)) {
      historyLine = jsvSkipNameAndUnLock(jsvLock(jsvGetNextSibling(idx)));
    }
    jsvUnLock(idx);
  } else {
    if (previous) historyLine = jsvSkipNameAndUnLock(jsvGetArrayItem(history, jsvGetArrayLength(history)-1));
    // if next, we weren't using history so couldn't go forwards
  }
  jsvUnLock(history);
  return historyLine;
}

bool jsiIsInHistory(JsVar *line) {
  JsVar *history = jsiGetHistory();
  if (!history) return false;
  JsVar *historyFound = jsvGetIndexOf(history, line, true/*exact*/);
  bool inHistory = historyFound!=0;
  jsvUnLock2(historyFound, history);
  return inHistory;
}

void jsiReplaceInputLine(JsVar *newLine) {
  if (jsiShowInputLine()) {
    jsiMoveCursorChar(inputLine, inputCursorPos, 0); // move cursor to start
    jsiConsoleEraseAfterCursor(); // delete all to right and down
    jsiConsolePrintStringVarWithNewLineChar(newLine,0,':');
  }
  jsiInputLineCursorMoved();
  jsvUnLock(inputLine);
  inputLine = jsvLockAgain(newLine);
  inputCursorPos = jsvGetStringLength(inputLine);
}

void jsiChangeToHistory(bool previous) {
  JsVar *nextHistory = jsiGetHistoryLine(previous);
  if (nextHistory) {
    jsiReplaceInputLine(nextHistory);
    jsvUnLock(nextHistory);
    hasUsedHistory = true;
  } else if (!previous) { // if next, but we have something, just clear the line
    if (jsiShowInputLine()) {
      jsiMoveCursorChar(inputLine, inputCursorPos, 0); // move cursor to start
      jsiConsoleEraseAfterCursor(); // delete all to right and down
    }
    jsiInputLineCursorMoved();
    jsvUnLock(inputLine);
    inputLine = jsvNewFromEmptyString();
    inputCursorPos = 0;
  }
}

void jsiIsAboutToEditInputLine() {
  // we probably plan to do something with the line now - check it wasn't in history
  // and if it was, duplicate it
  if (hasUsedHistory) {
    hasUsedHistory = false;
    if (jsiIsInHistory(inputLine)) {
      JsVar *newLine = jsvCopy(inputLine, false);
      if (newLine) { // could have been out of memory!
        jsiInputLineCursorMoved();
        jsvUnLock(inputLine);
        inputLine = newLine;
      }
    }
  }
}

void jsiHandleDelete(bool isBackspace) {
  size_t l = jsvGetStringLength(inputLine);
  if (isBackspace && inputCursorPos==0) return; // at beginning of line
  if (!isBackspace && inputCursorPos>=l) return; // at end of line
  // work out if we are deleting a newline
  bool deleteNewline = (isBackspace && jsvGetCharInString(inputLine,inputCursorPos-1)=='\n') ||
      (!isBackspace && jsvGetCharInString(inputLine,inputCursorPos)=='\n');
  // If we mod this to keep the string, use jsiIsAboutToEditInputLine
  if (deleteNewline && jsiShowInputLine()) {
    jsiConsoleEraseAfterCursor(); // erase all in front
    if (isBackspace) {
      // delete newline char
      jsiConsolePrint("\x08 "); // delete and then send space
      jsiMoveCursorChar(inputLine, inputCursorPos, inputCursorPos-1); // move cursor back
      jsiInputLineCursorMoved();
    }
  }

  JsVar *v = jsvNewFromEmptyString();
  size_t p = inputCursorPos;
  if (isBackspace) p--;
  if (p>0) jsvAppendStringVar(v, inputLine, 0, p); // add before cursor (delete)
  if (p+1<l) jsvAppendStringVar(v, inputLine, p+1, JSVAPPENDSTRINGVAR_MAXLENGTH); // add the rest
  jsiInputLineCursorMoved();
  jsvUnLock(inputLine);
  inputLine=v;
  if (isBackspace)
    inputCursorPos--; // move cursor back

  // update the console
  if (jsiShowInputLine()) {
    if (deleteNewline) {
      // we already removed everything, so just put it back
      jsiConsolePrintStringVarWithNewLineChar(inputLine, inputCursorPos, ':');
      jsiMoveCursorChar(inputLine, jsvGetStringLength(inputLine), inputCursorPos); // move cursor back
    } else {
      // clear the character and move line back
      if (isBackspace) jsiConsolePrintChar(0x08);
      jsiConsolePrintStringVarUntilEOL(inputLine, inputCursorPos, 0xFFFFFFFF, true/*and backup*/);
    }
  }
}

void jsiHandleHome() {
  while (inputCursorPos>0 && jsvGetCharInString(inputLine,inputCursorPos-1)!='\n') {
    if (jsiShowInputLine()) jsiConsolePrintChar(0x08);
    inputCursorPos--;
  }
}

void jsiHandleEnd() {
  size_t l = jsvGetStringLength(inputLine);
  while (inputCursorPos<l && jsvGetCharInString(inputLine,inputCursorPos)!='\n') {
    if (jsiShowInputLine())
      jsiConsolePrintChar(jsvGetCharInString(inputLine,inputCursorPos));
    inputCursorPos++;
  }
}

/** Page up/down move cursor to beginnint or end */
void jsiHandlePageUpDown(bool isDown) {
  size_t x,y;
  jsvGetLineAndCol(inputLine, inputCursorPos, &y, &x);
  if (!isDown) { // up
    inputCursorPos = 0;
  } else { // down
    inputCursorPos = jsvGetStringLength(inputLine);
  }
  size_t newX=x,newY=y;
  jsvGetLineAndCol(inputLine, inputCursorPos, &newY, &newX);
  jsiMoveCursor(x,y,newX,newY);
}

void jsiHandleMoveUpDown(int direction) {
  size_t x,y, lines=jsvGetLinesInString(inputLine);
  jsvGetLineAndCol(inputLine, inputCursorPos, &y, &x);
  size_t newX=x,newY=y;
  newY = (size_t)((int)newY + direction);
  if (newY<1) newY=1;
  if (newY>lines) newY=lines;
  // work out cursor pos and feed back through - we might not be able to get right to the same place
  // if we move up
  inputCursorPos = jsvGetIndexFromLineAndCol(inputLine, newY, newX);
  jsvGetLineAndCol(inputLine, inputCursorPos, &newY, &newX);
  if (jsiShowInputLine()) {
    jsiMoveCursor(x,y,newX,newY);
  }
}

bool jsiAtEndOfInputLine() {
  size_t i = inputCursorPos, l = jsvGetStringLength(inputLine);
  while (i < l) {
    if (!isWhitespace(jsvGetCharInString(inputLine, i)))
      return false;
    i++;
  }
  return true;
}

void jsiCheckErrors() {
  if (jsiStatus & JSIS_EVENTEMITTER_INTERRUPTED) {
    jspSetInterrupted(false);
    jsiStatus &= ~JSIS_EVENTEMITTER_INTERRUPTED;
    jsiConsoleRemoveInputLine();
    jsiConsolePrint("Execution Interrupted during event processing.\n");
  }
  bool reportedError = false;
  JsVar *exception = jspGetException();
  if (exception) {
    if (jsiExecuteEventCallbackOn("process", JS_EVENT_PREFIX"uncaughtException", 1, &exception)) {
      jsvUnLock(exception);
      exception = jspGetException();
    }
  }
  if (exception) {
    jsiConsoleRemoveInputLine();
    jsiConsolePrintf("Uncaught %v\n", exception);
    reportedError = true;
    if (jsvIsObject(exception)) {
      JsVar *stackTrace = jsvObjectGetChildIfExists(exception, "stack");
      if (stackTrace) {
        jsiConsolePrintStringVar(stackTrace);
        jsvUnLock(stackTrace);
      }
    }
    jsvUnLock(exception);
  }
  if (jspIsInterrupted()
#ifdef USE_DEBUGGER
      && !(jsiStatus & JSIS_EXIT_DEBUGGER)
#endif
      ) {
    jsiConsoleRemoveInputLine();
    jsiConsolePrint("Execution Interrupted\n");
    jspSetInterrupted(false);
    reportedError = true;
  }
  JsVar *stackTrace = jspGetStackTrace();
  if (stackTrace) {
    if (reportedError)
      jsiConsolePrintStringVar(stackTrace);
    jsvUnLock(stackTrace);
  }
  if (jspHasError()) {
    // don't report an issue - we get unreported errors is process.on('unhandledException',)/etc is used
    //if (!reportedError) jsiConsolePrint("Error.\n");
    // remove any error flags
    execInfo.execute &= (JsExecFlags)~EXEC_ERROR_MASK;
  }
  if (lastJsErrorFlags != jsErrorFlags) {
    JsErrorFlags newErrors = jsErrorFlags & ~lastJsErrorFlags;
    if (newErrors & ~JSERR_WARNINGS_MASK) {
      JsVar *v = jswrap_espruino_getErrorFlagArray(newErrors);
      jsiExecuteEventCallbackOn("E", JS_EVENT_PREFIX"errorFlag", 1, &v);
      if (v) {
        jsiConsoleRemoveInputLine();
        jsiConsolePrintf("New interpreter error: %v\n", v);
        jsvUnLock(v);
      }
    }
    lastJsErrorFlags = jsErrorFlags;
  }
}


void jsiAppendStringToInputLine(const char *strToAppend) {
  // Add the string to our input line
  jsiIsAboutToEditInputLine();

  size_t strSize = 1;
  while (strToAppend[strSize]) strSize++;

  if (inputLineLength < 0)
    inputLineLength = (int)jsvGetStringLength(inputLine);

  if ((int)inputCursorPos>=inputLineLength) { // append to the end
    jsiAppendToInputLine(strToAppend);
  } else { // add in halfway through
    JsVar *v = jsvNewFromEmptyString();
    if (inputCursorPos>0) jsvAppendStringVar(v, inputLine, 0, inputCursorPos);
    jsvAppendString(v, strToAppend);
    jsvAppendStringVar(v, inputLine, inputCursorPos, JSVAPPENDSTRINGVAR_MAXLENGTH); // add the rest
    jsiInputLineCursorMoved();
    jsvUnLock(inputLine);
    inputLine=v;
    if (jsiShowInputLine()) jsiConsolePrintStringVarUntilEOL(inputLine, inputCursorPos, 0xFFFFFFFF, true/*and backup*/);
  }
  inputCursorPos += strSize; // no need for jsiInputLineCursorMoved(); as we just appended
  if (jsiShowInputLine()) {
    jsiConsolePrintString(strToAppend);
  }
}

#ifdef USE_TAB_COMPLETE

typedef struct {
  size_t partialLen;
  JsVar *partial;
  JsVar *possible;
  int matches;
  size_t lineLength;
} JsiTabCompleteData;

void jsiTabComplete_findCommon(void *cbdata, JsVar *key) {
  JsiTabCompleteData *data = (JsiTabCompleteData*)cbdata;
  if (jsvGetStringLength(key)>data->partialLen && jsvCompareString(data->partial, key, 0, 0, true)==0) {
    data->matches++;
    if (data->possible) {
      JsVar *v = jsvGetCommonCharacters(data->possible, key);
      jsvUnLock(data->possible);
      data->possible = v;
    } else {
      data->possible = jsvLockAgain(key);
    }
  }
}

void jsiTabComplete_printCommon(void *cbdata, JsVar *key) {
  JsiTabCompleteData *data = (JsiTabCompleteData*)cbdata;
  if (jsvGetStringLength(key)>data->partialLen && jsvCompareString(data->partial, key, 0, 0, true)==0) {
    // Print, but do as 2 columns
    if (data->lineLength==0) {
      jsiConsolePrintf("%v",key);
      data->lineLength = jsvGetStringLength(key);
    } else {
      if (data->lineLength>=20)
        data->lineLength=19; // force one space
      while (data->lineLength<20) {
        jsiConsolePrintChar(' ');
        data->lineLength++;
      }
      jsiConsolePrintf("%v\n",key);
      data->lineLength = 0;
    }
  }
}

void jsiTabComplete() {
  if (!jsvIsString(inputLine)) return;
  JsVar *object = 0;
  JsiTabCompleteData data;
  data.partial = 0;
  size_t partialStart = 0;

  JsLex lex;
  JsLex *oldLex = jslSetLex(&lex);
  jslInit(inputLine);
  while (lex.tk!=LEX_EOF && (lex.tokenStart+1)<=inputCursorPos) {
    if (lex.tk=='.') {
      jsvUnLock(object);
      object = data.partial;
      data.partial = 0;
    } else if (lex.tk==LEX_ID) {
      jsvUnLock(data.partial);
      data.partial = jslGetTokenValueAsVar();
      partialStart = lex.tokenStart+1;
    } else {
      jsvUnLock(object);
      object = 0;
      jsvUnLock(data.partial);
      data.partial = 0;
    }
    jslGetNextToken();
  }
  jslKill();
  jslSetLex(oldLex);
  if (!object && !data.partial) {
    return;
  }
  if (data.partial) {
    data.partialLen = jsvGetStringLength(data.partial);
    size_t actualPartialLen = inputCursorPos + 1 - partialStart;
    if (actualPartialLen > data.partialLen) {
      // we had a token but were past the end of it when asked
      // to autocomplete ---> no token
      jsvUnLock2(object, data.partial);
      return;
    } else if (actualPartialLen < data.partialLen) {
      JsVar *v = jsvNewFromStringVar(data.partial, 0, actualPartialLen);
      jsvUnLock(data.partial);
      data.partial = v;
      data.partialLen = actualPartialLen;
    }
  } else {
    data.partial = jsvNewFromEmptyString();
    data.partialLen = 0;
  }

  // If we had the name of an object here, try and look it up
  if (object) {
    char s[JSLEX_MAX_TOKEN_LENGTH];
    jsvGetString(object, s, sizeof(s));
    JsVar *v = jspGetNamedVariable(s);
    if (jsvIsVariableDefined(v)) {
      v = jsvSkipNameAndUnLock(v);
    } else {
      jsvUnLock(v);
      v = 0;
    }
    jsvUnLock(object);
    object = v;
    // If we couldn't look it up, don't offer any suggestions
    if (!v) {
      jsvUnLock(data.partial);
      return;
    }
  }
  if (!object) {
    // default to root scope
    object = jsvLockAgain(execInfo.root);
  }
  // Now try and autocomplete
  data.possible = 0;
  data.matches = 0;
  jswrap_object_keys_or_property_names_cb(object, JSWOKPF_INCLUDE_NON_ENUMERABLE|JSWOKPF_INCLUDE_PROTOTYPE|JSWOKPF_NO_INCLUDE_ARRAYBUFFER, jsiTabComplete_findCommon, &data);
  // If we've got >1 match and are at the end of a line, print hints
  if (data.matches>1) {
    // Remove the current line and add a newline
    jsiMoveCursorChar(inputLine, inputCursorPos, (size_t)inputLineLength);
    inputLineRemoved = true;
    jsiConsolePrint("\n\n");
    data.lineLength = 0;
    // Output hints
    jswrap_object_keys_or_property_names_cb(object, JSWOKPF_INCLUDE_NON_ENUMERABLE|JSWOKPF_INCLUDE_PROTOTYPE|JSWOKPF_NO_INCLUDE_ARRAYBUFFER, jsiTabComplete_printCommon, &data);
    if (data.lineLength) jsiConsolePrint("\n");
    jsiConsolePrint("\n");
    // Return the input line
    jsiConsoleReturnInputLine();
  }
  jsvUnLock2(object, data.partial);
  // apply the completion
  if (data.possible) {
    char buf[JSLEX_MAX_TOKEN_LENGTH];
    jsvGetString(data.possible, buf, sizeof(buf));
    if (data.partialLen < strlen(buf))
      jsiAppendStringToInputLine(&buf[data.partialLen]);
    jsvUnLock(data.possible);
  }
}
#endif // USE_TAB_COMPLETE

void jsiHandleNewLine(bool execute) {
  if (jsiAtEndOfInputLine()) { // at EOL so we need to figure out if we can execute or not
    if (execute && jsiCountBracketsInInput()<=0) { // actually execute!
      if (jsiShowInputLine()) {
        jsiConsolePrint("\n");
      }
      if (!(jsiStatus & JSIS_ECHO_OFF_FOR_LINE))
        inputLineRemoved = true;

      // Get line to execute, and reset inputLine
      JsVar *lineToExecute = jsvStringTrimRight(inputLine);
      jsiClearInputLine(false);
#ifdef USE_DEBUGGER
      if (jsiStatus & JSIS_IN_DEBUGGER) {
        jsiDebuggerLine(lineToExecute);
        jsiHistoryAddLine(lineToExecute);
        jsvUnLock(lineToExecute);
      } else
#endif
      {
        // execute!
        JsVar *v = jspEvaluateVar(lineToExecute, 0, jsiLineNumberOffset);
        // add input line to history
        bool isEmpty = jsvIsEmptyString(lineToExecute);
        // Don't store history if we're not echoing back to the console (it probably wasn't typed by the user)
        if (!isEmpty && jsiEcho())
          jsiHistoryAddLine(lineToExecute);
        jsvUnLock(lineToExecute);
        jsiLineNumberOffset = 0; // forget the current line number now
        // print result (but NOT if we had an error)
        if (jsiEcho() && !jspHasError() && !isEmpty) {
          jsiConsolePrintChar('=');
          jsfPrintJSON(v, JSON_LIMIT | JSON_SOME_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES | JSON_SHOW_OBJECT_NAMES | JSON_DROP_QUOTES);
          jsiConsolePrint("\n");
        }
        jsvUnLock(v);
      }
      jsiCheckErrors();
      // console will be returned next time around the input loop
      // if we had echo off just for this line, reinstate it!
      jsiStatus &= ~JSIS_ECHO_OFF_FOR_LINE;
    } else {
      // Brackets aren't all closed, so we're going to append a newline
      // without executing
      if (jsiShowInputLine()) jsiConsolePrint("\n:");
      jsiIsAboutToEditInputLine();
      jsiAppendToInputLine("\n");
      inputCursorPos++;
    }
  } else { // new line - but not at end of line!
    jsiIsAboutToEditInputLine();
    if (jsiShowInputLine()) jsiConsoleEraseAfterCursor(); // erase all in front
    JsVar *v = jsvNewFromEmptyString();
    if (inputCursorPos>0) jsvAppendStringVar(v, inputLine, 0, inputCursorPos);
    jsvAppendCharacter(v, '\n');
    jsvAppendStringVar(v, inputLine, inputCursorPos, JSVAPPENDSTRINGVAR_MAXLENGTH); // add the rest
    jsiInputLineCursorMoved();
    jsvUnLock(inputLine);
    inputLine=v;
    if (jsiShowInputLine()) { // now print the rest
      jsiConsolePrintStringVarWithNewLineChar(inputLine, inputCursorPos, ':');
      jsiMoveCursorChar(inputLine, jsvGetStringLength(inputLine), inputCursorPos+1); // move cursor back
    }
    inputCursorPos++;
  }
}


void jsiHandleChar(char ch) {
  //jsiConsolePrintf("[%d:%d]\n", inputState, ch);
  //
  // special stuff
  // 1 - Ctrl-a - beginning of line
  // 4 - Ctrl-d - backwards delete
  // 5 - Ctrl-e - end of line
  // 21 - Ctrl-u - delete line
  // 23 - Ctrl-w - delete word (currently just does the same as Ctrl-u)
  //
  // 27 then 91 then 68 ('D') - left
  // 27 then 91 then 67 ('C') - right
  // 27 then 91 then 65 ('A') - up
  // 27 then 91 then 66 ('B') - down
  // 27 then 91 then 70 - home
  // 27 then 91 then 72 - end
  //
  // 27 then 91 then 48-57 (numeric digits) then 'd' - set line number, used for that
  //                              inputLine and put into any declared functions
  // 27 then 91 then 49 ('1') then 126 - numpad home
  // 27 then 91 then 50 ('2') then 75 - Erases the entire current line.
  // 27 then 91 then 51 ('3') then 126 - backwards delete
  // 27 then 91 then 52 ('4') then 126 - numpad end
  // 27 then 91 then 53 ('5') then 126 - pgup
  // 27 then 91 then 54 ('6') then 126 - pgdn

  // 27 then 79 then 70 - home
  // 27 then 79 then 72 - end
  // 27 then 10 - alt enter

  if (jsiPasswordProtected()) {
    if (ch=='\r' || ch==10) {
      JsVar *pwd = jsvObjectGetChildIfExists(execInfo.hiddenRoot, PASSWORD_VARIABLE_NAME);
      // check password
      if (pwd && jsvCompareString(inputLine, pwd, 0, 0, false)==0)
        jsiStatus &= ~JSIS_PASSWORD_PROTECTED;
      jsvUnLock(pwd);
      jsiClearInputLine(false);
      if (jsiPasswordProtected()) {
        jsiConsolePrint("\n  Invalid password\npassword>");
      } else {
        jsiConsolePrint("\n  Logged in.\n");
        inputLineRemoved = true;
        jsiConsoleReturnInputLine();
      }
    } else {
      char str[2];
      str[0] = ch;
      str[1] = 0;
      if (jsvGetStringLength(inputLine)<20)
        jsiAppendToInputLine(str);
    }
    return;
  }

  if (ch == 0) {
    inputState = IS_NONE; // ignore 0 - it's scary
  } else if (ch == 1) { // Ctrl-a
    jsiHandleHome();
    // Ctrl-C (char code 3) gets handled in an IRQ
  } else if (ch == 4) { // Ctrl-d
    jsiHandleDelete(false/*not backspace*/);
  } else if (ch == 5) { // Ctrl-e
    jsiHandleEnd();
  } else if (ch == 21 || ch == 23) { // Ctrl-u or Ctrl-w
    jsiClearInputLine(true);
  } else if (ch == 27) {
    inputState = IS_HAD_27;
  } else if (inputState==IS_HAD_27) {
    inputState = IS_NONE;
    if (ch == 79)
      inputState = IS_HAD_27_79;
    else if (ch == 91)
      inputState = IS_HAD_27_91;
    else if (ch == 10)
      jsiHandleNewLine(false);
  } else if (inputState==IS_HAD_27_79) { // Numpad
    inputState = IS_NONE;
    if (ch == 70) jsiHandleEnd();
    else if (ch == 72) jsiHandleHome();
    else if (ch == 111) jsiHandleChar('/');
    else if (ch == 106) jsiHandleChar('*');
    else if (ch == 109) jsiHandleChar('-');
    else if (ch == 107) jsiHandleChar('+');
    else if (ch == 77) jsiHandleChar('\r');
  } else if (inputState==IS_HAD_27_91) {
    inputState = IS_NONE;
    if (ch>='0' && ch<='9') {
      inputStateNumber = (uint16_t)(ch-'0');
      inputState = IS_HAD_27_91_NUMBER;
    } else if (ch==68) { // left
      if (inputCursorPos>0 && jsvGetCharInString(inputLine,inputCursorPos-1)!='\n') {
        inputCursorPos--;
        if (jsiShowInputLine()) {
          jsiConsolePrint("\x1B[D"); // 27,91,68 - left
        }
      }
    } else if (ch==67) { // right
      if (inputCursorPos<jsvGetStringLength(inputLine) && jsvGetCharInString(inputLine,inputCursorPos)!='\n') {
        inputCursorPos++;
        if (jsiShowInputLine()) {
          jsiConsolePrint("\x1B[C"); // 27,91,67 - right
        }
      }
    } else if (ch==65) { // up
      size_t l = jsvGetStringLength(inputLine);
      if ((l==0 || jsiIsInHistory(inputLine)) && inputCursorPos==l)
        jsiChangeToHistory(true); // if at end of line
      else
        jsiHandleMoveUpDown(-1);
    } else if (ch==66) { // down
      size_t l = jsvGetStringLength(inputLine);
      if ((l==0 || jsiIsInHistory(inputLine)) && inputCursorPos==l)
        jsiChangeToHistory(false); // if at end of line
      else
        jsiHandleMoveUpDown(1);
    } else if (ch == 70) jsiHandleEnd();
    else if (ch == 72) jsiHandleHome();
    //else jsiConsolePrintf("[%d:%d]\n", inputState, ch); // debugging unknown escape sequence
  } else if (inputState==IS_HAD_27_91_NUMBER) {
    if (ch>='0' && ch<='9') {
      inputStateNumber = (uint16_t)(10*inputStateNumber + ch - '0');
    } else {
      if (ch=='d') jsiLineNumberOffset = inputStateNumber;
      else if (ch=='H' /* 75 */) {
        if (inputStateNumber==2) jsiClearInputLine(true); // Erase current line
      } else if (ch==126) {
        if (inputStateNumber==1) jsiHandleHome(); // Numpad Home
        else if (inputStateNumber==3) jsiHandleDelete(false/*not backspace*/); // Numpad (forwards) Delete
        else if (inputStateNumber==4) jsiHandleEnd(); // Numpad End
        else if (inputStateNumber==5) jsiHandlePageUpDown(0); // Page Up
        else if (inputStateNumber==6) jsiHandlePageUpDown(1); // Page Down
      }
      inputState = IS_NONE;
    }
  } else if (ch==16 && jsvGetStringLength(inputLine)==0) {
    /* DLE - Data Link Escape
    Espruino uses DLE on the start of a line to signal that just the line in
    question should be executed without echo */
    jsiStatus  |= JSIS_ECHO_OFF_FOR_LINE;
  } else {
    inputState = IS_NONE;
    if (ch == 0x08 || ch == 0x7F /*delete*/) {
      jsiHandleDelete(true /*backspace*/);
    } else if (ch == '\n' && inputState == IS_HAD_R) {
      inputState = IS_NONE; //  ignore \ r\n - we already handled it all on \r
    } else if (ch == '\r' || ch == '\n') {
      if (ch == '\r') inputState = IS_HAD_R;
      jsiHandleNewLine(true);
#ifdef USE_TAB_COMPLETE
    } else if (ch=='\t' && jsiEcho()) {
      jsiTabComplete();
#endif
    } else if (((unsigned char)ch)>=32 || ch=='\t') {
      char buf[2] = {ch,0};
      const char *strToAppend = (ch=='\t') ? "    " : buf;
      jsiAppendStringToInputLine(strToAppend);
    }
  }
}

/// Queue a function, string, or array (of funcs/strings) to be executed next time around the idle loop
void jsiQueueEvents(JsVar *object, JsVar *callback, JsVar **args, int argCount) { // an array of functions, a string, or a single function
  assert(argCount<10);
  JsVar *event = jsvNewObject();
  if (event) { // Could be out of memory error!
    jsvUnLock(jsvAddNamedChild(event, callback, "func"));
    if (argCount) {
      JsVar *arr = jsvNewArray(args, argCount);
      if (arr)
        jsvAddNamedChildAndUnLock(event, arr, "args");
    }
    if (object) jsvUnLock(jsvAddNamedChild(event, object, "this"));
    jsvArrayPushAndUnLock(events, event);
  }
}

bool jsiObjectHasCallbacks(JsVar *object, const char *callbackName) {
  JsVar *callback = jsvObjectGetChildIfExists(object, callbackName);
  bool hasCallbacks = !jsvIsUndefined(callback);
  jsvUnLock(callback);
  return hasCallbacks;
}

void jsiQueueObjectCallbacks(JsVar *object, const char *callbackName, JsVar **args, int argCount) {
  JsVar *callback = jsvObjectGetChildIfExists(object, callbackName);
  if (!callback) return;
  jsiQueueEvents(object, callback, args, argCount);
  jsvUnLock(callback);
}

void jsiExecuteEvents() {
  bool hasEvents = !jsvArrayIsEmpty(events);
  if (hasEvents) jsiSetBusy(BUSY_INTERACTIVE, true);
  while (!jsvArrayIsEmpty(events)) {
    JsVar *event = jsvSkipNameAndUnLock(jsvArrayPopFirst(events));
    // Get function to execute
    JsVar *func = jsvObjectGetChildIfExists(event, "func");
    JsVar *thisVar = jsvObjectGetChildIfExists(event, "this");
    JsVar *argsArray = jsvObjectGetChildIfExists(event, "args");
    // free actual event
    jsvUnLock(event);
    // now run..
    jsiExecuteEventCallbackArgsArray(thisVar, func, argsArray);
    jsvUnLock(argsArray);
    //jsPrint("Event Done\n");
    jsvUnLock2(func, thisVar);
  }
  if (hasEvents) {
    jsiSetBusy(BUSY_INTERACTIVE, false);
    if (jspIsInterrupted())
      jsiStatus |= JSIS_EVENTEMITTER_INTERRUPTED;
  }
}

NO_INLINE bool jsiExecuteEventCallbackArgsArray(JsVar *thisVar, JsVar *callbackVar, JsVar *argsArray) { // array of functions or single function
  unsigned int l = 0;
  JsVar **args = 0;
  if (argsArray) {
    assert(jsvIsArray(argsArray));
    l = (unsigned int)jsvGetArrayLength(argsArray);
    if (l) {
      args = alloca(sizeof(JsVar*) * l);
      if (!args) return false;
      jsvGetArrayItems(argsArray, l, args); // not very fast
    }
  }
  bool r = jsiExecuteEventCallback(thisVar, callbackVar, l, args);
  jsvUnLockMany(l, args);
  return r;
}

static NO_INLINE bool jsiExecuteEventCallbackInner(JsVar *thisVar, JsVar *callbackNoNames, unsigned int argCount, JsVar **argPtr) { // array of functions or single function
  if (!callbackNoNames) return false;

  bool ok = true;
  if (jsvIsArray(callbackNoNames)) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, callbackNoNames);
    while (ok && jsvObjectIteratorHasValue(&it) && !(jsiStatus & JSIS_EVENTEMITTER_STOP)) {
      JsVar *child = jsvObjectIteratorGetValue(&it); // name already skipped
      ok &= jsiExecuteEventCallbackInner(thisVar, child, argCount, argPtr);
      jsvUnLock(child);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
  } else if (jsvIsFunction(callbackNoNames)) {
    jsvUnLock(jspExecuteFunction(callbackNoNames, thisVar, (int)argCount, argPtr));
  } else if (jsvIsString(callbackNoNames)) {
    jsvUnLock(jspEvaluateVar(callbackNoNames, 0, 0));
  } else
    jsError("Unknown type of callback in Event Queue");
  return ok;
}

NO_INLINE bool jsiExecuteEventCallback(JsVar *thisVar, JsVar *callbackVar, unsigned int argCount, JsVar **argPtr) { // array of functions or single function
  JsVar *callbackNoNames = jsvSkipName(callbackVar);
  if (!callbackNoNames) return false;

  jsiStatus |= JSIS_EVENTEMITTER_PROCESSING;
  bool ok = jsiExecuteEventCallbackInner(thisVar, callbackNoNames, argCount, argPtr);
  jsvUnLock(callbackNoNames);
  jsiStatus &= ~(JSIS_EVENTEMITTER_PROCESSING|JSIS_EVENTEMITTER_STOP);
  if (!ok || jspIsInterrupted()) {
    jsiStatus |= JSIS_EVENTEMITTER_INTERRUPTED;
    return false;
  }
  return true;
}

// Execute the named Event callback on object, and return true if it exists
bool jsiExecuteEventCallbackName(JsVar *obj, const char *cbName, unsigned int argCount, JsVar **argPtr) {
  bool executed = false;
  if (jsvHasChildren(obj)) {
    JsVar *callback = jsvObjectGetChildIfExists(obj, cbName);
    if (callback) {
      jsiExecuteEventCallback(obj, callback, argCount, argPtr);
      executed = true;
    }
    jsvUnLock(callback);
  }
  return executed;
}

// Execute the named Event callback on the named object, and return true if it exists
bool jsiExecuteEventCallbackOn(const char *objectName, const char *cbName, unsigned int argCount, JsVar **argPtr) {
  JsVar *obj = jsvObjectGetChildIfExists(execInfo.root, objectName);
  bool executed = jsiExecuteEventCallbackName(obj, cbName, argCount, argPtr);
  jsvUnLock(obj);
  return executed;
}

/// Create a timeout in JS to execute the given native function (outside of an IRQ). Returns the index
JsVar *jsiSetTimeout(void (*functionPtr)(void), JsVarFloat milliseconds) {
  JsVar *fn = jsvNewNativeFunction((void (*)(void))functionPtr, JSWAT_VOID);
  if (!fn) return 0;
  JsVar *idx = jswrap_interface_setTimeout(fn, milliseconds, 0);
  jsvUnLock(fn);
  return idx;
}

bool jsiHasTimers() {
  if (!timerArray) return false;
  JsVar *timerArrayPtr = jsvLock(timerArray);
  bool hasTimers = !jsvArrayIsEmpty(timerArrayPtr);
  jsvUnLock(timerArrayPtr);
  return hasTimers;
}

/// Is the given watch object meant to be executed when the current value of the pin is pinIsHigh
bool jsiShouldExecuteWatch(JsVar *watchPtr, bool pinIsHigh) {
  int watchEdge = (int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(watchPtr, "edge"));
  return watchEdge==0 || // any edge
      (pinIsHigh && watchEdge>0) || // rising edge
      (!pinIsHigh && watchEdge<0); // falling edge
}

bool jsiIsWatchingPin(Pin pin) {
  if (jshGetPinShouldStayWatched(pin))
    return true;
  bool isWatched = false;
  JsVar *watchArrayPtr = jsvLock(watchArray);
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, watchArrayPtr);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *watchPtr = jsvObjectIteratorGetValue(&it);
    JsVar *pinVar = jsvObjectGetChildIfExists(watchPtr, "pin");
    if (jshGetPinFromVar(pinVar) == pin)
      isWatched = true;
    jsvUnLock2(pinVar, watchPtr);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  jsvUnLock(watchArrayPtr);
  return isWatched;
}

void jsiCtrlC() {
  // If password protected, don't let Ctrl-C break out of running code!
  if (jsiPasswordProtected())
    return;
  // Force a break...
  execInfo.execute |= EXEC_CTRL_C;
}

/** Grab as many characters as possible from the event queue for the given event
   and return a JsVar containing them. 'eventsHandled' is set to the number of
   extra events (not characters) is returned */
static JsVar *jsiExtractIOEventData(IOEvent *event, int *eventsHandled) {
  assert(eventsHandled);
  *eventsHandled = 0;

  JsVar *stringData = jsvNewFromEmptyString();
  if (stringData) {
    JsvStringIterator it;
    jsvStringIteratorNew(&it, stringData, 0);

    int i, chars = IOEVENTFLAGS_GETCHARS(event->flags);
    while (chars) {
      for (i=0;i<chars;i++) {
        jsvStringIteratorAppend(&it, event->data.chars[i]);
      }
      // look down the stack and see if there is more data
      if (jshIsTopEvent(IOEVENTFLAGS_GETTYPE(event->flags))) {
        jshPopIOEvent(event);
        (*eventsHandled)++;
        chars = IOEVENTFLAGS_GETCHARS(event->flags);
      } else
        chars = 0;
    }
    jsvStringIteratorFree(&it);
  }
  return stringData;
}

/** Take an event for a UART and handle the characters we're getting, potentially
 * grabbing more characters as well if it's easy. If more character events are
 * grabbed, the number of extra events (not characters) is returned */
int jsiHandleIOEventForSerial(JsVar *usartClass, IOEvent *event) {
  int eventsHandled = 0;
  JsVar *stringData = jsiExtractIOEventData(event,  &eventsHandled);
  if (stringData) {
    // Now run the handler
    jswrap_stream_pushData(usartClass, stringData, true);
    jsvUnLock(stringData);
  }
  return eventsHandled;
}

void jsiHandleIOEventForConsole(IOEvent *event) {
  int i, c = IOEVENTFLAGS_GETCHARS(event->flags);
  jsiSetBusy(BUSY_INTERACTIVE, true);
  for (i=0;i<c;i++) jsiHandleChar(event->data.chars[i]);
  jsiSetBusy(BUSY_INTERACTIVE, false);
}

void jsiIdle() {
  // This is how many times we have been here and not done anything.
  // It will be zeroed if we do stuff later
  if (loopsIdling<255) loopsIdling++;

  // Handle hardware-related idle stuff (like checking for pin events)
  bool wasBusy = false;
  IOEvent event;
  // ensure we can't get totally swamped by having more events than we can process.
  // Just process what was in the event queue at the start
  int maxEvents = jshGetEventsUsed();

  while ((maxEvents--)>0 && jshPopIOEvent(&event)) {
    jsiSetBusy(BUSY_INTERACTIVE, true);
    wasBusy = true;

    IOEventFlags eventType = IOEVENTFLAGS_GETTYPE(event.flags);

    loopsIdling = 0; // because we're not idling
    if (eventType == consoleDevice) {
      jsiHandleIOEventForConsole(&event);
      /** don't allow us to read data when the device is our
       console device. It slows us down and just causes pain. */
    } else if (DEVICE_IS_SERIAL(eventType)) {
      // ------------------------------------------------------------------------ SERIAL CALLBACK
      JsVar *usartClass = jsvSkipNameAndUnLock(jsiGetClassNameFromDevice(eventType));
      if (jsvIsObject(usartClass)) {
        maxEvents -= jsiHandleIOEventForSerial(usartClass, &event);
      }
      jsvUnLock(usartClass);
#if USART_COUNT>0
    } else if (DEVICE_IS_USART_STATUS(eventType)) {
      // ------------------------------------------------------------------------ SERIAL STATUS CALLBACK
      JsVar *usartClass = jsvSkipNameAndUnLock(jsiGetClassNameFromDevice(IOEVENTFLAGS_GETTYPE(IOEVENTFLAGS_SERIAL_STATUS_TO_SERIAL(event.flags))));
      if (jsvIsObject(usartClass)) {
        if (event.flags & EV_SERIAL_STATUS_FRAMING_ERR)
          jsiExecuteEventCallbackName(usartClass, JS_EVENT_PREFIX"framing", 0, 0);
        if (event.flags & EV_SERIAL_STATUS_PARITY_ERR)
          jsiExecuteEventCallbackName(usartClass, JS_EVENT_PREFIX"parity", 0, 0);
      }
      jsvUnLock(usartClass);
#endif
#ifdef BLUETOOTH
    } else if ((eventType == EV_BLUETOOTH_PENDING) || (eventType == EV_BLUETOOTH_PENDING_DATA)) {
      maxEvents -= jsble_exec_pending(&event);
#endif
#ifdef I2C_SLAVE
    } else if (DEVICE_IS_I2C(eventType)) {
      // ------------------------------------------------------------------------ I2C CALLBACK
      JsVar *i2cClass = jsvSkipNameAndUnLock(jsiGetClassNameFromDevice(eventType));
      if (jsvIsObject(i2cClass)) {
        uint8_t addr = event.data.time&0xff;
        int len = event.data.time>>8;
        JsVar *obj = jsvNewObject();
        if (obj) {
          jsvObjectSetChildAndUnLock(obj, "addr", jsvNewFromInteger(addr&0x7F));
          jsvObjectSetChildAndUnLock(obj, "length", jsvNewFromInteger(len));
          jsiExecuteEventCallbackName(i2cClass, (addr&0x80) ? JS_EVENT_PREFIX"read" : JS_EVENT_PREFIX"write", 1, &obj);
          jsvUnLock(obj);
        }
      }
      jsvUnLock(i2cClass);
#endif
    } else if (DEVICE_IS_EXTI(eventType)) { // ---------------------------------------------------------------- PIN WATCH
      // we have an event... find out what it was for...
      // Check everything in our Watch array
      JsVar *watchArrayPtr = jsvLock(watchArray);
      JsvObjectIterator it;
      jsvObjectIteratorNew(&it, watchArrayPtr);
      while (jsvObjectIteratorHasValue(&it)) {
        bool hasDeletedWatch = false;
        JsVar *watchPtr = jsvObjectIteratorGetValue(&it);
        Pin pin = jshGetPinFromVarAndUnLock(jsvObjectGetChildIfExists(watchPtr, "pin"));

        if (jshIsEventForPin(&event, pin)) {
          /** Work out event time. Events time is only stored in 32 bits, so we need to
           * use the correct 'high' 32 bits from the current time.
           *
           * We know that the current time is always newer than the event time, so
           * if the bottom 32 bits of the current time is less than the bottom
           * 32 bits of the event time, we need to subtract a full 32 bits worth
           * from the current time.
           */
          JsSysTime time = jshGetSystemTime();
          if (((unsigned int)time) < (unsigned int)event.data.time)
            time = time - 0x100000000LL;
          // finally, mask in the event's time
          JsSysTime eventTime = (time & ~0xFFFFFFFFLL) | (JsSysTime)event.data.time;

          // Now actually process the event
          bool pinIsHigh = (event.flags&EV_EXTI_IS_HIGH)!=0;

          bool executeNow = false;
          JsVarInt debounce = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(watchPtr, "debounce"));
          if (debounce<=0) {
            executeNow = true;
          } else { // Debouncing - use timeouts to ensure we only fire at the right time
            // store the current state of the pin
            bool oldWatchState = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(watchPtr, "state"));
            JsVar *timeout = jsvObjectGetChildIfExists(watchPtr, "timeout");
            if (timeout) { // if we had a timeout, update the callback time
              JsSysTime timeoutTime = jsiLastIdleTime + (JsSysTime)jsvGetLongIntegerAndUnLock(jsvObjectGetChildIfExists(timeout, "time"));
              jsvUnLock(jsvObjectSetChild(timeout, "time", jsvNewFromLongInteger((JsSysTime)(eventTime - jsiLastIdleTime) + debounce)));
              jsvObjectSetChildAndUnLock(timeout, "state", jsvNewFromBool(pinIsHigh));
              if (eventTime > timeoutTime && pinIsHigh!=oldWatchState) {
                // timeout should have fired, but we didn't get around to executing it!
                // Do it now (with the old timeout time)
                executeNow = true;
                eventTime = timeoutTime - debounce;
                jsvObjectSetChildAndUnLock(watchPtr, "state", jsvNewFromBool(pinIsHigh));
                // Remove the timeout
                JsVar *idArr = jsvNewArray(&timeout, 1);
                jswrap_interface_clearTimeout(idArr);
                jsvUnLock(idArr);
                jsvObjectRemoveChild(watchPtr, "timeout");
              }
            } else if (pinIsHigh!=oldWatchState) { // else create a new timeout
              timeout = jsvNewObject();
              if (timeout) {
                jsvObjectSetChild(timeout, "watch", watchPtr); // no unlock
                jsvObjectSetChildAndUnLock(timeout, "time", jsvNewFromLongInteger((JsSysTime)(eventTime - jsiLastIdleTime) + debounce));
                jsvObjectSetChildAndUnLock(timeout, "callback", jsvObjectGetChildIfExists(watchPtr, "callback"));
                jsvObjectSetChildAndUnLock(timeout, "lastTime", jsvObjectGetChildIfExists(watchPtr, "lastTime"));
                jsvObjectSetChildAndUnLock(timeout, "pin", jsvNewFromPin(pin));
                jsvObjectSetChildAndUnLock(timeout, "state", jsvNewFromBool(pinIsHigh));
                // Add to timer array
                jsiTimerAdd(timeout);
                // Add to our watch
                jsvObjectSetChild(watchPtr, "timeout", timeout); // no unlock
              }
            }
            jsvUnLock(timeout);
          }

          // If we want to execute this watch right now...
          if (executeNow) {
            JsVar *timePtr = jsvNewFromFloat(jshGetMillisecondsFromTime(eventTime)/1000);
            if (jsiShouldExecuteWatch(watchPtr, pinIsHigh)) { // edge triggering
              JsVar *watchCallback = jsvObjectGetChildIfExists(watchPtr, "callback");
              bool watchRecurring = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(watchPtr,  "recur"));
              JsVar *data = jsvNewObject();
              if (data) {
                jsvObjectSetChildAndUnLock(data, "state", jsvNewFromBool(pinIsHigh));
                jsvObjectSetChildAndUnLock(data, "lastTime", jsvObjectGetChildIfExists(watchPtr, "lastTime"));
                // set both data.time, and watch.lastTime in one go
                jsvObjectSetChild(data, "time", timePtr); // no unlock
                jsvObjectSetChildAndUnLock(data, "pin", jsvNewFromPin(pin));
                Pin dataPin = jshGetEventDataPin(eventType);
                if (jshIsPinValid(dataPin))
                  jsvObjectSetChildAndUnLock(data, "data", jsvNewFromBool((event.flags&EV_EXTI_DATA_PIN_HIGH)!=0));
              }
              if (!jsiExecuteEventCallback(0, watchCallback, 1, &data) && watchRecurring) {
                jsError("Ctrl-C while processing watch - removing it.");
                jsErrorFlags |= JSERR_CALLBACK;
                watchRecurring = false;
              }
              jsvUnLock(data);
              if (!watchRecurring) {
                // free all
                jsvObjectIteratorRemoveAndGotoNext(&it, watchArrayPtr);
                hasDeletedWatch = true;
                if (!jsiIsWatchingPin(pin))
                  jshPinWatch(pin, false, JSPW_NONE);
              }
              jsvUnLock(watchCallback);
            }
            jsvObjectSetChildAndUnLock(watchPtr, "lastTime", timePtr);
          }
        }

        jsvUnLock(watchPtr);
        if (!hasDeletedWatch)
          jsvObjectIteratorNext(&it);
      }
      jsvObjectIteratorFree(&it);
      jsvUnLock(watchArrayPtr);
    }
  }

  // Reset Flow control if it was set...
  if (jshGetEventsUsed() < IOBUFFER_XON) {
    jshSetFlowControlAllReady();
  }

  // Check timers
  JsSysTime minTimeUntilNext = JSSYSTIME_MAX;
  JsSysTime time = jshGetSystemTime();
  JsSysTime timePassed = time - jsiLastIdleTime;
  jsiLastIdleTime = time;
#ifndef EMBEDDED
  // add time to Ctrl-C counter, checking for overflow
  uint32_t oldTimeSinceCtrlC = jsiTimeSinceCtrlC;
  jsiTimeSinceCtrlC += (uint32_t)timePassed;
  if (oldTimeSinceCtrlC > jsiTimeSinceCtrlC)
    jsiTimeSinceCtrlC = 0xFFFFFFFF;
#endif

  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsvObjectIterator it;
  // Go through all intervals and decrement time
  jsvObjectIteratorNew(&it, timerArrayPtr);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *timerPtr = jsvObjectIteratorGetValue(&it);
    JsSysTime timerTime = (JsSysTime)jsvGetLongIntegerAndUnLock(jsvObjectGetChildIfExists(timerPtr, "time"));
    JsSysTime timeUntilNext = timerTime - timePassed;
    jsvObjectSetChildAndUnLock(timerPtr, "time", jsvNewFromLongInteger(timeUntilNext));
    jsvUnLock(timerPtr);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  // Now go through intervals and execute if needed
  do {
    jsiStatus = jsiStatus & ~JSIS_TIMERS_CHANGED;
    jsvObjectIteratorNew(&it, timerArrayPtr);
    while (jsvObjectIteratorHasValue(&it) && !(jsiStatus & JSIS_TIMERS_CHANGED)) {
      bool hasDeletedTimer = false;
      JsVar *timerPtr = jsvObjectIteratorGetValue(&it);
      JsSysTime timerTime = (JsSysTime)jsvGetLongIntegerAndUnLock(jsvObjectGetChildIfExists(timerPtr, "time"));
      if (timerTime<=0) {
        // we're now doing work
        jsiSetBusy(BUSY_INTERACTIVE, true);
        wasBusy = true;
        JsVar *timerCallback = jsvObjectGetChildIfExists(timerPtr, "callback");
        JsVar *watchPtr = jsvObjectGetChildIfExists(timerPtr, "watch"); // for debounce - may be undefined
        bool exec = true;
        JsVar *data = 0;
        if (watchPtr) {
          bool watchState = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(watchPtr, "state"));
          bool timerState = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(timerPtr, "state"));
          jsvObjectSetChildAndUnLock(watchPtr, "state", jsvNewFromBool(timerState));
          exec = false;
          if (watchState!=timerState) {
            // Create the 'time' variable that will be passed to the user and stored as last time
            JsVarInt delay = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(watchPtr, "debounce"));
            JsVar *timePtr = jsvNewFromFloat(jshGetMillisecondsFromTime(jsiLastIdleTime+timerTime-delay)/1000);
            // If it's the right edge...
            if (jsiShouldExecuteWatch(watchPtr, timerState)) {
              data = jsvNewObject();
              // if we were from a watch then we were delayed by the debounce time...
              if (data) {
                exec = true;
                // if it was a watch, set the last state up
                jsvObjectSetChildAndUnLock(data, "state", jsvNewFromBool(timerState));
                // set up the lastTime variable of data to what was in the watch
                jsvObjectSetChildAndUnLock(data, "lastTime", jsvObjectGetChildIfExists(watchPtr, "lastTime"));
                // set up the watches lastTime to this one
                jsvObjectSetChild(data, "time", timePtr); // don't unlock - use this later
                jsvObjectSetChildAndUnLock(data, "pin", jsvObjectGetChildIfExists(watchPtr, "pin"));
              }
            }
            // Update lastTime regardless of which edge we're watching
            jsvObjectSetChildAndUnLock(watchPtr, "lastTime", timePtr);
          }
        }
        bool removeTimer = false;
        if (exec) {
          bool execResult;
          if (data) {
            execResult = jsiExecuteEventCallback(0, timerCallback, 1, &data);
          } else {
            JsVar *argsArray = jsvObjectGetChildIfExists(timerPtr, "args");
            execResult = jsiExecuteEventCallbackArgsArray(0, timerCallback, argsArray);
            jsvUnLock(argsArray);
          }
          if (!execResult) {
            JsVar *interval = jsvObjectGetChildIfExists(timerPtr, "interval");
            if (interval) { // if interval then it's setInterval not setTimeout
              jsvUnLock(interval);
              jsError("Ctrl-C while processing interval - removing it.");
              jsErrorFlags |= JSERR_CALLBACK;
              removeTimer = true;
            }
          }
        }
        jsvUnLock(data);
        if (watchPtr) { // if we had a watch pointer, be sure to remove us from it
          jsvObjectRemoveChild(watchPtr, "timeout");
          // Deal with non-recurring watches
          if (exec) {
            bool watchRecurring = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(watchPtr,  "recur"));
            if (!watchRecurring) {
              JsVar *watchArrayPtr = jsvLock(watchArray);
              JsVar *watchNamePtr = jsvGetIndexOf(watchArrayPtr, watchPtr, true);
              if (watchNamePtr) {
                jsvRemoveChild(watchArrayPtr, watchNamePtr);
                jsvUnLock(watchNamePtr);
              }
              jsvUnLock(watchArrayPtr);
              Pin pin = jshGetPinFromVarAndUnLock(jsvObjectGetChildIfExists(watchPtr, "pin"));
              if (!jsiIsWatchingPin(pin))
                jshPinWatch(pin, false, JSPW_NONE);
            }
          }
          jsvUnLock(watchPtr);
        }
        // Load interval *after* executing code, in case it has changed
        JsVar *interval = jsvObjectGetChildIfExists(timerPtr, "interval");
        if (!removeTimer && interval) {
          timerTime = timerTime + jsvGetLongInteger(interval);
          jsvObjectSetChildAndUnLock(timerPtr, "time", jsvNewFromLongInteger(timerTime));
        } else {
          // free
          // Beware... may have already been removed!
          jsvObjectIteratorRemoveAndGotoNext(&it, timerArrayPtr);
          hasDeletedTimer = true;
          timerTime = -1;
        }
        jsvUnLock2(timerCallback,interval);
      }
      // update the time until the next timer
      if (timerTime>=0 && timerTime < minTimeUntilNext)
        minTimeUntilNext = timerTime;
      // update the timer's time
      if (!hasDeletedTimer)
        jsvObjectIteratorNext(&it);
      jsvUnLock(timerPtr);
    }
    jsvObjectIteratorFree(&it);
  } while (jsiStatus & JSIS_TIMERS_CHANGED);
  jsvUnLock(timerArrayPtr);
  /* We might have left the timers loop with stuff to do because the contents of it
   * changed. It's not a big deal because it could only have changed because a timer
   * got executed - so `wasBusy` got set and we know we're going to go around the
   * loop again before sleeping.
   */

  // Check for events that might need to be processed from other libraries
  if (jswIdle()) wasBusy = true;

  // Just in case we got any events to do and didn't clear loopsIdling before
  if (wasBusy || !jsvArrayIsEmpty(events) )
    loopsIdling = 0;

  if (wasBusy)
    jsiSetBusy(BUSY_INTERACTIVE, false);

  // execute any outstanding events
  if (!jspIsInterrupted()) {
    jsiExecuteEvents();
  }

  // check for TODOs
  if (jsiStatus&JSIS_TODO_MASK) {
    jsiSetBusy(BUSY_INTERACTIVE, true);
    unsigned int oldJsVarsSize = jsvGetMemoryTotal(); // we must remember the old memory size - mainly for ESP32 where it can change at boot time
    JsiStatus s = jsiStatus;
    if ((s&JSIS_TODO_RESET) == JSIS_TODO_RESET) {
      // shut down everything and start up again
      jsiKill();
      jsvKill();
      jshReset();
      jsvInit(oldJsVarsSize);
      jsiSemiInit(false, NULL/* no filename */); // don't autoload
      jsiStatus &= (JsiStatus)~JSIS_TODO_RESET;
    }
    if ((s&JSIS_TODO_FLASH_SAVE) == JSIS_TODO_FLASH_SAVE) {
      jsvGarbageCollect(); // nice to have everything all tidy!
      jsiSoftKill();
      jspSoftKill();
      jsvSoftKill();
      jsfSaveToFlash();
      jshReset();
      jsvSoftInit();
      jspSoftInit();
      jsiSoftInit(false /* not been reset */);
      jsiStatus &= (JsiStatus)~JSIS_TODO_FLASH_SAVE;
    }
    if ((s&JSIS_TODO_FLASH_LOAD) == JSIS_TODO_FLASH_LOAD) {
      JsVar *filenameVar = jsvObjectGetChildIfExists(execInfo.hiddenRoot,JSI_LOAD_CODE_NAME);
      // TODO: why can't we follow the same steps here for both?
      if (filenameVar) {
        JsfFileName filename = jsfNameFromVarAndUnLock(filenameVar);
        // no need to jsvObjectRemoveChild as we're shutting down anyway!
        // go through steps as if we're resetting
        jsiKill();
        jsvKill();
        jshReset();
        jsvInit(oldJsVarsSize);
        jsiSemiInit(false, &filename); // don't autoload code
        // load the code we specified
        JsVar *code = jsfReadFile(filename,0,0);
        if (code)
          jsvUnLock2(jspEvaluateVar(code,0,0), code);
      } else {
        jsiSoftKill();
        jspSoftKill();
        jsvSoftKill();
        jsvKill();
        jshReset();
        jsvInit(oldJsVarsSize);
        jsfLoadStateFromFlash();
        jsvSoftInit();
        jspSoftInit();
        jsiSoftInit(false /* not been reset */);
      }
      jsiStatus &= (JsiStatus)~JSIS_TODO_FLASH_LOAD;
    }
    jsiSetBusy(BUSY_INTERACTIVE, false);
  }

  // Kick the WatchDog if needed
  if (jsiStatus & JSIS_WATCHDOG_AUTO)
    jshKickWatchDog();

  /* if we've been around this loop, there is nothing to do, and
   * we have a spare 10ms then let's do some Garbage Collection
   * if we think we need to */
  if (loopsIdling==1 &&
      minTimeUntilNext > jshGetTimeFromMilliseconds(10) &&
      !jsvMoreFreeVariablesThan(JS_VARS_BEFORE_IDLE_GC)) {
    jsiSetBusy(BUSY_INTERACTIVE, true);
    jsvGarbageCollect();
    jsiSetBusy(BUSY_INTERACTIVE, false);
    /* Return here so we run around the idle loop again
     * and check whether any events came in during GC. If not
     * then we'll sleep. */
    return;
  }

  // Go to sleep!
  if (loopsIdling>=1 && // once around the idle loop without having done any work already (just in case)
#if defined(USB) && !defined(EMSCRIPTEN)
      !jshIsUSBSERIALConnected() && // if USB is on, no point sleeping (later, sleep might be more drastic)
#endif
      !jshHasEvents() //no events have arrived in the mean time
      ) {
    jshSleep(minTimeUntilNext);
  }
}

bool jsiLoop() {
  // idle stuff for hardware
  jshIdle();
  // Do general idle stuff
  jsiIdle();
  // check for and report errors
  jsiCheckErrors();

  // If Ctrl-C was pressed, clear the line
  if (execInfo.execute & EXEC_CTRL_C_MASK) {
    execInfo.execute = execInfo.execute & (JsExecFlags)~EXEC_CTRL_C_MASK;
    if (jsvIsEmptyString(inputLine)) {
#ifndef EMBEDDED
      if (jsiTimeSinceCtrlC < jshGetTimeFromMilliseconds(5000))
        exit(0); // exit if ctrl-c on empty input line
      else {
        jsiConsoleRemoveInputLine();
        jsiConsolePrintf("Press Ctrl-C again to exit\n");
      }
      jsiTimeSinceCtrlC = 0;
#endif
    }
    jsiClearInputLine(true);
  }

  // return console (if it was gone!)
  jsiConsoleReturnInputLine();

  return loopsIdling==0;
}



/** Output current interpreter state such that it can be copied to a new device */
void jsiDumpState(vcbprintf_callback user_callback, void *user_data) {
  JsvObjectIterator it;

  jsvObjectIteratorNew(&it, execInfo.root);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *child = jsvObjectIteratorGetKey(&it);
    JsVar *data = jsvObjectIteratorGetValue(&it);
    char childName[JSLEX_MAX_TOKEN_LENGTH];
    jsvGetString(child, childName, JSLEX_MAX_TOKEN_LENGTH);

    bool shouldIgnore = false;
#if defined(DUMP_IGNORE_VARIABLES)
    /* We may want to ignore some variables when dumping
     * so that we get a nice clean output. */
    const char *v = DUMP_IGNORE_VARIABLES;
    while (*v) {
      if (strcmp(v,childName)==0) shouldIgnore=true;
      v += strlen(v)+1;
    }
#endif

    if (shouldIgnore) {
      // Do nothing
    } else if (jswIsBuiltInObject(childName)) {
      jsiDumpObjectState(user_callback, user_data, child, data);
    } else if (jsvIsStringEqualOrStartsWith(child, JS_EVENT_PREFIX, true)) {
      // event on global object - skip it, as it'll be internal
    } else if (jsvIsStringEqual(child, JSI_TIMERS_NAME)) {
      // skip - done later
    } else if (jsvIsStringEqual(child, JSI_WATCHES_NAME)) {
      // skip - done later
    } else if (child->varData.str[0]==JS_HIDDEN_CHAR ||
        jshFromDeviceString(childName)!=EV_NONE) {
      // skip - don't care about this stuff
    } else if (!jsvIsNativeFunction(data)) { // just a variable/function!
      if (jsvIsFunction(data)) {
        // function-specific output
        cbprintf(user_callback, user_data, "function %v", child);
        jsfGetJSONForFunctionWithCallback(data, JSON_SHOW_DEVICES, user_callback, user_data);
        user_callback("\n", user_data);
        // print any prototypes we had
        jsiDumpObjectState(user_callback, user_data, child, data);
      } else {
        // normal variable definition
        cbprintf(user_callback, user_data, "var %v = ", child);
        bool hasProto = false;
        if (jsvIsObject(data)) {
          JsVar *proto = jsvObjectGetChildIfExists(data, JSPARSE_INHERITS_VAR);
          if (proto) {
            JsVar *protoName = jsvGetPathTo(execInfo.root, proto, 4, data);
            if (protoName) {
              cbprintf(user_callback, user_data, "Object.create(%v);\n", protoName);
              jsiDumpObjectState(user_callback, user_data, child, data);
              hasProto = true;
            }
          }
        }
        if (!hasProto) {
          jsiDumpJSON(user_callback, user_data, data, child);
          user_callback(";\n", user_data);
        }
      }
    }
    jsvUnLock2(data, child);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  // Now do timers
  JsVar *timerArrayPtr = jsvLock(timerArray);
  jsvObjectIteratorNew(&it, timerArrayPtr);
  jsvUnLock(timerArrayPtr);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *timer = jsvObjectIteratorGetValue(&it);
    JsVar *timerNumber = jsvObjectIteratorGetKey(&it);
    JsVar *timerCallback = jsvSkipOneNameAndUnLock(jsvFindChildFromString(timer, "callback", false));
    JsVar *timerInterval = jsvObjectGetChildIfExists(timer, "interval");
    user_callback(timerInterval ? "setInterval(" : "setTimeout(", user_data);
    jsiDumpJSON(user_callback, user_data, timerCallback, 0);
    cbprintf(user_callback, user_data, ", %f); // %v\n", jshGetMillisecondsFromTime(timerInterval ? jsvGetLongInteger(timerInterval) : jsvGetLongIntegerAndUnLock(jsvObjectGetChildIfExists(timer, "time"))), timerNumber);
    jsvUnLock3(timerInterval, timerCallback, timerNumber);
    // next
    jsvUnLock(timer);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  // Now do watches
  JsVar *watchArrayPtr = jsvLock(watchArray);
  jsvObjectIteratorNew(&it, watchArrayPtr);
  jsvUnLock(watchArrayPtr);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *watch = jsvObjectIteratorGetValue(&it);
    JsVar *watchCallback = jsvSkipOneNameAndUnLock(jsvFindChildFromString(watch, "callback", false));
    bool watchRecur = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(watch, "recur"));
    int watchEdge = (int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(watch, "edge"));
    JsVar *watchPin = jsvObjectGetChildIfExists(watch, "pin");
    JsVarInt watchDebounce = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(watch, "debounce"));
    user_callback("setWatch(", user_data);
    jsiDumpJSON(user_callback, user_data, watchCallback, 0);
    cbprintf(user_callback, user_data, ", %j, { repeat:%s, edge:'%s'",
        watchPin,
        watchRecur?"true":"false",
            (watchEdge<0)?"falling":((watchEdge>0)?"rising":"both"));
    if (watchDebounce>0)
      cbprintf(user_callback, user_data, ", debounce : %f", jshGetMillisecondsFromTime(watchDebounce));
    user_callback(" });\n", user_data);
    jsvUnLock2(watchPin, watchCallback);
    // next
    jsvUnLock(watch);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);

  // and now the actual hardware
  jsiDumpHardwareInitialisation(user_callback, user_data, true/*human readable*/);

  JsVar *code = jsfGetBootCodeFromFlash(false);
  if (code) {
    cbprintf(user_callback, user_data, "// Code saved with E.setBootCode\n");
    jslPrintTokenisedString(code, user_callback, user_data);
    jsvUnLock(code);
  }
}

JsVarInt jsiTimerAdd(JsVar *timerPtr) {
  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarInt itemIndex = jsvArrayAddToEnd(timerArrayPtr, timerPtr, 1) - 1;
  jsvUnLock(timerArrayPtr);
  return itemIndex;
}

void jsiTimersChanged() {
  jsiStatus |= JSIS_TIMERS_CHANGED;
}

#ifdef USE_DEBUGGER
void jsiDebuggerLoop() {
  // exit if:
  //   in debugger already
  //   echo is off for line (probably uploading)
  if (jsiStatus & (JSIS_IN_DEBUGGER|JSIS_ECHO_OFF_FOR_LINE)) return;

  execInfo.execute &= (JsExecFlags)~(
      EXEC_CTRL_C_MASK |
      EXEC_DEBUGGER_NEXT_LINE |
      EXEC_DEBUGGER_STEP_INTO |
      EXEC_DEBUGGER_FINISH_FUNCTION);
  jsiClearInputLine(true);
  jsiConsoleRemoveInputLine();
  jsiStatus = (jsiStatus & ~JSIS_ECHO_OFF_MASK) | JSIS_IN_DEBUGGER;

  if (lex) {
    char lineStr[9];
    // Get a string fo the form '1234    ' for the line number
    // ... but only if the line number was set, otherwise use spaces
#ifndef ESPR_NO_LINE_NUMBERS
    if (lex->lineNumberOffset) {
      itostr((JsVarInt)jslGetLineNumber() + (JsVarInt)lex->lineNumberOffset - 1, lineStr, 10);
    } else
#endif
    {
      lineStr[0]=0;
    }
    size_t lineLen = strlen(lineStr);
    while (lineLen < sizeof(lineStr)-1) lineStr[lineLen++]=' ';
    lineStr[lineLen] = 0;
    // print the line of code, prefixed by the line number, and with a pointer to the exact character in question
    jslPrintTokenLineMarker(vcbprintf_callback_jsiConsolePrintString, 0, lex->tokenLastStart, lineStr);
  }

  while (!(jsiStatus & JSIS_EXIT_DEBUGGER) &&
         !(execInfo.execute & EXEC_CTRL_C_MASK)) {
    jsiConsoleReturnInputLine();
    // idle stuff for hardware
    jshIdle();
    // Idle just for debug (much stuff removed) -------------------------------
    IOEvent event;
    // If we have too many events (> half full) drain the queue
    while (jshGetEventsUsed()>IOBUFFERMASK*1/2 &&
           !(jsiStatus & JSIS_EXIT_DEBUGGER) &&
           !(execInfo.execute & EXEC_CTRL_C_MASK)) {
      if (jshPopIOEvent(&event) && IOEVENTFLAGS_GETTYPE(event.flags)==consoleDevice)
        jsiHandleIOEventForConsole(&event);
    }
    // otherwise grab the remaining console events
    while (jshPopIOEventOfType(consoleDevice, &event) &&
           !(jsiStatus & JSIS_EXIT_DEBUGGER) &&
           !(execInfo.execute & EXEC_CTRL_C_MASK)) {
      jsiHandleIOEventForConsole(&event);
    }
    // -----------------------------------------------------------------------
  }
  jsiConsoleRemoveInputLine();
  if (execInfo.execute & EXEC_CTRL_C_MASK)
    execInfo.execute |= EXEC_INTERRUPTED;
  jsiStatus &= ~(JSIS_IN_DEBUGGER|JSIS_EXIT_DEBUGGER);
}

void jsiDebuggerPrintScope(JsVar *scope) {
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, scope);
  bool found = false;
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *k = jsvObjectIteratorGetKey(&it);
    JsVar *ks = jsvAsString(k);
    JsVar *v = jsvObjectIteratorGetValue(&it);
    size_t l = jsvGetStringLength(ks);

    if (!jsvIsStringEqual(ks, JSPARSE_RETURN_VAR)) {
      found = true;
      jsiConsolePrintChar(' ');
      if (jsvIsFunctionParameter(k)) {
        jsiConsolePrint("param ");
        l+=6;
      }
      jsiConsolePrintStringVar(ks);
      while (l<20) {
        jsiConsolePrintChar(' ');
        l++;
      }
      jsiConsolePrint(" : ");
      jsfPrintJSON(v, JSON_LIMIT | JSON_SOME_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES | JSON_SHOW_OBJECT_NAMES);
      jsiConsolePrint("\n");
    }

    jsvUnLock3(k, ks, v);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);

  if (!found) {
    jsiConsolePrint(" [No variables]\n");
  }
}

/// Interpret a line of input in the debugger
void jsiDebuggerLine(JsVar *line) {
  assert(jsvIsString(line));
  JsLex lex;
  JsLex *oldLex = jslSetLex(&lex);
  jslInit(line);
  bool handled = false;
  if (lex.tk == LEX_ID || lex.tk == LEX_R_CONTINUE) {
    // continue is a reserved word!

    handled = true;
    char *id = jslGetTokenValueAsString();

    if (!strcmp(id,"help") || !strcmp(id,"h")) {
      jsiConsolePrint("Commands:\n"
                      "help / h           - this information\n"
                      "quit / q / Ctrl-C  - Quit debug mode, break execution\n"
                      "reset              - Soft-reset Espruino\n"
                      "continue / c       - Continue execution\n"
                      "next / n           - execute to next line\n"
                      "step / s           - execute to next line, or step into function call\n"
                      "finish / f         - finish execution of the function call\n"
                      "print ... / p ...  - evaluate and print the next argument\n"
                      "info locals / i l)    - output local variables\n"
                      "info scopechain / i s - output all variables in all scopes\n");
    } else if (!strcmp(id,"quit") || !strcmp(id,"q")) {
      jsiStatus |= JSIS_EXIT_DEBUGGER;
      execInfo.execute |= EXEC_INTERRUPTED;
    } else if (!strcmp(id,"reset")) {
      jsiStatus = (JsiStatus)(jsiStatus & ~JSIS_TODO_MASK) | JSIS_EXIT_DEBUGGER | JSIS_TODO_RESET;
      execInfo.execute |= EXEC_INTERRUPTED;
    } else if (!strcmp(id,"continue") || !strcmp(id,"c")) {
      jsiStatus |= JSIS_EXIT_DEBUGGER;
    } else if (!strcmp(id,"next") || !strcmp(id,"n")) {
      jsiStatus |= JSIS_EXIT_DEBUGGER;
      execInfo.execute |= EXEC_DEBUGGER_NEXT_LINE;
    } else if (!strcmp(id,"step") || !strcmp(id,"s")) {
      jsiStatus |= JSIS_EXIT_DEBUGGER;
      execInfo.execute |= EXEC_DEBUGGER_NEXT_LINE|EXEC_DEBUGGER_STEP_INTO;
    } else if (!strcmp(id,"finish") || !strcmp(id,"f")) {
      jsiStatus |= JSIS_EXIT_DEBUGGER;
      execInfo.execute |= EXEC_DEBUGGER_FINISH_FUNCTION;
    } else if (!strcmp(id,"print") || !strcmp(id,"p")) {
      jslGetNextToken();
      JsExecInfo oldExecInfo = execInfo;
      execInfo.execute = EXEC_YES;
      JsVar *v = jsvSkipNameAndUnLock(jspParse());
      execInfo = oldExecInfo;
      jsiConsolePrintChar('=');
      jsfPrintJSON(v, JSON_LIMIT | JSON_SOME_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES | JSON_SHOW_OBJECT_NAMES);
      jsiConsolePrint("\n");
      jsvUnLock(v);
    } else if (!strcmp(id,"info") || !strcmp(id,"i")) {
       jslGetNextToken();
       id = jslGetTokenValueAsString();
       if (!strcmp(id,"locals") || !strcmp(id,"l")) {
         JsVar *scope = jspeiGetTopScope();
         if (scope == execInfo.root)
           jsiConsolePrint("No locals found\n");
         else {
           jsiConsolePrintf("Locals:\n--------------------------------\n");
           jsiDebuggerPrintScope(scope);
           jsiConsolePrint("\n\n");
         }
         jsvUnLock(scope);
       } else if (!strcmp(id,"scopechain") || !strcmp(id,"s")) {
         JsVar *scope = jspeiGetTopScope();
         if (scope == execInfo.root) jsiConsolePrint("No scopes found\n");
         jsvUnLock(scope);
         int i, l = jsvGetArrayLength(execInfo.scopesVar);
         for (i=0;i<l;i++) {
           scope = jsvGetArrayItem(execInfo.scopesVar, i);
           jsiConsolePrintf("Scope %d:\n--------------------------------\n", i);
           jsiDebuggerPrintScope(scope);
           jsiConsolePrint("\n\n");
           jsvUnLock(scope);
         }
       } else {
         jsiConsolePrint("Unknown command\n");
       }
    } else
      handled = false;
  }
  if (!handled) {
    jsiConsolePrint("In debug mode: Expected a simple ID, type 'help' for more info.\n");
  }

  jslKill();
  jslSetLex(oldLex);
}
#endif // USE_DEBUGGER
