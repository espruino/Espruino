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
 * See https://github.com/espruino/Espruino/pull/2454 for better understanding.
 */


#include "jsutils.h"

#if ESPR_NO_PROMISES!=1

#include "jswrap_promise.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrapper.h"

// field names
#define JS_PROMISE_STATE_NAME JS_HIDDEN_CHAR_STR"state" ///< Promise: the state of the promise, one of JsPromiseStates(below)
#define JS_PROMISE_THEN_NAME JS_HIDDEN_CHAR_STR"thn" ///< Promise: array of reactions for resolving
#define JS_PROMISE_CATCH_NAME JS_HIDDEN_CHAR_STR"cat" ///< Promise: array of reactions for rejecting
#define JS_PROMISE_REMAINING_NAME JS_HIDDEN_CHAR_STR"left" ///< Promise: int used in Promise.all to wait for as many promises are are needed
#define JS_PROMISE_RESULT_NAME JS_HIDDEN_CHAR_STR"res" ///< Promise: array used in Promise.all for results
#define JS_PROMISE_VALUE_NAME JS_HIDDEN_CHAR_STR"value" ///< Promise: the value of this promise when it's resolved

#define JS_PROMISE_ISRESOLVED_NAME JS_HIDDEN_CHAR_STR"resolved" // Prombox: boolean set when the Promise is resolved
#define JS_PROMISE_PROM_NAME JS_HIDDEN_CHAR_STR"prom" ///< Prombox: link to promise



/// Used in JS_PROMISE_STATE_NAME
typedef enum {
    JS_PROMISE_STATE_PENDING,
    JS_PROMISE_STATE_REJECTED,
    JS_PROMISE_STATE_FULFILLED
} JsPromiseStates;

/*JSON{
  "type" : "class",
  "class" : "Promise",
  "typescript": "Promise<T>",
  "ifndef" : "SAVE_ON_FLASH"
}
This is the built-in class for ES6 Promises
*/

static void _jswrap_prombox_resolve(JsVar *prombox, JsVar *data);
static void _jswrap_prombox_reject(JsVar *prombox, JsVar *data);
static void _jswrap_prombox_resolve_or_reject(JsVar *prombox, JsVar *data, bool resolving);
static JsVar *jspromise_create_prombox(JsVar ** promise);

static bool _jswrap_promise_is_promise(JsVar *promise) {
  JsVar *constr = jspGetConstructor(promise);
  bool isPromise = constr && (void*)constr->varData.native.ptr==(void*)jswrap_promise_constructor;
  jsvUnLock(constr);
  return isPromise;
}

static JsVar *_jswrap_promise_native_with_prombox(void (*fnPtr)(JsVar*,JsVar*), JsVar *prombox) {
  JsVar *fn = jsvNewNativeFunction((void (*)(void))fnPtr, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
  jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, prombox);
  return fn;
}

// A single reaction chain - this is recursive until a promise is returned or chain ends. data can be undefined/0
static void _jswrap_promise_reaction_call(JsVar *promise, JsVar *reaction, JsVar *data, JsVar *isThen) {
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
        execInfo.execute = oldExecute; // if there were errors executing the function, get rid of them
        JsVar *exception = jspGetException(); // if there was an exception, reject with it
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
      _jswrap_prombox_resolve_or_reject(nextPromBox, retVal, !threw); // resolve if threw=false

      if (callback) jsvUnLock(retVal);
      jsvUnLock(nextProm);
    }
    jsvUnLock(nextPromBox);
  }
}
// Value can be undefined/0
static void _jswrap_promise_queue_reaction(JsVar *promise, JsVar *reaction, JsVar *value, bool isThenCb) {
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
static void _jswrap_promise_seal(JsVar *promise, JsVar *data,bool resolving) {
  jsvObjectSetChildAndUnLock(promise, JS_PROMISE_STATE_NAME, jsvNewFromInteger(resolving ? JS_PROMISE_STATE_FULFILLED : JS_PROMISE_STATE_REJECTED));

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
    } else assert(!reactions);
    jsvUnLock(reactions);
  }
}

static void _jswrap_prombox_resolve_or_reject(JsVar *prombox, JsVar *data, bool resolving) {
  bool isResolved = jsvObjectGetBoolChild(prombox, JS_PROMISE_ISRESOLVED_NAME);
  if (isResolved)
    return;

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
      jsvUnLock2(promise, then);
      return;
    }

    JsVar *prombox = jsvNewObject();
    JsVar *jsResolve = _jswrap_promise_native_with_prombox(_jswrap_prombox_resolve, prombox);
    JsVar *jsReject = _jswrap_promise_native_with_prombox(_jswrap_prombox_reject, prombox);
    if (prombox) {
      jsvObjectSetChild(prombox, JS_PROMISE_PROM_NAME, promise);
      jsvObjectSetChildAndUnLock(prombox,JS_PROMISE_ISRESOLVED_NAME,jsvNewFromBool(false));

      if (isThenable) {
        JsVar *args[2] = {jsResolve,jsReject};
        JsExecFlags oldExecute = execInfo.execute;
        jsvUnLock(jspeFunctionCall(then, 0, data, false, 2, args));
        execInfo.execute = oldExecute; // if there were errors executing the function, get rid of them
        JsVar *exception = jspGetException(); // if there was an exception, reject with it
        if (exception) {
          _jswrap_prombox_reject(prombox, exception);
          jsvUnLock(exception);
        }
      } else {
        jsvUnLock(jswrap_promise_then(data,jsResolve,jsReject));
      }
      jsvUnLock3(jsResolve,jsReject,prombox);
    }
    jsvUnLock2(promise, then);
  }
}

static void _jswrap_prombox_resolve(JsVar *prombox, JsVar *data) {
  _jswrap_prombox_resolve_or_reject(prombox, data, true);
}

static void _jswrap_prombox_reject(JsVar *prombox, JsVar *data) {
  _jswrap_prombox_resolve_or_reject(prombox, data, false);
}


static void _jswrap_prombox_queueresolve_or_reject(JsVar *prombox, JsVar *data, bool isResolve) {
  JsVar *fn = _jswrap_promise_native_with_prombox(isResolve ? _jswrap_prombox_resolve : _jswrap_prombox_reject, prombox);
  if (!fn) return;
  jsiQueueEvents(prombox, fn, &data, 1);
  jsvUnLock(fn);
}

static void _jswrap_prombox_queueresolve(JsVar *prombox, JsVar *data) {
  _jswrap_prombox_queueresolve_or_reject(prombox, data, true);
}

static void _jswrap_prombox_queuereject(JsVar *prombox, JsVar *data) {
  _jswrap_prombox_queueresolve_or_reject(prombox, data, false);
}


static void jspromise_resolve_or_reject(JsVar *promise, JsVar *data, bool isResolve) {
  // give the promise a prombox - ensure resolve once only in c.
  // allows us to accept a promise instead of a prombox.
  JsVar *prombox = jsvNewObject();
  if (!prombox) {
    return;
  }
  jsvObjectSetChild(prombox, JS_PROMISE_PROM_NAME, promise);
  //jsvObjectSetChildAndUnLock(prombox,JS_PROMISE_ISRESOLVED_NAME,jsvNewFromBool(false)); // not setting will still; return false
  _jswrap_prombox_queueresolve_or_reject(prombox, data, isResolve);
  jsvUnLock(prombox);
}

void jspromise_resolve(JsVar *promise, JsVar *data) {
  jspromise_resolve_or_reject(promise, data, true);
}

void jspromise_reject(JsVar *promise, JsVar *data) {
  jspromise_resolve_or_reject(promise, data, false);
}

/// Create a new promise
JsVar *jspromise_create() {
  return jspNewObject(0, "Promise");
}

/// Create a promise and put it inside a promise box (which we return in the argument)
static JsVar *jspromise_create_prombox(JsVar ** promise) {
  JsVar *p = jspromise_create();
  if (!p) return 0;
  JsVar *box = jsvNewObject();
  if (!box) {
    jsvUnLock(p);
    return 0;
  }
  jsvObjectSetChildAndUnLock(p, JS_PROMISE_STATE_NAME, jsvNewFromInteger(JS_PROMISE_STATE_PENDING));
  jsvObjectSetChildAndUnLock(box, JS_PROMISE_PROM_NAME, p);
  //jsvObjectSetChildAndUnLock(box,JS_PROMISE_ISRESOLVED_NAME,jsvNewFromBool(false)); // not setting will still; return false

  *promise = p;
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
  JsVar * promise;
  JsVar *promBox = jspromise_create_prombox(&promise);
  if (!promBox) return 0;
  if (promise) {
    //this=prombox, data
    JsVar *resolve = _jswrap_promise_native_with_prombox(_jswrap_prombox_queueresolve, promBox);
    JsVar *reject = _jswrap_promise_native_with_prombox(_jswrap_prombox_queuereject, promBox);

    JsVar *args[2] = {resolve,reject};
    // call the executor
    // emulates try
    JsExecFlags oldExecute = execInfo.execute;
    jsvUnLock(jspeFunctionCall(executor, 0, promise, false, 2, args));
    execInfo.execute = oldExecute; // if there were errors executing the function, get rid of them
    // If we got an exception while executing, queue a rejection
    JsVar *exception = jspGetException();
    if (exception) {
      _jswrap_prombox_queuereject(promBox, exception);
      jsvUnLock(exception);
    }
    jsvUnLock2(resolve,reject);
  }
  jsvUnLock(promBox);
  return jsvLockAgain(promise);
}

static void _jswrap_prombox_all_resolve(JsVar *prombox, JsVar *index, JsVar *data) {
  JsVar * promise = jsvObjectGetChildIfExists(prombox, JS_PROMISE_PROM_NAME);
  if (promise) {
    JsVarInt remaining = jsvObjectGetIntegerChild(promise, JS_PROMISE_REMAINING_NAME);
    JsVar *arr = jsvObjectGetChildIfExists(promise, JS_PROMISE_RESULT_NAME);
    if (arr) {
      // set the result
      jsvSetArrayItem(arr, jsvGetInteger(index), data);
      // Update remaining list
      remaining--;
      jsvObjectSetChildAndUnLock(promise, JS_PROMISE_REMAINING_NAME, jsvNewFromInteger(remaining));
      if (remaining==0) {
        _jswrap_prombox_queueresolve(prombox, arr);
      }
      jsvUnLock(arr);
    }
    jsvUnLock(promise);
  }
}

static void _jswrap_prombox_all_reject(JsVar *prombox, JsVar *data) {
  JsVar * promise = jsvObjectGetChildIfExists(prombox, JS_PROMISE_PROM_NAME);
  if (promise) {
    JsVar *arr = jsvObjectGetChildIfExists(promise, JS_PROMISE_RESULT_NAME);
    if (arr) {
      // if not rejected before
      jsvUnLock(arr);
      jsvObjectRemoveChild(promise, JS_PROMISE_RESULT_NAME);
      _jswrap_prombox_queuereject(prombox, data);
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
  JsVar * promise;
  JsVar *promBox = jspromise_create_prombox(&promise);
  if (!promBox) return 0;
  if (promise) {
    JsVar *reject = _jswrap_promise_native_with_prombox(_jswrap_prombox_all_reject, promBox);
    if (!reject) return jsvLockAgainSafe(promise); // out of memory error
    JsVar *promiseResults = jsvNewEmptyArray();
    int promiseIndex = 0;
    int promisesComplete = 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, arr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *p = jsvObjectIteratorGetValue(&it);
      if (_jswrap_promise_is_promise(p)) {
        JsVar *resolve = jsvNewNativeFunction((void (*)(void))_jswrap_prombox_all_resolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS)|(JSWAT_JSVAR<<(JSWAT_BITS*2)));
        // NOTE: we use (this,JsVar,JsVar) rather than an int to avoid #2377 on emscripten, since that argspec is already used for forEach/otehrs
        // bind the index variable
        JsVar *indexVar = jsvNewFromInteger(promiseIndex);
        jsvAddFunctionParameter(resolve, 0, indexVar);
        jsvObjectSetChild(resolve, JSPARSE_FUNCTION_THIS_NAME, promBox); // bind 'this'
        jsvUnLock3(jswrap_promise_then(p, resolve, reject), resolve, indexVar);
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
      promise = jswrap_promise_resolve(promiseResults);
      jsvUnLock2(promise, promiseResults);
    } else { // return our new promise that will resolve when everything is done
      jsvObjectSetChildAndUnLock(promise, JS_PROMISE_REMAINING_NAME, jsvNewFromInteger(promiseIndex-promisesComplete));
      jsvObjectSetChildAndUnLock(promise, JS_PROMISE_RESULT_NAME, promiseResults);
    }
    jsvUnLock2(reject,promBox);
  }
  return jsvLockAgainSafe(promise);
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
  JsVar *promBox = jspromise_create_prombox(&promise);
  if (!promBox) return 0;
  if (promise) {
    _jswrap_prombox_queueresolve(promBox, data);
  }
  jsvUnLock(promBox);
  return jsvLockAgainSafe(promise);
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
  JsVar * promise;
  JsVar *promBox = jspromise_create_prombox(&promise);
  if (!promBox) return 0;
  if (promise) {
    _jswrap_prombox_queuereject(promBox,data);
  }
  jsvUnLock(promBox);
  return jsvLockAgainSafe(promise);
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
  if (jsvIsArray(c)) {
    jsvArrayPush(c, reaction);
  } else {
    jsvUnLock(c);
    jsvObjectSetChildAndUnLock(parent, name, jsvNewArray(&reaction, 1));
  }
  jsvUnLock2(c,reaction);
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
  JsVar *nextProm;
  JsVar *nextPromBox = jspromise_create_prombox(&nextProm);
  if (!nextPromBox) return 0;
  if (nextProm) {

    if (!jsvIsFunction(onFulfilled)) onFulfilled = 0;
    if (!jsvIsFunction(onRejected)) onRejected = 0;
    int s = jsvObjectGetIntegerChild(parent, JS_PROMISE_STATE_NAME);
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

        jsvUnLock2(value, reaction);
      }
    }
  }
  jsvUnLock(nextPromBox);
  return jsvLockAgainSafe(nextProm);
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
