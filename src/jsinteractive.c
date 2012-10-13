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
 IS_HAD_27_91,
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
bool echo;                  ///< do we provide any user feedback?
// ----------------------------------------------------------------------------
JsParse p; ///< The parser we're using for interactiveness
JsVar *inputline = 0; ///< The current input line
InputState inputState = 0; ///< state for dealing with cursor keys
JsVar *jsiHandleFunctionCall(JsExecInfo *execInfo, JsVar *a, const char *name); // forward decl
// ----------------------------------------------------------------------------

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

/// Change the console to a new location
void jsiSetConsoleDevice(IOEventFlags device) {
  if (device == consoleDevice) return;
  if (echo) {
    jsiConsolePrint("Console Moved to ");
    jsiConsolePrint(jshGetDeviceString(device));
    jsiConsolePrint("\n");
  }
  IOEventFlags oldDevice = consoleDevice;
  consoleDevice = device;
  if (echo) {
    jsiConsolePrint("Console Moved from ");
    jsiConsolePrint(jshGetDeviceString(oldDevice));
    jsiConsolePrint("\n>");
  }
}

void jsiConsolePrintChar(char data) {
  jshTransmit(consoleDevice, (unsigned char)data);
}

void jsiConsolePrint(const char *str) {
  while (*str) {
       if (*str == '\n') jsiConsolePrintChar('\r');
       jsiConsolePrintChar(*(str++));
  }
}

void jsiConsolePrintInt(int d) {
    char buf[32];
    itoa(d, buf, 10);
    jsiConsolePrint(buf);
}

/// Print the contents of a string var - directly
void jsiConsolePrintStringVar(JsVar *v) {
  assert(jsvIsString(v) || jsvIsName(v));
  JsVarRef r = jsvGetRef(v);
  while (r) {
    v = jsvLock(r);
    size_t l = jsvGetMaxCharactersInVar(v);
    char buf[JSVAR_DATA_STRING_MAX_LEN+1];
    memcpy(buf, v->varData.str, l);
    buf[l] = 0;
    jsiConsolePrint(buf);
    r = v->lastChild;
    jsvUnLock(v);
  }
}

void jsiConsolePrintPosition(struct JsLex *lex, int tokenPos) {
  int line,col;
  jslGetLineAndCol(lex, tokenPos, &line, &col);
  jsiConsolePrint("line ");
  jsiConsolePrintInt(line);
  jsiConsolePrint(" col ");
  jsiConsolePrintInt(col);
  jsiConsolePrint(" (char ");
  jsiConsolePrintInt(tokenPos);
  jsiConsolePrint(")\n");
}

/// Print the contents of a string var - directly
void jsiTransmitStringVar(IOEventFlags device, JsVar *v) {
  assert(jsvIsString(v) || jsvIsName(v));
  JsVarRef r = jsvGetRef(v);
  while (r) {
    v = jsvLock(r);
    size_t l = jsvGetMaxCharactersInVar(v);
    size_t i;
    for (i=0;i<l;i++)
      jshTransmit(device, (unsigned char)v->varData.str[i]);
    r = v->lastChild;
    jsvUnLock(v);
  }
}

void jsiSetBusy(bool isBusy) {
  if (pinBusyIndicator >= 0)
    jshPinOutput(pinBusyIndicator, isBusy);
}

JsVarRef _jsiInitSerialClass(IOEventFlags device, const char *serialName) {
  JsVarRef class = 0;
  JsVar *name = jsvFindChildFromString(p.root, serialName, true);
  if (name) {
    name->flags |= JSV_NATIVE;
    if (!name->firstChild) name->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
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

// Used when recovering after being flashed
// 'claim' anything we are using
void jsiSoftInit() {
  events = 0;
  inputline = jsvNewFromString("");

  // Load inbuild Classes
  classUSART1 = _jsiInitSerialClass(EV_USART1, "Serial1");
  classUSART2 = _jsiInitSerialClass(EV_USART2, "Serial2");
#ifdef USB
  classUSB = _jsiInitSerialClass(EV_USBSERIAL, "USB");
#endif

  // Load timer/watch arrays
  JsVar *timersName = jsvFindChildFromString(p.root, JSI_TIMERS_NAME, true);
  if (!timersName->firstChild)
    timersName->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_ARRAY)));
  timerArray = jsvRefRef(timersName->firstChild);
  jsvUnLock(timersName);

  JsVar *watchName = jsvFindChildFromString(p.root, JSI_WATCHES_NAME, true);
  if (!watchName->firstChild)
    watchName->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_ARRAY)));
  watchArray = jsvRefRef(watchName->firstChild);
  jsvUnLock(watchName);

  // Check any existing watches and set up interrupts for them
  JsVar *watchArrayPtr = jsvLock(watchArray);
  JsVarRef watch = watchArrayPtr->firstChild;
  while (watch) {
    JsVar *watchNamePtr = jsvLock(watch);
    JsVar *watchPin = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "pin", false));
    jshPinWatch(jshGetPinFromVar(watchPin), true);
    jsvUnLock(watchPin);
    watch = watchNamePtr->nextSibling;
    jsvUnLock(watchNamePtr);
  }
  jsvUnLock(watchArrayPtr);

  // Check any existing timers and try and set time correctly
  JsSysTime currentTime = jshGetSystemTime();
  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarRef timer = timerArrayPtr->firstChild;
  while (timer) {
    JsVar *timerNamePtr = jsvLock(timer);
    JsVar *timerTime = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "time", false));
    JsVarFloat interval = jsvGetDoubleAndUnLock(jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "interval", false)));
    jsvSetInteger(timerTime, currentTime + jshGetTimeFromMilliseconds(interval));
    jsvUnLock(timerTime);
    timer = timerNamePtr->nextSibling;
    jsvUnLock(timerNamePtr);
  }
  jsvUnLock(timerArrayPtr);

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
  jsvUnLock(inputline);
  inputline=0;

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
      JsVar *watchPin = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "pin", false));
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
  if (!initName->firstChild)
    initName->firstChild = jsvUnLock(jsvRef(jsvNewFromString("")));
  JsVar *initCode = jsvLock(initName->firstChild);
  if (!echo) jsvAppendString(initCode, "echo(0);");
  if (pinBusyIndicator != DEFAULT_BUSY_PIN_INDICATOR) {
    jsvAppendString(initCode, "setBusyIndicator(");
    jsvAppendInteger(initCode, pinBusyIndicator);
    jsvAppendString(initCode, ");");
  }
  jsvUnLock(initCode);
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

  if (echo) {
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

  JsVarRef r = jsvGetRef(inputline);
  while (r) {
    JsVar *v = jsvLock(r);
    size_t l = jsvGetMaxCharactersInVar(v);
    size_t i;
    for (i=0;i<l;i++) { 
      char ch = v->varData.str[i];
      if (ch=='{') brackets++;
      if (ch=='}') brackets--;
    }
    r = v->lastChild;
    jsvUnLock(v);
  }

  return brackets;
} 

void jsiHandleChar(char ch) {
  //jsPrint("  ["); jsPrintInt(ch); jsPrint("]  \n");
  //
  // special stuff
  // 27 then 91 then 68 - left
  // 27 then 91 then 67 - right
  // 27 then 91 then 65 - up
  // 27 then 91 then 66 - down
  // 27 then 91 then 51 then 126 - backwards delete
  // 27 then 91 then 49 then 126 - home
  // 27 then 79 then 70 - end

  if (inputState==IS_NONE && ch == 27) {
    inputState = IS_HAD_27;
  } else if (inputState==IS_HAD_27 && ch == 91) {
    inputState = IS_HAD_27_91;
  } else if (inputState==IS_HAD_27_91) {
    inputState = IS_NONE;
    if (ch==68) { // left
    }
    if (ch==67) { // right
      //jsiConsolePrintChar(27);jsiConsolePrintChar(91);jsiConsolePrintChar(67);
    }
    if (ch==65) { // up
    }
    if (ch==66) { // down
    }
  } else {
    inputState = IS_NONE;
    if (ch == 0x08 || ch == 0x7F /*delete*/) {
      int l = (int)jsvGetStringLength(inputline);
      if (l>0 && jsvGetCharInString(inputline,l-1)!='\n') { // not empty, or not new line
        // clear the character
        if (echo) {
          jsiConsolePrintChar(CHAR_DELETE_SEND);
          jsiConsolePrintChar(' ');
          jsiConsolePrintChar(CHAR_DELETE_SEND);
        }
        // FIXME hacky - should be able to just remove the end character without copying
        JsVar *v = jsvNewFromString("");
        if (l>1) jsvAppendStringVar(v, inputline, 0, (int)(l-1));
        jsvUnLock(inputline);
        inputline=v;
      } else {
        // no characters, don't allow delete
      }
    } else if (ch == '\n' && inputState == IS_HAD_R) {
      inputState = IS_NONE; //  ignore \ r\n - we already handled it all on \r
    } else if (ch == '\r' || ch == '\n') { // TODO: double newlines if \r\n?
      if (ch == '\r') inputState = IS_HAD_R;
      if (jsiCountBracketsInInput()<=0) {
        if (echo) {
          jsiConsolePrintChar('\r');
          jsiConsolePrintChar('\n');
        }

        JsVar *v = jspEvaluateVar(&p, inputline);
        jsvUnLock(inputline);

        if (echo) {
          jsiConsolePrintChar('=');
          jsfPrintJSON(v);
        }
        jsvUnLock(v);

        inputline = jsvNewFromString("");

        if (echo) jsiConsolePrint("\r\n>");
      } else {
        if (echo) jsiConsolePrint("\n:");
        jsvAppendCharacter(inputline, '\n');
      }
    } else {
      if (echo) jsiConsolePrintChar(ch);
      // Append the character to our input line
      jsvAppendCharacter(inputline, ch);
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
  if (hasEvents) jsiSetBusy(true);
  while (events) {
    JsVar *event = jsvLock(events);
    // Get function to execute
    JsVar *func = jsvSkipNameAndUnlock(jsvFindChildFromString(jsvGetRef(event), "func", false));
    JsVar *arg0 = jsvSkipNameAndUnlock(jsvFindChildFromString(jsvGetRef(event), "arg0", false));
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
  if (hasEvents) jsiSetBusy(false);
}

bool jsiHasTimers() {
  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarInt c = jsvGetArrayLength(timerArrayPtr);
  jsvUnLock(timerArrayPtr);
  return c>0;
}

void jsiIdle() {
  // Handle hardware-related idle stuff (like checking for pin events)
  IOEvent event;
  while (jshPopIOEvent(&event)) {
    if (IOEVENTFLAGS_GETTYPE(event.flags) == consoleDevice) {
      int i, c = IOEVENTFLAGS_GETCHARS(event.flags);
      jsiSetBusy(true);
      for (i=0;i<c;i++) jsiHandleChar(event.data.chars[i]);
      jsiSetBusy(false);
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
        JsVar *watchPin = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "pin", false));
        int pin = jshGetPinFromVar(watchPin); // TODO: could be faster?
        jsvUnLock(watchPin);

        if (jshIsEventForPin(&event, pin)) {
          JsVar *watchCallback = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "callback", false));
          JsVar *watchRecurring = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "recur", false));
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
  JsSysTime time = jshGetSystemTime();

  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarRef timer = timerArrayPtr->firstChild;
  while (timer) {
    JsVar *timerNamePtr = jsvLock(timer);
    timer = timerNamePtr->nextSibling; // ptr to next
    JsVar *timerTime = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "time", false));
    if (timerTime && time > jsvGetInteger(timerTime)) {
      JsVar *timerCallback = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "callback", false));
      JsVar *timerRecurring = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "recur", false));
      JsVar *data = jsvNewWithFlags(JSV_OBJECT);
      if (data) {
        JsVar *dataTime = jsvNewFromFloat(jshGetMillisecondsFromTime(jsvGetInteger(timerTime))/1000);
        if (dataTime) jsvUnLock(jsvAddNamedChild(data, dataTime, "time"));
        jsvUnLock(dataTime);
      }
      jsiQueueEvents(jsvGetRef(timerCallback), data);
      jsvUnLock(data);
      if (jsvGetBool(timerRecurring)) {
        JsVarFloat interval = jsvGetDoubleAndUnLock(jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "interval", false)));
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
  }
}

void jsiLoop() {
  // idle stuff for hardware
  jshIdle();
  // Do general idle stuff
  jsiIdle();
  
  if (jspIsInterrupted()) {
    jspSetInterrupted(false);
    jsiConsolePrint("Execution Interrupted.\r\n");
    if (echo) jsiConsolePrint(">");
    // clear input line
    jsvUnLock(inputline);
    inputline = jsvNewFromString("");
  }
}

/// Tries to get rid of some memory (by clearing command history). Returns true if it got rid of something, false if it didn't.
bool jsiFreeMoreMemory() {
  JsVar *history = jsvSkipNameAndUnlock(jsvFindChildFromString(p.root, JSI_HISTORY_NAME, false));
  if (!history) return 0;
  JsVar *item = jsvArrayPopFirst(history);
  bool freed = item!=0;
  jsvUnLock(item);
  jsvUnLock(history);
  return freed;
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
    JsVar *timerCallback = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "callback", false));
    bool recur = jsvGetBoolAndUnLock(jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "recur", false)));
    JsVar *timerInterval = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "interval", false));
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
     JsVar *watchCallback = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "callback", false));
     JsVar *watchRecur = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "recur", false));
     JsVar *watchPin = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "pin", false));
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
        jspParseDoubleFunction(&pinVar, &valueVar);
        int pin = jshGetPinFromVar(pinVar);
        jsvUnLock(pinVar);
        JsVarFloat value = jsvGetDouble(valueVar);
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
        jspParseDoubleFunction(&valueVar, &bitVar);
        JsVarInt value = jsvGetIntegerAndUnLock(valueVar);
        JsVarInt bit = jsvGetIntegerAndUnLock(bitVar);
        return jsvNewFromInteger( (value >> bit) & 1);
      }
      if (strcmp(name,"bitWrite")==0) {
        /*JS* function bitWrite(value, bitnum, bitdata)
         *JS*  Write the specified bit from the value. Lowest significance bit is 0
         */
        JsVar *valueVar, *bitVar, *dataVar;
        jspParseTripleFunction(&valueVar, &bitVar, &dataVar);
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
        jspParseDoubleFunction(&valueVar, &bitVar);
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
        jspParseDoubleFunction(&valueVar, &bitVar);
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
      if (strcmp(name,"clearTimeout")==0 || strcmp(name,"clearInterval")==0) {
        /*JS* function clearTimeout(id)
         *JS*  Clear the Timeout that was created with setTimeout, for example:
         *JS*   var id = setTimeout(function () { print("foo"); }, 1000);
         *JS*   clearTimeout(id);
         */
        /*JS* function clearInterval(id)
         *JS*  Clear the Interval that was created with setTimeout, for example:
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
          jsError("Unknown Timer");
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
          JsVar *pinVar = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "pin", false));
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
        jspParseTripleFunction(&pinVar, &valueVar, &timeVar);
        int pin = jshGetPinFromVar(pinVar);
        jsvUnLock(pinVar);
        bool value = jsvGetBool(valueVar);
        jsvUnLock(valueVar);
        JsVarFloat time = jsvGetDoubleAndUnLock(timeVar);
        //jsPrintInt((JsVarInt)(time*1000));
        jshPinPulse(pin, value, time);
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
        jspParseDoubleFunction(&pinVar, &valueVar);
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
        jspParseDoubleFunction(&func, &timeout);
        if (!jsvIsFunction(func) && !jsvIsString(func)) {
          jsError("Function or String not supplied!");
        }
        // Create a new timer
        JsVar *timerPtr = jsvNewWithFlags(JSV_OBJECT);
        JsVarFloat interval = jsvGetDouble(timeout);
        if (interval<0.1) interval=0.1;
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
        JsVarInt itemIndex = jsvArrayPush(timerArrayPtr, timerPtr) - 1;
        //jsPrint("TIMER AFTER ADD\n"); jsvTrace(timerArray,5);
        jsvUnLock(timerArrayPtr);
        jsvUnLock(timerPtr);
        jsvUnLock(func);
        jsvUnLock(timeout);
        //jsvTrace(jsiGetParser()->root, 0);
        return jsvNewFromInteger(itemIndex);
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
        jspParseTripleFunction(&funcVar, &pinVar, &recurringVar);
        if (!jsvIsFunction(funcVar) && !jsvIsString(funcVar)) {
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
        if (pinBusyIndicator<=0)
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
    if (strcmp(name,"trace")==0) {
      /*JS* function trace()
       *JS*  Output debugging information 
       */
      jspParseEmptyFunction();
      jsvTrace(p.root, 0);
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
