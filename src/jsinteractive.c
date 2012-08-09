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
#define CHAR_DELETE_RECV 0x08
#define CHAR_DELETE_SEND 0x08
#else
#define CHAR_DELETE_RECV 127
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

TODOFlags todo = TODO_NOTHING;
JsVarRef events = 0; // Linked List of events to execute
JsVarRef timerArray; // Linked List of timers to check and run
// ----------------------------------------------------------------------------
JsParse p; ///< The parser we're using for interactiveness
JsVar *inputline = 0; ///< The current input line
bool echo = true; ///< do we provide any user feedback?
int brackets = 0; ///<  how many brackets have we got on this line?
JsVar *jsiHandleFunctionCall(JsExecInfo *execInfo, JsVar *a, const char *name); // forward decl
// ----------------------------------------------------------------------------

// Used when recovering after being flashed
// 'claim' anything we are using
void jsiSoftInit() {
  events = 0;
  inputline = jsvNewFromString("");

  JsVar *timersName = jsvFindChildFromString(p.root, "timers", true);
  if (!timersName->firstChild)
    timersName->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_ARRAY)));
  timerArray = jsvRefRef(timersName->firstChild);
  jsvUnLock(timersName);

  // Check any existing timers and try and set time correctly
  JsSysTime currentTime = jshGetSystemTime();
  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarRef timer = timerArrayPtr->firstChild;
  while (timer) {
    JsVar *timerNamePtr = jsvLock(timer);
    JsVar *timerTime = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "time", false));
    JsVar *timerInterval = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "interval", false));
    jsvSetInteger(timerTime, currentTime + jsvGetInteger(timerInterval));
    jsvUnLock(timerInterval);
    jsvUnLock(timerTime);
    timer = timerNamePtr->nextSibling;
    jsvUnLock(timerNamePtr);
  }
  jsvUnLock(timerArrayPtr);
}

// Used when shutting down before flashing
// 'release' anything we are using, but ensure that it doesn't get freed
void jsiSoftKill() {
  jsvUnLock(inputline);
  inputline=0;
  if (events) {
    jsvUnRefRef(events);
    events=0;
  }
  if (timerArray) {
    jsvUnRefRef(timerArray);
    timerArray=0;
  }
}

void jsiInit(bool autoLoad) {
  jsvInit();
  jspInit(&p);
  // link in our functions
  jsfSetHandleFunctionCallDelegate(jsiHandleFunctionCall);


  /*for (i=0;i<IOPINS;i++)
     ioPinState[i].callbacks = 0;*/

  /* If flash contains any code, then we should
     Try and load from it... */
  if (autoLoad && jshFlashContainsCode())
    jshLoadFromFlash();

  jsiSoftInit();
  echo = true;
  brackets = 0;
  

  // rectangles @ http://www.network-science.de/ascii/
  jsPrint("\r\n _____ _            __ _____\r\n|_   _|_|___ _ _ __|  |   __|\r\n  | | | |   | | |  |  |__   |\r\n  |_| |_|_|_|_  |_____|_____|\r\n            |___|\r\n Copyright 2012 Gordon Williams\r\n                gw@pur3.co.uk\r\n-------------------------------- ");
  jsPrint("\r\n\r\n>");
}



void jsiKill() {
  jsiSoftKill();

  jspKill(&p);
  jsvKill();
}

void jsiHandleChar(char ch) {
  if (ch=='{') brackets++;
  if (ch=='}') brackets--;


  if (ch == CHAR_DELETE_RECV /*delete*/) {
    size_t l = jsvGetStringLength(inputline);
    if (l>0) {
      // clear the character
      if (echo) {
        jshTX(CHAR_DELETE_SEND);
        jshTX(' ');
        jshTX(CHAR_DELETE_SEND);
      }
      // FIXME hacky - should be able to just remove the end character without copying
      JsVar *v = jsvNewFromString("");
      if (l>1) jsvAppendStringVar(v, inputline, 0, (int)(l-1));
      jsvUnLock(inputline);
      inputline=v;
    } else {
      // no characters, don't allow delete
    }
  } else if (ch == '\r') {
    if (brackets<=0) {
      if (echo) {
        jshTX('\r');
        jshTX('\n');
      }

      JsVar *v = jspEvaluateVar(&p, inputline);
      jsvUnLock(inputline);

      if (echo) jshTX('=');

      jsfPrintJSON(v);
      jsvUnLock(v);

      inputline = jsvNewFromString("");
      brackets = 0;

      if (echo) jshTXStr("\r\n>");
    } else {
      if (echo) jshTXStr("\n:");
    }
  } else {
    if (echo) jshTX(ch);
    // Append the character to our input line
    char buf[2];
    buf[0] = ch;
    buf[1] = 0;
    jsvAppendString(inputline, buf);
  }
}

void jsiQueueEvents(JsVarRef callbacks) { // array of functions or single function
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
  if (jsvIsFunction(callbackVar)) {
    JsVar *event = jsvRef(jsvNewWithFlags(JSV_OBJECT|JSV_NATIVE));
    jsvUnLock(jsvAddNamedChild(jsvGetRef(event), jsvGetRef(callbackVar), "func"));
    if (lastEvent) {
      lastEvent->nextSibling = jsvGetRef(event);
      jsvUnLock(lastEvent);
    } else
      events = jsvGetRef(event);
    jsvUnLock(event);
    jsvUnLock(callbackVar);
  } else {
    // go through all callbacks
    JsVarRef next = callbackVar->firstChild;
    jsvUnLock(callbackVar);
    while (next) {
      //jsPrint("Queue Event\n");
      JsVar *child = jsvLock(next);
      // for each callback...
      JsVar *event = jsvRef(jsvNewWithFlags(JSV_OBJECT|JSV_NATIVE));
      jsvUnLock(jsvAddNamedChild(jsvGetRef(event), jsvGetRef(child), "func"));
      // TODO: load in parameters
      // add event to the events list
      if (lastEvent) {
        lastEvent->nextSibling = jsvGetRef(event);
        jsvUnLock(lastEvent);
      } else
        events = jsvGetRef(event);
      jsvUnLock(lastEvent);
      lastEvent = event;
      // go to next callback
      next = child->nextSibling;
      jsvUnLock(child);
    }
    // clean up
    jsvUnLock(lastEvent);
  }
}

void jsiExecuteEvents() {
  while (events) {
    JsVar *event = jsvLock(events);
    // Get function to execute
    JsVar *func = jsvSkipNameAndUnlock(jsvFindChildFromString(jsvGetRef(event), "func", false));
    // free + go to next
    events = event->nextSibling;
    event->nextSibling = 0;
    jsvUnRef(event);
    jsvUnLock(event);

    // now run..
    if (func) jspExecuteFunction(&p, func);
    //jsPrint("Event Done\n");
    jsvUnLock(func);
  }
}

void jsiIdle() {
  // Check each pin for any change
  /*int pin;
  for (pin=0;pin<IOPINS;pin++) {
    bool val = GPIO_ReadInputDataBit(IOPIN_DATA[pin].gpio, IOPIN_DATA[pin].pin) ? true : false;
    if (val != ioPinState[pin].value) { // there was a change...
      ioPinState[pin].value = val;
      // TODO: do we want an 'event' object that we pass into the call?
      queueAll(ioPinState[pin].callbacks); // queue callbacks for execution
      // TODO: clear callbacks list after execution?
    }
  }*/
  // Check timers
  JsSysTime time = jshGetSystemTime();

  JsVar *timerArrayPtr = jsvLock(timerArray);
  JsVarRef timer = timerArrayPtr->firstChild;
  while (timer) {
    JsVar *timerNamePtr = jsvLock(timer);
    JsVar *timerTime = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "time", false));
    if (timerTime && time > jsvGetInteger(timerTime)) {
      JsVar *timerCallback = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "callback", false));
      JsVar *timerRecurring = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "recur", false));
      jsiQueueEvents(jsvGetRef(timerCallback));
      if (jsvGetBool(timerRecurring)) {
        JsVar *timerInterval = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "interval", false));
        jsvSetInteger(timerTime, jsvGetInteger(timerTime)+jsvGetInteger(timerInterval));
        jsvUnLock(timerInterval);
      } else {
        // free all
        jsvRemoveChild(timerArrayPtr, timerNamePtr);
      }
      jsvUnLock(timerCallback);
      jsvUnLock(timerRecurring);

    }
    jsvUnLock(timerTime);
    timer = timerNamePtr->nextSibling;
    jsvUnLock(timerNamePtr);
  }
  jsvUnLock(timerArrayPtr);


  // execute any outstanding events
  jsiExecuteEvents();
  // check for TODOs
  if (todo) {
    if (todo & TODO_RESET) {
      todo &= ~TODO_RESET;
      // shut down everything and start up again
      jsiKill();
      jsiInit(false); // don't autoload
    }
    if (todo & TODO_FLASH_SAVE) {
      todo &= ~TODO_FLASH_SAVE;
      jsiSoftKill();
      jspSoftKill(&p);
      jshSaveToFlash();
      jspSoftInit(&p);
      jsiSoftInit();
    }
    if (todo & TODO_FLASH_LOAD) {
      todo &= ~TODO_FLASH_LOAD;
      jsiSoftKill();
      jspSoftKill(&p);
      jshLoadFromFlash();
      jspSoftInit(&p);
      jsiSoftInit();
    }
  }
}

void jsiLoop() {
  int ch = jshRX();
  if (ch>=0) {
    // We have data, use it!
    jsiHandleChar((char)ch);
  } else {
    // Do general idle stuff
    jsiIdle();
  }
}


/** Handle function calls - do this programatically, so we can save on RAM */
JsVar *jsiHandleFunctionCall(JsExecInfo *execInfo, JsVar *a, const char *name) {
  if (a==0) { // Special cases for we're just a basic function
    if (strcmp(name,"print")==0) {
      /*JS* function print(text)
       * Print the supplied string
       */
      JsVar *v = jsvAsString(jspParseSingleFunction(), true);
      jsvPrintStringVar(v);
      jsvUnLock(v);
      jsPrint("\n");
      return 0;
    }
    if (strcmp(name,"getTime")==0) {
      /*JS* function getTime()
       *  Return the current system time in Seconds (as a floating point number)
       */
      jspParseEmptyFunction();
      return jsvNewFromFloat((JsVarFloat)jshGetSystemTime() / (JsVarFloat)jshGetTimeFromMilliseconds(1000));
    }
    if (strcmp(name,"setTimeout")==0 || strcmp(name,"setInterval")==0) {
      /*JS* function setTimeout(function, timeout)
       * Call the function specified ONCE after the timeout in milliseconds.
       * This can also be removed using clearTimeout
       */
      /*JS* function setInterval(function, timeout)
       * Call the function specified REPEATEDLY after the timeout in milliseconds.
       * This can also be removed using clearInterval
       */
      bool recurring = strcmp(name,"setInterval")==0;
      JsVar *func, *timeout;
      jspParseDoubleFunction(&func, &timeout);
      if (!jsvIsFunction(func)) {
        jsError("Function not supplied!");
      }
      // Create a new timer
      JsVar *timerPtr = jsvNewWithFlags(JSV_OBJECT);
      JsVarInt interval = jshGetTimeFromMilliseconds(jsvGetDouble(timeout));
      if (interval<1) interval=1;
      JsVar *v;
      v = jsvNewFromInteger(jshGetSystemTime() + interval);
      jsvUnLock(jsvAddNamedChild(jsvGetRef(timerPtr), jsvGetRef(v), "time"));
      jsvUnLock(v);
      if (recurring) {
        v = jsvNewFromInteger(interval);
        jsvUnLock(jsvAddNamedChild(jsvGetRef(timerPtr), jsvGetRef(v), "interval"));
        jsvUnLock(v);
      }
      v = jsvNewFromBool(recurring);
      jsvUnLock(jsvAddNamedChild(jsvGetRef(timerPtr), jsvGetRef(v), "recur"));
      jsvUnLock(v);
      jsvUnLock(jsvAddNamedChild(jsvGetRef(timerPtr), jsvGetRef(func), "callback"));
      JsVar *timerArrayPtr = jsvLock(timerArray);
      JsVarInt itemIndex = jsvArrayPush(timerArrayPtr, timerPtr) - 1;
      jsvUnLock(timerArrayPtr);
      jsvUnLock(timerPtr);
      jsvUnLock(func);
      jsvUnLock(timeout);
      //jsvTrace(jsiGetParser()->root, 0);
      return jsvNewFromInteger(itemIndex);
    }
    if (strcmp(name,"clearTimeout")==0 || strcmp(name,"clearInterval")==0) {
      /*JS* function clearTimeout(id)
       *  Clear the Timeout that was created with setTimeout, for example:
       *   var id = setTimeout(function () { print("foo"); }, 1000);
       *   clearTimeout(id);
       */
      /*JS* function clearInterval(id)
       *  Clear the Interval that was created with setTimeout, for example:
       *   var id = setInterval(function () { print("foo"); }, 1000);
       *   clearInterval(id);
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
    if (strcmp(name,"input")==0) {
      /*JS* function input(pin)
       *  Get the digital value of the given pin.
       *  Pin can be an integer, or a string such as "A0","C13",etc
       */
      JsVar *pinVar = jspParseSingleFunction();
      int pin = jshGetPinFromVar(pinVar);
      jsvUnLock(pinVar);
      return jsvNewFromBool(jshPinInput(pin));
    }
    if (strcmp(name,"output")==0) {
      /*JS* function output(pin, value)
       *  Set the digital value of the given pin.
       *  Pin can be an integer, or a string such as "A0","C13",etc
       */
      JsVar *pinVar, *valueVar;
      jspParseDoubleFunction(&pinVar, &valueVar);
      int pin = jshGetPinFromVar(pinVar);
      jsvUnLock(pinVar);
      bool value = jsvGetBool(valueVar);
      jsvUnLock(valueVar);
      jshPinOutput(pin, value);
      return 0;
    }
    if (strcmp(name,"pulse")==0) {
      /*JS* function pulse(pin, value,time)
       *  Pulse the pin with the value for the given time in milliseconds
       *  eg. pulse("A0",1,500); pulses A0 high for 500ms
       *  Pin can be an integer, or a string such as "A0","C13",etc
       */
      JsVar *pinVar, *valueVar, *timeVar;
      jspParseTripleFunction(&pinVar, &valueVar, &timeVar);
      int pin = jshGetPinFromVar(pinVar);
      jsvUnLock(pinVar);
      bool value = jsvGetBool(valueVar);
      jsvUnLock(valueVar);
      JsVarFloat time = jsvGetDouble(timeVar);
      jsvUnLock(timeVar);
      jshPinPulse(pin, value, time);
      return 0;
    }

    if (strcmp(name,"load")==0) {
      /*JS* function load()
       * Load program memory out of flash */
      jspParseEmptyFunction();
      todo |= TODO_FLASH_LOAD;
      return 0;
    }
    if (strcmp(name,"save")==0) {
      /*JS* function save()
       * Save program memory into flash */
      jspParseEmptyFunction();
      todo |= TODO_FLASH_SAVE;
      return 0;
    }
    if (strcmp(name,"reset")==0) {
      /*JS* function reset()
       * Reset everything - clear program memory */
      jspParseEmptyFunction();
      todo |= TODO_RESET;
      return 0;
    }
    if (strcmp(name,"trace")==0) {
      jspParseEmptyFunction();
      jsvTrace(p.root, 0);
      return 0;
    }
/* TODO:
       "function setWatch(func,pin)",
       "function clearWatch(func,pin)",
 *
 *
 *
 */
  }
  // unhandled
  return JSFHANDLEFUNCTIONCALL_UNHANDLED;
}

JsParse *jsiGetParser() {
  return &p;
}
