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
#include "jswrap_promise.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrapper.h"

#define JS_PROMISE_THEN_NAME JS_HIDDEN_CHAR_STR"thn"
#define JS_PROMISE_CATCH_NAME JS_HIDDEN_CHAR_STR"cat"
#define JS_PROMISE_COUNT_NAME JS_HIDDEN_CHAR_STR"cnt"
#define JS_PROMISE_RESULT_NAME JS_HIDDEN_CHAR_STR"res"


/*

var p = new Promise( function(resolve) { resolve(1); });
p.then( function(value) {
        console.log("A"+value);  // 1
        return value + 1;
}).then( function(value) {
        console.log(value);  // 2
        return new Promise( function( resolve ) { resolve( 4 ); } );
}).then( function( value ) {
        console.log( value ); // 4
} );
p.then(function(value) {
        console.log("B"+value); // 1
});

produces:

A1
2
4

Should produce

A1
B1
2
4

Basically .then should return a new promise, rather than returning the same one.

_jswrap_promise_resolve should then execute every promise in the list.

 */

/*JSON{
  "type" : "class",
  "class" : "Promise",
  "ifndef" : "SAVE_ON_FLASH"
}
This is the built-in class for ES6 Promises
*/

void _jswrap_promise_queueresolve(JsVar *promise, JsVar *data);
void _jswrap_promise_add(JsVar *parent, JsVar *callback, const char *name);
void _jswrap_promise_resolve_or_reject(JsVar *promise, JsVar *data, JsVar *fn) {
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
  jsvObjectSetChild(promise, JS_PROMISE_THEN_NAME, 0); // remove 'resolve' and 'reject' handlers
  jsvObjectSetChild(promise, JS_PROMISE_CATCH_NAME, 0); // remove 'resolve' and 'reject' handlers
  JsVar *chainedPromise = jsvObjectGetChild(promise, "chain", 0);
  jsvObjectSetChild(promise, "chain", 0); // unlink chain
  if (chainedPromise) {
    JsVar *constr = jspGetConstructor(result);
    if (constr && (void*)constr->varData.native.ptr==(void*)jswrap_promise_constructor) {
      // if we were given a promise, loop its 'then' in here
      JsVar *fn = jsvNewNativeFunction((void (*)(void))_jswrap_promise_queueresolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
      if (fn) {
        jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, chainedPromise);
        _jswrap_promise_add(result, fn, JS_PROMISE_THEN_NAME);
        jsvUnLock(fn);
      }
    } else {
      _jswrap_promise_queueresolve(chainedPromise, result);
    }
    jsvUnLock(constr);
  }
  jsvUnLock2(result, chainedPromise);
}
void _jswrap_promise_resolve_or_reject_chain(JsVar *promise, JsVar *data, const char *eventName) {
  JsVar *fn = jsvObjectGetChild(promise, eventName, 0);
  // if we didn't have a catch, traverse the chain looking for one
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
  if (fn) _jswrap_promise_resolve_or_reject(promise, data, fn);
  jsvUnLock(fn);
}

void _jswrap_promise_resolve(JsVar *promise, JsVar *data) {
  _jswrap_promise_resolve_or_reject_chain(promise, data, JS_PROMISE_THEN_NAME);
}
void _jswrap_promise_queueresolve(JsVar *promise, JsVar *data) {
  JsVar *fn = jsvNewNativeFunction((void (*)(void))_jswrap_promise_resolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
  if (!fn) return;
  jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, promise);
  jsiQueueEvents(promise, fn, &data, 1);
  jsvUnLock(fn);
}

void _jswrap_promise_reject(JsVar *promise, JsVar *data) {
  _jswrap_promise_resolve_or_reject_chain(promise, data, JS_PROMISE_CATCH_NAME);
}
void _jswrap_promise_queuereject(JsVar *promise, JsVar *data) {
  JsVar *fn = jsvNewNativeFunction((void (*)(void))_jswrap_promise_reject, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
  if (!fn) return;
  jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, promise);
  jsiQueueEvents(promise, fn, &data, 1);
  jsvUnLock(fn);
}

void jswrap_promise_all_resolve(JsVar *promise, JsVar *data) {
  JsVarInt i = jsvGetIntegerAndUnLock(jsvObjectGetChild(promise, JS_PROMISE_COUNT_NAME, 0));
  JsVar *arr = jsvObjectGetChild(promise, JS_PROMISE_RESULT_NAME, 0);
  if (arr) {
    // we may have already rejected...
    if (jsvArrayPush(arr, data) == i) {
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
    jsvObjectSetChildAndUnLock(promise, JS_PROMISE_RESULT_NAME, 0);
    _jswrap_promise_queuereject(promise, data);
  }
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
  "return" : ["JsVar","A Promise"]
}
Create a new Promise. The executor function is executed immediately (before the constructor even returns)
and
 */
JsVar *jswrap_promise_constructor(JsVar *executor) {
  JsVar *obj = jspNewObject(0, "Promise");
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
    jsvUnLock(jspeFunctionCall(executor, 0, obj, false, 2, args));
    jsvUnLockMany(2, args);
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
  "return" : ["JsVar","A new Promise"]
}
Return a new promise that is resolved when all promises in the supplied
array are resolved.
*/
JsVar *jswrap_promise_all(JsVar *arr) {
  if (!jsvIsIterable(arr)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting something iterable, got %t", arr);
    return 0;
  }
  JsVar *promise = jspNewObject(0, "Promise");
  if (!promise) return 0;
  JsVar *resolve = jsvNewNativeFunction((void (*)(void))jswrap_promise_all_resolve, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
  JsVar *reject = jsvNewNativeFunction((void (*)(void))jswrap_promise_all_reject, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
  if (resolve && reject) {
    jsvObjectSetChild(resolve, JSPARSE_FUNCTION_THIS_NAME, promise);
    jsvObjectSetChild(reject, JSPARSE_FUNCTION_THIS_NAME, promise);
    int promises = 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, arr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *p = jsvObjectIteratorGetValue(&it);
      jsvUnLock(jswrap_promise_then(p, resolve, reject));
      jsvUnLock(p);
      promises++;
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);

    jsvObjectSetChildAndUnLock(promise, JS_PROMISE_COUNT_NAME, jsvNewFromInteger(promises));
    jsvObjectSetChildAndUnLock(promise, JS_PROMISE_RESULT_NAME, jsvNewEmptyArray());
  }
  jsvUnLock2(resolve, reject);
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
  "return" : ["JsVar","A new Promise"]
}
Return a new promise that is already resolved (at idle it'll
call `.then`)
*/
JsVar *jswrap_promise_resolve(JsVar *data) {
  JsVar *promise = jspNewObject(0, "Promise");
  if (!promise) return 0;
  _jswrap_promise_queueresolve(promise, data);
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
Return a new promise that is already rejected (at idle it'll
call `.catch`)
*/
JsVar *jswrap_promise_reject(JsVar *data) {
  JsVar *promise = jspNewObject(0, "Promise");
  if (!promise) return 0;
  _jswrap_promise_queuereject(promise, data);
  return promise;
}

void _jswrap_promise_add(JsVar *parent, JsVar *callback, const char *name) {
  if (!jsvIsFunction(callback)) {
    jsExceptionHere(JSET_TYPEERROR, "Callback must be a function, got %t", callback);
    return;
  }
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
    ["onRejected","JsVar","A callback that is called when this promise is rejected (or nothing)"]
  ],
  "return" : ["JsVar","The original Promise"]
}
 */
JsVar *jswrap_promise_then(JsVar *parent, JsVar *onFulfilled, JsVar *onRejected) {
  _jswrap_promise_add(parent, onFulfilled, JS_PROMISE_THEN_NAME);
  if (onRejected)
    _jswrap_promise_add(parent, onRejected, JS_PROMISE_CATCH_NAME);
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
  _jswrap_promise_add(parent, onRejected, JS_PROMISE_CATCH_NAME);
  return jswrap_promise_get_chained_promise(parent);
}
