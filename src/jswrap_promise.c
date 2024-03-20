/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2016 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * ES6 Promise implementation
 * ----------------------------------------------------------------------------
 */

//#define PROMISE_DEBUG
#include "jsutils.h"

#if ESPR_NO_PROMISES!=1

#include "jswrap_promise.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrapper.h"

#define JS_PROMISE_THEN_NAME JS_HIDDEN_CHAR_STR"thn"
#define JS_PROMISE_CATCH_NAME JS_HIDDEN_CHAR_STR"cat"
#define JS_PROMISE_REMAINING_NAME JS_HIDDEN_CHAR_STR"left"
#define JS_PROMISE_RESULT_NAME JS_HIDDEN_CHAR_STR"res"
#define JS_PROMISE_ISRESOLVED_NAME JS_HIDDEN_CHAR_STR"resolved"
#define JS_PROMISE_PROM_NAME JS_HIDDEN_CHAR_STR"prom"
#define JS_PROMISE_VALUE_NAME JS_HIDDEN_CHAR_STR"value"
#define JS_PROMISE_STATE_NAME JS_HIDDEN_CHAR_STR"state"

enum JS_PROMISE_STATE_ENUM {
    JS_PROMISE_STATE_PENDING,
    JS_PROMISE_STATE_REJECTED,
    JS_PROMISE_STATE_FULFILLED
};

/*JSON{
  "type" : "class",
  "class" : "Promise",
  "typescript": "Promise<T>",
  "ifndef" : "SAVE_ON_FLASH"
}
This is the built-in class for ES6 Promises
*/

void _jswrap_promise_resolve(JsVar *prombox, JsVar *data);
void _jswrap_promise_reject(JsVar *prombox, JsVar *data);

bool _jswrap_promise_is_promise(JsVar *promise) {
  JsVar *constr = jspGetConstructor(promise);
  bool isPromise = constr && (void*)constr->varData.native.ptr==(void*)jswrap_promise_constructor;
  jsvUnLock(constr);
  return isPromise;
}

// A single reaction chain - this is recursive until a promise is returned or chain ends. data can be undefined/0
void _jswrap_promise_reaction_call(JsVar *promise, JsVar *reaction, JsVar *data, JsVar *isThen) {
  JsVar *exceptionName = jsvFindChildFromString(execInfo.hiddenRoot, JSPARSE_EXCEPTION_VAR);
  if (exceptionName) {
    jsvUnLock(exceptionName);
    return;
  }
  JsVar * nextPromBox = jsvObjectGetChildIfExists(reaction, "nextBox");
  JsVar * nextProm = jsvObjectGetChildIfExists(nextPromBox, JS_PROMISE_PROM_NAME);
  if (nextPromBox) {
    if (nextProm) {
      bool threw = false;
      JsVar * retVal;
      JsVar * callback = jsvObjectGetChildIfExists(reaction, "cb");
      if (callback) {
        JsExecFlags oldExecute = execInfo.execute;
        retVal = jspeFunctionCall(callback, 0, promise, false, 1, &data);
        execInfo.execute = oldExecute;
        JsVar *exception = jspGetException();
        if (exception) {
          threw = true;
          jsvUnLock(retVal);
          retVal = exception;
        }
        jsvUnLock(callback);
      }
      else {
        //pass-through
        if (!jsvGetBool(isThen)) {
          threw = true;
        }
        retVal = data;
      }
      if (threw) _jswrap_promise_reject(nextPromBox, retVal);
      else _jswrap_promise_resolve(nextPromBox, retVal);

      if (callback) jsvUnLock(retVal);
      jsvUnLock(nextProm);
    }
    jsvUnLock(nextPromBox);
  }
}
// Value can be undefined/0
void _jswrap_promise_queue_reaction(JsVar *promise, JsVar *reaction, JsVar *value, bool isThenCb) {
  JsVar *fn = jsvNewNativeFunction((void (*)(void))_jswrap_promise_reaction_call, 
    JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS)|(JSWAT_JSVAR<<JSWAT_BITS*2)|(JSWAT_JSVAR<<JSWAT_BITS*3));
  if (fn) {
    jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, promise); // bind 'this'
    JsVar *bIsThenCb = jsvNewFromBool(isThenCb);
    JsVar *args[] = {reaction, value, bIsThenCb};
    jsiQueueEvents(promise, fn, args, 3);
    jsvUnLock2(fn, bIsThenCb);
  }
}
void _jswrap_promise_seal(JsVar *promise, JsVar *data,bool resolving) {
  if (resolving) {
    jsvObjectSetChildAndUnLock(promise, JS_PROMISE_STATE_NAME, jsvNewFromInteger(JS_PROMISE_STATE_FULFILLED));
  } else {
    jsvObjectSetChildAndUnLock(promise, JS_PROMISE_STATE_NAME, jsvNewFromInteger(JS_PROMISE_STATE_REJECTED));
  }

  const char *eventName = resolving ? JS_PROMISE_THEN_NAME : JS_PROMISE_CATCH_NAME;
  JsVar *reactions = jsvObjectGetChildIfExists(promise, eventName);
  if (!reactions) {
    if (!resolving) {
      jsExceptionHere(JSET_ERROR, "Unhandled promise rejection: %v", data);
      // If there was an exception with a stack trace, pass it through so we can keep adding stack to it
      JsVar *stack = 0;
      if (jsvIsObject(data) && (stack=jsvObjectGetChildIfExists(data, "stack"))) {
        jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, JSPARSE_STACKTRACE_VAR, stack);
      }
    }
  } else {
    if (jsvIsArray(reactions)) {
      JsvObjectIterator it;
      jsvObjectIteratorNew(&it, reactions);   
      while (jsvObjectIteratorHasValue(&it)) {
        JsVar *reaction = jsvObjectIteratorGetValue(&it);
        _jswrap_promise_queue_reaction(promise,reaction,data,resolving);
        jsvUnLock(reaction);
        jsvObjectIteratorNext(&it);
      }
      jsvObjectIteratorFree(&it);
    } else if (reactions) {
      _jswrap_promise_queue_reaction(promise,reactions,data,resolving);
    }
    jsvUnLock(reactions);
  }
}

void _jswrap_promise_resolve_or_reject(JsVar *prombox, JsVar *data, bool resolving) {
  JsVar *isResolved = jsvObjectGetChildIfExists(prombox, JS_PROMISE_ISRESOLVED_NAME);
  if (jsvGetBool(isResolved)){
    jsvUnLock(isResolved);
    return;
  }
  jsvUnLock(isResolved);
  jsvObjectSetChildAndUnLock(prombox, JS_PROMISE_ISRESOLVED_NAME, jsvNewFromBool(true));
  JsVar * promise = jsvObjectGetChildIfExists(prombox, JS_PROMISE_PROM_NAME);
  if (promise) {
    if (jsvIsEqual(data,promise)) {
      jsExceptionHere(JSET_ERROR,"Illegal resolving to self");
      jsvUnLock(promise);
      return;
    }

    jsvObjectSetChild(promise, JS_PROMISE_VALUE_NAME, data);
    if (!resolving || !jsvIsObject(data) ) {
      _jswrap_promise_seal(promise, data, resolving);
      jsvUnLock(promise);
      return;
    }
    bool isProm = _jswrap_promise_is_promise(data);
    bool isThenable = false;
    JsVar *then = jsvObjectGetChildIfExists(data,"then");
    if (jsvIsFunction(then)) isThenable = true;
    
    if (!isThenable && !isProm) {
      _jswrap_promise_seal(promise, data, resolving);
      jsvUnLock(promise);
      return;
    }
    
    JsVar *jsResolve = jsvNewNativeFunction((void (*)(void))_jswrap_promise_resolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
    JsVar *jsReject = jsvNewNativeFunction((void (*)(void))_jswrap_promise_reject, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));

    JsVar *prombox = jsvNewObject();
    if (prombox) {
      jsvObjectSetChild(prombox, JS_PROMISE_PROM_NAME, promise);
      jsvObjectSetChildAndUnLock(prombox,JS_PROMISE_ISRESOLVED_NAME,jsvNewFromBool(false));

      if (jsResolve) jsvObjectSetChild(jsResolve, JSPARSE_FUNCTION_THIS_NAME, prombox);
      if (jsReject) jsvObjectSetChild(jsReject, JSPARSE_FUNCTION_THIS_NAME, prombox);
      
      if (isThenable) {
        JsVar *args[2] = {jsResolve,jsReject};
        JsExecFlags oldExecute = execInfo.execute;
        jsvUnLock(jspeFunctionCall(then, 0, data, false, 2, args));
        execInfo.execute = oldExecute;
        jsvUnLock(then);
        JsVar *exception = jspGetException();
        if (exception) {
          _jswrap_promise_reject(prombox, exception);
          jsvUnLock(exception);
        }
      } else {
        jsvUnLock(jswrap_promise_then(data,jsResolve,jsReject));
      }
      jsvUnLock3(jsResolve,jsReject,prombox);
    }
    jsvUnLock(promise);
  }
}

void _jswrap_promise_resolve(JsVar *prombox, JsVar *data) {
  _jswrap_promise_resolve_or_reject(prombox, data, true);
}

void _jswrap_promise_reject(JsVar *prombox, JsVar *data) {
  _jswrap_promise_resolve_or_reject(prombox, data, false);
}

void _jswrap_promise_queueresolve(JsVar *prombox, JsVar *data) {
  JsVar *fn = jsvNewNativeFunction((void (*)(void))_jswrap_promise_resolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
  if (!fn) return;
  jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, prombox); // bind 'this'
  jsiQueueEvents(prombox, fn, &data, 1);
  jsvUnLock(fn);
}

void _jswrap_promise_queuereject(JsVar *prombox, JsVar *data) {
  JsVar *fn = jsvNewNativeFunction((void (*)(void))_jswrap_promise_reject, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
  if (!fn) return;
  jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, prombox); // bind 'this'
  jsiQueueEvents(prombox, fn, &data, 1);
  jsvUnLock(fn);
}

void jspromise_resolve(JsVar *prombox, JsVar *data) {
  _jswrap_promise_queueresolve(prombox, data);
}

void jspromise_reject(JsVar *prombox, JsVar *data) {
  _jswrap_promise_queuereject(prombox, data);
}

//Now returns a promise box.
JsVar *jspromise_create() {
  JsVar *p = jspNewObject(0, "Promise");
  if (!p) return 0;
  JsVar *box = jsvNewObject();
  if (!box) {
    jsvUnLock(p);
    return 0;
  }
  jsvObjectSetChildAndUnLock(p, JS_PROMISE_STATE_NAME, jsvNewFromInteger(JS_PROMISE_STATE_PENDING));

  jsvObjectSetChildAndUnLock(box, JS_PROMISE_PROM_NAME, p);
  jsvObjectSetChildAndUnLock(box,JS_PROMISE_ISRESOLVED_NAME,jsvNewFromBool(false));
  return box;
}

/*JSON{
  "type" : "constructor",
  "class" : "Promise",
  "name" : "Promise",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_promise_constructor",
  "params" : [
    ["executor","JsVar","A function of the form `function (resolve, reject)`"]
  ],
  "return" : ["JsVar","A Promise"],
  "typescript": "new<T>(executor: (resolve: (value: T) => void, reject: (reason?: any) => void) => void): Promise<T>;"
}
Create a new Promise. The executor function is executed immediately (before the
constructor even returns) and
 */
JsVar *jswrap_promise_constructor(JsVar *executor) {
  if (!executor) {
    jsExceptionHere(JSET_ERROR,"Executor function required in promise constructor");
    return 0;
  }
  JsVar *promBox = jspromise_create();
  if (!promBox) return 0;
  JsVar * promise = jsvObjectGetChildIfExists(promBox, JS_PROMISE_PROM_NAME);
  if (promise) {
    //this=prombox, data
    JsVar *resolve = jsvNewNativeFunction((void (*)(void))_jswrap_promise_queueresolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
    JsVar *reject = jsvNewNativeFunction((void (*)(void))_jswrap_promise_queuereject, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));

    // 'this' == prombox.
    if (resolve) jsvObjectSetChild(resolve, JSPARSE_FUNCTION_THIS_NAME, promBox);
    if (reject) jsvObjectSetChild(reject, JSPARSE_FUNCTION_THIS_NAME, promBox);
    
    JsVar *args[2] = {resolve,reject};
    // call the executor
    //emulates try
    
    JsExecFlags oldExecute = execInfo.execute;
    jsvUnLock(jspeFunctionCall(executor, 0, promise, false, 2, args));
    execInfo.execute = oldExecute;

    JsVar *exception = jspGetException();
    if (exception) {
      _jswrap_promise_queuereject(promBox, exception);
      jsvUnLock(exception);
    }
    jsvUnLock2(resolve,reject);
  }
  jsvUnLock(promBox);
  return promise;
}

void jswrap_promise_all_resolve(JsVar *prombox, JsVar *index, JsVar *data) {
  JsVar * promise = jsvObjectGetChildIfExists(prombox, JS_PROMISE_PROM_NAME);
  if (promise) {
    JsVarInt remaining = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(promise, JS_PROMISE_REMAINING_NAME));
    JsVar *arr = jsvObjectGetChildIfExists(promise, JS_PROMISE_RESULT_NAME);
    if (arr) {
      // set the result
      jsvSetArrayItem(arr, jsvGetInteger(index), data);
      // Update remaining list
      remaining--;
      jsvObjectSetChildAndUnLock(promise, JS_PROMISE_REMAINING_NAME, jsvNewFromInteger(remaining));
      if (remaining==0) {
        _jswrap_promise_queueresolve(prombox, arr);
      }
      jsvUnLock(arr);
    }
    jsvUnLock(promise);
  }
}

void jswrap_promise_all_reject(JsVar *prombox, JsVar *data) {
  JsVar * promise = jsvObjectGetChildIfExists(prombox, JS_PROMISE_PROM_NAME);
  if (promise) {
    JsVar *arr = jsvObjectGetChildIfExists(promise, JS_PROMISE_RESULT_NAME);
    if (arr) {
      // if not rejected before
      jsvUnLock(arr);
      jsvObjectRemoveChild(promise, JS_PROMISE_RESULT_NAME);
      _jswrap_promise_queuereject(prombox, data);
    }
    jsvUnLock(promise);
  }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Promise",
  "name" : "all",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_promise_all",
  "params" : [
    ["promises","JsVar","An array of promises"]
  ],
  "return" : ["JsVar","A new Promise"],
  "typescript": "all(promises: Promise<any>[]): Promise<void>;"
}
Return a new promise that is resolved when all promises in the supplied array
are resolved.
*/
JsVar *jswrap_promise_all(JsVar *arr) {
  if (!jsvIsIterable(arr)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting something iterable, got %t", arr);
    return 0;
  }
  JsVar *promBox = jspromise_create();
  if (!promBox) return 0;
  JsVar * promise = jsvObjectGetChildIfExists(promBox, JS_PROMISE_PROM_NAME);
  if (promise) {  
    JsVar *reject = jsvNewNativeFunction((void (*)(void))jswrap_promise_all_reject, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
    if (!reject) return promise; // out of memory error
    jsvObjectSetChild(reject, JSPARSE_FUNCTION_THIS_NAME, promBox); // bind 'this'
    JsVar *promiseResults = jsvNewEmptyArray();
    int promiseIndex = 0;
    int promisesComplete = 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, arr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *p = jsvObjectIteratorGetValue(&it);
      if (_jswrap_promise_is_promise(p)) {
        JsVar *resolve = jsvNewNativeFunction((void (*)(void))jswrap_promise_all_resolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS)|(JSWAT_JSVAR<<(JSWAT_BITS*2)));
        // NOTE: we use (this,JsVar,JsVar) rather than an int to avoid #2377 on emscripten, since that argspec is already used for forEach/otehrs
        // bind the index variable
        JsVar *indexVar = jsvNewFromInteger(promiseIndex);
        jsvAddFunctionParameter(resolve, 0, indexVar);
        jsvUnLock(indexVar);
        jsvObjectSetChild(resolve, JSPARSE_FUNCTION_THIS_NAME, promBox); // bind 'this'
        jsvUnLock2(jswrap_promise_then(p, resolve, reject), resolve);
      } else {
        jsvSetArrayItem(promiseResults, promiseIndex, p);
        promisesComplete++;
      }
      jsvUnLock(p);
      promiseIndex++;
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    if (promisesComplete==promiseIndex) { // already all sorted - return a resolved promise
      jsvUnLock(promise);
      promise = jswrap_promise_resolve(promiseResults);
      jsvUnLock(promiseResults);
    } else { // return our new promise that will resolve when everything is done
      jsvObjectSetChildAndUnLock(promise, JS_PROMISE_REMAINING_NAME, jsvNewFromInteger(promiseIndex-promisesComplete));
      jsvObjectSetChildAndUnLock(promise, JS_PROMISE_RESULT_NAME, promiseResults);
    }
    jsvUnLock2(reject,promBox);
  }
  return promise;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Promise",
  "name" : "resolve",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_promise_resolve",
  "params" : [
    ["promises","JsVar","Data to pass to the `.then` handler"]
  ],
  "return" : ["JsVar","A new Promise"],
  "typescript": "resolve<T extends any>(promises: T): Promise<T>;"
}
Return a new promise that is already resolved (at idle it'll call `.then`)
*/
JsVar *jswrap_promise_resolve(JsVar *data) {
  JsVar *promise = 0;
  // return the promise passed as value, if the value was a promise object.
  if (_jswrap_promise_is_promise(data))
    return jsvLockAgain(data);
  // If the value is a thenable (i.e. has a "then" method), the
  // returned promise will "follow" that thenable, adopting its eventual state
  if (jsvIsObject(data)) {
    JsVar *then = jsvObjectGetChildIfExists(data,"then");
    if (jsvIsFunction(then))
      promise = jswrap_promise_constructor(then);
    jsvUnLock(then);
    if (promise) return promise;
  }
  // otherwise the returned promise will be fulfilled with the value.
  JsVar *promBox = jspromise_create();
  if (!promBox) return 0;
  promise = jsvObjectGetChildIfExists(promBox, JS_PROMISE_PROM_NAME);
  if (promise) {
    jspromise_resolve(promBox, data);
  }
  jsvUnLock(promBox);
  return promise;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Promise",
  "name" : "reject",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_promise_reject",
  "params" : [
    ["promises","JsVar","Data to pass to the `.catch` handler"]
  ],
  "return" : ["JsVar","A new Promise"]
}
Return a new promise that is already rejected (at idle it'll call `.catch`)
*/
/*
  Promise.reject()
*/
JsVar *jswrap_promise_reject(JsVar *data) {
  // otherwise the returned promise will be fulfilled with the value.
  JsVar *promBox = jspromise_create();
  if (!promBox) return 0;
  JsVar * promise = jsvObjectGetChildIfExists(promBox, JS_PROMISE_PROM_NAME);
  if (promise) {
    jspromise_reject(promBox, data);
  }
  jsvUnLock(promBox);
  return promise;
}


/*
Reactions
*/
JsVar *_jswrap_promise_new_reaction(JsVar *nextPromBox, JsVar *callback) {
  JsVar *reaction = jsvNewObject();
  if (!reaction) return 0;
  jsvObjectSetChild(reaction, "cb", callback);
  jsvObjectSetChild(reaction, "nextBox", nextPromBox);
  return reaction;
}

void _jswrap_promise_add_reaction(JsVar *parent, JsVar * nextPromBox, JsVar *callback, bool resolve) {
  // reaction = [{cb:,box:},...]
  JsVar *reaction = _jswrap_promise_new_reaction(nextPromBox,callback);
  if (!reaction) return;
  const char *name = resolve ? JS_PROMISE_THEN_NAME : JS_PROMISE_CATCH_NAME;
  JsVar *c = jsvObjectGetChildIfExists(parent, name);
  if (!c) {
    jsvObjectSetChild(parent, name, reaction);
  } else {
    if (jsvIsArray(c)) {
      jsvArrayPush(c, reaction);
    } else {
      JsVar *fns[2] = {c,reaction};
      JsVar *arr = jsvNewArray(fns, 2);
      jsvObjectSetChildAndUnLock(parent, name, arr);
    }
    jsvUnLock(c);
  }
  jsvUnLock(reaction);
}
/*JSON{
  "type" : "method",
  "class" : "Promise",
  "name" : "then",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_promise_then",
  "params" : [
    ["onFulfilled","JsVar","A callback that is called when this promise is resolved"],
    ["onRejected","JsVar","[optional] A callback that is called when this promise is rejected (or nothing)"]
  ],
  "return" : ["JsVar","The original Promise"],
  "typescript": "then<TResult1 = T, TResult2 = never>(onfulfilled?: ((value: T) => TResult1 | Promise<TResult1>) | undefined | null, onrejected?: ((reason: any) => TResult2 | Promise<TResult2>) | undefined | null): Promise<TResult1 | TResult2>;"
}
 */
JsVar *jswrap_promise_then(JsVar *parent, JsVar *onFulfilled, JsVar *onRejected) {
  JsVar *nextPromBox = jspromise_create();
  if (!nextPromBox) return 0;
  JsVar * nextProm = jsvObjectGetChildIfExists(nextPromBox, JS_PROMISE_PROM_NAME);
  if (nextProm) {
    if (!jsvIsFunction(onFulfilled)) onFulfilled = 0;
    if (!jsvIsFunction(onRejected)) onRejected = 0;
    JsVar *state = jsvObjectGetChildIfExists(parent, JS_PROMISE_STATE_NAME);
    if (state) {
      int s = jsvGetIntegerAndUnLock(state);
      if (s == JS_PROMISE_STATE_PENDING) { 
        // Create reaction and attach to promise.
        _jswrap_promise_add_reaction(parent, nextPromBox, onFulfilled, true);
        _jswrap_promise_add_reaction(parent, nextPromBox, onRejected, false);
      } else {
        // case: Already resolved, fire Reaction.
        JsVar *resolveNowCallback = s == JS_PROMISE_STATE_FULFILLED ? onFulfilled : onRejected;
        JsVar *reaction = _jswrap_promise_new_reaction(nextPromBox,resolveNowCallback);
        if (reaction) {
          JsVar *value = jsvObjectGetChildIfExists(parent, JS_PROMISE_VALUE_NAME);
          // Already resolved, go straight to firing reaction.
          _jswrap_promise_queue_reaction(parent,reaction,value,false);
          if (value) jsvUnLock(value);
          jsvUnLock(reaction);
        }
      }
    } //state 
  }
  jsvUnLock(nextPromBox);
  return nextProm;
}
/*JSON{
  "type" : "method",
  "class" : "Promise",
  "name" : "catch",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_promise_catch",
  "params" : [
    ["onRejected","JsVar","A callback that is called when this promise is rejected"]
  ],
  "return" : ["JsVar","The original Promise"]
}
 */
JsVar *jswrap_promise_catch(JsVar *parent, JsVar *onRejected) {
  return jswrap_promise_then(parent,0,onRejected);
}

#endif // ESPR_NO_PROMISES
