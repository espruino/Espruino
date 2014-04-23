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
#include "jswrapper.h"
#include "jswrap_json.h"
#include "jswrap_io.h"

#ifdef ARM
#define CHAR_DELETE_SEND 0x08
#else
#define CHAR_DELETE_SEND '\b'
#endif

// ----------------------------------------------------------------------------
typedef struct TimerState {
  JsSysTime time;
  JsSysTime interval;
  bool recurring;
  JsVarRef callback; // a calback, or 0
} TimerState;

typedef enum {
 IS_NONE,
 IS_HAD_R,
 IS_HAD_27,
 IS_HAD_27_79,
 IS_HAD_27_91,
 IS_HAD_27_91_49,
 IS_HAD_27_91_50,
 IS_HAD_27_91_51,
 IS_HAD_27_91_52,
 IS_HAD_27_91_53,
 IS_HAD_27_91_54,
} InputState;

TODOFlags todo = TODO_NOTHING;
JsVar *events = 0; // Array of events to execute
JsVarRef timerArray = 0; // Linked List of timers to check and run
JsVarRef watchArray = 0; // Linked List of input watches to check and run
// ----------------------------------------------------------------------------
IOEventFlags consoleDevice = DEFAULT_CONSOLE_DEVICE; ///< The console device for user interaction
Pin pinBusyIndicator = DEFAULT_BUSY_PIN_INDICATOR;
Pin pinSleepIndicator = DEFAULT_SLEEP_PIN_INDICATOR;
bool echo;                  ///< do we provide any user feedback?
bool allowDeepSleep;
// ----------------------------------------------------------------------------
JsVar *inputLine = 0; ///< The current input line
JsvStringIterator inputLineIterator; ///< Iterator that points to the end of the input line
int inputLineLength = -1;
bool inputLineRemoved = false;
size_t inputCursorPos = 0; ///< The position of the cursor in the input line
InputState inputState = 0; ///< state for dealing with cursor keys
bool hasUsedHistory = false; ///< Used to speed up - if we were cycling through history and then edit, we need to copy the string
unsigned char loopsIdling; ///< How many times around the loop have we been entirely idle?
bool interruptedDuringEvent; ///< Were we interrupted while executing an event? If so may want to clear timers
// ----------------------------------------------------------------------------

IOEventFlags jsiGetDeviceFromClass(JsVar *class) {
  // Built-in classes have their object data set to the device name
  return jshFromDeviceString(class->varData.str);
}

JsVar *jsiGetClassNameFromDevice(IOEventFlags device) {
  const char *deviceName = jshGetDeviceString(device);
  return jsvFindChildFromString(execInfo.root, deviceName, false);
}

static inline bool jsiShowInputLine() {
  return echo && !inputLineRemoved;
}

/// Called when the input line/cursor is modified *and its iterator should be reset*
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


/// Change the console to a new location
void jsiSetConsoleDevice(IOEventFlags device) {
  if (device == consoleDevice) return;

  if (!jshIsDeviceInitialised(device)) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    jshUSARTSetup(device, &inf);
  }

  jsiConsoleRemoveInputLine();
  if (echo) { // intentionally not using jsiShowInputLine()
    jsiConsolePrint("Console Moved to ");
    jsiConsolePrint(jshGetDeviceString(device));
    jsiConsolePrint("\n");
  }
  IOEventFlags oldDevice = consoleDevice;
  consoleDevice = device;
  if (echo) { // intentionally not using jsiShowInputLine()
    jsiConsolePrint("Console Moved from ");
    jsiConsolePrint(jshGetDeviceString(oldDevice));
    jsiConsolePrint("\n");
  }
}

/// Get the device that the console is currently on
IOEventFlags jsiGetConsoleDevice() {
  return consoleDevice;
}

NO_INLINE void jsiConsolePrintChar(char data) {
  jshTransmit(consoleDevice, (unsigned char)data);
}

NO_INLINE void jsiConsolePrint(const char *str) {
  while (*str) {
    if (*str == '\n') jsiConsolePrintChar('\r');
    jsiConsolePrintChar(*(str++));
  }
}

void jsiConsolePrintf(const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsiConsolePrint,0, fmt, argp);
  va_end(argp);
}


NO_INLINE void jsiConsolePrintInt(JsVarInt d) {
    char buf[32];
    itoa(d, buf, 10);
    jsiConsolePrint(buf);
}


/// Print the contents of a string var from a character position until end of line (adding an extra ' ' to delete a character if there was one)
void jsiConsolePrintStringVarUntilEOL(JsVar *v, size_t fromCharacter, bool andBackup) {
  int chars = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, fromCharacter);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (ch == '\n') break;
    jsiConsolePrintChar(ch);
    chars++;
    jsvStringIteratorNext(&it);
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
    char ch = jsvStringIteratorGetChar(&it);
    if (ch == '\n') jsiConsolePrintChar('\r');
    jsiConsolePrintChar(ch);
    if (ch == '\n' && newLineCh) jsiConsolePrintChar(newLineCh);
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
}

/// Print the contents of a string var - directly
void jsiConsolePrintStringVar(JsVar *v) {
  jsiConsolePrintStringVarWithNewLineChar(v,0,0);
}

/** Assuming that we are at the end of the string, this backs up
 * and deletes it */
void jsiConsoleEraseStringVarBackwards(JsVar *v) {
  assert(jsvHasCharacterData(v));

  size_t line, lines = jsvGetLinesInString(v);
  for (line=lines;line>0;line--) {
    size_t i,chars = jsvGetCharsOnLine(v, line);
    if (line==lines) {
      for (i=0;i<chars;i++) jsiConsolePrintChar(0x08); // move cursor back
    }
    for (i=0;i<chars;i++) jsiConsolePrintChar(' '); // move cursor forwards and wipe out
    for (i=0;i<chars;i++) jsiConsolePrintChar(0x08); // move cursor back
    if (line>1) { 
      // clear the character before - this would have had a colon
      jsiConsolePrint("\x08 ");
      // move cursor up      
      jsiConsolePrint("\x1B[A"); // 27,91,65 - up
    }
  }
}

/** Assuming that we are at fromCharacter position in the string var,
 * erase everything that comes AFTER and return the cursor to 'fromCharacter'
 * On newlines, if erasePrevCharacter, we remove the character before too. */
void jsiConsoleEraseStringVarFrom(JsVar *v, size_t fromCharacter, bool erasePrevCharacter) {
  assert(jsvHasCharacterData(v));
  size_t cursorLine, cursorCol;
  jsvGetLineAndCol(v, fromCharacter, &cursorLine, &cursorCol);
  // delete contents of current line
  size_t i, chars = jsvGetCharsOnLine(v, cursorLine);
  for (i=cursorCol;i<=chars;i++) jsiConsolePrintChar(' ');
  for (i=0;i<chars;i++) jsiConsolePrintChar(0x08); // move cursor back

  size_t line, lines = jsvGetLinesInString(v);
  for (line=cursorLine+1;line<=lines;line++) {
    jsiConsolePrint("\x1B[B"); // move down
    chars = jsvGetCharsOnLine(v, line);
    for (i=0;i<chars;i++) jsiConsolePrintChar(' '); // move cursor forwards and wipe out
    for (i=0;i<chars;i++) jsiConsolePrintChar(0x08); // move cursor back
    if (erasePrevCharacter) {
      jsiConsolePrint("\x08 "); // move cursor back and insert space
    }
  }
  // move the cursor back up
  for (line=cursorLine+1;line<=lines;line++)
    jsiConsolePrint("\x1B[A"); // 27,91,65 - up
  // move the cursor forwards
  for (i=1;i<cursorCol;i++)
    jsiConsolePrint("\x1B[C"); // 27,91,67 - right
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
    if (echo && inputLine) { // intentionally not using jsiShowInputLine()
      jsiMoveCursorChar(inputLine, inputCursorPos, 0);
      jsiConsoleEraseStringVarFrom(inputLine, 0, true);
      jsiConsolePrintChar(0x08); // go back to start of line
    }
  }
}

/// If the input line has been removed, return it
void jsiReturnInputLine() {
  if (inputLineRemoved) {
    inputLineRemoved = false;
    if (echo) { // intentionally not using jsiShowInputLine()
      jsiConsolePrintChar('>'); // show the prompt
      jsiConsolePrintStringVarWithNewLineChar(inputLine, 0, ':');
      jsiMoveCursorChar(inputLine, jsvGetStringLength(inputLine), inputCursorPos);
    }
  }
}

void jsiConsolePrintPosition(struct JsLex *lex, size_t tokenPos) {
  size_t line,col;
  jsvGetLineAndCol(lex->sourceVar, tokenPos, &line, &col);
  jsiConsolePrintf("line %d col %d\n",line,col);
}

void jsiConsolePrintTokenLineMarker(struct JsLex *lex, size_t tokenPos) {
  size_t line = 1,col = 1;
  jsvGetLineAndCol(lex->sourceVar, tokenPos, &line, &col);
  size_t startOfLine = jsvGetIndexFromLineAndCol(lex->sourceVar, line, 1);
  jsiConsolePrintStringVarUntilEOL(lex->sourceVar, startOfLine, false);
  jsiConsolePrint("\n");
  while (col-- > 0) jsiConsolePrintChar(' ');
  jsiConsolePrint("^\n");
}


/// Print the contents of a string var to a device - directly
void jsiTransmitStringVar(IOEventFlags device, JsVar *v) {
  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    jshTransmit(device, (unsigned char)ch);
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
}

void jsiClearInputLine() {
  jsiConsoleRemoveInputLine();
  // clear input line
  jsvUnLock(inputLine);
  inputLine = jsvNewFromEmptyString();
  jsiInputLineCursorMoved();
}

void jsiSetBusy(JsiBusyDevice device, bool isBusy) {
  static JsiBusyDevice business = 0;

  if (isBusy)
    business |= device;
  else
    business &= (JsiBusyDevice)~device;

  if (pinBusyIndicator != PIN_UNDEFINED)
    jshPinOutput(pinBusyIndicator, business!=0);
}

void jsiSetSleep(JsiSleepType isSleep) {
  if (pinSleepIndicator != PIN_UNDEFINED)
    jshPinOutput(pinSleepIndicator, isSleep == JSI_SLEEP_AWAKE);
}

static JsVarRef _jsiInitNamedArray(const char *name) {
  JsVar *arrayName = jsvFindChildFromString(execInfo.root, name, true);
  if (!arrayName) return 0; // out of memory
  if (!arrayName->firstChild) {
    JsVar *array = jsvNewWithFlags(JSV_ARRAY);
    if (!array) { // out of memory
      jsvUnLock(arrayName);
      return 0;
    }

    arrayName->firstChild = jsvGetRef(jsvRef(array));
    jsvUnLock(array);
  }
  JsVarRef arrayRef = jsvRefRef(arrayName->firstChild);
  jsvUnLock(arrayName);
  return arrayRef;
}

// Used when recovering after being flashed
// 'claim' anything we are using
void jsiSoftInit() {
  jswInit();

  events = jsvNewWithFlags(JSV_ARRAY);
  inputLine = jsvNewFromEmptyString();
  inputCursorPos = 0;
  jsiInputLineCursorMoved();
  inputLineIterator.var = 0;

  allowDeepSleep = false;

  // Load timer/watch arrays
  timerArray = _jsiInitNamedArray(JSI_TIMERS_NAME);
  watchArray = _jsiInitNamedArray(JSI_WATCHES_NAME);

  // Now run initialisation code
  JsVar *initName = jsvFindChildFromString(execInfo.root, JSI_INIT_CODE_NAME, false);
  if (initName && initName->firstChild) {
    //jsiConsolePrint("Running initialisation code...\n");
    JsVar *initCode = jsvLock(initName->firstChild);
    jsvUnLock(jspEvaluateVar(initCode, 0));
    jsvUnLock(initCode);
    jsvRemoveChild(execInfo.root, initName);
  }
  jsvUnLock(initName);

  // Check any existing watches and set up interrupts for them
  if (watchArray) {
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, watchArrayPtr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *watch = jsvArrayIteratorGetElement(&it);
      JsVar *watchPin = jsvObjectGetChild(watch, "pin", 0);
      jshPinWatch(jshGetPinFromVar(watchPin), true);
      jsvUnLock(watchPin);
      jsvUnLock(watch);
      jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    jsvUnLock(watchArrayPtr);
  }

  // Check any existing timers and try and set time correctly
  if (timerArray) {
    JsSysTime currentTime = jshGetSystemTime();
    JsVar *timerArrayPtr = jsvLock(timerArray);
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, timerArrayPtr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *timer = jsvArrayIteratorGetElement(&it);
      JsVar *timerTime = jsvObjectGetChild(timer, "time", 0);
      JsVarInt interval = jsvGetIntegerAndUnLock(jsvObjectGetChild(timer, "interval", 0));
      jsvSetInteger(timerTime, currentTime + interval);
      jsvUnLock(timerTime);
      jsvUnLock(timer);
      jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    jsvUnLock(timerArrayPtr);
  }
  // And look for onInit function
  JsVar *onInit = jsvFindChildFromString(execInfo.root, JSI_ONINIT_NAME, false);
  if (onInit && onInit->firstChild) {
    if (echo) jsiConsolePrint("Running onInit()...\n");
    JsVar *func = jsvSkipName(onInit);
    if (jsvIsFunction(func))
      jspExecuteFunction(func, 0, 0, (JsVar**)0);
    else if (jsvIsString(func))
      jsvUnLock(jspEvaluateVar(func, 0));
    else
      jsError("onInit is not a Function or a String");
  }
  jsvUnLock(onInit);
}

/** Append the code required to initialise a serial port to this string */
void jsiAppendSerialInitialisation(JsVar *str, const char *serialName, bool addCallbacks) {
  JsVar *serialVar = jsvObjectGetChild(execInfo.root, serialName, 0);
  if (serialVar) {
    if (addCallbacks) {
      JsVar *onData = jsvSkipOneNameAndUnLock(jsvFindChildFromString(serialVar, USART_CALLBACK_NAME, false));
      if (onData) {
        JsVar *onDataStr = jsvAsString(onData, true/*unlock*/);
        jsvAppendString(str, serialName);
        jsvAppendString(str, ".onData(");
        jsvAppendStringVarComplete(str, onDataStr);
        jsvAppendString(str, ");\n");
        jsvUnLock(onDataStr);
      }
    }
    JsVar *baud = jsvObjectGetChild(serialVar, USART_BAUDRATE_NAME, 0);
    JsVar *options = jsvObjectGetChild(serialVar, DEVICE_OPTIONS_NAME, 0);
    if (baud || options) {
      int baudrate = (int)jsvGetInteger(baud);
      if (baudrate <= 0) baudrate = DEFAULT_BAUD_RATE;
      jsvAppendPrintf(str, "%s.setup(%d", serialName, baudrate);
      if (jsvIsObject(options)) {
        jsvAppendString(str, ", ");
        jsfGetJSON(options, str, JSON_SHOW_DEVICES);
      }
      jsvAppendString(str, ");\n");
    }
    jsvUnLock(baud);
    jsvUnLock(options);
    jsvUnLock(serialVar);
  }
}

/** Append the code required to initialise a SPI port to this string */
void jsiAppendDeviceInitialisation(JsVar *str, const char *deviceName) {
  JsVar *deviceVar = jsvObjectGetChild(execInfo.root, deviceName, 0);
  if (deviceVar) {
    JsVar *options = jsvObjectGetChild(deviceVar, DEVICE_OPTIONS_NAME, 0);
    if (options) {
      jsvAppendString(str, deviceName);
      jsvAppendString(str, ".setup(");
      if (jsvIsObject(options)) {
        jsfGetJSON(options, str, JSON_SHOW_DEVICES);
      }
      jsvAppendString(str, ");\n");
    }
    jsvUnLock(options);
    jsvUnLock(deviceVar);
  }
}

/** Append all the code required to initialise hardware to this string */
void jsiAppendHardwareInitialisation(JsVar *str, bool addCallbacks) {
  if (!echo) jsvAppendString(str, "echo(0);");
  if (pinBusyIndicator != DEFAULT_BUSY_PIN_INDICATOR) {
    jsvAppendPrintf(str, "setBusyIndicator(%p);\n", pinBusyIndicator);
  }
  if (pinSleepIndicator != DEFAULT_BUSY_PIN_INDICATOR) {
    jsvAppendPrintf(str, "setSleepIndicator(%p);\n", pinSleepIndicator);
  }
  if (allowDeepSleep) {
    jsvAppendPrintf(str, "setDeepSleep(1);\n");
  }

  jsiAppendSerialInitialisation(str, "USB", addCallbacks);
  int i;
  for (i=0;i<USARTS;i++)
    jsiAppendSerialInitialisation(str, jshGetDeviceString(EV_SERIAL1+i), addCallbacks);
  for (i=0;i<SPIS;i++)
    jsiAppendDeviceInitialisation(str, jshGetDeviceString(EV_SPI1+i));
  for (i=0;i<I2CS;i++)
    jsiAppendDeviceInitialisation(str, jshGetDeviceString(EV_I2C1+i));
  // pins
  Pin pin;
  for (pin=0;jshIsPinValid(pin) && pin<255;pin++) {
    if (IS_PIN_USED_INTERNALLY(pin)) continue;
    JshPinState state = jshPinGetState(pin);
    JshPinState statem = state&JSHPINSTATE_MASK;
    if (statem == JSHPINSTATE_GPIO_OUT || statem == JSHPINSTATE_GPIO_OUT_OPENDRAIN) {
      bool isOn = (state&JSHPINSTATE_PIN_IS_ON)!=0;
      if (!isOn && IS_PIN_A_LED(pin)) continue;
      jsvAppendPrintf(str, "digitalWrite(%p,%d);\n",pin,isOn?1:0);
    } else if (/*statem == JSHPINSTATE_GPIO_IN ||*/statem == JSHPINSTATE_GPIO_IN_PULLUP || statem == JSHPINSTATE_GPIO_IN_PULLDOWN) {
      // don't bother with normal inputs, as they come up in this state (ish) anyway
      const char *s = "";
      if (statem == JSHPINSTATE_GPIO_IN_PULLUP) s="_pullup";
      if (statem == JSHPINSTATE_GPIO_IN_PULLDOWN) s="_pulldown";
      jsvAppendPrintf(str, "pinMode(%p,\"input%s\");\n",pin,s);
    }

    if (statem == JSHPINSTATE_GPIO_OUT_OPENDRAIN)
      jsvAppendPrintf(str, "pinMode(%p,\"opendrain\");\n",pin);
  }
}

// Used when shutting down before flashing
// 'release' anything we are using, but ensure that it doesn't get freed
void jsiSoftKill() {
  jsvUnLock(inputLine);
  inputLine=0;
  inputCursorPos = 0;
  jsiInputLineCursorMoved();

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
    JsVarRef watch = watchArrayPtr->firstChild;
    while (watch) {
      JsVar *watchNamePtr = jsvLock(watch);
      JsVar *watchPin = jsvSkipNameAndUnLock(jsvFindChildFromStringRef(watchNamePtr->firstChild, "pin", false));
      jshPinWatch(jshGetPinFromVar(watchPin), false);
      jsvUnLock(watchPin);
      watch = watchNamePtr->nextSibling;
      jsvUnLock(watchNamePtr);
    }
    jsvUnLock(watchArrayPtr);

    jsvUnRefRef(watchArray);
    watchArray=0;
  }
  // Save initialisation information
  JsVar *initCode = jsvNewFromEmptyString();
  if (initCode) { // out of memory
    jsiAppendHardwareInitialisation(initCode, false);
    jsvObjectSetChild(execInfo.root, JSI_INIT_CODE_NAME, initCode);
    jsvUnLock(initCode);
  }

  jswKill();
}

void jsiInit(bool autoLoad) {
  jsvInit();
  jspInit();

  /*for (i=0;i<IOPINS;i++)
     ioPinState[i].callbacks = 0;*/

  // Set state
  interruptedDuringEvent = false;
  // Set defaults
  echo = true;
  consoleDevice = DEFAULT_CONSOLE_DEVICE;
  pinBusyIndicator = DEFAULT_BUSY_PIN_INDICATOR;
  if (jshIsUSBSERIALConnected())
    consoleDevice = EV_USBSERIAL;

  /* If flash contains any code, then we should
     Try and load from it... */
  bool loadFlash = autoLoad && jshFlashContainsCode();
  if (loadFlash) {
    jspSoftKill();
    jsvSoftKill();
    jshLoadFromFlash();
    jsvSoftInit();
    jspSoftInit();
  }
  //jsvTrace(jsvGetRef(execInfo.root), 0)

  // Softinit may run initialisation code that will overwrite defaults
  jsiSoftInit();

  if (echo) { // intentionally not using jsiShowInputLine()
    if (!loadFlash) {
      jsiConsolePrint(
#ifndef LINUX
              // set up terminal to avoid word wrap
              "\e[?7l"
#endif
              // rectangles @ http://www.network-science.de/ascii/
              "\n"
              " _____                 _ \n"
              "|   __|___ ___ ___ _ _|_|___ ___ \n"
              "|   __|_ -| . |  _| | | |   | . |\n"
              "|_____|___|  _|_| |___|_|_|_|___|\n"    
              "          |_| http://espruino.com\n"
              " "JS_VERSION" Copyright 2014 G.Williams\n");
    }
    jsiConsolePrint("\n"); // output new line
    inputLineRemoved = true; // we need to put the input line back...
  }
}



void jsiKill() {
  jsiSoftKill();

  jspKill();
  jsvKill();
}

int jsiCountBracketsInInput() {
  int brackets = 0;

  JsLex lex;
  jslInit(&lex, inputLine);
  while (lex.tk!=LEX_EOF && lex.tk!=LEX_UNFINISHED_COMMENT) {
    if (lex.tk=='{' || lex.tk=='[' || lex.tk=='(') brackets++;
    if (lex.tk=='}' || lex.tk==']' || lex.tk==')') brackets--;
    if (brackets<0) break; // closing bracket before opening!
    jslGetNextToken(&lex);
  }
  if (lex.tk==LEX_UNFINISHED_COMMENT)
    brackets=1000; // if there's an unfinished comment, we're in the middle of something
  jslKill(&lex);

  return brackets;
} 

/// Tries to get rid of some memory (by clearing command history). Returns true if it got rid of something, false if it didn't.
bool jsiFreeMoreMemory() {
  JsVar *history = jsvObjectGetChild(execInfo.root, JSI_HISTORY_NAME, 0);
  if (!history) return 0;
  JsVar *item = jsvArrayPopFirst(history);
  bool freed = item!=0;
  jsvUnLock(item);
  jsvUnLock(history);
  // TODO: could also free the array structure?
  return freed;
}

// Add a new line to the command history
void jsiHistoryAddLine(JsVar *newLine) {
  if (!newLine || jsvGetStringLength(newLine)==0) return;
  JsVar *history = jsvFindChildFromString(execInfo.root, JSI_HISTORY_NAME, true);
  if (!history) return; // out of memory
  // ensure we actually have the history array
  if (!history->firstChild) {
    JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
    if (!arr) {// out of memory
      jsvUnLock(history);
      return;
    }
    history->firstChild = jsvGetRef(jsvRef(arr));
    jsvUnLock(arr);
  }
  history = jsvSkipNameAndUnLock(history);
  // if it was already in history, remove it - we'll put it back in front
  JsVar *alreadyInHistory = jsvGetArrayIndexOf(history, newLine, false/*not exact*/);
  if (alreadyInHistory) {
    jsvRemoveChild(history, alreadyInHistory);
    jsvUnLock(alreadyInHistory);
  }
  // put it back in front
  jsvArrayPush(history, newLine);
  jsvUnLock(history);
}

JsVar *jsiGetHistoryLine(bool previous /* next if false */) {
  JsVar *history = jsvObjectGetChild(execInfo.root, JSI_HISTORY_NAME, 0);
  JsVar *historyLine = 0;
  if (history) {
    JsVar *idx = jsvGetArrayIndexOf(history, inputLine, true/*exact*/); // get index of current line
    if (idx) {
      if (previous && idx->prevSibling) {
        historyLine = jsvSkipNameAndUnLock(jsvLock(idx->prevSibling));
      } else if (!previous && idx->nextSibling) {
        historyLine = jsvSkipNameAndUnLock(jsvLock(idx->nextSibling));
      }
      jsvUnLock(idx);
    } else {
      if (previous) historyLine = jsvSkipNameAndUnLock(jsvArrayGetLast(history));
      // if next, we weren't using history so couldn't go forwards
    }
    
    jsvUnLock(history);
  }
  return historyLine;
}

bool jsiIsInHistory(JsVar *line) {
  JsVar *history = jsvObjectGetChild(execInfo.root, JSI_HISTORY_NAME, 0);
  if (!history) return false;
  JsVar *historyFound = jsvGetArrayIndexOf(history, line, true/*exact*/);
  bool inHistory = historyFound!=0;
  jsvUnLock(historyFound);
  jsvUnLock(history);
  return inHistory;
}

void jsiReplaceInputLine(JsVar *newLine) {
  if (jsiShowInputLine()) {
    size_t oldLen =  jsvGetStringLength(inputLine);
    jsiMoveCursorChar(inputLine, inputCursorPos, oldLen); // move cursor to end
    jsiConsoleEraseStringVarBackwards(inputLine);
    jsiConsolePrintStringVarWithNewLineChar(newLine,0,':');
  }
  jsvUnLock(inputLine);
  inputLine = jsvLockAgain(newLine);
  inputCursorPos = jsvGetStringLength(inputLine);
  jsiInputLineCursorMoved();
}

void jsiChangeToHistory(bool previous) {
  JsVar *nextHistory = jsiGetHistoryLine(previous);
  if (nextHistory) {
    jsiReplaceInputLine(nextHistory);
    jsvUnLock(nextHistory);
    hasUsedHistory = true;
  } else if (!previous) { // if next, but we have something, just clear the line
    if (jsiShowInputLine()) {
      jsiConsoleEraseStringVarBackwards(inputLine);
    }
    jsvUnLock(inputLine);
    inputLine = jsvNewFromEmptyString();
    inputCursorPos = 0;
    jsiInputLineCursorMoved();
  }
}

void jsiIsAboutToEditInputLine() {
  // we probably plan to do something with the line now - check it wasn't in history
  // and if it was, duplicate it
  if (hasUsedHistory) {
    hasUsedHistory = false;
    if (jsiIsInHistory(inputLine)) {
      JsVar *newLine = jsvCopy(inputLine);
      if (newLine) { // could have been out of memory!
        jsvUnLock(inputLine);
        inputLine = newLine;
        jsiInputLineCursorMoved();
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
    jsiConsoleEraseStringVarFrom(inputLine, inputCursorPos, true/*before newline*/); // erase all in front
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
  jsvUnLock(inputLine);
  inputLine=v;
  jsiInputLineCursorMoved();
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
      jsiConsolePrintStringVarUntilEOL(inputLine, inputCursorPos, true/*and backup*/);
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

void jsiHandleNewLine(bool execute) {
  if (jsiAtEndOfInputLine()) { // at EOL so we need to figure out if we can execute or not
    if (execute && jsiCountBracketsInInput()<=0) { // actually execute!
      if (jsiShowInputLine()) {
        jsiConsolePrint("\n");
      }
      inputLineRemoved = true;

      // Get line to execute, and reset inputLine
      JsVar *lineToExecute = jsvStringTrimRight(inputLine);
      jsvUnLock(inputLine);
      inputLine = jsvNewFromEmptyString();
      inputCursorPos = 0;
      jsiInputLineCursorMoved();
      // execute!
      JsVar *v = jspEvaluateVar(lineToExecute, 0);
      // add input line to history
      jsiHistoryAddLine(lineToExecute);
      jsvUnLock(lineToExecute);
      // print result (but NOT if we had an error)
      if (echo && !jspHasError()) {
        jsiConsolePrintChar('=');
        jsfPrintJSON(v, JSON_LIMIT | JSON_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES);
        jsiConsolePrint("\n");
      }
      jsvUnLock(v);
      // console will be returned next time around the input loop
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
    if (jsiShowInputLine()) jsiConsoleEraseStringVarFrom(inputLine, inputCursorPos, false/*no need to erase the char before*/); // erase all in front
    JsVar *v = jsvNewFromEmptyString();
    if (inputCursorPos>0) jsvAppendStringVar(v, inputLine, 0, inputCursorPos);
    jsvAppendCharacter(v, '\n');
    jsvAppendStringVar(v, inputLine, inputCursorPos, JSVAPPENDSTRINGVAR_MAXLENGTH); // add the rest
    jsvUnLock(inputLine);
    inputLine=v;
    if (jsiShowInputLine()) { // now print the rest
      jsiConsolePrintStringVarWithNewLineChar(inputLine, inputCursorPos, ':');
      jsiMoveCursorChar(inputLine, jsvGetStringLength(inputLine), inputCursorPos+1); // move cursor back
    }
    inputCursorPos++;
    jsiInputLineCursorMoved();
  }
}

void jsiHandleChar(char ch) {
  // jsiConsolePrintf("[%d:%d]\n", inputState, ch);
  //
  // special stuff
  // 27 then 91 then 68 - left
  // 27 then 91 then 67 - right
  // 27 then 91 then 65 - up
  // 27 then 91 then 66 - down
  // 27 then 91 then 50 then 75 - Erases the entire current line.
  // 27 then 91 then 51 then 126 - backwards delete
  // 27 then 91 then 52 then 126 - numpad end
  // 27 then 91 then 49 then 126 - numpad home
  // 27 then 91 then 53 then 126 - pgup
  // 27 then 91 then 54 then 126 - pgdn
  // 27 then 79 then 70 - home
  // 27 then 79 then 72 - end
  // 27 then 10 - alt enter


  if (ch == 0) {
    inputState = IS_NONE; // ignore 0 - it's scary
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
    if (ch==68) { // left
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
    } else if (ch==49) {
      inputState=IS_HAD_27_91_49;
    } else if (ch==50) {
      inputState=IS_HAD_27_91_50;
    } else if (ch==51) {
      inputState=IS_HAD_27_91_51;
    } else if (ch==52) {
      inputState=IS_HAD_27_91_52;
    } else if (ch==53) {
      inputState=IS_HAD_27_91_53;
    } else if (ch==54) {
      inputState=IS_HAD_27_91_54;
    }
  } else if (inputState==IS_HAD_27_91_49) {
    inputState = IS_NONE;
    if (ch==126) { // Numpad Home
      jsiHandleHome();
    }
  } else if (inputState==IS_HAD_27_91_50) {
    inputState = IS_NONE;
    if (ch==75) { // Erase current line
      jsiClearInputLine();
    }
  } else if (inputState==IS_HAD_27_91_51) {
    inputState = IS_NONE;
    if (ch==126) { // Numpad (forwards) Delete
      jsiHandleDelete(false/*not backspace*/);
    }
  } else if (inputState==IS_HAD_27_91_52) {
    inputState = IS_NONE;
    if (ch==126) { // Numpad End
      jsiHandleEnd();
    }
  } else if (inputState==IS_HAD_27_91_53) {
      inputState = IS_NONE;
      if (ch==126) { // Page Up
        jsiHandlePageUpDown(0);
      }
  } else if (inputState==IS_HAD_27_91_54) {
      inputState = IS_NONE;
      if (ch==126) { // Page Down
        jsiHandlePageUpDown(1);
      }
  } else {  
    inputState = IS_NONE;
    if (ch == 0x08 || ch == 0x7F /*delete*/) {
      jsiHandleDelete(true /*backspace*/);
    } else if (ch == '\n' && inputState == IS_HAD_R) {
      inputState = IS_NONE; //  ignore \ r\n - we already handled it all on \r
    } else if (ch == '\r' || ch == '\n') { 
      if (ch == '\r') inputState = IS_HAD_R;
      jsiHandleNewLine(true);
    } else if (ch>=32 || ch=='\t') {
      // Add the character to our input line
      jsiIsAboutToEditInputLine();
      char buf[2] = {ch,0};
      const char *strToAppend = (ch=='\t') ? "    " : buf;
      size_t strSize = (ch=='\t') ? 4 : 1;

      if (inputLineLength < 0)
        inputLineLength = (int)jsvGetStringLength(inputLine);

      if ((int)inputCursorPos>=inputLineLength) { // append to the end
        jsiAppendToInputLine(strToAppend);
      } else { // add in halfway through
        JsVar *v = jsvNewFromEmptyString();
        if (inputCursorPos>0) jsvAppendStringVar(v, inputLine, 0, inputCursorPos);
        jsvAppendString(v, strToAppend);
        jsvAppendStringVar(v, inputLine, inputCursorPos, JSVAPPENDSTRINGVAR_MAXLENGTH); // add the rest
        jsvUnLock(inputLine);
        inputLine=v;
        jsiInputLineCursorMoved();
        if (jsiShowInputLine()) jsiConsolePrintStringVarUntilEOL(inputLine, inputCursorPos, true/*and backup*/);
      }
      inputCursorPos += strSize; // no need for jsiInputLineCursorMoved(); as we just appended
      if (jsiShowInputLine()) {
        jsiConsolePrint(strToAppend);
      }
    }
  }
}

void jsiQueueEvents(JsVarRef callbacks, JsVar *arg0, JsVar *arg1) { // array of functions or single function
  if (!callbacks) return;

  JsVar *callbackVar = jsvLock(callbacks);
  // if it is a single callback, just add it
  if (jsvIsFunction(callbackVar) || jsvIsString(callbackVar)) {
    JsVar *event = jsvNewWithFlags(JSV_OBJECT);
    if (event) { // Could be out of memory error!
      jsvUnLock(jsvAddNamedChild(event, callbackVar, "func"));
      if (arg0) jsvUnLock(jsvAddNamedChild(event, arg0, "arg0"));
      if (arg1) jsvUnLock(jsvAddNamedChild(event, arg1, "arg1"));
      jsvArrayPushAndUnLock(events, event);
    }
    jsvUnLock(callbackVar);
  } else {
    assert(jsvIsArray(callbackVar));
    // go through all callbacks
    JsVarRef next = callbackVar->firstChild;
    jsvUnLock(callbackVar);
    while (next) {
      //jsPrint("Queue Event\n");
      JsVar *child = jsvLock(next);
      
      // for each callback...
      JsVar *event = jsvNewWithFlags(JSV_OBJECT);
      if (event) { // Could be out of memory error!
        jsvUnLock(jsvAddNamedChild(event, child, "func"));
        if (arg0) jsvUnLock(jsvAddNamedChild(event, arg0, "arg0"));
        if (arg1) jsvUnLock(jsvAddNamedChild(event, arg1, "arg1"));
        // add event to the events list
        jsvArrayPushAndUnLock(events, event);
        // go to next callback
      }
      next = child->nextSibling;
      jsvUnLock(child);
    }
  }
}

bool jsiObjectHasCallbacks(JsVar *object, const char *callbackName) {
  JsVar *callback = jsvObjectGetChild(object, callbackName, 0);
  bool hasCallbacks = !jsvIsUndefined(callback);
  jsvUnLock(callback);
  return hasCallbacks;
}

void jsiQueueObjectCallbacks(JsVar *object, const char *callbackName, JsVar *arg0, JsVar *arg1) {
  JsVar *callback = jsvObjectGetChild(object, callbackName, 0);
  if (!callback) return;
  jsiQueueEvents(jsvGetRef(callback), arg0, arg1);
  jsvUnLock(callback);
}

void jsiExecuteEvents() {
  bool hasEvents = !jsvArrayIsEmpty(events);
  bool wasInterrupted = jspIsInterrupted();
  if (hasEvents) jsiSetBusy(BUSY_INTERACTIVE, true);
  while (!jsvArrayIsEmpty(events)) {
    JsVar *event = jsvSkipNameAndUnLock(jsvArrayPopFirst(events));
    // Get function to execute
    JsVar *func = jsvObjectGetChild(event, "func", 0);
    JsVar *args[2];
    args[0] = jsvObjectGetChild(event, "arg0", 0);
    args[1] = jsvObjectGetChild(event, "arg1", 0);
    // free
    jsvUnLock(event);


    // now run..
    if (func) {
      if (jsvIsFunction(func))
        jspExecuteFunction(func, 0, 2, args);
      else if (jsvIsString(func))
        jsvUnLock(jspEvaluateVar(func, 0));
      else 
        jsError("Unknown type of callback in Event Queue");
    }
    //jsPrint("Event Done\n");
    jsvUnLock(func);
    jsvUnLock(args[0]);
    jsvUnLock(args[1]);
  }
  if (hasEvents) {
    jsiSetBusy(BUSY_INTERACTIVE, false);
    if (!wasInterrupted && jspIsInterrupted())
      interruptedDuringEvent = true;
  }
}

NO_INLINE static bool jsiExecuteEventCallback(JsVar *callbackVar, JsVar *arg0, JsVar *arg1) { // array of functions or single function
  bool wasInterrupted = jspHasError();
  JsVar *callbackNoNames = jsvSkipName(callbackVar);

  if (callbackNoNames) {
    if (jsvIsArray(callbackNoNames)) {
      JsVarRef next = callbackNoNames->firstChild;
      while (next) {
        JsVar *child = jsvLock(next);
        jsiExecuteEventCallback(child, arg0, arg1);
        next = child->nextSibling;
        jsvUnLock(child);
      }
    } else if (jsvIsFunction(callbackNoNames)) {
       JsVar *args[2] = { arg0, arg1 };
       JsVar *parent = 0;
       jspExecuteFunction(callbackNoNames, parent, 2, args);
    } else if (jsvIsString(callbackNoNames))
      jsvUnLock(jspEvaluateVar(callbackNoNames, 0));
    else 
      jsError("Unknown type of callback in Event Queue");
    jsvUnLock(callbackNoNames);
  }
  if (!wasInterrupted && jspHasError()) {
    interruptedDuringEvent = true;
    return false;
  }
  return true;
}

bool jsiHasTimers() {
  if (!timerArray) return false;
  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarInt c = jsvGetArrayLength(timerArrayPtr);
  jsvUnLock(timerArrayPtr);
  return c>0;
}

/// Is the given watch object meant to be executed when the current value of the pin is pinIsHigh
bool jsiShouldExecuteWatch(JsVar *watchPtr, bool pinIsHigh) {
  int watchEdge = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(watchPtr, "edge", 0));
  return watchEdge==0 || // any edge
         (pinIsHigh && watchEdge>0) || // rising edge
         (!pinIsHigh && watchEdge<0); // falling edge
}

bool jsiIsWatchingPin(Pin pin) {
  bool isWatched = false;
  JsVar *watchArrayPtr = jsvLock(watchArray);
  JsvArrayIterator it;
  jsvArrayIteratorNew(&it, watchArrayPtr);
  while (jsvArrayIteratorHasElement(&it)) {
    JsVar *watchPtr = jsvArrayIteratorGetElement(&it);
    JsVar *pinVar = jsvObjectGetChild(watchPtr, "pin", 0);
    if (jshGetPinFromVar(pinVar) == pin)
      isWatched = true;
    jsvUnLock(pinVar);
    jsvUnLock(watchPtr);
    jsvArrayIteratorNext(&it);
  }
  jsvArrayIteratorFree(&it);
  jsvUnLock(watchArrayPtr);
  return isWatched;
}

void jsiIdle() {
  // This is how many times we have been here and not done anything.
  // It will be zeroed if we do stuff later
  if (loopsIdling<255) loopsIdling++;

  // Handle hardware-related idle stuff (like checking for pin events)
  bool wasBusy = false;
  IOEvent event;
  while (jshPopIOEvent(&event)) {
    jsiSetBusy(BUSY_INTERACTIVE, true);
    wasBusy = true;

    IOEventFlags eventType = IOEVENTFLAGS_GETTYPE(event.flags);

    loopsIdling = 0; // because we're not idling
    if (eventType == consoleDevice) {
      int i, c = IOEVENTFLAGS_GETCHARS(event.flags);
      jsiSetBusy(BUSY_INTERACTIVE, true);
      for (i=0;i<c;i++) jsiHandleChar(event.data.chars[i]);
      jsiSetBusy(BUSY_INTERACTIVE, false);
    }



    if (DEVICE_IS_USART(eventType)) {
      // ------------------------------------------------------------------------ SERIAL CALLBACK
      JsVar *usartClass = jsvSkipNameAndUnLock(jsiGetClassNameFromDevice(IOEVENTFLAGS_GETTYPE(event.flags)));
      if (usartClass) {
        JsVar *callback = jsvFindChildFromString(usartClass, USART_CALLBACK_NAME, false);

        if (callback) {
          int i, c = IOEVENTFLAGS_GETCHARS(event.flags);

// Part of hackish solution to 7 bit support on STM32
#ifdef STM32
          unsigned char bytesize = 8;
          unsigned char parity   = 0;
          JsVar *options    = jsvObjectGetChild(usartClass, DEVICE_OPTIONS_NAME, 0);

          if(jsvIsObject(options)) {
            bytesize = (unsigned char)jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "bytesize", 0));
            JsVar *v = jsvObjectGetChild(options, "parity", 0);

            if(jsvIsString(v)) {
              parity = 0xFF;
              char s[8] = "";

              jsvGetString(v, s, sizeof(s) - 1);

              if(!strcmp(s, "o") || !strcmp(s, "odd")) {
                parity = 1;
              }
              else if(!strcmp(s, "e") || !strcmp(s, "even")) {
                parity = 2;
              }
            }
            else if(jsvIsInt(v)) {
              parity = (unsigned char)jsvGetInteger(v);
            }

            jsvUnLock(v);
          }

          jsvUnLock(options);
#endif

          for (i=0; i<c; i++) {
            JsVar *data = jsvNewWithFlags(JSV_OBJECT);

            if (data) {
              JsVar *dataTime = jsvNewFromString("X");

#ifdef STM32 
              if(bytesize == 7 && parity > 0) {
                dataTime->varData.str[0] = event.data.chars[i] & 0x7F;
              }
              else if(bytesize == 8 && parity > 0) {
                dataTime->varData.str[0] = event.data.chars[i] & 0xFF;
              }
              else {
                dataTime->varData.str[0] = event.data.chars[i];
              }
#else
              dataTime->varData.str[0] = event.data.chars[i];
#endif

              if (dataTime) jsvUnLock(jsvAddNamedChild(data, dataTime, "data"));
              jsvUnLock(dataTime);
            }
            
            if (!jsiExecuteEventCallback(callback, data, 0)) {
              jsError("Error processing Serial data handler - removing it.");
              jsvSetValueOfName(callback, 0);
            }
            jsvUnLock(data);
          }
        }
        jsvUnLock(callback);
        jsvUnLock(usartClass);
      }
    } else if (DEVICE_IS_EXTI(eventType)) { // ---------------------------------------------------------------- PIN WATCH
      // we have an event... find out what it was for...
      // Check everything in our Watch array
      JsVar *watchArrayPtr = jsvLock(watchArray);
      JsVarRef watchName = watchArrayPtr->firstChild;
      while (watchName) {
        JsVar *watchNamePtr = jsvLock(watchName); // effectively the array index
        JsVar *watchPtr = jsvSkipName(watchNamePtr);
        Pin pin = jshGetPinFromVarAndUnLock(jsvObjectGetChild(watchPtr, "pin", 0));

        if (jshIsEventForPin(&event, pin)) {
          bool pinIsHigh = (event.flags&EV_EXTI_IS_HIGH)!=0;

          JsVarInt debounce = jsvGetIntegerAndUnLock(jsvObjectGetChild(watchPtr, "debounce", 0));
          if (debounce>0) { // Debouncing - use timeouts to ensure we only fire at the right time
            JsVar *timeout = jsvObjectGetChild(watchPtr, "timeout", 0);
            if (timeout) { // if we had a timeout, update the callback time
              JsVar *timerTime = jsvObjectGetChild(timeout, "time", JSV_INTEGER);
              jsvSetInteger(timerTime, event.data.time+debounce);
              jsvUnLock(timerTime);
            } else { // else create a new timeout
              timeout = jsvNewWithFlags(JSV_OBJECT);
              if (timeout) {
                jsvObjectSetChild(timeout, "watch", watchPtr); // no unlock
                jsvUnLock(jsvObjectSetChild(timeout, "time", jsvNewFromInteger(event.data.time+debounce)));
                jsvUnLock(jsvObjectSetChild(timeout, "callback", jsvObjectGetChild(watchPtr, "callback", 0)));
                jsvUnLock(jsvObjectSetChild(timeout, "lastTime", jsvObjectGetChild(watchPtr, "lastTime", 0)));
                // Add to timer array
                jsiTimerAdd(timeout);
                // Add to our watch
                jsvObjectSetChild(watchPtr, "timeout", timeout); // no unlock
              }
            }
            jsvUnLock(timeout);
            // store the current state here
            jsvUnLock(jsvObjectSetChild(watchPtr, "state", jsvNewFromBool(pinIsHigh)));
          } else { // Not debouncing - just execute normally
            JsVar *timePtr = jsvNewFromFloat(jshGetMillisecondsFromTime(event.data.time)/1000);
            if (jsiShouldExecuteWatch(watchPtr, pinIsHigh)) { // edge triggering
              JsVar *watchCallback = jsvObjectGetChild(watchPtr, "callback", 0);
              bool watchRecurring = jsvGetBoolAndUnLock(jsvObjectGetChild(watchPtr,  "recur", 0));
              JsVar *data = jsvNewWithFlags(JSV_OBJECT);
              if (data) {
                jsvUnLock(jsvObjectSetChild(data, "lastTime", jsvObjectGetChild(watchPtr, "lastTime", 0)));
                // set both data.time, and watch.lastTime in one go
                jsvObjectSetChild(data, "time", timePtr); // no unlock
                jsvUnLock(jsvObjectSetChild(data, "state", jsvNewFromBool(pinIsHigh)));
              }
              if (!jsiExecuteEventCallback(watchCallback, data, 0) && watchRecurring) {
                jsError("Error processing Watch - removing it.");
                watchRecurring = false;
              }
              jsvUnLock(data);
              if (!watchRecurring) {
                // free all
                jsvRemoveChild(watchArrayPtr, watchNamePtr);
                if (!jsiIsWatchingPin(pin))
                  jshPinWatch(pin, false);
              }
              jsvUnLock(watchCallback);
            }
            jsvUnLock(jsvObjectSetChild(watchPtr, "lastTime", timePtr));
          }
        }

        jsvUnLock(watchPtr);
        watchName = watchNamePtr->nextSibling;
        jsvUnLock(watchNamePtr);
      }
      jsvUnLock(watchArrayPtr);
    }
  }

  // Reset Flow control if it was set...
  if (jshGetEventsUsed() < IOBUFFER_XON) { 
    int i;
    for (i=0;i<USARTS;i++)
      jshSetFlowControlXON(EV_SERIAL1+i, true);
  }

  // Check timers
  JsSysTime minTimeUntilNext = JSSYSTIME_MAX;
  JsSysTime time = jshGetSystemTime();

  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarRef timer = timerArrayPtr->firstChild;
  while (timer) {
    JsVar *timerNamePtr = jsvLock(timer);
    timer = timerNamePtr->nextSibling; // ptr to next
    JsVar *timerPtr = jsvSkipName(timerNamePtr);
    JsVar *timerTime = jsvObjectGetChild(timerPtr, "time", 0);
    JsSysTime timeUntilNext = jsvGetInteger(timerTime) - time;
    if (timeUntilNext < minTimeUntilNext)
      minTimeUntilNext = timeUntilNext;
    if (timerTime && timeUntilNext<=0) {
      // we're now doing work
      jsiSetBusy(BUSY_INTERACTIVE, true);
      wasBusy = true;
      JsVar *timerCallback = jsvObjectGetChild(timerPtr, "callback", 0);
      JsVar *watchPtr = jsvObjectGetChild(timerPtr, "watch", 0); // for debounce - may be undefined
      bool exec = true;
      JsVar *data = jsvNewWithFlags(JSV_OBJECT);
      if (data) {
        JsVar *timePtr = jsvNewFromFloat(jshGetMillisecondsFromTime(jsvGetInteger(timerTime))/1000);
        // if it was a watch, set the last state up
        if (watchPtr) {
          bool state = jsvGetBoolAndUnLock(jsvObjectSetChild(data, "state", jsvObjectGetChild(watchPtr, "state", 0)));
          exec = jsiShouldExecuteWatch(watchPtr, state);
          // set up the lastTime variable of data to what was in the watch
          jsvUnLock(jsvObjectSetChild(data, "lastTime", jsvObjectGetChild(watchPtr, "lastTime", 0)));
          // set up the watches lastTime to this one
          jsvObjectSetChild(watchPtr, "lastTime", timePtr); // don't unlock
        }
        jsvUnLock(jsvObjectSetChild(data, "time", timePtr));
      }
      bool intervalRecurring = jsvGetBoolAndUnLock(jsvObjectGetChild(timerPtr, "recur", 0));
      if (exec) {
        if (!jsiExecuteEventCallback(timerCallback, data, 0) && intervalRecurring) {
          jsError("Error processing interval - removing it.");
          intervalRecurring = false;
        }
      }
      jsvUnLock(data);
      if (watchPtr) { // if we had a watch pointer, be sure to remove us from it
        jsvObjectSetChild(watchPtr, "timeout", 0);
        // Deal with non-recurring watches
        if (exec) {
          bool watchRecurring = jsvGetBoolAndUnLock(jsvObjectGetChild(watchPtr,  "recur", 0));
          if (!watchRecurring) {
            JsVar *watchArrayPtr = jsvLock(watchArray);
            JsVar *watchNamePtr = jsvGetArrayIndexOf(watchArrayPtr, watchPtr, true);
            if (watchNamePtr) {
              jsvRemoveChild(watchArrayPtr, watchNamePtr);
              jsvUnLock(watchNamePtr);
            }
            jsvUnLock(watchArrayPtr);
            Pin pin = jshGetPinFromVarAndUnLock(jsvObjectGetChild(watchPtr, "pin", 0));
            if (!jsiIsWatchingPin(pin))
              jshPinWatch(pin, false);
          }
        }
        jsvUnLock(watchPtr);
      }

      if (intervalRecurring) {
        JsVarInt interval = jsvGetIntegerAndUnLock(jsvObjectGetChild(timerPtr, "interval", 0));
        if (interval<=0)
          jsvSetInteger(timerTime, time); // just set to current system time
        else
          jsvSetInteger(timerTime, jsvGetInteger(timerTime)+interval);
      } else {
        // free all
        JsVar *foundChild = jsvGetArrayIndexOf(timerArrayPtr, timerPtr, true);
        if (foundChild) {
          // check it exists - could have been removed during jsiExecuteEventCallback!
          jsvRemoveChild(timerArrayPtr, timerNamePtr);
          jsvUnLock(foundChild);
        }
      }
      jsvUnLock(timerCallback);

    }
    jsvUnLock(timerTime);
    jsvUnLock(timerPtr);
    JsVarRef currentTimer = timerNamePtr->nextSibling;
    jsvUnLock(timerNamePtr);
    if (currentTimer != timer) {
      // Whoa! the timer list has changed!
      minTimeUntilNext = 0; // make sure we don't sleep
      break; // get out of here, sort it out next time around idle loop
    }
  }
  jsvUnLock(timerArrayPtr);

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
  if (interruptedDuringEvent) {
    jspSetInterrupted(false);
    interruptedDuringEvent = false;
    jsiConsolePrint("Execution Interrupted during event processing.\n");
  }

  // check for TODOs
  if (todo) {
    jsiSetBusy(BUSY_INTERACTIVE, true);
    if (todo & TODO_RESET) {
      todo &= (TODOFlags)~TODO_RESET;
      // shut down everything and start up again
      jsiKill();
      jsiInit(false); // don't autoload
    }
    if (todo & TODO_FLASH_SAVE) {
      todo &= (TODOFlags)~TODO_FLASH_SAVE;

      jsvGarbageCollect(); // nice to have everything all tidy!
      jsiSoftKill();
      jspSoftKill();
      jsvSoftKill();
      jshSaveToFlash();
      jsvSoftInit();
      jspSoftInit();
      jsiSoftInit();
    }
    if (todo & TODO_FLASH_LOAD) {
      todo &= (TODOFlags)~TODO_FLASH_LOAD;

      jsiSoftKill();
      jspSoftKill();
      jsvSoftKill();
      jshLoadFromFlash();
      jsvSoftInit();
      jspSoftInit();
      jsiSoftInit();
    }
    jsiSetBusy(BUSY_INTERACTIVE, false);
  }

  /* if we've been around this loop, there is nothing to do, and
   * we have a spare 10ms then let's do some Garbage Collection
   * just in case. */
  if (loopsIdling==1 &&
      minTimeUntilNext > jshGetTimeFromMilliseconds(10)) {
    jsiSetBusy(BUSY_INTERACTIVE, true);
    jsvGarbageCollect();
    jsiSetBusy(BUSY_INTERACTIVE, false);
  }
  // Go to sleep!
  if (loopsIdling>1 && // once around the idle loop without having done any work already (just in case)
#ifdef USB
      !jshIsUSBSERIALConnected() && // if USB is on, no point sleeping (later, sleep might be more drastic)
#endif
      !jshHasEvents() && //no events have arrived in the mean time
      !jshHasTransmitData()/* && //nothing left to send over serial?
      minTimeUntilNext > SYSTICK_RANGE*5/4*/) { // we are sure we won't miss anything - leave a little leeway (SysTick will wake us up!)
    jshSleep(minTimeUntilNext);
  }
}

bool jsiLoop() {
  // idle stuff for hardware
  jshIdle();
  // Do general idle stuff
  jsiIdle();
  
  if (jspIsInterrupted()) {
    jspSetInterrupted(false);
    jsiClearInputLine();
  }

  // return console (if it was gone!)
  jsiReturnInputLine();

  return loopsIdling==0;
}

/** Output extra functions defined in an object such that they can be copied to a new device */
void jsiDumpObjectState(JsVar *parentName, JsVar *parent) {
  JsVarRef childRef = parent->firstChild;
  while (childRef) {
    JsVar *child = jsvLock(childRef);
    JsVar *data = jsvSkipName(child);
    if (jsvIsStringEqual(child, JSPARSE_PROTOTYPE_VAR)) {
      JsVarRef protoRef = data->firstChild;
      while (protoRef) {
         JsVar *proto = jsvLock(protoRef);
         jsiConsolePrintStringVar(parentName);
         jsiConsolePrint(".prototype.");
         jsiConsolePrintStringVar(proto);
         jsiConsolePrint(" = ");
         JsVar *protoData = jsvSkipName(proto);
         jsfPrintJSON(protoData, JSON_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES);
         jsvUnLock(protoData);
         jsiConsolePrint(";\n");
         protoRef = proto->nextSibling;
         jsvUnLock(proto);
       }
    } else {
      jsiConsolePrintStringVar(parentName);
      jsiConsolePrint(".");
      jsiConsolePrintStringVar(child);
      jsiConsolePrint(" = ");
      jsfPrintJSON(data, JSON_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES);
      jsiConsolePrint(";\n");

    }
    jsvUnLock(data);
    childRef = child->nextSibling;
    jsvUnLock(child);
  }
}

/** Output current interpreter state such that it can be copied to a new device */
void jsiDumpState() {
  JsVarRef childRef = execInfo.root->firstChild;
  while (childRef) {
    JsVar *child = jsvLock(childRef);
    char childName[JSLEX_MAX_TOKEN_LENGTH];
    jsvGetString(child, childName, JSLEX_MAX_TOKEN_LENGTH);

    JsVar *data = jsvSkipName(child);
    if (jspIsCreatedObject(data) || jswIsBuiltInObject(childName)) {
      jsiDumpObjectState(child, data);
    } else if (jsvIsStringEqual(child, JSI_TIMERS_NAME)) {
      // skip - done later
    } else if (jsvIsStringEqual(child, JSI_WATCHES_NAME)) {
      // skip - done later
    } else if (child->varData.str[0]==JS_HIDDEN_CHAR ||
               jshFromDeviceString(childName)!=EV_NONE) {
      // skip - don't care about this stuff
    } else if (!jsvIsNative(data)) { // just a variable/function!
      if (jsvIsFunction(data)) {
        // function-specific output
        jsiConsolePrint("function ");
        jsiConsolePrintStringVar(child);
        jsfPrintJSONForFunction(data, JSON_SHOW_DEVICES);
        jsiConsolePrint("\n");
        // print any prototypes we had
        JsVar *proto = jsvObjectGetChild(data, JSPARSE_PROTOTYPE_VAR, 0);
        if (proto) {
          JsVarRef protoRef = proto->firstChild;
          jsvUnLock(proto);
          while (protoRef) {
            JsVar *protoName = jsvLock(protoRef);
            JsVar *protoData = jsvSkipName(protoName);
            jsiConsolePrintStringVar(child);
            jsiConsolePrint(".prototype.");
            jsiConsolePrintStringVar(protoName);
            jsiConsolePrint(" = ");
            jsfPrintJSON(protoData, JSON_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES);
            jsiConsolePrint(";\n");
            jsvUnLock(protoData);
            protoRef = protoName->nextSibling;
            jsvUnLock(protoName);
          }
        }
      } else {
        // normal variable definition
        jsiConsolePrint("var ");
        jsiConsolePrintStringVar(child);
        if (!jsvIsUndefined(data)) {
          jsiConsolePrint(" = ");
          jsfPrintJSON(data, JSON_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES);
        }
        jsiConsolePrint(";\n");
      }
    }
    jsvUnLock(data);
    childRef = child->nextSibling;
    jsvUnLock(child);
  }
  // Now do timers
  JsvArrayIterator it;

  JsVar *timerArrayPtr = jsvLock(timerArray);
  jsvArrayIteratorNew(&it, timerArrayPtr);
  jsvUnLock(timerArrayPtr);
  while (jsvArrayIteratorHasElement(&it)) {
    JsVar *timer = jsvArrayIteratorGetElement(&it);
    JsVar *timerCallback = jsvSkipOneNameAndUnLock(jsvFindChildFromString(timer, "callback", false));
    bool recur = jsvGetBoolAndUnLock(jsvObjectGetChild(timer, "recur", 0));
    JsSysTime timerInterval = (JsSysTime)jsvGetIntegerAndUnLock(jsvObjectGetChild(timer, "interval", 0));
    jsiConsolePrint(recur ? "setInterval(" : "setTimeout(");
    jsfPrintJSON(timerCallback, JSON_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES);
    jsiConsolePrintf(", %f);\n", jshGetMillisecondsFromTime(timerInterval));
    jsvUnLock(timerCallback);
    // next
    jsvUnLock(timer);
    jsvArrayIteratorNext(&it);
  }
  jsvArrayIteratorFree(&it);
  // Now do watches
  JsVar *watchArrayPtr = jsvLock(watchArray);
  jsvArrayIteratorNew(&it, watchArrayPtr);
  jsvUnLock(watchArrayPtr);
  while (jsvArrayIteratorHasElement(&it)) {
    JsVar *watch = jsvArrayIteratorGetElement(&it);
    JsVar *watchCallback = jsvSkipOneNameAndUnLock(jsvFindChildFromString(watch, "callback", false));
    bool watchRecur = jsvGetBoolAndUnLock(jsvObjectGetChild(watch, "recur", 0));
    int watchEdge = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(watch, "edge", 0));
    JsVar *watchPin = jsvObjectGetChild(watch, "pin", 0);
    jsiConsolePrint("setWatch(");
    jsfPrintJSON(watchCallback, JSON_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES);
    jsiConsolePrint(", ");
    jsfPrintJSON(watchPin, JSON_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES);
    jsiConsolePrint(", { repeat:");
    jsiConsolePrint(watchRecur?"true":"false");
    jsiConsolePrint(", edge:");
    if (watchEdge<0) jsiConsolePrint("'falling'");
    else if (watchEdge>0) jsiConsolePrint("'rising'");
    else jsiConsolePrint("'both'");
    jsiConsolePrint(" });\n");
    jsvUnLock(watchPin);
    jsvUnLock(watchCallback);
    // next
    jsvUnLock(watch);
    jsvArrayIteratorNext(&it);
  }
  jsvArrayIteratorFree(&it);

  // and now serial
  JsVar *str = jsvNewFromEmptyString();
  jsiAppendHardwareInitialisation(str, true);
  jsiConsolePrintStringVar(str);
  jsvUnLock(str);
}

void jsiSetTodo(TODOFlags newTodo) {
  todo = newTodo;
}

JsVarInt jsiTimerAdd(JsVar *timerPtr) {
  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarInt itemIndex = jsvArrayPushWithInitialSize(timerArrayPtr, timerPtr, 1) - 1;
  jsvUnLock(timerArrayPtr);
  return itemIndex;
}
