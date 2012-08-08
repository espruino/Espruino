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
#define MAX_TIMERS 4
typedef struct TimerState {
  JsSysTime time;
  JsSysTime interval;
  bool recurring;
  JsVarRef callback; // a calback, or 0
} TimerState;
TimerState timers[MAX_TIMERS]; // Timers available


JsVarRef events = 0; // Linked List of events to execute
// ----------------------------------------------------------------------------
JsParse p; ///< The parser we're using for interactiveness
JsVar *inputline = 0; ///< The current input line
bool echo = true; ///< do we provide any user feedback?
int brackets = 0; ///<  how many brackets have we got on this line?
JsVar *jsiHandleFunctionCall(JsExecInfo *execInfo, JsVar *a, const char *name); // forward decl
// ----------------------------------------------------------------------------

void jsiInit() {
  jsvInit();
  jspInit(&p);
  // link in our functions
  jsfSetHandleFunctionCallDelegate(jsiHandleFunctionCall);


  int i;
  /*for (i=0;i<IOPINS;i++)
     ioPinState[i].callbacks = 0;*/
  for (i=0;i<MAX_TIMERS;i++)
     timers[i].interval = 0;

  inputline = jsvNewFromString("");
  echo = true;
  brackets = 0;
  events = 0;

  // rectangles @ http://www.network-science.de/ascii/
  jsPrint("\r\n _____ _            __ _____\r\n|_   _|_|___ _ _ __|  |   __|\r\n  | | | |   | | |  |  |__   |\r\n  |_| |_|_|_|_  |_____|_____|\r\n            |___|\r\n Copyright 2012 Gordon Williams\r\n                gw@pur3.co.uk\r\n-------------------------------- ");
  jsPrint("\r\n\r\n>");
}

void jsiKill() {
  jsvUnLock(inputline);
  inputline=0;
  if (events) {
    jsvUnRefRef(events);
    events=0;
  }

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
  int i;
  for (i=0;i<MAX_TIMERS;i++) {
    if (timers[i].interval && time > timers[i].time) {
      // queue
      jsiQueueEvents(timers[i].callback);
      if (timers[i].recurring) {
        timers[i].time += timers[i].interval;
      } else {
        // free all
        timers[i].interval = 0;
        jsvUnRefRef(timers[i].callback);
        timers[i].callback = 0;
      }
    }
  }
  // execute any outstanding events
  jsiExecuteEvents();
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
      // find an empty timer
      int timer = 0;
      while (timer<MAX_TIMERS && timers[timer].interval!=0) timer++;
      // then
      if (timer >= MAX_TIMERS) {
        jsError("Too many timers!");
        return 0;
      } else {
        timers[timer].interval = jshGetTimeFromMilliseconds(jsvGetDouble(timeout));
        if (timers[timer].interval<1) timers[timer].interval=1;
        timers[timer].time = jshGetSystemTime() + timers[timer].interval;
        timers[timer].recurring = recurring;
        timers[timer].callback = jsvGetRef(jsvRef(func));
        return jsvNewFromInteger(timer);
      }
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
      int id = (int)jsvGetInteger(idVar);
      jsvUnLock(idVar);

      if (id>=0 && id<MAX_TIMERS && timers[id].interval) {
        timers[id].interval = 0;
        jsvUnRefRef(timers[id].callback);
        timers[id].callback = 0;
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
