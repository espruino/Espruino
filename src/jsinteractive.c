/*
 * jsinteractive.c
 *
 *  Created on: 8 Aug 2012
 *      Author: gw
 */

#include "jsutils.h"
#include "jsinteractive.h"
#include "jsfunctions.h"
#include "jshardware.h"

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
 TODO_NOTHING = 0,
 TODO_FLASH_SAVE = 1,
 TODO_FLASH_LOAD = 2,
 TODO_RESET = 4,
} TODOFlags;

typedef enum {
 IS_NONE,
 IS_HAD_R,
 IS_HAD_27,
 IS_HAD_27_79,
 IS_HAD_27_91,
 IS_HAD_27_91_49,
 IS_HAD_27_91_51,
 IS_HAD_27_91_52,
} InputState;

TODOFlags todo = TODO_NOTHING;
JsVarRef events = 0; // Linked List of events to execute
JsVarRef timerArray = 0; // Linked List of timers to check and run
JsVarRef watchArray = 0; // Linked List of input watches to check and run
JsVarRef classUSART1 = 0;
JsVarRef classUSART2 = 0;
#ifdef USB
JsVarRef classUSB = 0;
#endif
// ----------------------------------------------------------------------------
IOEventFlags consoleDevice = EV_NONE; ///< The console device for user interaction
int pinBusyIndicator = DEFAULT_BUSY_PIN_INDICATOR;
int pinSleepIndicator = DEFAULT_SLEEP_PIN_INDICATOR;
bool echo;                  ///< do we provide any user feedback?
// ----------------------------------------------------------------------------
JsParse p; ///< The parser we're using for interactiveness
JsVar *inputLine = 0; ///< The current input line
bool inputLineRemoved = false;
int inputCursorPos = 0; ///< The position of the cursor in the input line
InputState inputState = 0; ///< state for dealing with cursor keys
bool hasUsedHistory = false; ///< Used to speed up - if we were cycling through history and then edit, we need to copy the string
int loopsIdling; ///< How many times around the loop have we been entirely idle?
JsVar *jsiHandleFunctionCall(JsExecInfo *execInfo, JsVar *a, const char *name); // forward decl
// ----------------------------------------------------------------------------
#define TIMER_MIN_INTERVAL 0.1 // in milliseconds
#define USART_CALLBACK_NAME "_callback"
#define USART_BAUDRATE_NAME "_baudrate"
#define JSI_WATCHES_NAME "watches"
#define JSI_TIMERS_NAME "timers"
#define JSI_HISTORY_NAME "history"

bool isUSART(JsVarRef ref) {
  return  ref==classUSART1
        || ref==classUSART2
#ifdef USB
        || ref==classUSB
#endif
      ;
}
IOEventFlags getDeviceFromClass(JsVarRef ref) {
  if (ref==classUSART1) return EV_USART1;
  if (ref==classUSART2) return EV_USART2;
#ifdef USB
  if (ref==classUSB) return EV_USBSERIAL;
#endif
  return EV_NONE;
}
JsVarRef getClassFromDevice(IOEventFlags device) {
  if (device==EV_USART1) return classUSART1;
  if (device==EV_USART2) return classUSART2;
#ifdef USB
  if (device==EV_USBSERIAL) return classUSB;
#endif
  return 0;
}

static inline bool jsiShowInputLine() {
  return echo && !inputLineRemoved;
}

/// Change the console to a new location
void jsiSetConsoleDevice(IOEventFlags device) {
  if (device == consoleDevice) return;
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

void jsiConsolePrintChar(char data) {
  jshTransmit(consoleDevice, (unsigned char)data);
}

void jsiConsolePrintCharEscaped(char data) {
  if (data=='\n') {
    jsiConsolePrintChar('\\');
    jsiConsolePrintChar('n');
  } else if (data=='\r') {
    jsiConsolePrintChar('\\');
    jsiConsolePrintChar('r');
  } else if (data=='"') {
      jsiConsolePrintChar('\\');
      jsiConsolePrintChar('"');
  } else
    jsiConsolePrintChar(data);
}

void jsiConsolePrint(const char *str) {
  while (*str) {
       if (*str == '\n') jsiConsolePrintChar('\r');
       jsiConsolePrintChar(*(str++));
  }
}

void jsiConsolePrintEscaped(const char *str) {
  while (*str) {
       if (*str == '\n') jsiConsolePrintCharEscaped('\r');
       jsiConsolePrintCharEscaped(*(str++));
  }
}

void jsiConsolePrintInt(int d) {
    char buf[32];
    itoa(d, buf, 10);
    jsiConsolePrint(buf);
}

/// Print the contents of a string var from a character position until end of line (adding an extra ' ' to delete a character if there was one)
void jsiConsolePrintStringVarUntilEOL(JsVar *v, int fromCharacter, bool andBackup) {
  assert(jsvIsString(v) || jsvIsName(v));
  int chars = 0;
  JsVarRef r = jsvGetRef(v);
  bool done = false;
  while (!done && r) {
    v = jsvLock(r);
    size_t l = jsvGetMaxCharactersInVar(v);
    size_t i;
    for (i=0;i<l;i++) {
      char ch = v->varData.str[i];
      if (!ch) break;
      fromCharacter--;
      if (fromCharacter<0) { 
        if (ch == '\n') {
          done = true;
          break;
        }
        jsiConsolePrintChar(ch);
        chars++;
      }
    }
    r = v->lastChild;
    jsvUnLock(v);
  }
  if (andBackup) {
    jsiConsolePrintChar(' ');chars++;
    while (chars--) jsiConsolePrintChar(0x08); //delete
  }
}

/** Print the contents of a string var - directly - starting from the given character, and
 * using newLineCh to prefix new lines (if it is not 0). */
void jsiConsolePrintStringVarWithNewLineChar(JsVar *v, int fromCharacter, char newLineCh) {
  assert(jsvIsString(v));
  int n = 0;
  JsVarRef r = jsvGetRef(v);
  while (r) {
    v = jsvLock(r);
    size_t l = jsvGetMaxCharactersInVar(v);
    size_t i;
    for (i=0;i<l;i++) {
      char ch = v->varData.str[i];
      if (!ch) break;
      if (n>=fromCharacter) {
        if (ch == '\n') jsiConsolePrintChar('\r');
        jsiConsolePrintChar(ch);
        if (ch == '\n' && newLineCh) jsiConsolePrintChar(newLineCh);
      }
      n++;
    }
    r = v->lastChild;
    jsvUnLock(v);
  }
}

/// Print the contents of a string var - directly
void jsiConsolePrintStringVar(JsVar *v) {
  jsiConsolePrintStringVarWithNewLineChar(v,0,0);
}

/** Assuming that we are at the end of the string, this backs up
 * and deletes it */
void jsiConsoleEraseStringVarBackwards(JsVar *v) {
  assert(jsvIsString(v) || jsvIsName(v));

  int line, lines = jsvGetLinesInString(v);
  for (line=lines;line>0;line--) {
    int i,chars = jsvGetCharsOnLine(v, line);
    if (line==lines) {
      for (i=0;i<chars;i++) jsiConsolePrintChar(0x08); // move cursor back
    }
    for (i=0;i<chars;i++) jsiConsolePrintChar(' '); // move cursor forwards and wipe out
    for (i=0;i<chars;i++) jsiConsolePrintChar(0x08); // move cursor back
    if (line>1) { 
      // clear the character before - this would have had a colon
      jsiConsolePrintChar(0x08);
      jsiConsolePrintChar(' ');
      // move cursor up      
      jsiConsolePrintChar(27);
      jsiConsolePrintChar(91); 
      jsiConsolePrintChar(65);
    }
  }
}

/** Assuming that we are at fromCharacter position in the string var,
 * erase everything that comes AFTER and return the cursor to 'fromCharacter'
 * On newlines, if erasePrevCharacter, we remove the character before too. */
void jsiConsoleEraseStringVarFrom(JsVar *v, int fromCharacter, bool erasePrevCharacter) {
  assert(jsvIsString(v) || jsvIsName(v));
  int cursorLine, cursorCol;
  jsvGetLineAndCol(v, fromCharacter, &cursorLine, &cursorCol);
  // delete contents of current line
  int i, chars = jsvGetCharsOnLine(v, cursorLine);
  for (i=cursorCol;i<=chars;i++) jsiConsolePrintChar(' ');
  for (i=0;i<chars;i++) jsiConsolePrintChar(0x08); // move cursor back

  int line, lines = jsvGetLinesInString(v);
  for (line=cursorLine+1;line<=lines;line++) {
    jsiConsolePrintChar(27);
    jsiConsolePrintChar(91);
    jsiConsolePrintChar(66); // move down
    chars = jsvGetCharsOnLine(v, line);
    for (i=0;i<chars;i++) jsiConsolePrintChar(' '); // move cursor forwards and wipe out
    for (i=0;i<chars;i++) jsiConsolePrintChar(0x08); // move cursor back
    if (erasePrevCharacter) {
      jsiConsolePrintChar(0x08); // move cursor back
      jsiConsolePrintChar(' ');
    }
  }
  // move the cursor back up
  for (line=cursorLine+1;line<=lines;line++) {
    jsiConsolePrintChar(27);
    jsiConsolePrintChar(91);
    jsiConsolePrintChar(65);
  }
  // move the cursor forwards
  for (i=1;i<cursorCol;i++) {
    jsiConsolePrintChar(27);
    jsiConsolePrintChar(91);
    jsiConsolePrintChar(67);
  }
}

void jsiMoveCursor(int oldX, int oldY, int newX, int newY) {
  // see http://www.termsys.demon.co.uk/vtansi.htm - we could do this better
  // move cursor
  while (oldX < newX) {
    jsiConsolePrintChar(27);
    jsiConsolePrintChar(91);
    jsiConsolePrintChar(67);
    oldX++;
  }
  while (oldX > newX) {
    jsiConsolePrintChar(27);
    jsiConsolePrintChar(91);
    jsiConsolePrintChar(68);
    oldX--;
  }
  while (oldY < newY) {
    jsiConsolePrintChar(27);
    jsiConsolePrintChar(91);
    jsiConsolePrintChar(66);
    oldY++;
  }
  while (oldY > newY) {
    jsiConsolePrintChar(27);
    jsiConsolePrintChar(91);
    jsiConsolePrintChar(65);
    oldY--;
  }
}

void jsiMoveCursorChar(JsVar *v, int fromCharacter, int toCharacter) {
  if (fromCharacter==toCharacter) return;
  int oldX, oldY;
  jsvGetLineAndCol(v, fromCharacter, &oldY, &oldX);
  int newX, newY;
  jsvGetLineAndCol(v, toCharacter, &newY, &newX);
  jsiMoveCursor(oldX, oldY, newX, newY);
}

/// If the input line was shown in the console, remove it
void jsiConsoleRemoveInputLine() {
  if (!inputLineRemoved) {
    inputLineRemoved = true;
    if (echo) { // intentionally not using jsiShowInputLine()
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
      jsiConsolePrintChar('\r'); // carriage return
      jsiConsolePrintChar('>');
      jsiConsolePrintStringVarWithNewLineChar(inputLine, 0, ':');
      jsiMoveCursorChar(inputLine, (int)jsvGetStringLength(inputLine), inputCursorPos);
    }
  }
}

void jsiConsolePrintPosition(struct JsLex *lex, int tokenPos) {
  int line,col;
  JsVar *v = jsvLock(lex->sourceVarRef);
  jsvGetLineAndCol(v, tokenPos, &line, &col);
  jsvUnLock(v);
  jsiConsolePrint("line ");
  jsiConsolePrintInt(line);
  jsiConsolePrint(" col ");
  jsiConsolePrintInt(col);
  jsiConsolePrint(" (char ");
  jsiConsolePrintInt(tokenPos);
  jsiConsolePrint(")\n");
}

void jsiConsolePrintTokenLineMarker(struct JsLex *lex, int tokenPos) {
  int line = 1,col = 1;
  JsVar *v = jsvLock(lex->sourceVarRef);
  jsvGetLineAndCol(v, tokenPos, &line, &col);
  int startOfLine = jsvGetIndexFromLineAndCol(v, line, 1);
  jsiConsolePrintStringVarUntilEOL(v, startOfLine, false);
  jsiConsolePrint("\n");
  while (col-- > 1) jsiConsolePrintChar(' ');
  jsiConsolePrintChar('^');
  jsiConsolePrint("\n");
  jsvUnLock(v);
}


/// Print the contents of a string var - directly
void jsiTransmitStringVar(IOEventFlags device, JsVar *v) {
  assert(jsvIsString(v) || jsvIsName(v));
  JsVarRef r = jsvGetRef(v);
  while (r) {
    v = jsvLock(r);
    size_t l = jsvGetMaxCharactersInVar(v);
    size_t i;
    for (i=0;i<l;i++) {
      char ch = v->varData.str[i];
      if (!ch) break;
      jshTransmit(device, (unsigned char)ch);
    }
    r = v->lastChild;
    jsvUnLock(v);
  }
}

void jsiSetBusy(JsiBusyDevice device, bool isBusy) {
  static JsiBusyDevice business = 0;

  if (isBusy)
    business |= device;
  else
    business &= (JsiBusyDevice)~device;

  if (pinBusyIndicator >= 0)
    jshPinOutput(pinBusyIndicator, business!=0);
}

void jsiSetSleep(bool isSleep) {
  if (pinSleepIndicator >= 0)
    jshPinOutput(pinSleepIndicator, isSleep);
}

JsVarRef _jsiInitSerialClass(IOEventFlags device, const char *serialName) {
  JsVarRef class = 0;
  JsVar *name = jsvFindChildFromString(p.root, serialName, true);
  if (name) {
    name->flags |= JSV_NATIVE;
    if (!name->firstChild) {
      JsVar *obj = jsvNewWithFlags(JSV_OBJECT);
      if (!obj) { // out of memory
        jsvUnLock(name);
        return 0;
      }
      name->firstChild = jsvUnLock(jsvRef(obj));
    }
    class = jsvRefRef(name->firstChild);
    // if baud rate is set, restore it
    if (device != EV_USBSERIAL) {
      JsVar *baudRate = jsvFindChildFromString(class, USART_BAUDRATE_NAME, false);
      if (baudRate) {
        jshUSARTSetup(device, (int)jsvGetIntegerAndUnLock(jsvSkipName(baudRate)));
      }
      jsvUnLock(baudRate);
    }
    jsvUnLock(name);
  } else {
    jsError("Out of memory while creating Serial class");
  }
  return class;
}

static JsVarRef _jsiInitNamedArray(const char *name) {
  JsVar *arrayName = jsvFindChildFromString(p.root, name, true);
  if (!arrayName) return 0; // out of memory
  if (!arrayName->firstChild) {
    JsVar *array = jsvNewWithFlags(JSV_ARRAY);
    if (!array) { // out of memory
      jsvUnLock(arrayName);
      return 0;
    }
    arrayName->firstChild = jsvUnLock(jsvRef(array));
  }
  JsVarRef arrayRef = jsvRefRef(arrayName->firstChild);
  jsvUnLock(arrayName);
  return arrayRef;
}

// Used when recovering after being flashed
// 'claim' anything we are using
void jsiSoftInit() {
  events = 0;
  inputLine = jsvNewFromString("");
  inputCursorPos = 0;

  // Load inbuild Classes
  classUSART1 = _jsiInitSerialClass(EV_USART1, "Serial1");
  classUSART2 = _jsiInitSerialClass(EV_USART2, "Serial2");
#ifdef USB
  classUSB = _jsiInitSerialClass(EV_USBSERIAL, "USB");
#endif

  // Load timer/watch arrays
  timerArray = _jsiInitNamedArray(JSI_TIMERS_NAME);
  watchArray = _jsiInitNamedArray(JSI_WATCHES_NAME);

  // Check any existing watches and set up interrupts for them
  if (watchArray) {
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsVarRef watch = watchArrayPtr->firstChild;
    while (watch) {
      JsVar *watchNamePtr = jsvLock(watch);
      JsVar *watchPin = jsvSkipNameAndUnLock(jsvFindChildFromString(watchNamePtr->firstChild, "pin", false));
      jshPinWatch(jshGetPinFromVar(watchPin), true);
      jsvUnLock(watchPin);
      watch = watchNamePtr->nextSibling;
      jsvUnLock(watchNamePtr);
    }
    jsvUnLock(watchArrayPtr);
  }

  // Check any existing timers and try and set time correctly
  if (timerArray) {
    JsSysTime currentTime = jshGetSystemTime();
    JsVar *timerArrayPtr = jsvLock(timerArray);
    JsVarRef timer = timerArrayPtr->firstChild;
    while (timer) {
      JsVar *timerNamePtr = jsvLock(timer);
      JsVar *timerTime = jsvSkipNameAndUnLock(jsvFindChildFromString(timerNamePtr->firstChild, "time", false));
      JsVarFloat interval = jsvGetFloatAndUnLock(jsvSkipNameAndUnLock(jsvFindChildFromString(timerNamePtr->firstChild, "interval", false)));
      jsvSetInteger(timerTime, currentTime + jshGetTimeFromMilliseconds(interval));
      jsvUnLock(timerTime);
      timer = timerNamePtr->nextSibling;
      jsvUnLock(timerNamePtr);
    }
    jsvUnLock(timerArrayPtr);
  }
  // Now run initialisation code
  JsVar *initName = jsvFindChildFromString(p.root, "__init", false);
  if (initName && initName->firstChild) {
    //jsPrint("Running initialisation code...\n");
    JsVar *initCode = jsvLock(initName->firstChild);
    jsvUnLock(jspEvaluateVar(&p, initCode));
    jsvUnLock(initCode);
    JsVar *root = jsvLock(p.root);
    jsvRemoveChild(root, initName);
    jsvUnLock(root);
  }
  jsvUnLock(initName);
}

// Used when shutting down before flashing
// 'release' anything we are using, but ensure that it doesn't get freed
void jsiSoftKill() {
  jsvUnLock(inputLine);
  inputLine=0;
  inputCursorPos = 0;

  // UnRef inbuild Classes
  if (classUSART1) {
    jsvUnRefRef(classUSART1);
    classUSART1 = 0;
  }
  if (classUSART2) {
    jsvUnRefRef(classUSART2);
    classUSART2 = 0;
  }
#ifdef USB
  if (classUSB) {
    jsvUnRefRef(classUSB);
    classUSB = 0;
  }
#endif

  // Unref Watches/etc
  if (events) {
    jsvUnRefRef(events);
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
      JsVar *watchPin = jsvSkipNameAndUnLock(jsvFindChildFromString(watchNamePtr->firstChild, "pin", false));
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
  JsVar *initName = jsvFindChildFromString(p.root, "__init", true);
  if (initName->firstChild) {
    jsvUnRefRef(initName->firstChild); 
    initName->firstChild = 0;
  }
  JsVar *initCode = jsvNewFromString("");
  if (initCode) { // out of memory
    initName->firstChild = jsvGetRef(jsvRef(initCode));
    if (!echo) jsvAppendString(initCode, "echo(0);");
    if (pinBusyIndicator != DEFAULT_BUSY_PIN_INDICATOR) {
      jsvAppendString(initCode, "setBusyIndicator(");
      jsvAppendInteger(initCode, pinBusyIndicator);
      jsvAppendString(initCode, ");");
    }
    if (pinSleepIndicator != DEFAULT_BUSY_PIN_INDICATOR) {
      jsvAppendString(initCode, "setSleepIndicator(");
      jsvAppendInteger(initCode, pinSleepIndicator);
      jsvAppendString(initCode, ");");
    }
    jsvUnLock(initCode);
  }
  jsvUnLock(initName);
}

void jsiInit(bool autoLoad) {
  jsvInit();
  jspInit(&p);
  // link in our functions
  jsfSetHandleFunctionCallDelegate(jsiHandleFunctionCall);


  /*for (i=0;i<IOPINS;i++)
     ioPinState[i].callbacks = 0;*/

  // Set defaults
  echo = true;
  consoleDevice = DEFAULT_CONSOLE_DEVICE;
  pinBusyIndicator = DEFAULT_BUSY_PIN_INDICATOR;
  if (jshIsUSBSERIALConnected())
    consoleDevice = EV_USBSERIAL;

  /* If flash contains any code, then we should
     Try and load from it... */
  if (autoLoad && jshFlashContainsCode()) {
    jspSoftKill(&p);
    jsvSoftKill();
    jshLoadFromFlash();
    jsvSoftInit();
    jspSoftInit(&p);
  }
  //jsvTrace(jsiGetParser()->root, 0);

  // Softinit may run initialisation code that will overwrite defaults
  jsiSoftInit();

  if (echo) { // intentionally not using jsiShowInputLine()
    // set up terminal to avoid word wrap
    jsiConsolePrint("\e[?7l");
    // rectangles @ http://www.network-science.de/ascii/
    jsiConsolePrint("\r\n _____                 _ \r\n"
              "|   __|___ ___ ___ _ _|_|___ ___ \r\n"
              "|   __|_ -| . |  _| | | |   | . |\r\n"
              "|_____|___|  _|_| |___|_|_|_|___|\r\n"
              "          |_|                " JS_VERSION "\r\n"
              "   Copyright 2012 Gordon Williams\r\n"
              "                    gw@pur3.co.uk\r\n"
              "---------------------------------\r\n"
              "This version is for personal use\r\n"
              "only. If you were sold this on a\r\n"
              "device, please contact us.\r\n"
              "---------------------------------\r\n");
    jsiConsolePrint("\r\n>");
  }
}



void jsiKill() {
  jsiSoftKill();

  jspKill(&p);
  jsvKill();
}

int jsiCountBracketsInInput() {
  int brackets = 0;

  JsLex lex;
  jslInit(&lex, inputLine, 0, -1);
  while (lex.tk!=LEX_EOF) {
    if (lex.tk=='{' || lex.tk=='[' || lex.tk=='(') brackets++;
    if (lex.tk=='}' || lex.tk==']' || lex.tk==')') brackets--;
    if (brackets<0) break; // closing bracket before opening!
    jslGetNextToken(&lex);
  }
  jslKill(&lex);

  return brackets;
} 

/// Tries to get rid of some memory (by clearing command history). Returns true if it got rid of something, false if it didn't.
bool jsiFreeMoreMemory() {
  JsVar *history = jsvSkipNameAndUnLock(jsvFindChildFromString(p.root, JSI_HISTORY_NAME, false));
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
  JsVar *history = jsvFindChildFromString(p.root, JSI_HISTORY_NAME, true);
  if (!history) return; // out of memory
  // ensure we actually have the history array
  if (!history->firstChild) {
    JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
    if (!arr) {// out of memory
      jsvUnLock(history);
      return;
    }
    history->firstChild = jsvUnLock(jsvRef(arr));
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
  JsVar *history = jsvSkipNameAndUnLock(jsvFindChildFromString(p.root, JSI_HISTORY_NAME, false));
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
  JsVar *history = jsvSkipNameAndUnLock(jsvFindChildFromString(p.root, JSI_HISTORY_NAME, false));
  if (!history) return false;
  bool inHistory = jsvUnLock(jsvGetArrayIndexOf(history, line, true/*exact*/));
  jsvUnLock(history);
  return inHistory;
}

void jsiReplaceInputLine(JsVar *newLine) {
  if (jsiShowInputLine()) {
    int oldLen =  (int)jsvGetStringLength(inputLine);
    jsiMoveCursorChar(inputLine, inputCursorPos, oldLen); // move cursor to end
    jsiConsoleEraseStringVarBackwards(inputLine);
    jsiConsolePrintStringVarWithNewLineChar(newLine,0,':');
  }
  jsvUnLock(inputLine);
  inputLine = jsvLockAgain(newLine);
  inputCursorPos = (int)jsvGetStringLength(inputLine);
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
    inputLine = jsvNewFromString("");
    inputCursorPos = 0;
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
      }
    }
  }
}

void jsiHandleDelete(bool isBackspace) {
  int l = (int)jsvGetStringLength(inputLine);
  if (isBackspace && inputCursorPos<=0) return; // at beginning of line
  if (!isBackspace && inputCursorPos>=l) return; // at end of line
  // work out if we are deleting a newline
  bool deleteNewline = (isBackspace && jsvGetCharInString(inputLine,inputCursorPos-1)=='\n') ||
                       (!isBackspace && jsvGetCharInString(inputLine,inputCursorPos)=='\n');
  // If we mod this to keep the string, use jsiIsAboutToEditInputLine
  if (deleteNewline && jsiShowInputLine()) {
    jsiConsoleEraseStringVarFrom(inputLine, inputCursorPos, true/*before newline*/); // erase all in front
    if (isBackspace) {
      // delete newline char
      jsiConsolePrintChar(0x08);
      jsiConsolePrintChar(' ');
      jsiMoveCursorChar(inputLine, inputCursorPos, inputCursorPos-1); // move cursor back
    }
  }

  JsVar *v = jsvNewFromString("");
  int p = inputCursorPos;
  if (isBackspace) p--;
  if (p>0) jsvAppendStringVar(v, inputLine, 0, p); // add before cursor (delete)
  if (p+1<l) jsvAppendStringVar(v, inputLine, p+1, JSVAPPENDSTRINGVAR_MAXLENGTH); // add the rest
  jsvUnLock(inputLine);
  inputLine=v;
  if (isBackspace)
    inputCursorPos--; // move cursor back

  // update the console
  if (jsiShowInputLine()) {
    if (deleteNewline) {
      // we already removed everything, so just put it back
      jsiConsolePrintStringVarWithNewLineChar(inputLine, inputCursorPos, ':');
      jsiMoveCursorChar(inputLine, (int)jsvGetStringLength(inputLine), inputCursorPos); // move cursor back
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
  int l = (int)jsvGetStringLength(inputLine);
  while (inputCursorPos<l && jsvGetCharInString(inputLine,inputCursorPos)!='\n') {
    if (jsiShowInputLine())
      jsiConsolePrintChar(jsvGetCharInString(inputLine,inputCursorPos));
    inputCursorPos++;
  }
}

void jsiHandleMoveUpDown(int direction) {
  int x,y, lines=jsvGetLinesInString(inputLine);
  jsvGetLineAndCol(inputLine, inputCursorPos, &y, &x);
  int newX=x,newY=y;
  newY+=direction;
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
  int i = inputCursorPos, l = (int)jsvGetStringLength(inputLine);
  while (i < l) {
    if (!isWhitespace(jsvGetCharInString(inputLine, i)))
        return false;
    i++;
  }
  return true;
}

void jsiHandleChar(char ch) {
  //jsiConsolePrint("  ["); jsiConsolePrintInt(inputState);jsiConsolePrint(":");jsiConsolePrintInt(ch); jsiConsolePrint("]  \n");
  //
  // special stuff
  // 27 then 91 then 68 - left
  // 27 then 91 then 67 - right
  // 27 then 91 then 65 - up
  // 27 then 91 then 66 - down
  // 27 then 91 then 51 then 126 - backwards delete
  // 27 then 91 then 52 then 126 - numpad end
  // 27 then 91 then 49 then 126 - numpad home
  // 27 then 79 then 70 - home
  // 27 then 79 then 72 - end

  if (ch == 0) {
    inputState = IS_NONE; // numpad stuff - but treat it like it didn't happen
  } else if (ch == 27) {
    inputState = IS_HAD_27;
  } else if (inputState==IS_HAD_27) {
    inputState = IS_NONE;
    if (ch == 79)
      inputState = IS_HAD_27_79;
    else if (ch == 91)
      inputState = IS_HAD_27_91;
  } else if (inputState==IS_HAD_27_79) {
    inputState = IS_NONE;
    if (ch == 70)
      jsiHandleEnd();
    else if (ch == 72)
      jsiHandleHome();
  } else if (inputState==IS_HAD_27_91) {
    inputState = IS_NONE;
    if (ch==68) { // left
      if (inputCursorPos>0 && jsvGetCharInString(inputLine,inputCursorPos-1)!='\n') {
        inputCursorPos--;
        if (jsiShowInputLine()) {
          jsiConsolePrintChar(27);
          jsiConsolePrintChar(91);
          jsiConsolePrintChar(68);
        }
      }
    } else if (ch==67) { // right
      if (inputCursorPos<(int)jsvGetStringLength(inputLine) && jsvGetCharInString(inputLine,inputCursorPos)!='\n') {
        inputCursorPos++;
        if (jsiShowInputLine()) {
          jsiConsolePrintChar(27);
          jsiConsolePrintChar(91);
          jsiConsolePrintChar(67);
        }
      }
    } else if (ch==65) { // up
      int l = (int)jsvGetStringLength(inputLine);
      if ((l==0 || jsiIsInHistory(inputLine)) && inputCursorPos==l)
        jsiChangeToHistory(true); // if at end of line
      else
        jsiHandleMoveUpDown(-1);
    } else if (ch==66) { // down
      int l = (int)jsvGetStringLength(inputLine);
      if ((l==0 || jsiIsInHistory(inputLine)) && inputCursorPos==l)
        jsiChangeToHistory(false); // if at end of line
      else
        jsiHandleMoveUpDown(1);
    } else if (ch==49) {
      inputState=IS_HAD_27_91_49;
    } else if (ch==51) {
      inputState=IS_HAD_27_91_51;
    } else if (ch==52) {
      inputState=IS_HAD_27_91_52;
    }
  } else if (inputState==IS_HAD_27_91_49) {
    inputState = IS_NONE;
    if (ch==126) { // Numpad Home
      jsiHandleHome();
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
  } else {  
    inputState = IS_NONE;
    if (ch == 0x08 || ch == 0x7F /*delete*/) {
      jsiHandleDelete(true /*backspace*/);
    } else if (ch == '\n' && inputState == IS_HAD_R) {
      inputState = IS_NONE; //  ignore \ r\n - we already handled it all on \r
    } else if (ch == '\r' || ch == '\n') { 
      if (jsiAtEndOfInputLine()) { // ignore unless at EOL
        if (ch == '\r') inputState = IS_HAD_R;
        if (jsiCountBracketsInInput()<=0) { // actually execute!
          if (jsiShowInputLine()) {
            jsiConsolePrintChar('\r');
            jsiConsolePrintChar('\n');
          }
          inputLineRemoved = true;

          // Get line to execute, and reset inputLine
          JsVar *lineToExecute = inputLine;
          inputLine = jsvNewFromString("");
          inputCursorPos = 0;
          // execute!
          JsVar *v = jspEvaluateVar(&p, lineToExecute);
          // add input line to history
          jsiHistoryAddLine(lineToExecute);
          jsvUnLock(lineToExecute);
          // print result
          if (echo) { // intentionally not using jsiShowInputLine()
            jsiConsolePrintChar('=');
            jsfPrintJSON(v);
            jsiConsolePrint("\r\n");
          }
          jsvUnLock(v);
          // console will be returned next time around the input loop
        } else {
          if (jsiShowInputLine()) jsiConsolePrint("\n:");
          jsiIsAboutToEditInputLine();
          jsvAppendCharacter(inputLine, '\n');
          inputCursorPos++; 
        }
      } else { // new line - but not at end of line!
        jsiIsAboutToEditInputLine();
        if (jsiShowInputLine()) jsiConsoleEraseStringVarFrom(inputLine, inputCursorPos, false/*no need to erase the char before*/); // erase all in front
        JsVar *v = jsvNewFromString("");
        if (inputCursorPos>0) jsvAppendStringVar(v, inputLine, 0, inputCursorPos);
        jsvAppendCharacter(v, '\n');
        jsvAppendStringVar(v, inputLine, inputCursorPos, JSVAPPENDSTRINGVAR_MAXLENGTH); // add the rest
        jsvUnLock(inputLine);
        inputLine=v;
        if (jsiShowInputLine()) { // now print the rest
          jsiConsolePrintStringVarWithNewLineChar(inputLine, inputCursorPos, ':');
          jsiMoveCursorChar(inputLine, (int)jsvGetStringLength(inputLine), inputCursorPos+1); // move cursor back
        }
        inputCursorPos++;
      }
    } else {
      // Add the character to our input line
      jsiIsAboutToEditInputLine();
      int l = (int)jsvGetStringLength(inputLine);
      bool hasTab = ch=='\t';
      if (inputCursorPos>=l) {
        if (hasTab) jsvAppendString(inputLine, "    ");
        else jsvAppendCharacter(inputLine, ch);
      } else {
        JsVar *v = jsvNewFromString("");
        if (inputCursorPos>0) jsvAppendStringVar(v, inputLine, 0, inputCursorPos);
        if (hasTab) jsvAppendString(v, "    ");
        else jsvAppendCharacter(v, ch);
        jsvAppendStringVar(v, inputLine, inputCursorPos, JSVAPPENDSTRINGVAR_MAXLENGTH); // add the rest
        jsvUnLock(inputLine);
        inputLine=v;
        if (jsiShowInputLine()) jsiConsolePrintStringVarUntilEOL(inputLine, inputCursorPos, true/*and backup*/);
      }
      inputCursorPos += hasTab ? 4 : 1;
      if (jsiShowInputLine()) {
        if (hasTab) jsiConsolePrint("    ");
        else jsiConsolePrintChar(ch);
      }
    }
  }
}

void jsiQueueEvents(JsVarRef callbacks, JsVar *arg0) { // array of functions or single function
  if (!callbacks) return;
  // find the last event in our queue
  JsVar *lastEvent = 0;
  if (events) {
    lastEvent = jsvLock(events);
    while (lastEvent->nextSibling) {
      JsVar *next = jsvLock(lastEvent->nextSibling);
      jsvUnLock(lastEvent);
      lastEvent = next;
    }
  }

  JsVar *callbackVar = jsvLock(callbacks);
  // if it is a single callback, just add it
  if (jsvIsFunction(callbackVar) || jsvIsString(callbackVar)) {
    JsVar *event = jsvNewWithFlags(JSV_OBJECT|JSV_NATIVE);
    if (event) { // Could be out of memory error!
      event = jsvRef(event);
      jsvUnLock(jsvAddNamedChild(event, callbackVar, "func"));
      if (arg0) jsvUnLock(jsvAddNamedChild(event, arg0, "arg0"));
      if (lastEvent) {
        lastEvent->nextSibling = jsvGetRef(event);
        jsvUnLock(lastEvent);
      } else
        events = jsvGetRef(event);
      jsvUnLock(event);
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
      JsVar *event = jsvNewWithFlags(JSV_OBJECT|JSV_NATIVE);
      if (event) { // Could be out of memory error!
        event = jsvRef(event);
        jsvUnLock(jsvAddNamedChild(event, child, "func"));
        if (arg0) jsvUnLock(jsvAddNamedChild(event, arg0, "arg0"));
        // add event to the events list
        if (lastEvent) {
          lastEvent->nextSibling = jsvGetRef(event);
          jsvUnLock(lastEvent);
        } else
          events = jsvGetRef(event);
        jsvUnLock(lastEvent);
        lastEvent = event;
        // go to next callback
      }
      next = child->nextSibling;
      jsvUnLock(child);
    }
    // clean up
    jsvUnLock(lastEvent);
  }
}

void jsiExecuteEvents() {
  bool hasEvents = events;
  if (hasEvents) jsiSetBusy(BUSY_INTERACTIVE, true);
  while (events) {
    JsVar *event = jsvLock(events);
    // Get function to execute
    JsVar *func = jsvSkipNameAndUnLock(jsvFindChildFromString(jsvGetRef(event), "func", false));
    JsVar *arg0 = jsvSkipNameAndUnLock(jsvFindChildFromString(jsvGetRef(event), "arg0", false));
    // free + go to next
    events = event->nextSibling;
    event->nextSibling = 0;
    jsvUnRef(event);
    jsvUnLock(event);

    // now run..
    if (func) {
      if (jsvIsFunction(func))
        jspExecuteFunction(&p, func, arg0);
      else if (jsvIsString(func))
        jsvUnLock(jspEvaluateVar(&p, func));
      else 
        jsError("Unknown type of callback in Event Queue");
    }
    //jsPrint("Event Done\n");
    jsvUnLock(func);
    jsvUnLock(arg0);
  }
  if (hasEvents) jsiSetBusy(BUSY_INTERACTIVE, false);
}

bool jsiHasTimers() {
  if (!timerArray) return false;
  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarInt c = jsvGetArrayLength(timerArrayPtr);
  jsvUnLock(timerArrayPtr);
  return c>0;
}

void jsiIdle() {
  // This is how many times we have been here and not done anything.
  // It will be zeroed if we do stuff later
  loopsIdling++;
  // Handle hardware-related idle stuff (like checking for pin events)
  IOEvent event;
  while (jshPopIOEvent(&event)) {
    loopsIdling = 0; // because we're not idling
    if (IOEVENTFLAGS_GETTYPE(event.flags) == consoleDevice) {
      int i, c = IOEVENTFLAGS_GETCHARS(event.flags);
      jsiSetBusy(BUSY_INTERACTIVE, true);
      for (i=0;i<c;i++) jsiHandleChar(event.data.chars[i]);
      jsiSetBusy(BUSY_INTERACTIVE, false);
    }


    if (DEVICE_IS_USART(IOEVENTFLAGS_GETTYPE(event.flags))) {
      // ------------------------------------------------------------------------ SERIAL CALLBACK
      JsVarRef usartClass = getClassFromDevice(IOEVENTFLAGS_GETTYPE(event.flags));
      if (usartClass) {
        JsVar *callback = jsvFindChildFromString(usartClass, USART_CALLBACK_NAME, false);
        if (callback) {
          int i, c = IOEVENTFLAGS_GETCHARS(event.flags);
          for (i=0;i<c;i++) {
            JsVar *data = jsvNewWithFlags(JSV_OBJECT);
            if (data) {
              char buf[2];
              buf[0] = event.data.chars[i];
              buf[1] = 0;
              JsVar *dataTime = jsvNewFromString(buf);
              if (dataTime) jsvUnLock(jsvAddNamedChild(data, dataTime, "data"));
              jsvUnLock(dataTime);
            }
            jsiQueueEvents(jsvGetRef(callback), data);
            jsvUnLock(data);
          }
        }
        jsvUnLock(callback);
      }
    } else { // ---------------------------------------------------------------- PIN WATCH
      // we have an event... find out what it was for...
      JsVar *watchArrayPtr = jsvLock(watchArray);
      JsVarRef watch = watchArrayPtr->firstChild;
      while (watch) {
        JsVar *watchNamePtr = jsvLock(watch); // effectively the array index
        JsVar *watchPin = jsvSkipNameAndUnLock(jsvFindChildFromString(watchNamePtr->firstChild, "pin", false));
        int pin = jshGetPinFromVar(watchPin); // TODO: could be faster?
        jsvUnLock(watchPin);

        if (jshIsEventForPin(&event, pin)) {
          JsVar *watchCallback = jsvSkipNameAndUnLock(jsvFindChildFromString(watchNamePtr->firstChild, "callback", false));
          JsVar *watchRecurring = jsvSkipNameAndUnLock(jsvFindChildFromString(watchNamePtr->firstChild, "recur", false));
          JsVar *data = jsvNewWithFlags(JSV_OBJECT);
          if (data) {
            JsVar *dataTime = jsvNewFromFloat(jshGetMillisecondsFromTime(event.data.time)/1000);
            if (dataTime) jsvUnLock(jsvAddNamedChild(data, dataTime, "time"));
            jsvUnLock(dataTime);
          }
          jsiQueueEvents(jsvGetRef(watchCallback), data);
          jsvUnLock(data);
          if (!jsvGetBool(watchRecurring)) {
            // free all
            jsvRemoveChild(watchArrayPtr, watchNamePtr);
          }
          jsvUnLock(watchCallback);
          jsvUnLock(watchRecurring);
        }
        watch = watchNamePtr->nextSibling;
        jsvUnLock(watchNamePtr);
      }
      jsvUnLock(watchArrayPtr);
    }
  }

  // Check timers
  JsSysTime minTimeUntilNext = JSSYSTIME_MAX;
  JsSysTime time = jshGetSystemTime();

  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarRef timer = timerArrayPtr->firstChild;
  while (timer) {
    JsVar *timerNamePtr = jsvLock(timer);
    timer = timerNamePtr->nextSibling; // ptr to next
    JsVar *timerTime = jsvSkipNameAndUnLock(jsvFindChildFromString(timerNamePtr->firstChild, "time", false));
    JsSysTime timeUntilNext = jsvGetInteger(timerTime) - time;
    if (timeUntilNext < minTimeUntilNext)
      minTimeUntilNext = timeUntilNext;
    if (timerTime && timeUntilNext<=0) {
      loopsIdling = 0; // because we're not idling
      JsVar *timerCallback = jsvSkipNameAndUnLock(jsvFindChildFromString(timerNamePtr->firstChild, "callback", false));
      JsVar *timerRecurring = jsvSkipNameAndUnLock(jsvFindChildFromString(timerNamePtr->firstChild, "recur", false));
      JsVar *data = jsvNewWithFlags(JSV_OBJECT);
      if (data) {
        JsVar *dataTime = jsvNewFromFloat(jshGetMillisecondsFromTime(jsvGetInteger(timerTime))/1000);
        if (dataTime) jsvUnLock(jsvAddNamedChild(data, dataTime, "time"));
        jsvUnLock(dataTime);
      }
      jsiQueueEvents(jsvGetRef(timerCallback), data);
      jsvUnLock(data);
      if (jsvGetBool(timerRecurring)) {
        JsVarFloat interval = jsvGetFloatAndUnLock(jsvSkipNameAndUnLock(jsvFindChildFromString(timerNamePtr->firstChild, "interval", false)));
        if (interval<=0)
          jsvSetInteger(timerTime, time); // just set to current system time
        else
          jsvSetInteger(timerTime, jsvGetInteger(timerTime)+jshGetTimeFromMilliseconds(interval));
      } else {
        // free all
        jsvRemoveChild(timerArrayPtr, timerNamePtr);
      }
      jsvUnLock(timerCallback);
      jsvUnLock(timerRecurring);

    }
    jsvUnLock(timerTime);
    jsvUnLock(timerNamePtr);
  }
  jsvUnLock(timerArrayPtr);

  // Just in case we got any events to do and didn't clear loopsIdling before
  if (events) loopsIdling = 0;

  // TODO: could now sort events by time?
  // execute any outstanding events
  if (!jspIsInterrupted()) {
    jsiExecuteEvents();
  
    if (jspIsInterrupted()) {
      jsiConsolePrint("Execution Interrupted during event processing - clearing all timers.\r\n");
      JsVar *timerArrayPtr = jsvLock(timerArray);
      jsvRemoveAllChildren(timerArrayPtr);
      jsvUnLock(timerArrayPtr);
    }
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
      jsiSoftKill();
      jspSoftKill(&p);
      jsvSoftKill();
      jsvGarbageCollect(); // nice to have everything all tidy!
      jshSaveToFlash();
      jsvSoftInit();
      jspSoftInit(&p);
      jsiSoftInit();
    }
    if (todo & TODO_FLASH_LOAD) {
      todo &= (TODOFlags)~TODO_FLASH_LOAD;
      jsiSoftKill();
      jspSoftKill(&p);
      jsvSoftKill();
      jshLoadFromFlash();
      jsvSoftInit();
      jspSoftInit(&p);
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
#ifdef ARM
  if (loopsIdling>2 && // once around the idle loop without having done any work already (just in case)
#ifdef USB
      !jshIsUSBSERIALConnected() && // if USB is on, no point sleeping (later, sleep might be more drastic)
#endif
      !jshHasEvents() && //no events have arrived in the mean time
      !jshHasTransmitData() && //nothing left to send over serial?
      minTimeUntilNext > SYSTICK_RANGE*5/4) { // we are sure we won't miss anything - leave a little leeway (SysTick will wake us up!)
    jsiSetSleep(true);
    jshSleep();
    jsiSetSleep(false);
  }
#endif
}

void jsiLoop() {
  // idle stuff for hardware
  jshIdle();
  // Do general idle stuff
  jsiIdle();
  
  if (jspIsInterrupted()) {
    jspSetInterrupted(false);
    jsiConsoleRemoveInputLine();
    jsiConsolePrint("Execution Interrupted.\r\n");
    // clear input line
    jsvUnLock(inputLine);
    inputLine = jsvNewFromString("");
  }

  // return console (if it was gone!)
  jsiReturnInputLine();
}

/** Output extra functions defined in an object such that they can be copied to a new device */
void jsiDumpObjectState(JsVar *parentName, JsVar *parent) {
  JsVarRef childRef = parent->firstChild;
  while (childRef) {
    JsVar *child = jsvLock(childRef);
    JsVar *data = jsvSkipName(child);
    jsiConsolePrintStringVar(parentName);
    jsiConsolePrint(".");
    jsiConsolePrintStringVar(child);
    jsiConsolePrint(" = ");
    jsfPrintJSON(data);
    jsiConsolePrint(";\n");
    jsvUnLock(data);
    childRef = child->nextSibling;
    jsvUnLock(child);
  }
}

/** Output current interpreter state such that it can be copied to a new device */
void jsiDumpState() {
  JsVar *parent = jsvLock(p.root);
  JsVarRef childRef = parent->firstChild;
  jsvUnLock(parent);
  while (childRef) {
    JsVar *child = jsvLock(childRef);
    JsVar *data = jsvSkipName(child);
    if (jspIsCreatedObject(&p, data)) {
      jsiDumpObjectState(child, data);
    } else if (jsvIsStringEqual(child, JSI_TIMERS_NAME)) {
      // skip - done later
    } else if (jsvIsStringEqual(child, JSI_WATCHES_NAME)) {
      // skip - done later
    } else if (jsvIsStringEqual(child, JSI_HISTORY_NAME) ||
                jsvIsStringEqual(child, "USB") ||
                jsvIsStringEqual(child, "Serial1") ||
                jsvIsStringEqual(child, "Serial2")) {
      // skip - don't care about this stuff
    } else if (!jsvIsNative(data)) { // just a variable/function!
      jsiConsolePrint("var ");
      jsiConsolePrintStringVar(child);
      jsiConsolePrint(" = ");
      jsfPrintJSON(data);
      jsiConsolePrint(";\n");
    }
    jsvUnLock(data);
    childRef = child->nextSibling;
    jsvUnLock(child);
  }
  // Now do timers
  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarRef timerRef = timerArrayPtr->firstChild;
  jsvUnLock(timerArrayPtr);
  while (timerRef) {
    JsVar *timerNamePtr = jsvLock(timerRef);
    JsVar *timerCallback = jsvSkipOneNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "callback", false));
    bool recur = jsvGetBoolAndUnLock(jsvSkipNameAndUnLock(jsvFindChildFromString(timerNamePtr->firstChild, "recur", false)));
    JsVar *timerInterval = jsvSkipNameAndUnLock(jsvFindChildFromString(timerNamePtr->firstChild, "interval", false));
    jsiConsolePrint(recur ? "setInterval(" : "setTimeout(");
    jsfPrintJSON(timerCallback);
    jsiConsolePrint(", ");
    jsfPrintJSON(timerInterval);
    jsiConsolePrint(");\n");
    jsvUnLock(timerInterval);
    jsvUnLock(timerCallback);
    // next
    timerRef = timerNamePtr->nextSibling;
    jsvUnLock(timerNamePtr);
  }
  // Now do watches
  {
   JsVar *watchArrayPtr = jsvLock(watchArray);
   JsVarRef watchRef = watchArrayPtr->firstChild;
   jsvUnLock(watchArrayPtr);
   while (watchRef) {
     JsVar *watchNamePtr = jsvLock(watchRef);
     JsVar *watchCallback = jsvSkipOneNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "callback", false));
     JsVar *watchRecur = jsvSkipNameAndUnLock(jsvFindChildFromString(watchNamePtr->firstChild, "recur", false));
     JsVar *watchPin = jsvSkipNameAndUnLock(jsvFindChildFromString(watchNamePtr->firstChild, "pin", false));
     jsiConsolePrint("setWatch(");
     jsfPrintJSON(watchCallback);
     jsiConsolePrint(", ");
     jsfPrintJSON(watchPin);
     jsiConsolePrint(", ");
     jsfPrintJSON(watchRecur);
     jsiConsolePrint(");\n");
     jsvUnLock(watchPin);
     jsvUnLock(watchRecur);
     jsvUnLock(watchCallback);
     // next
     watchRef = watchNamePtr->nextSibling;
     jsvUnLock(watchNamePtr);
   }
  }
}

/** Handle function calls - do this programatically, so we can save on RAM */
JsVar *jsiHandleFunctionCall(JsExecInfo *execInfo, JsVar *a, const char *name) {
  if (a==0) { // ----------------------------------------   SYSTEM-WIDE
    // Handle pins - eg LED1 or D5
    int pin = jshGetPinFromString(name);
    if (pin>=0) {
      jspParseVariableName();
      return jsvNewFromInteger(pin);
    }
    // Special cases for we're just a basic function
    if (name[0]=='a') {
      if (strcmp(name,"analogRead")==0) {
        /*JS* function analogRead(pin)
         *JS*  Get the analog value of the given pin as a value between 0 and 1.
         *JS*  This is different to Arduino which only returns an integer between 0 and 1023
         *JS*  Pin can be an integer, or a string such as "A0","C13",etc
         *JS*  However only pins connected to an ADC will work (see the datasheet)
         */
        JsVar *pinVar = jspParseSingleFunction();
        int pin = jshGetPinFromVar(pinVar);
        jsvUnLock(pinVar);
        return jsvNewFromFloat(jshPinAnalog(pin));
      }
      if (strcmp(name,"analogWrite")==0) {
        /*JS* function analogWrite(pin, value)
         *JS*  Set the analog value of the given pin. Value is between 0 and 1
         *JS*  Analog values are output as PWM digital. If the pin is not capable a warning will be output
         *JS*  Pin can be an integer, or a string such as "A0","C13",etc
         */
        JsVar *pinVar, *valueVar;
        jspParseFunction(0, &pinVar, &valueVar, 0, 0);
        int pin = jshGetPinFromVar(pinVar);
        jsvUnLock(pinVar);
        JsVarFloat value = jsvGetFloat(valueVar);
        jsvUnLock(valueVar);
        jshPinAnalogOutput(pin, value);
        return 0;
      }
    } else if (name[0]=='b') {
      if (strcmp(name,"bitRead")==0) {
        /*JS* function bitRead(value, bitnum)
         *JS*  Get the specified bit from the value. Lowest significance bit is 0
         */
        JsVar *valueVar, *bitVar;
        jspParseFunction(0, &valueVar, &bitVar, 0, 0);
        JsVarInt value = jsvGetIntegerAndUnLock(valueVar);
        JsVarInt bit = jsvGetIntegerAndUnLock(bitVar);
        return jsvNewFromInteger( (value >> bit) & 1);
      }
      if (strcmp(name,"bitWrite")==0) {
        /*JS* function bitWrite(value, bitnum, bitdata)
         *JS*  Write the specified bit from the value. Lowest significance bit is 0
         */
        JsVar *valueVar, *bitVar, *dataVar;
        jspParseFunction(0,&valueVar, &bitVar, &dataVar, 0);
        JsVarInt value = jsvGetInteger(valueVar);
        JsVarInt bit = jsvGetIntegerAndUnLock(bitVar);
        JsVarInt data = jsvGetIntegerAndUnLock(dataVar);
        if (jsvIsNumeric(valueVar)) jsvSetInteger(valueVar, (value & ~(1<<bit)) | ((data?1:0)<<bit));
        jsvUnLock(valueVar);
        return 0;
      }
      if (strcmp(name,"bitSet")==0) {
        /*JS* function bitSet(value, bitnum)
         *JS*  Set the given bit in the value. Lowest significance bit is 0
         */
        JsVar *valueVar, *bitVar;
        jspParseFunction(0, &valueVar, &bitVar, 0, 0);
        JsVarInt value = jsvGetInteger(valueVar);
        JsVarInt bit = jsvGetIntegerAndUnLock(bitVar);
        if (jsvIsNumeric(valueVar)) jsvSetInteger(valueVar, value | (1<<bit));
        jsvUnLock(valueVar);
        return 0;
      }
      if (strcmp(name,"bitClear")==0) {
        /*JS* function bitClear(value, bitnum)
         *JS*  Clear the given bit in the value. Lowest significance bit is 0
         */
        JsVar *valueVar, *bitVar;
        jspParseFunction(0, &valueVar, &bitVar, 0, 0);
        JsVarInt value = jsvGetInteger(valueVar);
        JsVarInt bit = jsvGetIntegerAndUnLock(bitVar);
        if (jsvIsNumeric(valueVar)) jsvSetInteger(valueVar, value & ~(1<<bit));
        jsvUnLock(valueVar);
        return 0;
      }
      if (strcmp(name,"bit")==0) {
        /*JS* function bit(bitnum)
         *JS*  Get the value of the specified bit (0->1, 1->2, 2->4, 3->8 etc). Lowest significance bit is 0
         */
        JsVar *bitVar = jspParseSingleFunction();
        JsVarInt bit = jsvGetIntegerAndUnLock(bitVar);
        return jsvNewFromInteger(1 << bit);
      }
    } else if (name[0]=='c') {
      if (strcmp(name,"changeInterval")==0) {
        /*JS* function changeInterval(id)
         *JS*  Change the Interval on a callback created with setInterval, for example:
         *JS*   var id = setInterval(function () { print("foo"); }, 1000); <-- every second
         *JS*   changeInterval(id, 1500); <-- not runs every 1.5 seconds
         *JS*   This takes effect the text time the callback is called (so it is not immediate).
         */
        JsVar *idVar, *intervalVal;
        jspParseFunction(0, &idVar, &intervalVal, 0, 0);
        JsVarFloat interval = jsvGetFloatAndUnLock(intervalVal);
        if (interval<TIMER_MIN_INTERVAL) interval=TIMER_MIN_INTERVAL;
        JsVar *timerName = jsvFindChildFromVar(timerArray, idVar, false);
        jsvUnLock(idVar);

        if (timerName) {
          JsVar *timer = jsvSkipNameAndUnLock(timerName);
          JsVar *v = jsvNewFromFloat(interval);
          jsvUnLock(jsvSetNamedChild(timer, v, "interval"));
          jsvUnLock(v);
          jsvUnLock(timer);
          // timerName already unlocked
        } else {
          jsError("Unknown Interval");
        }
        return 0;
      }
      if (strcmp(name,"clearTimeout")==0 || strcmp(name,"clearInterval")==0) {
        /*JS* function clearTimeout(id)
         *JS*  Clear the Timeout that was created with setTimeout, for example:
         *JS*   var id = setTimeout(function () { print("foo"); }, 1000);
         *JS*   clearTimeout(id);
         */
        /*JS* function clearInterval(id)
         *JS*  Clear the Interval that was created with setInterval, for example:
         *JS*   var id = setInterval(function () { print("foo"); }, 1000);
         *JS*   clearInterval(id);
         */
        JsVar *idVar = jspParseSingleFunction();
        JsVar *child = jsvFindChildFromVar(timerArray, idVar, false);
        jsvUnLock(idVar);

        if (child) {
          JsVar *timerArrayPtr = jsvLock(timerArray);
          jsvRemoveChild(timerArrayPtr, child);
          jsvUnLock(child);
          jsvUnLock(timerArrayPtr);
        } else {
          jsError("Unknown Timer or Interval");
        }
        return 0;
      }
      if (strcmp(name,"clearWatch")==0) {
        /*JS* function clearWatch(id)
         *JS*  Clear the Watch that was created with setWatch.
         */
        JsVar *idVar = jspParseSingleFunction();
        JsVar *watchNamePtr = jsvFindChildFromVar(watchArray, idVar, false);
        jsvUnLock(idVar);

        if (watchNamePtr) { // child is a 'name'
          JsVar *pinVar = jsvSkipNameAndUnLock(jsvFindChildFromString(watchNamePtr->firstChild, "pin", false));
          jshPinWatch(jshGetPinFromVar(pinVar), false);
          jsvUnLock(pinVar);


          JsVar *watchArrayPtr = jsvLock(watchArray);
          jsvRemoveChild(watchArrayPtr, watchNamePtr);
          jsvUnLock(watchNamePtr);
          jsvUnLock(watchArrayPtr);
        } else {
          jsError("Unknown Watch");
        }
        return 0;
      }
    } else if (name[0]=='d') {
      if (strcmp(name,"digitalPulse")==0) {
        /*JS* function digitalPulse(pin,value,time)
         *JS*  Pulse the pin with the value for the given time in milliseconds
         *JS*  eg. pulse("A0",1,500); pulses A0 high for 500ms
         *JS*  Pin can be an integer, or a string such as "A0","C13",etc
         */
        JsVar *pinVar, *valueVar, *timeVar;
        jspParseFunction(0, &pinVar, &valueVar, &timeVar, 0);
        int pin = jshGetPinFromVar(pinVar);
        jsvUnLock(pinVar);
        bool value = jsvGetBool(valueVar);
        jsvUnLock(valueVar);
        JsVarFloat time = jsvGetFloatAndUnLock(timeVar);
        if (time<0) { 
          jsWarn("Pulse Time given for digitalPulse is less that or equal to 0");
        } else {
          //jsPrintInt((JsVarInt)(time*1000));
          jshPinPulse(pin, value, time);
        }
        return 0;
      }
      if (strcmp(name,"digitalRead")==0) {
        /*JS* function digitalRead(pin)
         *JS*  Get the digital value of the given pin.
         *JS*  Pin can be an integer, or a string such as "A0","C13",etc.
         *JS*  If pin is an array of pins, eg. ["A2","A1","A0"] in which case an integer representing that value will be returned (first array element is the MSB).
         */
        JsVar *pinVar = jspParseSingleFunction();
        if (jsvIsArray(pinVar)) {
          int pins = 0;
          JsVarInt value = 0;
          JsVarRef pinName = pinVar->firstChild;
          while (pinName) {
            JsVar *pinNamePtr = jsvLock(pinName);
            JsVar *pinPtr = jsvSkipName(pinNamePtr);
            value = (value<<1) | jshPinInput(jshGetPinFromVar(pinPtr));
            jsvUnLock(pinPtr);
            pinName = pinNamePtr->nextSibling;
            jsvUnLock(pinNamePtr);
            pins++;
          }
          jsvUnLock(pinVar);
          if (pins==0) return 0; // return undefined if array empty
          return jsvNewFromInteger(value);
        } else {
          int pin = jshGetPinFromVar(pinVar);
          jsvUnLock(pinVar);
          return jsvNewFromBool(jshPinInput(pin));
        }
      }
      if (strcmp(name,"digitalWrite")==0) {
        /*JS* function digitalWrite(pin, value)
         *JS*  Set the digital value of the given pin.
         *JS*  Pin can be an integer, or a string such as "A0","C13",etc
         *JS*  If pin is an array of pins, eg. ["A2","A1","A0"] in which case value will be treated as an integer where the first array element is the MSB
         */
        JsVar *pinVar, *valueVar;
        jspParseFunction(0, &pinVar, &valueVar, 0, 0);
        if (jsvIsArray(pinVar)) {
          JsVarInt value = jsvGetInteger(valueVar);
          jsvUnLock(valueVar);
          JsVarRef pinName = pinVar->lastChild; // NOTE: start at end and work back!
          while (pinName) {
            JsVar *pinNamePtr = jsvLock(pinName);
            JsVar *pinPtr = jsvSkipName(pinNamePtr);
            jshPinOutput(jshGetPinFromVar(pinPtr), value&1);
            jsvUnLock(pinPtr);
            pinName = pinNamePtr->prevSibling;
            jsvUnLock(pinNamePtr);
            value = value>>1; // next bit down
          }
          jsvUnLock(pinVar);
          return 0;
        } else {
          int pin = jshGetPinFromVar(pinVar);
          jsvUnLock(pinVar);
          bool value = jsvGetBool(valueVar);
          jsvUnLock(valueVar);
          jshPinOutput(pin, value);
          return 0;
        }
      }
    } else if (name[0]=='s') {
      if (strcmp(name,"setTimeout")==0 || strcmp(name,"setInterval")==0) {
        /*JS* function setTimeout(function, timeout)
         *JS*  Call the function specified ONCE after the timeout in milliseconds.
         *JS*  The function may also take an argument, which is an object containing a field called 'time', which is the time in seconds at which the timer happened
         *JS*  This can also be removed using clearTimeout
         */
        /*JS* function setInterval(function, timeout)
         *JS*  Call the function specified REPEATEDLY after the timeout in milliseconds.
         *JS*  The function may also take an argument, which is an object containing a field called 'time', which is the time in seconds at which the timer happened
         *JS*  This can also be removed using clearInterval
         */
        bool recurring = strcmp(name,"setInterval")==0;
        JsVar *func, *timeout;
        jspParseFunction(JSP_NOSKIP_A, &func, &timeout, 0, 0);
        JsVar *skippedFunc = jsvSkipName(func);
        JsVar *itemIndex = 0;
        if (!jsvIsFunction(skippedFunc) && !jsvIsString(skippedFunc)) {
          jsError("Function or String not supplied!");
        } else {
          // Create a new timer
          JsVar *timerPtr = jsvNewWithFlags(JSV_OBJECT);
          JsVarFloat interval = jsvGetFloat(timeout);
          if (interval<TIMER_MIN_INTERVAL) interval=TIMER_MIN_INTERVAL;
          JsVar *v;
          v = jsvNewFromInteger(jshGetSystemTime() + jshGetTimeFromMilliseconds(interval));
          jsvUnLock(jsvAddNamedChild(timerPtr, v, "time"));
          jsvUnLock(v);
          v = jsvNewFromFloat(interval);
          jsvUnLock(jsvAddNamedChild(timerPtr, v, "interval"));
          jsvUnLock(v);
          v = jsvNewFromBool(recurring);
          jsvUnLock(jsvAddNamedChild(timerPtr, v, "recur"));
          jsvUnLock(v);
          jsvUnLock(jsvAddNamedChild(timerPtr, func, "callback"));
          //jsPrint("TIMER BEFORE ADD\n"); jsvTrace(timerArray,5);
          JsVar *timerArrayPtr = jsvLock(timerArray);
          itemIndex = jsvNewFromInteger(jsvArrayPush(timerArrayPtr, timerPtr) - 1);
          //jsPrint("TIMER AFTER ADD\n"); jsvTrace(timerArray,5);
          jsvUnLock(timerArrayPtr);
          jsvUnLock(timerPtr);
        }
        jsvUnLock(skippedFunc);
        jsvUnLock(func);
        jsvUnLock(timeout);
        //jsvTrace(jsiGetParser()->root, 0);
        return itemIndex;
      }
      if (strcmp(name,"setWatch")==0) {
        /*JS* function setWatch(function, pin, repeat)
         *JS*  Call the function specified ONCE (if repeat==false or undefined) or
         *JS*  REPEATEDLY if (repeat==true) when the pin changes
         *JS*  The function may also take an argument, which is an object containing a field called 'time', which is the time in seconds at which the pin changed state
         *JS*  This can also be removed using clearWatch
         */
        JsVarInt itemIndex = -1;
        JsVar *funcVar, *pinVar, *recurringVar;
        jspParseFunction(0, &funcVar, &pinVar, &recurringVar, 0);
        JsVar *skippedFunc = jsvSkipName(funcVar);
        if (!jsvIsFunction(skippedFunc) && !jsvIsString(skippedFunc)) {
          jsError("Function or String not supplied!");
        } else {
          int pin = jshGetPinFromVar(pinVar);

          // Create a new watch
          JsVar *watchPtr = jsvNewWithFlags(JSV_OBJECT);
          JsVar *v;
          v = jsvNewFromInteger(pin);
          jsvUnLock(jsvAddNamedChild(watchPtr, v, "pin"));
          jsvUnLock(v);
          v = jsvNewFromBool(jsvGetBool(recurringVar));
          jsvUnLock(jsvAddNamedChild(watchPtr, v, "recur"));
          jsvUnLock(v);
          jsvUnLock(jsvAddNamedChild(watchPtr, funcVar, "callback"));
          JsVar *watchArrayPtr = jsvLock(watchArray);
          itemIndex = jsvArrayPush(watchArrayPtr, watchPtr) - 1;
          jsvUnLock(watchArrayPtr);
          jsvUnLock(watchPtr);
          jshPinWatch(pin, true);
        }
        jsvUnLock(skippedFunc);
        jsvUnLock(funcVar);
        jsvUnLock(pinVar);
        jsvUnLock(recurringVar);
        //jsvTrace(jsiGetParser()->root, 0);

        return (itemIndex>=0) ? jsvNewFromInteger(itemIndex) : 0/*undefined*/;
      }
    } else {
      if (strcmp(name,"highByte")==0) {
        /*JS* function highByte(value)
         *JS*  Return the high (second) byte of the value
         */
        JsVar *valueVar = jspParseSingleFunction();
        JsVarInt value = jsvGetIntegerAndUnLock(valueVar);
        return jsvNewFromInteger((value>>8) & 0xFF);
      }
      if (strcmp(name,"lowByte")==0) {
        /*JS* function lowByte(value)
         *JS*  Return the low byte of the value
         */
        JsVar *valueVar = jspParseSingleFunction();
        JsVarInt value = jsvGetIntegerAndUnLock(valueVar);
        return jsvNewFromInteger(value & 0xFF);
      }
      if (strcmp(name,"print")==0) {
        /*JS* function print(text)
         *JS*  Print the supplied string
         */
        JsVar *v = jsvAsString(jspParseSingleFunction(), true);
        jsiConsoleRemoveInputLine();
        jsiConsolePrintStringVar(v);
        jsvUnLock(v);
        jsiConsolePrint("\n");
        return 0;
      }
    }

    if (strcmp(name,"dump")==0) {
      /*JS* function dump()
       *JS*  Output current interpreter state such that it can be copied to a new device
       */
      jspParseEmptyFunction();
      jsiDumpState();
      return 0;
    }
    if (strcmp(name,"echo")==0) {
      /*JS* function echo(yesorno)
       *JS*  Should TinyJS echo what you type back to you? true = yes (Default), false = no.
       *JS*  When echo is off, the result of executing a command is not returned.
       *JS*  Instead, you must use 'print' to send output.
       */
      bool b = jsvGetBoolAndUnLock(jspParseSingleFunction());
      echo = b;
      return 0;
    }

    if (strcmp(name,"getTime")==0) {
      /*JS* function getTime()
       *JS*  Return the current system time in Seconds (as a floating point number)
       */
      jspParseEmptyFunction();
      return jsvNewFromFloat((JsVarFloat)jshGetSystemTime() / (JsVarFloat)jshGetTimeFromMilliseconds(1000));
    }
    if (strcmp(name,"load")==0) {
      /*JS* function load()
       *JS*  Load program memory out of flash 
       */
      jspParseEmptyFunction();
      todo |= TODO_FLASH_LOAD;
      return 0;
    }
    if (strcmp(name,"save")==0) {
      /*JS* function save()
       *JS*  Save program memory into flash 
       */
      jspParseEmptyFunction();
      todo |= TODO_FLASH_SAVE;
      return 0;
    }
    if (strcmp(name,"reset")==0) {
      /*JS* function reset()
       *JS*  Reset everything - clear program memory
       */
      jspParseEmptyFunction();
      todo |= TODO_RESET;
      return 0;
    }
    if (strcmp(name,"edit")==0) {
      /*JS* function edit(functionName/"functionName")
       *JS*  Fill the console with the contents of the given function, so you can edit it.
       */
      JsVar *funcName = 0;
      jspParseFunction(JSP_NOSKIP_A, &funcName,0,0,0);
      if (jsvIsString(funcName)) {
        JsVar *func = 0;
        if (jsvIsName(funcName))
          func = jsvSkipName(funcName);
        else
          func = jsvSkipNameAndUnLock(jsvFindChildFromVar(p.root, funcName, 0));
        if (jsvIsFunction(func)) {
          JsVar *newLine = jsvNewFromString("");
          if (newLine) {
            jsvAppendStringVarComplete(newLine, funcName);
            jsvAppendString(newLine, " = ");
            JsVar *funcData = jsvAsString(func, false);
            if (funcData)
              jsvAppendStringVarComplete(newLine, funcData);
            jsvUnLock(funcData);
            jsvAppendString(newLine, ";");
            jsiReplaceInputLine(newLine);
            jsvUnLock(newLine);
          }
        } else
          jsError("Edit should be called with the name of a function");
        jsvUnLock(func);
      } else {
        jsError("Edit should be called with edit(funcName) or edit('funcName')");
      }
      jsvUnLock(funcName);
      return 0;
    }
    if (strcmp(name,"setBusyIndicator")==0) {
      /*JS* function setBusyIndicator(pin)
       *JS*  When Espruino is busy, set the pin specified here
       *JS*  high. Set this to undefined to disable the feature.
       */
      JsVar *pinVar = jspParseSingleFunction();
      int oldPin = pinBusyIndicator;
      if (jsvIsUndefined(pinVar)) {
        pinBusyIndicator = -1;
      } else {
        pinBusyIndicator = jshGetPinFromVar(pinVar);
        if (pinBusyIndicator<0)
          jsError("Invalid pin!");
      }
      jsvUnLock(pinVar);
      // we should be busy right now anyway, so set stuff up right
      if (pinBusyIndicator!=oldPin) {
        if (oldPin>=0) jshPinOutput(oldPin, 0);
        jshPinOutput(pinBusyIndicator, 1);
      }
      return 0; // handled
    }
    if (strcmp(name,"setSleepIndicator")==0) {
      /*JS* function setSleepIndicator(pin)
       *JS*  When Espruino is asleep, set the pin specified here
       *JS*  high. Set this to undefined to disable the feature.
       */
      JsVar *pinVar = jspParseSingleFunction();
      if (jsvIsUndefined(pinVar)) {
        pinSleepIndicator = -1;
      } else {
        pinSleepIndicator = jshGetPinFromVar(pinVar);
        if (pinSleepIndicator<0)
          jsError("Invalid pin!");
      }
      jsvUnLock(pinVar);
      return 0; // handled
    }
    if (strcmp(name,"trace")==0) {
      /*JS* function trace()
       *JS*  Output debugging information 
       */
      jspParseEmptyFunction();
      jsvTrace(p.root, 0);
      return 0;
    }
    if (strcmp(name,"dotty")==0) {
      /*JS* function trace()
       *JS*  Output debugging information
       */
      jspParseEmptyFunction();
      jsvDottyOutput();
      return 0;
    }
    // ----------------------------------------   END OF SYSTEM-WIDE
  } else if (isUSART(jsvGetRef(a))) { // ----------------------------------------   USART
    IOEventFlags device = getDeviceFromClass(jsvGetRef(a));
    if (strcmp(name,"setConsole")==0) {
      /*JS* [Serial1|Serial2|USB].setConsole()
       *JS*  Set this Serial port as the port for the console
       */
      jspParseEmptyFunction();
      jsiSetConsoleDevice(device);
      return 0;
    }
    if (strcmp(name,"setup")==0) {
      /*JS* [Serial1|Serial2].setup(baudrate)
       *JS*  Set this Serial port up on the default bins, with the given baud rate. For example Serial1.setup(9600)
       */
      JsVarInt baud = jsvGetIntegerAndUnLock(jspParseSingleFunction());
      if (baud<=0) baud = DEFAULT_BAUD_RATE;
      jshUSARTSetup(device, (int)baud);
      // Set baud rate in object, so we can initialise it on startup
      JsVar *baudVar = jsvNewFromInteger(baud);
      jsvUnLock(jsvSetNamedChild(a, baudVar, USART_BAUDRATE_NAME));
      jsvUnLock(baudVar);
      return 0;
    }
    if (strcmp(name,"print")==0 || strcmp(name,"println")==0) {
      /*JS* [Serial1|Serial2|USB].print(string)
       *JS*  Print a string to the serial port - without a line feed
       */
      /*JS* [Serial1|Serial2|USB].println(string)
       *JS*  Print a line to the serial port (newline character sent are '\r\n')
       */
      bool newLine = strcmp(name,"println")==0;
      JsVar *str = jsvAsString(jspParseSingleFunction(), true);
      jsiTransmitStringVar(device,str);
      jsvUnLock(str);
      if (newLine) {
        jshTransmit(device, (unsigned char)'\r');
        jshTransmit(device, (unsigned char)'\n');
      }
      return 0;
    }
    if (strcmp(name,"onData")==0) {
      /*JS* [Serial1|Serial2|USB].onData(function(e) {..})
       *JS*  When a character is received on this serial port, the function supplied to onData
       *JS*  gets called. The character received is in 'e.data' as a String.
       *JS*  Only one function can ever be supplied, so calling onData(undefined) will stop any function being called
       */
      JsVar *funcVar = jspParseSingleFunction();
      if (!jsvIsFunction(funcVar) && !jsvIsString(funcVar)) {
        jsError("Function or String not supplied!");
      } else {
        jsvUnLock(jsvSetNamedChild(a, funcVar, USART_CALLBACK_NAME));
      }
      jsvUnLock(funcVar);
      return 0;
    }
    // ----------------------------------------   END OF USART
  }
  // unhandled
  return JSFHANDLEFUNCTIONCALL_UNHANDLED;
}

JsParse *jsiGetParser() {
  return &p;
}
