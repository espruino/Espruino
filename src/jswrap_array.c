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
 * This file is designed to be parsed during the build process
 *
 * JavaScript methods for Arrays
 * ----------------------------------------------------------------------------
 */
#include "jswrap_array.h"
#include "jswrap_functions.h" // jswrap_isNaN
#include "jsparse.h"

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


/*JSON{
  "type" : "class",
  "class" : "Array",
  "check" : "jsvIsArray(var)",
  "typescript" : "Array<T>"
}
This is the built-in JavaScript class for arrays.

Arrays can be defined with ```[]```, ```new Array()```, or ```new
Array(length)```
 */

/*JSON{
  "type" : "constructor",
  "class" : "Array",
  "name" : "Array",
  "generate" : "jswrap_array_constructor",
  "params" : [
    ["args","JsVarArray","The length of the array OR any number of items to add to the array"]
  ],
  "return" : ["JsVar","An Array"],
  "typescript" : [
    "new(arrayLength?: number): any[];",
    "new<T>(arrayLength: number): T[];",
    "new<T>(...items: T[]): T[];",
    "(arrayLength?: number): any[];",
    "<T>(arrayLength: number): T[];",
    "<T>(...items: T[]): T[];"
  ]
}
Create an Array. Either give it one integer argument (>=0) which is the length
of the array, or any number of arguments
 */
JsVar *jswrap_array_constructor(JsVar *args) {
  assert(args);
  if (jsvGetArrayLength(args)==1) {
    JsVar *firstArg = jsvSkipNameAndUnLock(jsvGetArrayItem(args,0));
    if (jsvIsNumeric(firstArg)) {
      JsVarFloat f = jsvGetFloat(firstArg);
      JsVarInt count = jsvGetInteger(firstArg);
      jsvUnLock(firstArg);
      if (f!=count || count<0) {
        jsExceptionHere(JSET_ERROR, "Invalid array length");
        return 0;
      } else {
        JsVar *arr = jsvNewEmptyArray();
        if (!arr) return 0; // out of memory
        jsvSetArrayLength(arr, count, false);
        return arr;
      } 
    } else {
      jsvUnLock(firstArg); 
    }
  }
  // Otherwise, we just return the array!
  return jsvLockAgain(args);
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "toString",
  "generate" : "jswrap_object_toString",
  "params" : [
    ["radix","JsVar","unused"]
  ],
  "return" : ["JsVar","A String representing the array"],
  "typescript" : "toString(): string;"
}
Convert the Array to a string
 */

/*JSON{
  "type" : "property",
  "class" : "Array",
  "name" : "length",
  "generate" : "jswrap_object_length",
  "return" : ["JsVar","The length of the array"],
  "typescript" : "length: number;"
}
Find the length of the array
 */

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "indexOf",
  "generate" : "jswrap_array_indexOf",
  "params" : [
    ["value","JsVar","The value to check for"],
    ["startIndex","int","(optional) the index to search from, or 0 if not specified"]
  ],
  "return" : ["JsVar","the index of the value in the array, or -1"],
  "typescript" : "indexOf(value: T, startIndex?: number): number;"
}
Return the index of the value in the array, or -1
 */
JsVar *jswrap_array_indexOf(JsVar *parent, JsVar *value, JsVarInt startIdx) {
  JsVar *idxName = jsvGetIndexOfFull(parent, value, false/*not exact*/, true/*integer indices only*/, startIdx);
  // but this is the name - we must turn it into a var
  if (idxName == 0) return jsvNewFromInteger(-1); // not found!
  return jsvNewFromInteger(jsvGetIntegerAndUnLock(idxName));
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "includes",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_array_includes",
  "params" : [
    ["value","JsVar","The value to check for"],
    ["startIndex","int","(optional) the index to search from, or 0 if not specified"]
  ],
  "return" : ["bool","`true` if the array includes the value, `false` otherwise"],
  "typescript" : "includes(value: T, startIndex?: number): boolean;"
}
Return `true` if the array includes the value, `false` otherwise
 */
bool jswrap_array_includes(JsVar *arr, JsVar *value, JsVarInt startIdx) {
  if (startIdx<0) startIdx+=jsvGetLength(arr);
  if (startIdx<0) startIdx=0;
  bool isNaN = jsvIsFloat(value) && isnan(jsvGetFloat(value));
  if (!jsvIsIterable(arr)) return 0;
  JsvIterator it;
  jsvIteratorNew(&it, arr, JSIF_DEFINED_ARRAY_ElEMENTS);
  while (jsvIteratorHasElement(&it)) {
    JsVar *childIndex = jsvIteratorGetKey(&it);
    if (jsvIsInt(childIndex) && jsvGetInteger(childIndex)>=startIdx) {
      JsVar *childValue = jsvIteratorGetValue(&it);
      if (childValue==value ||
          jsvMathsOpTypeEqual(childValue, value) ||
          (isNaN && jsvIsFloat(childValue) && isnan(jsvGetFloat(childValue)))) {
        jsvUnLock2(childIndex,childValue);
        jsvIteratorFree(&it);
        return true;
      }
      jsvUnLock(childValue);
    }
    jsvUnLock(childIndex);
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);
  return false;
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "join",
  "generate" : "jswrap_array_join",
  "params" : [
    ["separator","JsVar","The separator"]
  ],
  "return" : ["JsVar","A String representing the Joined array"],
  "typescript" : "join(separator?: string): string;"
}
Join all elements of this array together into one string, using 'separator'
between them. e.g. ```[1,2,3].join(' ')=='1 2 3'```
 */
JsVar *jswrap_array_join(JsVar *parent, JsVar *filler) {
  if (!jsvIsIterable(parent)) return 0;
  if (jsvIsUndefined(filler))
    filler = jsvNewFromString(","); // the default it seems
  else
    filler = jsvAsString(filler);
  if (!filler) return 0; // out of memory
  JsVar *str = jsvArrayJoin(parent, filler, true/*ignoreNull*/);
  jsvUnLock(filler);
  return str;
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "push",
  "generate" : "jswrap_array_push",
  "params" : [
    ["arguments","JsVarArray","One or more arguments to add"]
  ],
  "return" : ["int","The new size of the array"],
  "typescript" : "push(...arguments: T[]): number;"
}
Push a new value onto the end of this array'

This is the opposite of `[1,2,3].unshift(0)`, which adds one or more elements to
the beginning of the array.
 */
JsVarInt jswrap_array_push(JsVar *parent, JsVar *args) {
  if (!jsvIsArray(parent)) return -1;
  JsVarInt len = -1;
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, args);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *el = jsvObjectIteratorGetValue(&it);
    len = jsvArrayPush(parent, el);
    jsvUnLock(el);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  if (len<0) 
    len = jsvGetArrayLength(parent);
  return len;
}


/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "pop",
  "generate_full" : "jsvSkipNameAndUnLock(jsvArrayPop(parent))",
  "return" : ["JsVar","The value that is popped off"],
  "typescript" : "pop(): T | undefined;"
}
Remove and return the value on the end of this array.

This is the opposite of `[1,2,3].shift()`, which removes an element from the
beginning of the array.
 */

/// return types for _jswrap_array_iterate_with_callback
typedef enum {
  RETURN_BOOL,
  RETURN_ARRAY,  // an array of elements
  RETURN_ARRAY_ELEMENT,
  RETURN_ARRAY_INDEX
} AIWCReturnType;

/// General purpose looping function - re-use as much as possible
static JsVar *_jswrap_array_iterate_with_callback(
    const char *name,  //< use this in error messages
    JsVar *parent,     //< stuff to iterate over
    JsVar *funcVar,    //< function to be called for each array element
    JsVar *thisVar,    //< 'this' when executing funcVar
    AIWCReturnType returnType, // what does this fn return?
    bool isBoolCallback, //< the callback function returns a boolean
    bool expectedValue   //< the expected value returned by the callback
    ) {
  if (!jsvIsIterable(parent)) {
    jsExceptionHere(JSET_ERROR, "Array.%s can only be called on something iterable", name);
    return 0;
  }
  if (!jsvIsFunction(funcVar)) {
    jsExceptionHere(JSET_ERROR, "Array.%s's first argument should be a function", name);
    return 0;
  }
  if (!jsvIsUndefined(thisVar) && !jsvIsObject(thisVar)) {
    jsExceptionHere(JSET_ERROR, "Array.%s's second argument should be undefined, or an object", name);
    return 0;
  }
  JsVar *result = 0;
  if (returnType == RETURN_ARRAY)
    result = jsvNewEmptyArray();
  bool isDone = false;
  if (result || returnType!=RETURN_ARRAY) {
    JsvIterator it;
    jsvIteratorNew(&it, parent, JSIF_DEFINED_ARRAY_ElEMENTS);
    while (jsvIteratorHasElement(&it) && !isDone) {
      JsVar *index = jsvIteratorGetKey(&it);
      if (jsvIsInt(index)) {
        JsVarInt idxValue = jsvGetInteger(index);
        JsVar *value, *args[3], *cb_result;
        value = jsvIteratorGetValue(&it);
        args[0] = value;
        args[1] = jsvNewFromInteger(idxValue); // child is a variable name, create a new variable for the index
        args[2] = parent;
        jsvIteratorNext(&it); // go to next
        cb_result = jspeFunctionCall(funcVar, 0, thisVar, false, 3, args);
        jsvUnLock(args[1]);
        if (cb_result) {
          bool matched;
          if (isBoolCallback)
            matched = (jsvGetBool(cb_result) == expectedValue);
          if (returnType == RETURN_ARRAY) {
            if (isBoolCallback) { // filter
              if (matched) {
                jsvArrayPush(result, value);
              }
            } else { // map
              JsVar *name = jsvNewFromInteger(idxValue);
              if (name) { // out of memory?
                jsvMakeIntoVariableName(name, cb_result);
                jsvAddName(result, name);
                jsvUnLock(name);
              }
            }
          } else if (isBoolCallback) {
            if (returnType == RETURN_ARRAY_ELEMENT ||
                returnType == RETURN_ARRAY_INDEX) {
              if (matched) {
                result = (returnType == RETURN_ARRAY_ELEMENT) ?
                    jsvLockAgain(value) :
                    jsvNewFromInteger(jsvGetInteger(index));
                isDone = true;
              }
            } else if (!matched) // eg for .some
              isDone = true;
          }
          jsvUnLock(cb_result);
        }
        jsvUnLock(value);
      } else {
        // just skip forward anyway
        jsvIteratorNext(&it);
      }
      jsvUnLock(index);
    }
    jsvIteratorFree(&it);
  }
  /* boolean result depends on whether the loop terminated
     early for 'some' or completed for 'every' */
  if (returnType == RETURN_BOOL && isBoolCallback) {
    result = jsvNewFromBool(isDone != expectedValue);
  }
  return result;
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "map",
  "generate" : "jswrap_array_map",
  "params" : [
    ["function","JsVar","Function used to map one item to another"],
    ["thisArg","JsVar","if specified, the function is called with 'this' set to thisArg (optional)"]
  ],
  "return" : ["JsVar","An array containing the results"],
  "typescript" : "map<U>(callbackfn: (value: T, index: number, array: T[]) => U, thisArg?: any): U[];"
}
Return an array which is made from the following: ```A.map(function) =
[function(A[0]), function(A[1]), ...]```
 */
JsVar *jswrap_array_map(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  return _jswrap_array_iterate_with_callback("map", parent, funcVar, thisVar, RETURN_ARRAY, false, false);
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "forEach",
  "generate" : "jswrap_array_forEach",
  "params" : [
    ["function","JsVar","Function to be executed"],
    ["thisArg","JsVar","[optional] If specified, the function is called with 'this' set to thisArg (optional)"]
  ],
  "typescript" : "forEach(callbackfn: (value: T, index: number, array: T[]) => void, thisArg?: any): void;"
}
Executes a provided function once per array element.
 */
void jswrap_array_forEach(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  _jswrap_array_iterate_with_callback("forEach", parent, funcVar, thisVar, RETURN_BOOL, false, false);
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "filter",
  "generate" : "jswrap_array_filter",
  "params" : [
    ["function","JsVar","Function to be executed"],
    ["thisArg","JsVar","if specified, the function is called with 'this' set to thisArg (optional)"]
  ],
  "return" : ["JsVar","An array containing the results"],
  "typescript" : [
    "filter<S extends T>(predicate: (value: T, index: number, array: T[]) => value is S, thisArg?: any): S[];",
    "filter(predicate: (value: T, index: number, array: T[]) => unknown, thisArg?: any): T[];"
  ]
}
Return an array which contains only those elements for which the callback
function returns 'true'
 */
JsVar *jswrap_array_filter(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  return _jswrap_array_iterate_with_callback("filter", parent, funcVar, thisVar, RETURN_ARRAY, true, true);
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "find",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_array_find",
  "params" : [
    ["function","JsVar","Function to be executed"]
  ],
  "return" : ["JsVar","The array element where `function` returns `true`, or `undefined`"],
  "typescript" : [
    "find<S extends T>(predicate: (this: void, value: T, index: number, obj: T[]) => value is S): S | undefined;",
    "find(predicate: (value: T, index: number, obj: T[]) => unknown): T | undefined;"
  ]
}
Return the array element where `function` returns `true`, or `undefined` if it
doesn't returns `true` for any element.

```
["Hello","There","World"].find(a=>a[0]=="T")
// returns "There"
```
 */
JsVar *jswrap_array_find(JsVar *parent, JsVar *funcVar) {
  return _jswrap_array_iterate_with_callback("find", parent, funcVar, 0, RETURN_ARRAY_ELEMENT, true, true);
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "findIndex",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_array_findIndex",
  "params" : [
    ["function","JsVar","Function to be executed"]
  ],
  "return" : ["JsVar","The array element's index where `function` returns `true`, or `-1`"],
  "typescript" : "findIndex(predicate: (value: T, index: number, obj: T[]) => unknown): number;"
}
Return the array element's index where `function` returns `true`, or `-1` if it
doesn't returns `true` for any element.

```
["Hello","There","World"].findIndex(a=>a[0]=="T")
// returns 1
```
 */
JsVar *jswrap_array_findIndex(JsVar *parent, JsVar *funcVar) {
  JsVar *v = _jswrap_array_iterate_with_callback("findIndex", parent, funcVar, 0, RETURN_ARRAY_INDEX, true, true);
  if (v) return v;
  return jsvNewFromInteger(-1);
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "some",
  "generate" : "jswrap_array_some",
  "params" : [
    ["function","JsVar","Function to be executed"],
    ["thisArg","JsVar","if specified, the function is called with 'this' set to thisArg (optional)"]
  ],
  "return" : ["JsVar","A boolean containing the result"],
  "typescript" : "some(predicate: (value: T, index: number, array: T[]) => unknown, thisArg?: any): boolean;"
}
Return 'true' if the callback returns 'true' for any of the elements in the
array
 */
JsVar *jswrap_array_some(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  return _jswrap_array_iterate_with_callback("some", parent, funcVar, thisVar, RETURN_BOOL, true, false);
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "every",
  "generate" : "jswrap_array_every",
  "params" : [
    ["function","JsVar","Function to be executed"],
    ["thisArg","JsVar","if specified, the function is called with 'this' set to thisArg (optional)"]
  ],
  "return" : ["JsVar","A boolean containing the result"],
  "typescript" : "every(predicate: (value: T, index: number, array: T[]) => unknown, thisArg?: any): boolean;"
}
Return 'true' if the callback returns 'true' for every element in the array
 */
JsVar *jswrap_array_every(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  return _jswrap_array_iterate_with_callback("every", parent, funcVar, thisVar, RETURN_BOOL, true, true);
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "reduce",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_array_reduce",
  "params" : [
    ["callback","JsVar","Function used to reduce the array"],
    ["initialValue","JsVar","if specified, the initial value to pass to the function"]
  ],
  "return" : ["JsVar","The value returned by the last function called"],
  "typescript" : "reduce(callback: (previousValue: T, currentValue: T, currentIndex: number, array: T[]) => T, initialValue?: T): T;"
}
Execute `previousValue=initialValue` and then `previousValue =
callback(previousValue, currentValue, index, array)` for each element in the
array, and finally return previousValue.
 */
JsVar *jswrap_array_reduce(JsVar *parent, JsVar *funcVar, JsVar *initialValue) {
  const char *name = "reduce";
  if (!jsvIsIterable(parent)) {
    jsExceptionHere(JSET_ERROR, "Array.%s can only be called on something iterable", name);
    return 0;
  }
  if (!jsvIsFunction(funcVar)) {
    jsExceptionHere(JSET_ERROR, "Array.%s's first argument should be a function", name);
    return 0;
  }
  JsVar *previousValue = jsvLockAgainSafe(initialValue);
  JsvIterator it;
  jsvIteratorNew(&it, parent, JSIF_DEFINED_ARRAY_ElEMENTS);
  if (!previousValue) {
    bool isDone = false;
    while (!isDone && jsvIteratorHasElement(&it)) {
      JsVar *index = jsvIteratorGetKey(&it);
      if (jsvIsInt(index)) {
        previousValue = jsvIteratorGetValue(&it);
        isDone = true;
      }
      jsvUnLock(index);
      jsvIteratorNext(&it);
    }
    if (!previousValue) {
      jsExceptionHere(JSET_ERROR, "Array.%s without initial value required non-empty array", name);
    }
  }
  while (jsvIteratorHasElement(&it)) {
    JsVar *index = jsvIteratorGetKey(&it);
    if (jsvIsInt(index)) {
      JsVarInt idxValue = jsvGetInteger(index);

      JsVar *args[4];
      args[0] = previousValue;
      args[1] = jsvIteratorGetValue(&it);
      args[2] = jsvNewFromInteger(idxValue); // child is a variable name, create a new variable for the index
      args[3] = parent;
      previousValue = jspeFunctionCall(funcVar, 0, 0, false, 4, args);
      jsvUnLockMany(3,args);
    }
    jsvUnLock(index);
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);

  return previousValue;
}


/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "splice",
  "generate" : "jswrap_array_splice",
  "params" : [
    ["index","int","Index at which to start changing the array. If negative, will begin that many elements from the end"],
    ["howMany","JsVar","An integer indicating the number of old array elements to remove. If howMany is 0, no elements are removed."],
    ["elements","JsVarArray","One or more items to add to the array"]
  ],
  "return" : ["JsVar","An array containing the removed elements. If only one element is removed, an array of one element is returned."],
  "typescript" : "splice(index: number, howMany?: number, ...elements: T[]): T[];"
}
Both remove and add items to an array
 */
JsVar *jswrap_array_splice(JsVar *parent, JsVarInt index, JsVar *howManyVar, JsVar *elements) {
  if (!jsvIsArray(parent)) return 0;
  JsVarInt len = jsvGetArrayLength(parent);
  if (index<0) index+=len;
  if (index<0) index=0;
  if (index>len) index=len;
  JsVarInt howMany = len; // how many to delete!
  if (jsvIsNumeric(howManyVar)) howMany = jsvGetInteger(howManyVar);
  if (howMany > len-index) howMany = len-index;
  JsVarInt newItems = jsvGetArrayLength(elements);
  JsVarInt shift = newItems-howMany;

  bool needToAdd = false;
  JsVar *result = jsvNewEmptyArray();

  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, parent);
  while (jsvObjectIteratorHasValue(&it) && !needToAdd) {
    bool goToNext = true;
    JsVar *idxVar = jsvObjectIteratorGetKey(&it);
    if (idxVar && jsvIsInt(idxVar)) {
      JsVarInt idx = jsvGetInteger(idxVar);
      if (idx<index) {
        // do nothing...
      } else if (idx<index+howMany) { // must delete
        if (result) { // append to result array
          JsVar *el = jsvObjectIteratorGetValue(&it);
          jsvArrayPushAndUnLock(result, el);
        }
        // delete
        goToNext = false;
        JsVar *toRemove = jsvObjectIteratorGetKey(&it);
        jsvObjectIteratorNext(&it);
        jsvRemoveChild(parent, toRemove);
        jsvUnLock(toRemove);
      } else { // we're greater than the amount we need to remove now
        needToAdd = true;
        goToNext = false;
      }
    }
    jsvUnLock(idxVar);
    if (goToNext) jsvObjectIteratorNext(&it);
  }
  // now we add everything
  JsVar *beforeIndex = jsvObjectIteratorGetKey(&it);
  JsvObjectIterator itElement;
  jsvObjectIteratorNew(&itElement, elements);
  while (jsvObjectIteratorHasValue(&itElement)) {
    JsVar *element = jsvObjectIteratorGetValue(&itElement);
    jsvArrayInsertBefore(parent, beforeIndex, element);
    jsvUnLock(element);
    jsvObjectIteratorNext(&itElement);
  }
  jsvObjectIteratorFree(&itElement);
  jsvUnLock(beforeIndex);
  // And finally renumber
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *idxVar = jsvObjectIteratorGetKey(&it);
    if (idxVar && jsvIsInt(idxVar)) {
      jsvSetInteger(idxVar, jsvGetInteger(idxVar)+shift);
    }
    jsvUnLock(idxVar);
    jsvObjectIteratorNext(&it);
  }
  // free
  jsvObjectIteratorFree(&it);

  // and reset array size
  jsvSetArrayLength(parent, len + shift, false);

  return result;
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "shift",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_array_shift",
  "params" : [

  ],
  "return" : ["JsVar","The element that was removed"],
  "typescript" : "shift(): T | undefined;"
}
Remove and return the first element of the array.

This is the opposite of `[1,2,3].pop()`, which takes an element off the end.
 */
JsVar *jswrap_array_shift(JsVar *parent) {
  // just use splice, as this does all the hard work for us
  JsVar *nRemove = jsvNewFromInteger(1);
  JsVar *elements = jsvNewEmptyArray();
  JsVar *arr = jswrap_array_splice(parent, 0, nRemove, elements);
  jsvUnLock2(elements, nRemove);
  // unpack element from the array
  JsVar *el = 0;
  if (jsvIsArray(arr))
    el = jsvSkipNameAndUnLock(jsvArrayPop(arr));
  jsvUnLock(arr);
  return el;
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "unshift",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_array_unshift",
  "params" : [
    ["elements","JsVarArray","One or more items to add to the beginning of the array"]
  ],
  "return" : ["int","The new array length"],
  "typescript" : "unshift(...elements: T[]): number;"
}
Add one or more items to the start of the array, and return its new length.

This is the opposite of `[1,2,3].push(4)`, which puts one or more elements on
the end.
 */
JsVarInt jswrap_array_unshift(JsVar *parent, JsVar *elements) {
  // just use splice, as this does all the hard work for us
  JsVar *nRemove = jsvNewFromInteger(0);
  jsvUnLock2(jswrap_array_splice(parent, 0, nRemove, elements), nRemove);
  // return new length
  return jsvGetLength(parent);
}


/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "slice",
  "generate" : "jswrap_array_slice",
  "params" : [
    ["start","int","Start index"],
    ["end","JsVar","End index (optional)"]
  ],
  "return" : ["JsVar","A new array"],
  "typescript" : "slice(start?: number, end?: number): T[];"
}
Return a copy of a portion of this array (in a new array)
 */
JsVar *jswrap_array_slice(JsVar *parent, JsVarInt start, JsVar *endVar) {
  JsVarInt len = jsvGetLength(parent);
  JsVarInt end = len;

  if (!jsvIsUndefined(endVar))
    end = jsvGetInteger(endVar);

  JsVarInt k = 0;
  JsVarInt final = len;
  JsVar *array = jsvNewEmptyArray();

  if (!array) return 0;

  if (start<0) k = max((len + start), 0);
  else k = min(start, len);

  if (end<0) final = max((len + end), 0);
  else final = min(end, len);

  bool isDone = false;

  JsvIterator it;
  jsvIteratorNew(&it, parent, JSIF_EVERY_ARRAY_ELEMENT);

  while (jsvIteratorHasElement(&it) && !isDone) {
    JsVarInt idx = jsvGetIntegerAndUnLock(jsvIteratorGetKey(&it));

    if (idx < k) {
      jsvIteratorNext(&it);
    } else {
      if (k < final) {
        // TODO: could skip sparse array items?
        jsvArrayPushAndUnLock(array, jsvIteratorGetValue(&it));
        jsvIteratorNext(&it);
        k++;
      } else {
        isDone = true;
      }
    }
  }

  jsvIteratorFree(&it);

  return array;
}


/*JSON{
  "type" : "staticmethod",
  "class" : "Array",
  "name" : "isArray",
  "generate_full" : "jsvIsArray(var)",
  "params" : [
    ["var","JsVar","The variable to be tested"]
  ],
  "return" : ["bool","True if var is an array, false if not."],
  "typescript" : "isArray(arg: any): arg is any[];"
}
Returns true if the provided object is an array
 */


NO_INLINE static JsVarInt _jswrap_array_sort_compare(JsVar *a, JsVar *b, JsVar *compareFn) {
  if (compareFn) {
    JsVar *args[2] = {a,b};
    JsVarFloat f = jsvGetFloatAndUnLock(jspeFunctionCall(compareFn, 0, 0, false, 2, args));
    if (f==0) return 0;
    return (f<0)?-1:1;
  } else {
    JsVar *sa = jsvAsString(a);
    JsVar *sb = jsvAsString(b);
    JsVarInt r = jsvCompareString(sa,sb, 0, 0, false);
    jsvUnLock2(sa, sb);
    return r;
  }
}

NO_INLINE static void _jswrap_array_sort(JsvIterator *head, int n, JsVar *compareFn) {
  if (n < 2) return; // sort done!

  JsvIterator pivot;
  jsvIteratorClone(&pivot, head);
  bool pivotLowest = true; // is the pivot the lowest value in here?
  JsVar *pivotValue = jsvIteratorGetValue(&pivot);
  /* We're just going to use the first entry (head) as the pivot...
   * We'll move along with our iterator 'it', and if it < pivot then we'll
   * swap the values over (hence moving pivot forwards)  */

  int nlo = 0, nhigh = 0;
  JsvIterator it;
  jsvIteratorClone(&it, head); //
  jsvIteratorNext(&it);


  /* Partition and count sizes. */
  while (--n && !jspIsInterrupted()) {
    JsVar *itValue = jsvIteratorGetValue(&it);
    JsVarInt cmp = _jswrap_array_sort_compare(itValue, pivotValue, compareFn);
    if (cmp<=0) {
      if (cmp<0) pivotLowest = false;
      nlo++;
      /* 'it' <= 'pivot', so we need to move it behind.
         In this diagram, P=pivot, L=it

               l l l l l P h h h h h L
                         |  \       /
                          \  \_____/_
                          _\______/  \
                         / |         |
                         | |         |
               l l l l l L P h h h h h

         It makes perfect sense now...
       */
      // first, get the old pivot value and overwrite it with the iterator value
      jsvIteratorSetValue(&pivot, itValue); // no unlock needed
      // now move pivot forwards, and set 'it' to the value the new pivot has
      jsvIteratorNext(&pivot);
      jsvUnLock(jsvIteratorSetValue(&it, jsvIteratorGetValue(&pivot)));
      // finally set the pivot iterator to the pivot's value again
      jsvIteratorSetValue(&pivot, pivotValue); // no unlock needed
    } else {
      nhigh++;
      // Great, 'it' > 'pivot' so it's in the right place
    }
    jsvUnLock(itValue);
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);
  jsvUnLock(pivotValue);

  if (jspIsInterrupted()) {
    jsvIteratorFree(&pivot);
    return;
  }

  // now recurse. Do RHS first because we can
  // free the pivot early if we do this
  jsvIteratorNext(&pivot);
  _jswrap_array_sort(&pivot, nhigh, compareFn);
  jsvIteratorFree(&pivot);
  // LHS
  /* If the pivot is the lowest number in this chunk of numbers, then
   * we know that anything to the left of it must be joint equal to it.
   * In that casem there's no need to sort it. */
  if (!pivotLowest)
    _jswrap_array_sort(head, nlo, compareFn);
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "sort",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_array_sort",
  "params" : [
    ["var","JsVar","A function to use to compare array elements (or undefined)"]
  ],
  "return" : ["JsVar","This array object"],
  "typescript" : "sort(compareFn?: (a: T, b: T) => number): T[];"
}
Do an in-place quicksort of the array
 */
JsVar *jswrap_array_sort (JsVar *array, JsVar *compareFn) {
  if (!jsvIsUndefined(compareFn) && !jsvIsFunction(compareFn)) {
    jsExceptionHere(JSET_ERROR, "Expecting compare function, got %t", compareFn);
    return 0;
  }
  JsvIterator it;

  /* Arrays can be sparse and the iterators don't handle this
    (we're not going to mess with indices) so we have to count
     up the number of elements manually.

     FIXME: sort is broken for sparse arrays anyway (it basically
     ignores all the 'undefined' entries). I wonder whether just
     compacting the array down to start from 0 before we start would
     fix this?
   */
  int n=0;
  if (jsvIsArray(array) || jsvIsObject(array)) {
    jsvIteratorNew(&it, array, JSIF_EVERY_ARRAY_ELEMENT);
    while (jsvIteratorHasElement(&it)) {
      n++;
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  } else {
    n = (int)jsvGetLength(array);
  }

  jsvIteratorNew(&it, array, JSIF_EVERY_ARRAY_ELEMENT);
  _jswrap_array_sort(&it, n, compareFn);
  jsvIteratorFree(&it);
  return jsvLockAgain(array);
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "concat",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_array_concat",
  "params" : [
    ["args","JsVarArray","Any items to add to the array"]
  ],
  "return" : ["JsVar","An Array"],
  "typescript" : "concat(...args: (T | T[])[]): T[];"
}
Create a new array, containing the elements from this one and any arguments, if
any argument is an array then those elements will be added.
 */
JsVar *jswrap_array_concat(JsVar *parent, JsVar *args) {
  JsVar *result = jsvNewEmptyArray();

  JsvObjectIterator argsIt;
  jsvObjectIteratorNew(&argsIt, args);

  // Append parent's elements first (parent is always an array)
  JsVar *source = jsvLockAgain(parent);
  do {
    if (jsvIsArray(source)) {
      jsvArrayPushAll(result, source, false);
    } else
      jsvArrayPush(result, source);
    // Next, append arguments
    jsvUnLock(source);
    source = jsvObjectIteratorHasValue(&argsIt) ? jsvObjectIteratorGetValue(&argsIt) : 0;
    jsvObjectIteratorNext(&argsIt);
  } while (source);

  jsvObjectIteratorFree(&argsIt);
  return result;
}

/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "fill",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_array_fill",
  "params" : [
    ["value","JsVar","The value to fill the array with"],
    ["start","int","Optional. The index to start from (or 0). If start is negative, it is treated as length+start where length is the length of the array"],
    ["end","JsVar","Optional. The index to end at (or the array length). If end is negative, it is treated as length+end."]
  ],
  "return" : ["JsVar","This array"],
  "typescript" : "fill(value: T, start: number, end?: number): T[];"
}
Fill this array with the given value, for every index `>= start` and `< end`
 */
JsVar *jswrap_array_fill(JsVar *parent, JsVar *value, JsVarInt start, JsVar *endVar) {
  if (!jsvIsIterable(parent)) return 0;

  JsVarInt length = jsvGetLength(parent);
  if (start < 0) start = start + length;
  if (start < 0) return 0;
  JsVarInt end = jsvIsNumeric(endVar) ? jsvGetInteger(endVar) : length;
  if (end < 0) end = end + length;
  if (end < 0) return 0;


  JsvIterator it;
  jsvIteratorNew(&it, parent, JSIF_EVERY_ARRAY_ELEMENT);
  while (jsvIteratorHasElement(&it) && !jspIsInterrupted()) {
    JsVarInt idx = jsvGetIntegerAndUnLock(jsvIteratorGetKey(&it));
    if (idx>=start && idx<end) {
      jsvIteratorSetValue(&it, value);
    }
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);
  return jsvLockAgain(parent);
}

/** recursive reverse, because we're dealing with a linked list that
 * MAY only be linked in one direction (eg. string/arraybuffer).
 */
void _jswrap_array_reverse_block(JsVar *parent, JsvIterator *it, int items) {
  assert(items > 1);

  JsvIterator ita, itb;
  jsvIteratorClone(&ita, it);
  jsvIteratorClone(&itb, it);

  // move second pointer halfway through (round up)
  int i;
  for (i=(items+1)/2;i>0;i--)
    jsvIteratorNext(&itb);
  // recurse if >3 items. If 3 we can cope with it here
  if (items > 3) {
    _jswrap_array_reverse_block(parent, &ita, items/2);
    _jswrap_array_reverse_block(parent, &itb, items/2);
  }
  // start flipping values (round down for how many)
  for (i=items/2;i>0;i--) {
    JsVar *va = jsvIteratorGetValue(&ita);
    JsVar *vb = jsvIteratorGetValue(&itb);
    jsvIteratorSetValue(&ita, vb);
    jsvIteratorSetValue(&itb, va);
    jsvUnLock2(va, vb);
    // if it's an array, we need to swap the key values too
    if (jsvIsArray(parent)) {
      JsVar *ka = jsvIteratorGetKey(&ita);
      JsVar *kb = jsvIteratorGetKey(&itb);
      JsVarInt kva = jsvGetInteger(ka);
      JsVarInt kvb = jsvGetInteger(kb);
      jsvSetInteger(ka, kvb);
      jsvSetInteger(kb, kva);
      jsvUnLock2(ka, kb);
    }

    jsvIteratorNext(&ita);
    jsvIteratorNext(&itb);
  }
  // now recurse!
  jsvIteratorFree(&ita);
  jsvIteratorFree(&itb);
}


/*JSON{
  "type" : "method",
  "class" : "Array",
  "name" : "reverse",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_array_reverse",
  "return" : ["JsVar","The array, but reversed."],
  "typescript" : "reverse(): T[];"
}
Reverse all elements in this array (in place)
 */
JsVar *jswrap_array_reverse(JsVar *parent) {
  if (!jsvIsIterable(parent) || jsvIsObject(parent)) return 0;


  int len = 0;
  if (jsvIsArray(parent)) {
    /* arrays are sparse, so we must handle them differently.
     * We work out how many NUMERIC keys they have, and we
     * reverse only those. Then, we reverse the key values too */
    JsvIterator it;
    jsvIteratorNew(&it, parent, JSIF_DEFINED_ARRAY_ElEMENTS);
    while (jsvIteratorHasElement(&it)) {
      JsVar *k = jsvIteratorGetKey(&it);
      if (jsvIsInt(k)) len++;
      jsvUnLock(k);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  } else
    len = jsvGetLength(parent);

  JsvIterator it;
  jsvIteratorNew(&it, parent, JSIF_DEFINED_ARRAY_ElEMENTS);
  if (len>1) {
    _jswrap_array_reverse_block(parent, &it, len);
  }
  // if it's an array, we must change the values on the keys
  if (jsvIsArray(parent)) {
    JsVarInt last = jsvGetArrayLength(parent)-1;
    while (jsvIteratorHasElement(&it)) {
      JsVar *k = jsvIteratorGetKey(&it);
      jsvSetInteger(k, last-jsvGetInteger(k));
      jsvUnLock(k);
      jsvIteratorNext(&it);
    }
  }
  jsvIteratorFree(&it);

  return jsvLockAgain(parent);
}
