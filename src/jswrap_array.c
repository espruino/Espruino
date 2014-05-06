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
#include "jsparse.h"

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


/*JSON{ "type":"class",
        "class" : "Array",
        "check" : "jsvIsArray(var)",
        "description" : ["This is the built-in JavaScript class for arrays.",
                         "Arrays can be defined with ```[]```, ```new Array()```, or ```new Array(length)```" ]
}*/

/*JSON{ "type":"constructor", "class": "Array",  "name": "Array",
         "description" : "Create an Array. Either give it one integer argument (>=0) which is the length of the array, or any number of arguments ",
         "generate" : "jswrap_array_constructor",
         "params" : [ [ "args", "JsVarArray", "The length of the array OR any number of items to add to the array" ] ],
         "return" : [ "JsVar", "An Array" ]

}*/
JsVar *jswrap_array_constructor(JsVar *args) {
  assert(args);
  if (jsvGetArrayLength(args)==1) {
    JsVar *firstArg = jsvSkipNameAndUnLock(jsvArrayGetLast(args)); // also the first!
    if (jsvIsInt(firstArg) && jsvGetInteger(firstArg)>=0) {
      JsVarInt count = jsvGetInteger(firstArg);
      // we cheat - no need to fill the array - just the last element
      if (count>0) {
        JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
        if (!arr) return 0; // out of memory
        JsVar *idx = jsvMakeIntoVariableName(jsvNewFromInteger(count-1), 0);
        if (idx) { // could be out of memory
          jsvAddName(arr, idx);
          jsvUnLock(idx);
        }
        jsvUnLock(firstArg);
        return arr;
      }
    }
    jsvUnLock(firstArg);
  }
  // Otherwise, we just return the array!
  return jsvLockAgain(args);
}

/*JSON{ "type":"method", "class": "Array", "name" : "indexOf",
         "description" : "Return the index of the value in the array, or -1",
         "generate" : "jswrap_array_indexOf",
         "params" : [ [ "value", "JsVar", "The value to check for"] ],
         "return" : ["JsVar", "the index of the value in the array, or -1"]
}*/
JsVar *jswrap_array_indexOf(JsVar *parent, JsVar *value) {
  JsVar *idxName = jsvGetArrayIndexOf(parent, value, false/*not exact*/);
  // but this is the name - we must turn it into a var
  if (idxName == 0) return jsvNewFromInteger(-1); // not found!
  JsVar *idx = jsvCopyNameOnly(idxName, false/* no children */, false/* Make sure this is not a name*/);
  jsvUnLock(idxName);
  return idx;
}

/*JSON{ "type":"method", "class": "Array", "name" : "join",
         "description" : "Join all elements of this array together into one string, using 'separator' between them. eg. ```[1,2,3].join(' ')=='1 2 3'```",
         "generate" : "jswrap_array_join",
         "params" : [ [ "separator", "JsVar", "The separator"] ],
         "return" : ["JsVar", "A String representing the Joined array"]
}*/
JsVar *jswrap_array_join(JsVar *parent, JsVar *filler) {
  if (!jsvIsIterable(parent)) return 0;
  if (jsvIsUndefined(filler))
    filler = jsvNewFromString(","); // the default it seems
  else
    filler = jsvAsString(filler, false);
  if (!filler) return 0; // out of memory
  JsVar *str = jsvArrayJoin(parent, filler);
  jsvUnLock(filler);
  return str;
}

/*JSON{ "type":"method", "class": "Array", "name" : "push",
         "description" : "Push a new value onto the end of this array'",
         "generate" : "jswrap_array_push",
         "params" : [ [ "arguments", "JsVarArray", "One or more arguments to add"] ],
         "return" : ["int", "The new size of the array"]
}*/
JsVarInt jswrap_array_push(JsVar *parent, JsVar *args) {
  if (!jsvIsArray(parent)) return -1;
  JsVarInt len = -1;
  JsvArrayIterator it;
  jsvArrayIteratorNew(&it, args);
  while (jsvArrayIteratorHasElement(&it)) {
    JsVar *el = jsvArrayIteratorGetElement(&it);
    len = jsvArrayPush(parent, el);
    jsvUnLock(el);
    jsvArrayIteratorNext(&it);
  }
  jsvArrayIteratorFree(&it);
  if (len<0) 
    len = jsvGetArrayLength(parent);
  return len;
}


/*JSON{ "type":"method", "class": "Array", "name" : "pop",
         "description" : "Pop a new value off of the end of this array",
         "generate_full" : "jsvArrayPop(parent)",
         "return" : ["JsVar", "The value that is popped off"]
}*/

JsVar *_jswrap_array_iterate_with_callback(const char *name, JsVar *parent, JsVar *funcVar, JsVar *thisVar, bool wantArray, bool isBoolCallback, bool expectedValue) {
  if (!jsvIsIterable(parent)) {
    jsError("Array.%s can only be called on something iterable", name);
    return 0;
  }
  if (!jsvIsFunction(funcVar)) {
    jsError("Array.%s's first argument should be a function", name);
    return 0;
  }
  if (!jsvIsUndefined(thisVar) && !jsvIsObject(thisVar)) {
    jsError("Array.%s's second argument should be undefined, or an object", name);
    return 0;
  }
  JsVar *result = 0;
  if (wantArray)
    result = jsvNewWithFlags(JSV_ARRAY);
  bool isDone = false;
  if (result || !wantArray) {
    JsvIterator it;
    jsvIteratorNew(&it, parent);
    while (jsvIteratorHasElement(&it) && !isDone) {
      JsVar *index = jsvIteratorGetKey(&it);
      if (jsvIsInt(index)) {
        JsVarInt idxValue = jsvGetInteger(index);

        JsVar *args[3], *cb_result;
        args[0] = jsvIteratorGetValue(&it);
        args[1] = jsvNewFromInteger(idxValue); // child is a variable name, create a new variable for the index
        args[2] = parent;
        cb_result = jspeFunctionCall(funcVar, 0, thisVar, false, 3, args);
        jsvUnLock(args[0]);
        jsvUnLock(args[1]);
        if (cb_result) {
          bool matched;
          if (isBoolCallback)
            matched = (jsvGetBool(cb_result) == expectedValue);
          if (wantArray) {
            if (isBoolCallback) { // filter
              if (matched) {
                jsvArrayPushAndUnLock(result, jsvIteratorGetValue(&it));
              }
			} else { // map
              JsVar *name = jsvNewFromInteger(idxValue);
              if (name) { // out of memory?
                jsvMakeIntoVariableName(name, cb_result);
                jsvAddName(result, name);
                jsvUnLock(name);
              }
			}
          } else {
            // break the loop early if expecting a particular value and didn't get it
            if (isBoolCallback && !matched)
              isDone = true;
          }
          jsvUnLock(cb_result);
        }
      }
      jsvUnLock(index);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  }
  /* boolean result depends on whether the loop terminated
     early for 'some' or completed for 'every' */
  if (!wantArray && isBoolCallback) {
    result = jsvNewFromBool(isDone != expectedValue);
  }
  return result;
}

/*JSON{ "type":"method", "class": "Array", "name" : "map",
         "description" : "Return an array which is made from the following: ```A.map(function) = [function(A[0]), function(A[1]), ...]```",
         "generate" : "jswrap_array_map",
         "params" : [ [ "function", "JsVar", "Function used to map one item to another"] ,
                      [ "thisArg", "JsVar", "if specified, the function is called with 'this' set to thisArg (optional)"] ],
         "return" : ["JsVar", "An array containing the results"]
}*/
JsVar *jswrap_array_map(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  return _jswrap_array_iterate_with_callback("map", parent, funcVar, thisVar, true, false, false);
}

/*JSON{ "type":"method", "class": "Array", "name" : "forEach",
         "description" : "Executes a provided function once per array element.",
         "generate" : "jswrap_array_forEach",
         "params" : [ [ "function", "JsVar", "Function to be executed"] ,
                      [ "thisArg", "JsVar", "if specified, the function is called with 'this' set to thisArg (optional)"] ]
}*/
void jswrap_array_forEach(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  _jswrap_array_iterate_with_callback("forEach", parent, funcVar, thisVar, false, false, false);
}

/*JSON{ "type":"method", "class": "Array", "name" : "filter",
         "description" : "Return an array which contains only those elements for which the callback function returns 'true'",
         "generate" : "jswrap_array_filter",
         "params" : [ [ "function", "JsVar", "Function to be executed"] ,
                      [ "thisArg", "JsVar", "if specified, the function is called with 'this' set to thisArg (optional)"] ],
         "return" : ["JsVar", "An array containing the results"]
}*/
JsVar *jswrap_array_filter(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  return _jswrap_array_iterate_with_callback("filter", parent, funcVar, thisVar, true, true, true);
}

/*JSON{ "type":"method", "class": "Array", "name" : "some",
         "description" : "Return 'true' if the callback returns 'true' for any of the elements in the array",
         "generate" : "jswrap_array_some",
         "params" : [ [ "function", "JsVar", "Function to be executed"] ,
                      [ "thisArg", "JsVar", "if specified, the function is called with 'this' set to thisArg (optional)"] ],
         "return" : ["JsVar", "A boolean containing the result"]
}*/
JsVar *jswrap_array_some(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  return _jswrap_array_iterate_with_callback("some", parent, funcVar, thisVar, false, true, false);
}

/*JSON{ "type":"method", "class": "Array", "name" : "every",
         "description" : "Return 'true' if the callback returns 'true' for every element in the array",
         "generate" : "jswrap_array_every",
         "params" : [ [ "function", "JsVar", "Function to be executed"] ,
                      [ "thisArg", "JsVar", "if specified, the function is called with 'this' set to thisArg (optional)"] ],
         "return" : ["JsVar", "A boolean containing the result"]
}*/
JsVar *jswrap_array_every(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  return _jswrap_array_iterate_with_callback("every", parent, funcVar, thisVar, false, true, true);
}

/*JSON{ "type":"method", "class": "Array", "name" : "reduce", "ifndef" : "SAVE_ON_FLASH",
         "description" : "Execute `previousValue=initialValue` and then `previousValue = callback(previousValue, currentValue, index, array)` for each element in the array, and finally return previousValue.",
         "generate" : "jswrap_array_reduce",
         "params" : [ [ "callback", "JsVar", "Function used to reduce the array"] ,
                      [ "initialValue", "JsVar", "if specified, the initial value to pass to the function"] ],
         "return" : ["JsVar", "The value returned by the last function called"]
}*/
JsVar *jswrap_array_reduce(JsVar *parent, JsVar *funcVar, JsVar *initialValue) {
  const char *name = "reduce";
  if (!jsvIsIterable(parent)) {
    jsError("Array.%s can only be called on something iterable", name);
    return 0;
  }
  if (!jsvIsFunction(funcVar)) {
    jsError("Array.%s's first argument should be a function", name);
    return 0;
  }
  JsVar *previousValue = initialValue ? jsvLockAgain(initialValue) : 0;
  JsvIterator it;
  jsvIteratorNew(&it, parent);
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
      jsvUnLock(args[0]);
      jsvUnLock(args[1]);
      jsvUnLock(args[2]);
    }
    jsvUnLock(index);
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);

  return previousValue;
}


/*JSON{ "type":"method", "class": "Array", "name" : "splice",
         "description" : "Both remove and add items to an array",
         "generate" : "jswrap_array_splice",
         "params" : [ [ "index", "int", "Index at which to start changing the array. If negative, will begin that many elements from the end"],
                      [ "howMany", "JsVar", "An integer indicating the number of old array elements to remove. If howMany is 0, no elements are removed."],
                      [ "elements", "JsVarArray", "One or more items to add to the array" ] ],
         "return" : ["JsVar", "An array containing the removed elements. If only one element is removed, an array of one element is returned."]
}*/
JsVar *jswrap_array_splice(JsVar *parent, JsVarInt index, JsVar *howManyVar, JsVar *elements) {
  if (!jsvIsArray(parent)) return 0;
  JsVarInt len = jsvGetArrayLength(parent);
  if (index<0) index+=len;
  if (index<0) index=0;
  if (index>len) index=len;
  JsVarInt howMany = len; // how many to delete!
  if (jsvIsInt(howManyVar)) howMany = jsvGetInteger(howManyVar);
  if (howMany > len-index) howMany = len-index;
  JsVarInt newItems = jsvGetArrayLength(elements);
  JsVarInt shift = newItems-howMany;

  bool needToAdd = false;
  JsVar *result = jsvNewWithFlags(JSV_ARRAY);

  JsvArrayIterator it;
  jsvArrayIteratorNew(&it, parent);
  while (jsvArrayIteratorHasElement(&it) && !needToAdd) {
    bool goToNext = true;
    JsVar *idxVar = jsvArrayIteratorGetIndex(&it);
    if (idxVar && jsvIsInt(idxVar)) {
      JsVarInt idx = jsvGetInteger(idxVar);
      if (idx<index) {
        // do nothing...
      } else if (idx<index+howMany) { // must delete
        if (result) { // append to result array
          JsVar *el = jsvArrayIteratorGetElement(&it);
          jsvArrayPushAndUnLock(result, el);
        }
        // delete
        goToNext = false;
        JsVar *toRemove = jsvArrayIteratorGetIndex(&it);
        jsvArrayIteratorNext(&it);
        jsvRemoveChild(parent, toRemove);
        jsvUnLock(toRemove);
      } else { // we're greater than the amount we need to remove now
        needToAdd = true;
        goToNext = false;
      }
    }
    jsvUnLock(idxVar);
    if (goToNext) jsvArrayIteratorNext(&it);
  }
  // now we add everything
  JsVar *beforeIndex = jsvArrayIteratorGetIndex(&it);
  JsvArrayIterator itElement;
  jsvArrayIteratorNew(&itElement, elements);
  while (jsvArrayIteratorHasElement(&itElement)) {
    JsVar *element = jsvArrayIteratorGetElement(&itElement);
    jsvArrayInsertBefore(parent, beforeIndex, element);
    jsvUnLock(element);
    jsvArrayIteratorNext(&itElement);
  }
  jsvArrayIteratorFree(&itElement);
  jsvUnLock(beforeIndex);
  // And finally renumber
  while (jsvArrayIteratorHasElement(&it)) {
    JsVar *idxVar = jsvArrayIteratorGetIndex(&it);
    if (idxVar && jsvIsInt(idxVar)) {
      jsvSetInteger(idxVar, jsvGetInteger(idxVar)+shift);
    }
    jsvUnLock(idxVar);
    jsvArrayIteratorNext(&it);
  }
  // free
  jsvArrayIteratorFree(&it);

  return result;
}

/*JSON{ "type":"method", "class": "Array", "name" : "shift", "ifndef" : "SAVE_ON_FLASH",
         "description" : "Remove the first element of the array, and return it",
         "generate" : "jswrap_array_shift",
         "params" : [ ],
         "return" : ["JsVar", "The element that was removed"]
}*/
JsVar *jswrap_array_shift(JsVar *parent) {
  // just use splice, as this does all the hard work for us
  JsVar *nRemove = jsvNewFromInteger(1);
  JsVar *elements = jsvNewWithFlags(JSV_ARRAY);
  JsVar *arr = jswrap_array_splice(parent, 0, nRemove, elements);
  jsvUnLock(elements);
  jsvUnLock(nRemove);
  // unpack element from the array
  JsVar *el = 0;
  if (jsvIsArray(arr))
    el = jsvArrayPop(arr);
  jsvUnLock(arr);
  return el;
}

/*JSON{ "type":"method", "class": "Array", "name" : "unshift", "ifndef" : "SAVE_ON_FLASH",
         "description" : "Remove the first element of the array, and return it",
         "generate" : "jswrap_array_unshift",
         "params" : [ [ "elements", "JsVarArray", "One or more items to add to the beginning of the array" ] ],
         "return" : ["int", "The new array length"]
}*/
JsVarInt jswrap_array_unshift(JsVar *parent, JsVar *elements) {
  // just use splice, as this does all the hard work for us
  JsVar *nRemove = jsvNewFromInteger(0);
  jsvUnLock(jswrap_array_splice(parent, 0, nRemove, elements));
  jsvUnLock(nRemove);
  // return new length
  return jsvGetLength(parent);
}


/*JSON{ "type":"method", "class": "Array", "name" : "slice",
         "description" : "Return a copy of a portion of the calling array",
         "generate" : "jswrap_array_slice",
         "params" : [ [ "start", "int", "Start index"],
                      [ "end", "JsVar", "End index (optional)"] ],
         "return" : ["JsVar", "A new array"]
}*/
JsVar *jswrap_array_slice(JsVar *parent, JsVarInt start, JsVar *endVar) {
  JsVarInt len = jsvGetLength(parent);
  JsVarInt end = len;

  if (!jsvIsUndefined(endVar))
    end = jsvGetInteger(endVar);

  JsVarInt k = 0;
  JsVarInt final = len;
  JsVar *array = jsvNewWithFlags(JSV_ARRAY);

  if (!array) return 0;

  if (start<0) k = max((len + start), 0);
  else k = min(start, len);

  if (end<0) final = max((len + end), 0);
  else final = min(end, len);

  bool isDone = false;

  JsvIterator it;
  jsvIteratorNew(&it, parent);

  while (jsvIteratorHasElement(&it) && !isDone) {
    JsVarInt idx = jsvGetIntegerAndUnLock(jsvIteratorGetKey(&it));

    if (idx < k) {
      jsvIteratorNext(&it);
    } else {
      if (k < final) {
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


/*JSON{ "type":"staticmethod", "class": "Array", "name" : "isArray",
         "description" : "Returns true if the provided object is an array",
         "generate_full" : "jsvIsArray(var)",
         "params" : [ [ "var", "JsVar", "The variable to be tested"] ],
         "return" : ["bool", "True if var is an array, false if not."]
}*/


NO_INLINE static bool _jswrap_array_sort_leq(JsVar *a, JsVar *b, JsVar *compareFn) {
  if (compareFn) {
    JsVar *args[2] = {a,b};
    JsVarInt r = jsvGetIntegerAndUnLock(jspeFunctionCall(compareFn, 0, 0, false, 2, args));
    return r<0;
  } else {
    JsVar *sa = jsvAsString(a, false);
    JsVar *sb = jsvAsString(b, false);
    bool r = jsvCompareString(sa,sb, 0, 0, false) < 0;
    jsvUnLock(sa);
    jsvUnLock(sb);
    return r;
  }
}

NO_INLINE static void _jswrap_array_sort(JsvIterator *head, int n, JsVar *compareFn) {
  if (n < 2) return; // sort done!

  JsvIterator pivot = jsvIteratorClone(head);
  JsVar *pivotValue = jsvIteratorGetValue(&pivot);
  /* We're just going to use the first entry (head) as the pivot...
   * We'll move along with our iterator 'it', and if it < pivot then we'll
   * swap the values over (hence moving pivot forwards)  */

  int nlo = 0, nhigh = 0;
  JsvIterator it = jsvIteratorClone(head); //
  jsvIteratorNext(&it);

  /* Partition and count sizes. */
  while (--n && !jspIsInterrupted()) {
    JsVar *itValue = jsvIteratorGetValue(&it);
    if (_jswrap_array_sort_leq(itValue, pivotValue, compareFn)) {
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

  if (jspIsInterrupted()) return;

  // now recurse
  _jswrap_array_sort(head, nlo, compareFn);
  jsvIteratorNext(&pivot);
  _jswrap_array_sort(&pivot, nhigh, compareFn);
  jsvIteratorFree(&pivot);
}

/*JSON{ "type":"method", "class": "Array", "name" : "sort", "ifndef" : "SAVE_ON_FLASH",
         "description" : "Do an in-place quicksort of the array",
         "generate" : "jswrap_array_sort",
         "params" : [ [ "var", "JsVar", "A function to use to compare array elements (or undefined)"] ],
         "return" : [ "JsVar", "This array object" ]
}*/
JsVar *jswrap_array_sort (JsVar *array, JsVar *compareFn) {
  if (!jsvIsUndefined(compareFn) && !jsvIsFunction(compareFn)) {
    jsError("Expecting compare function, got %t", compareFn);
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
    jsvIteratorNew(&it, array);
    while (jsvIteratorHasElement(&it)) {
      n++;
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  } else {
    n = (int)jsvGetLength(array);
  }

  jsvIteratorNew(&it, array);
  _jswrap_array_sort(&it, n, compareFn);
  jsvIteratorFree(&it);
  return jsvLockAgain(array);
}

/*JSON{ "type":"method", "class": "Array", "name" : "concat", "ifndef" : "SAVE_ON_FLASH",
         "description" : "Create a new array, containing the elements from this one and any arguments, if any argument is an array then those elements will be added.",
         "generate" : "jswrap_array_concat",
         "params" : [ [ "args", "JsVarArray", "Any items to add to the array" ] ],
         "return" : [ "JsVar", "An Array" ]

}*/
JsVar *jswrap_array_concat(JsVar *parent, JsVar *args) {
  JsVar *result = jsvNewWithFlags(JSV_ARRAY);

  JsvArrayIterator argsIt;
  jsvArrayIteratorNew(&argsIt, args);

  // Append parent's elements first (parent is always an array)
  JsVar *source = jsvLockAgain(parent);
  do {
    if (jsvIsArray(source)) {
      JsvArrayIterator it;
      jsvArrayIteratorNew(&it, source);
      while (jsvArrayIteratorHasElement(&it)) {
        jsvArrayPushAndUnLock(result, jsvArrayIteratorGetElement(&it));
        jsvArrayIteratorNext(&it);
      }
      jsvArrayIteratorFree(&it);
    } else
      jsvArrayPush(result, source);
    // Next, append arguments
    jsvUnLock(source);
    source = jsvArrayIteratorHasElement(&argsIt) ? jsvArrayIteratorGetElement(&argsIt) : 0;
    jsvArrayIteratorNext(&argsIt);
  } while (source);

  jsvArrayIteratorFree(&argsIt);
  return result;
}

/*JSON{ "type":"method", "class": "Array", "name" : "fill", "ifndef" : "SAVE_ON_FLASH",
         "description" : "Fill this array with the given value, for every index `>= start` and `< end`",
         "generate" : "jswrap_array_fill",
         "params" : [ [ "value", "JsVar", "The value to fill the array with" ],
                      [ "start", "int", "Optional. The index to start from (or 0). If start is negative, it is treated as length+start where length is the length of the array" ],
                      [ "end", "JsVar", "Optional. The index to end at (or the array length). If end is negative, it is treated as length+end." ]  ],
         "return" : ["JsVar", "This array"]
}*/
JsVar *jswrap_array_fill(JsVar *parent, JsVar *value, JsVarInt start, JsVar *endVar) {
  if (!jsvIsIterable(parent)) return 0;

  JsVarInt length = jsvGetLength(parent);
  if (start < 0) start = start + length;
  if (start < 0) return 0;
  JsVarInt end = jsvIsNumeric(endVar) ? jsvGetInteger(endVar) : length;
  if (end < 0) end = end + length;
  if (end < 0) return 0;


  JsvIterator it;
  jsvIteratorNew(&it, parent);
  while (jsvIteratorHasElement(&it)) {
    JsVarInt idx = jsvGetIntegerAndUnLock(jsvIteratorGetKey(&it));
    if (idx>=start && idx<end) {
      jsvIteratorSetValue(&it, value);
    }
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);

  return jsvLockAgain(parent);
}
