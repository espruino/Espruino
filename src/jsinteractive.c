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
JsVarRef timerArray = 0; // Linked List of timers to check and run
JsVarRef watchArray = 0; // Linked List of input watches to check and run
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

  JsVar *watchName = jsvFindChildFromString(p.root, "watches", true);
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
  if (autoLoad && jshFlashContainsCode()) {
    jspSoftKill(&p);
    jsvSoftKill();
    jshLoadFromFlash();
    jsvSoftInit();
    jspSoftInit(&p);
  }
  //jsvTrace(jsiGetParser()->root, 0);

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

      if (jspIsInterrupted()) {
        jspSetInterrupted(false);
        jshTXStr("Execution Interrupted.\r\n");
      }

      if (echo) jshTX('=');

      jsfPrintJSON(v);
      jsvUnLock(v);

      inputline = jsvNewFromString("");
      brackets = 0;

      if (echo) jshTXStr("\r\n>");
    } else {
      if (echo) jshTXStr("\n:");
      jsvAppendCharacter(inputline, '\n');
    }
  } else {
    if (echo) jshTX(ch);
    // Append the character to our input line
    jsvAppendCharacter(inputline, ch);
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
    jsvUnLock(jsvAddNamedChild(event, callbackVar, "func"));
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
      jsvUnLock(jsvAddNamedChild(event, child, "func"));
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
  // Handle hardware-related idle stuff (like checking for pin events)
  IOEvent event;
  while (jshPopIOEvent(&event)) {
    // we have an event... find out what it was for...
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsVarRef watch = watchArrayPtr->firstChild;
    while (watch) {
      JsVar *watchNamePtr = jsvLock(watch);
      JsVar *watchPin = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "time", false));
      int pin = jshGetPinFromVar(watchPin); // TODO: could be faster?
      jsvUnLock(watchPin);

      if (jshIsEventForPin(&event, pin)) {
        JsVar *watchCallback = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "callback", false));
        JsVar *watchRecurring = jsvSkipNameAndUnlock(jsvFindChildFromString(watchNamePtr->firstChild, "recur", false));
        jsiQueueEvents(jsvGetRef(watchCallback));
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
      jsiQueueEvents(jsvGetRef(timerCallback));
      if (jsvGetBool(timerRecurring)) {
        JsVar *timerInterval = jsvSkipNameAndUnlock(jsvFindChildFromString(timerNamePtr->firstChild, "interval", false));
        JsVarInt interval = jsvGetIntegerAndUnLock(timerInterval);
        if (interval<=0)
          jsvSetInteger(timerTime, time); // just set to current system time
        else
          jsvSetInteger(timerTime, jsvGetInteger(timerTime)+interval);
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
  jsiExecuteEvents();
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
       *JS* Print the supplied string
       */
      JsVar *v = jsvAsString(jspParseSingleFunction(), true);
      jsvPrintStringVar(v);
      jsvUnLock(v);
      jsPrint("\n");
      return 0;
    }
    if (strcmp(name,"getTime")==0) {
      /*JS* function getTime()
       *JS*  Return the current system time in Seconds (as a floating point number)
       */
      jspParseEmptyFunction();
      return jsvNewFromFloat((JsVarFloat)jshGetSystemTime() / (JsVarFloat)jshGetTimeFromMilliseconds(1000));
    }
    if (strcmp(name,"setTimeout")==0 || strcmp(name,"setInterval")==0) {
      /*JS* function setTimeout(function, timeout)
       *JS*  Call the function specified ONCE after the timeout in milliseconds.
       *JS*  This can also be removed using clearTimeout
       */
      /*JS* function setInterval(function, timeout)
       *JS*  Call the function specified REPEATEDLY after the timeout in milliseconds.
       *JS*  This can also be removed using clearInterval
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
      jsvUnLock(jsvAddNamedChild(timerPtr, v, "time"));
      jsvUnLock(v);
      if (recurring) {
        v = jsvNewFromInteger(interval);
        jsvUnLock(jsvAddNamedChild(timerPtr, v, "interval"));
        jsvUnLock(v);
      }
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

    if (strcmp(name,"digitalRead")==0) {
      /*JS* function digitalRead(pin)
       *JS*  Get the digital value of the given pin.
       *JS*  Pin can be an integer, or a string such as "A0","C13",etc
       */
      JsVar *pinVar = jspParseSingleFunction();
      int pin = jshGetPinFromVar(pinVar);
      jsvUnLock(pinVar);
      return jsvNewFromBool(jshPinInput(pin));
    }
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
    if (strcmp(name,"digitalWrite")==0) {
      /*JS* function digitalWrite(pin, value)
       *JS*  Set the digital value of the given pin.
       *JS*  Pin can be an integer, or a string such as "A0","C13",etc
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
    if (strcmp(name,"setWatch")==0) {
      /*JS* function setWatch(function, pin, repeat)
       *JS*  Call the function specified ONCE (if repeat==false or undefined) or
       *JS*  REPEATEDLY if (repeat==true) when the pin changes
       *JS*  This can also be removed using clearWatch
       */
      JsVar *funcVar, *pinVar, *recurringVar;
      jspParseTripleFunction(&funcVar, &pinVar, &recurringVar);
      if (!jsvIsFunction(funcVar)) {
        jsError("Function not supplied!");
      }
      // Create a new watch
      JsVar *watchPtr = jsvNewWithFlags(JSV_OBJECT);
      JsVar *v;
      v = jsvSkipName(pinVar);
      jsvUnLock(jsvAddNamedChild(watchPtr, v, "pin"));
      jsvUnLock(v);
      v = jsvNewFromBool(jsvGetBool(recurringVar));
      jsvUnLock(jsvAddNamedChild(watchPtr, v, "recur"));
      jsvUnLock(v);
      jsvUnLock(jsvAddNamedChild(watchPtr, funcVar, "callback"));
      JsVar *watchArrayPtr = jsvLock(watchArray);
      JsVarInt itemIndex = jsvArrayPush(watchArrayPtr, watchPtr) - 1;
      jsvUnLock(watchArrayPtr);
      jsvUnLock(watchPtr);
      jsvUnLock(funcVar);

      jshPinWatch(jshGetPinFromVar(pinVar), true);
      jsvUnLock(pinVar);
      jsvUnLock(recurringVar);
      //jsvTrace(jsiGetParser()->root, 0);

      return jsvNewFromInteger(itemIndex);
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
        jshPinWatch(jshGetPinFromVar(pinVar), true);
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

    if (strcmp(name,"load")==0) {
      /*JS* function load()
       *JS*  Load program memory out of flash */
      jspParseEmptyFunction();
      todo |= TODO_FLASH_LOAD;
      return 0;
    }
    if (strcmp(name,"save")==0) {
      /*JS* function save()
       *JS*  Save program memory into flash */
      jspParseEmptyFunction();
      todo |= TODO_FLASH_SAVE;
      return 0;
    }
    if (strcmp(name,"reset")==0) {
      /*JS* function reset()
       *JS*  Reset everything - clear program memory */
      jspParseEmptyFunction();
      todo |= TODO_RESET;
      return 0;
    }
    if (strcmp(name,"trace")==0) {
      /*JS* function trace()
       *JS*  Output debugging information */
      jspParseEmptyFunction();
      jsvTrace(p.root, 0);
      return 0;
    }

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
    if (strcmp(name,"lowByte")==0) {
      /*JS* function lowByte(value)
       *JS*  Return the low byte of the value
       */
      JsVar *valueVar = jspParseSingleFunction();
      JsVarInt value = jsvGetIntegerAndUnLock(valueVar);
      return jsvNewFromInteger(value & 0xFF);
    }
    if (strcmp(name,"highByte")==0) {
      /*JS* function highByte(value)
       *JS*  Return the high (second) byte of the value
       */
      JsVar *valueVar = jspParseSingleFunction();
      JsVarInt value = jsvGetIntegerAndUnLock(valueVar);
      return jsvNewFromInteger((value>>8) & 0xFF);
    }
/* TODO:
analogWrite(pin, value, [freq])
addWatch -> attachInterrupt(pin, handler, mode)
clearWatch -> detachInterrupt(pin)
delay(milliseconds)
getPinMode(pin) -> pinMode
pinMode(pin, direction, [mux], [pullup], [slew])
shiftOut(dataPin, clockPin, bitOrder, val)

 */
  }
  // unhandled
  return JSFHANDLEFUNCTIONCALL_UNHANDLED;
}

JsParse *jsiGetParser() {
  return &p;
}
