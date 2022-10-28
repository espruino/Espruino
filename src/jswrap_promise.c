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

#include "jsutils.h"

#ifndef ESPR_NO_PROMISES

#include "jswrap_promise.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrapper.h"

#define JS_PROMISE_THEN_NAME JS_HIDDEN_CHAR_STR"thn"
#define JS_PROMISE_CATCH_NAME JS_HIDDEN_CHAR_STR"cat"
#define JS_PROMISE_REMAINING_NAME JS_HIDDEN_CHAR_STR"left"
#define JS_PROMISE_RESULT_NAME JS_HIDDEN_CHAR_STR"res"
#define JS_PROMISE_RESOLVED_NAME "resolved"


/*JSON{
  "type" : "class",
  "class" : "Promise",
  "typescript": "Promise<T>",
  "ifndef" : "SAVE_ON_FLASH"
}
This is the built-in class for ES6 Promises
*/

void _jswrap_promise_queueresolve(JsVar *promise, JsVar *data);
void _jswrap_promise_queuereject(JsVar *promise, JsVar *data);
void _jswrap_promise_add(JsVar *parent, JsVar *callback, bool resolve);

bool _jswrap_promise_is_promise(JsVar *promise) {
  JsVar *constr = jspGetConstructor(promise);
  bool isPromise = constr && (void*)constr->varData.native.ptr==(void*)jswrap_promise_constructor;
  jsvUnLock(constr);
  return isPromise;
}


void _jswrap_promise_resolve_or_reject(JsVar *promise, JsVar *data, JsVar *fn) {
  // remove any existing handlers since we already have them in `fn`
  // If while we're iterating below a function re-adds to the chain then
  // we can execute that later
  // https://github.com/espruino/Espruino/issues/894#issuecomment-402553934
  jsvObjectRemoveChild(promise, JS_PROMISE_THEN_NAME); // remove 'resolve' and 'reject' handlers
  jsvObjectRemoveChild(promise, JS_PROMISE_CATCH_NAME); // remove 'resolve' and 'reject' handlers
  JsVar *chainedPromise = jsvObjectGetChild(promise, "chain", 0);
  jsvObjectRemoveChild(promise, "chain"); // unlink chain
  // execute handlers from `fn`
  JsVar *result = 0;
  if (jsvIsArray(fn)) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, fn);
    bool first = true;
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *f = jsvObjectIteratorGetValue(&it);
      JsVar *v = jspExecuteFunction(f, promise, 1, &data);
      if (first) {
        first = false;
        result = v;
      } else jsvUnLock(v);
      jsvUnLock(f);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
  } else if (fn) {
    result = jspExecuteFunction(fn, promise, 1, &data);
  }

  JsVar *exception = jspGetException();
  if (exception) {
    _jswrap_promise_queuereject(chainedPromise, exception);
    jsvUnLock3(exception, result, chainedPromise);
    return;
  }

  if (chainedPromise) {
    if (_jswrap_promise_is_promise(result)) {
      // if we were given a promise, loop its 'then' in here
      JsVar *fnres = jsvNewNativeFunction((void (*)(void))_jswrap_promise_queueresolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
      JsVar *fnrej = jsvNewNativeFunction((void (*)(void))_jswrap_promise_queuereject, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
      if (fnres && fnrej) {
        jsvObjectSetChild(fnres, JSPARSE_FUNCTION_THIS_NAME, chainedPromise); // bind 'this'
        jsvObjectSetChild(fnrej, JSPARSE_FUNCTION_THIS_NAME, chainedPromise); // bind 'this'
        _jswrap_promise_add(result, fnres, true);
        _jswrap_promise_add(result, fnrej, false);
      }
      jsvUnLock2(fnres,fnrej);
    } else {
      _jswrap_promise_queueresolve(chainedPromise, result);
    }
  }
  jsvUnLock2(result, chainedPromise);
}
void _jswrap_promise_resolve_or_reject_chain(JsVar *promise, JsVar *data, bool resolve) {
  const char *eventName = resolve ? JS_PROMISE_THEN_NAME : JS_PROMISE_CATCH_NAME;
  // if we didn't have a catch, traverse the chain looking for one
  JsVar *fn = jsvObjectGetChild(promise, eventName, 0);
  if (!fn) {
    JsVar *chainedPromise = jsvObjectGetChild(promise, "chain", 0);
    while (chainedPromise) {
      fn = jsvObjectGetChild(chainedPromise, eventName, 0);
      if (fn) {
        _jswrap_promise_resolve_or_reject(chainedPromise, data, fn);
        jsvUnLock2(fn, chainedPromise);
        return;
      }
      JsVar *n = jsvObjectGetChild(chainedPromise, "chain", 0);
      jsvUnLock(chainedPromise);
      chainedPromise = n;
    }
  }
  if (resolve)
    jsvObjectSetChild(promise, JS_PROMISE_RESOLVED_NAME, data);
  if (fn) {
    _jswrap_promise_resolve_or_reject(promise, data, fn);
    jsvUnLock(fn);
  } else if (!resolve) {
    JsVar *previouslyResolved = jsvFindChildFromString(promise, JS_PROMISE_RESOLVED_NAME, false);
    if (!previouslyResolved)
      jsExceptionHere(JSET_ERROR, "Unhandled promise rejection: %v", data);
    jsvUnLock(previouslyResolved);
  }
}

void _jswrap_promise_resolve(JsVar *promise, JsVar *data) {
  _jswrap_promise_resolve_or_reject_chain(promise, data, true);
}
void _jswrap_promise_queueresolve(JsVar *promise, JsVar *data) {
  JsVar *fn = jsvNewNativeFunction((void (*)(void))_jswrap_promise_resolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
  if (!fn) return;
  jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, promise); // bind 'this'
  jsiQueueEvents(promise, fn, &data, 1);
  jsvUnLock(fn);
}

void _jswrap_promise_reject(JsVar *promise, JsVar *data) {
  _jswrap_promise_resolve_or_reject_chain(promise, data, false);
}
void _jswrap_promise_queuereject(JsVar *promise, JsVar *data) {
  JsVar *fn = jsvNewNativeFunction((void (*)(void))_jswrap_promise_reject, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
  if (!fn) return;
  jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, promise); // bind 'this'
  jsiQueueEvents(promise, fn, &data, 1);
  jsvUnLock(fn);
}

void jswrap_promise_all_resolve(JsVar *promise, JsVarInt index, JsVar *data) {
  JsVarInt remaining = jsvGetIntegerAndUnLock(jsvObjectGetChild(promise, JS_PROMISE_REMAINING_NAME, 0));
  JsVar *arr = jsvObjectGetChild(promise, JS_PROMISE_RESULT_NAME, 0);
  if (arr) {
    // set the result
    jsvSetArrayItem(arr, index, data);
    // Update remaining list
    remaining--;
    jsvObjectSetChildAndUnLock(promise, JS_PROMISE_REMAINING_NAME, jsvNewFromInteger(remaining));
    if (remaining==0) {
      _jswrap_promise_queueresolve(promise, arr);
    }
    jsvUnLock(arr);
  }
}
void jswrap_promise_all_reject(JsVar *promise, JsVar *data) {
  JsVar *arr = jsvObjectGetChild(promise, JS_PROMISE_RESULT_NAME, 0);
  if (arr) {
    // if not rejected before
    jsvUnLock(arr);
    jsvObjectRemoveChild(promise, JS_PROMISE_RESULT_NAME);
    _jswrap_promise_queuereject(promise, data);
  }
}

/// Create a new promise
JsVar *jspromise_create() {
  return jspNewObject(0, "Promise");
}

/// Resolve the given promise
void jspromise_resolve(JsVar *promise, JsVar *data) {
  _jswrap_promise_queueresolve(promise, data);
}

/// Reject the given promise
void jspromise_reject(JsVar *promise, JsVar *data) {
  _jswrap_promise_queuereject(promise, data);
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
  JsVar *obj = jspromise_create();
  if (obj) {
    // create resolve and reject
    JsVar *args[2] = {
        jsvNewNativeFunction((void (*)(void))_jswrap_promise_queueresolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS)),
        jsvNewNativeFunction((void (*)(void))_jswrap_promise_queuereject, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS))
    };
    // bind 'this' to functions
    if (args[0]) jsvObjectSetChild(args[0], JSPARSE_FUNCTION_THIS_NAME, obj);
    if (args[1]) jsvObjectSetChild(args[1], JSPARSE_FUNCTION_THIS_NAME, obj);
    // call the executor
    JsExecFlags oldExecute = execInfo.execute;
    if (executor) jsvUnLock(jspeFunctionCall(executor, 0, obj, false, 2, args));
    execInfo.execute = oldExecute;
    jsvUnLockMany(2, args);
    JsVar *exception = jspGetException();
    if (exception) {
      _jswrap_promise_queuereject(obj, exception);
      jsvUnLock(exception);
    }
  }
  return obj;
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
  JsVar *promise = jspNewObject(0, "Promise");
  if (!promise) return 0;
  JsVar *reject = jsvNewNativeFunction((void (*)(void))jswrap_promise_all_reject, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
  if (reject) {
    jsvObjectSetChild(reject, JSPARSE_FUNCTION_THIS_NAME, promise); // bind 'this'
    JsVar *promiseResults = jsvNewEmptyArray();
    int promiseIndex = 0;
    int promisesComplete = 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, arr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *p = jsvObjectIteratorGetValue(&it);
      if (_jswrap_promise_is_promise(p)) {
        JsVar *resolve = jsvNewNativeFunction((void (*)(void))jswrap_promise_all_resolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_INT32<<JSWAT_BITS)|(JSWAT_JSVAR<<(JSWAT_BITS*2)));
        // bind the index variable
        JsVar *indexVar = jsvNewFromInteger(promiseIndex);
        jsvAddFunctionParameter(resolve, 0, indexVar);
        jsvUnLock(indexVar);
        jsvObjectSetChild(resolve, JSPARSE_FUNCTION_THIS_NAME, promise); // bind 'this'
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

    jsvObjectSetChildAndUnLock(promise, JS_PROMISE_REMAINING_NAME, jsvNewFromInteger(promiseIndex-promisesComplete));
    jsvObjectSetChildAndUnLock(promise, JS_PROMISE_RESULT_NAME, promiseResults);
  }
  jsvUnLock(reject);
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
    JsVar *then = jsvObjectGetChild(data,"then",0);
    if (jsvIsFunction(then))
      promise = jswrap_promise_constructor(then);
    jsvUnLock(then);
    if (promise) return promise;
  }
  // otherwise the returned promise will be fulfilled with the value.
  promise = jspromise_create();
  if (!promise) return 0;
  jspromise_resolve(promise, data);
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
JsVar *jswrap_promise_reject(JsVar *data) {
  JsVar *promise = jspromise_create();
  if (!promise) return 0;
  jspromise_reject(promise, data);
  return promise;
}

void _jswrap_promise_add(JsVar *parent, JsVar *callback, bool resolve) {
  if (!jsvIsFunction(callback)) {
    jsExceptionHere(JSET_TYPEERROR, "Callback must be a function, got %t", callback);
    return;
  }

  bool resolveImmediately = false;
  JsVar *resolveImmediatelyValue = 0;

  if (resolve) {
    // Check to see if promise has already been resolved
    /* Note: we use jsvFindChildFromString not ObjectGetChild so we get the name.
     * If we didn't then we wouldn't know if it was resolved, but with undefined */
    JsVar *resolved = jsvFindChildFromString(parent, JS_PROMISE_RESOLVED_NAME, 0);
    if (resolved) {
      resolveImmediately = true;
      resolveImmediatelyValue = jsvSkipNameAndUnLock(resolved);
    }
  }


  const char *name = resolve ? JS_PROMISE_THEN_NAME : JS_PROMISE_CATCH_NAME;
  JsVar *c = jsvObjectGetChild(parent, name, 0);
  if (!c) {
    jsvObjectSetChild(parent, name, callback);
  } else {
    if (jsvIsArray(c)) {
      jsvArrayPush(c, callback);
    } else {
      JsVar *fns[2] = {c,callback};
      JsVar *arr = jsvNewArray(fns, 2);
      jsvObjectSetChild(parent, name, arr);
      jsvUnLock(arr);
    }
    jsvUnLock(c);
  }

  if (resolveImmediately) { // If so, queue a resolve event
    _jswrap_promise_queueresolve(parent, resolveImmediatelyValue);
    jsvUnLock(resolveImmediatelyValue);
  }
}

static JsVar *jswrap_promise_get_chained_promise(JsVar *parent) {
  JsVar *chainedPromise = jsvObjectGetChild(parent, "chain", 0);
  if (!chainedPromise) {
    chainedPromise = jspNewObject(0, "Promise");
    jsvObjectSetChild(parent, "chain", chainedPromise);
  }
  return chainedPromise;
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
  _jswrap_promise_add(parent, onFulfilled, true);
  if (onRejected)
    _jswrap_promise_add(parent, onRejected, false);
  return jswrap_promise_get_chained_promise(parent);
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
  _jswrap_promise_add(parent, onRejected, false);
  return jswrap_promise_get_chained_promise(parent);
}
#endif // ESPR_NO_PROMISES
