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
 * JavaScript methods for Objects and Functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_object.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrapper.h"
#include "jswrap_stream.h"
#include "jswrap_functions.h"
#ifdef __MINGW32__
#include "malloc.h" // needed for alloca
#endif//__MINGW32__

/*JSON{
  "type" : "class",
  "class" : "Hardware",
  "check" : "jsvIsRoot(var)"
}
This is the built-in class for the Espruino device. It is the 'root scope', as 'Window' is for JavaScript on the desktop.
*/
/*JSON{
  "type" : "class",
  "class" : "Object",
  "check" : "jsvIsObject(var)"
}
This is the built-in class for Objects
*/
/*JSON{
  "type" : "class",
  "class" : "Function",
  "check" : "jsvIsFunction(var)"
}
This is the built-in class for Functions
*/

/*JSON{
  "type" : "property",
  "class" : "Object",
  "name" : "length",
  "generate" : "jswrap_object_length",
  "return" : ["JsVar","The length of the object"]
}
Find the length of the object
*/
JsVar *jswrap_object_length(JsVar *parent) {
  JsVarInt l;
  if (jsvIsArray(parent)) {
    l = jsvGetArrayLength(parent);
  } else if (jsvIsArrayBuffer(parent)) {
    l = (JsVarInt)jsvGetArrayBufferLength(parent);
  } else if (jsvIsString(parent)) {
    l = (JsVarInt)jsvGetStringLength(parent);
  } else if (jsvIsFunction(parent)) {
    JsVar *args = jsvGetFunctionArgumentLength(parent);
    l = jsvGetArrayLength(args);
    jsvUnLock(args);
  } else
    return 0;
  return jsvNewFromInteger(l);
}

/*JSON{
  "type" : "method",
  "class" : "Object",
  "name" : "valueOf",
  "generate" : "jswrap_object_valueOf",
  "return" : ["JsVar","The primitive value of this object"]
}
Returns the primitive value of this object.
*/
JsVar *jswrap_object_valueOf(JsVar *parent) {
  if (!parent) {
    jsExceptionHere(JSET_TYPEERROR, "Invalid type %t for valueOf", parent);
    return 0;
  }
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Object",
  "name" : "toString",
  "generate" : "jswrap_object_toString",
  "params" : [
    ["radix","JsVar","If the object is an integer, the radix (between 2 and 36) to use. NOTE: Setting a radix does not work on floating point numbers."]
  ],
  "return" : ["JsVar","A String representing the object"]
}
Convert the Object to a string
*/
JsVar *jswrap_object_toString(JsVar *parent, JsVar *arg0) {
  if (jsvIsInt(arg0) && jsvIsNumeric(parent)) {
    JsVarInt radix = jsvGetInteger(arg0);
    if (radix>=2 && radix<=36) {
      char buf[JS_NUMBER_BUFFER_SIZE];
      if (jsvIsInt(parent))
        itostr(jsvGetInteger(parent), buf, (unsigned int)radix);
      else
        ftoa_bounded_extra(jsvGetFloat(parent), buf, sizeof(buf), (int)radix, -1);
      return jsvNewFromString(buf);
    }
  }
  return jsvAsString(parent, false);
}

/*JSON{
  "type" : "method",
  "class" : "Object",
  "name" : "clone",
  "generate" : "jswrap_object_clone",
  "return" : ["JsVar","A copy of this Object"]
}
Copy this object completely
*/
JsVar *jswrap_object_clone(JsVar *parent) {
  return jsvCopy(parent);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Object",
  "name" : "keys",
  "generate_full" : "jswrap_object_keys_or_property_names(object, false)",
  "params" : [
    ["object","JsVar","The object to return keys for"]
  ],
  "return" : ["JsVar","An array of strings - one for each key on the given object"]
}
Return all enumerable keys of the given object
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "Object",
  "name" : "getOwnPropertyNames",
  "generate_full" : "jswrap_object_keys_or_property_names(object, true)",
  "params" : [
    ["object","JsVar","The Object to return a list of property names for"]
  ],
  "return" : ["JsVar","An array of the Object's own properties"]
}
Returns an array of all properties (enumerable or not) found directly on a given object.

**Note:** This doesn't currently work as it should for built-in objects and their prototypes. See bug #380
*/

// This is for Object.keys and Object.
JsVar *jswrap_object_keys_or_property_names(JsVar *obj, bool includeNonEnumerable) {
  // strings are iterable, but we shouldn't try and show keys for them
  if (jsvIsIterable(obj) && !jsvIsString(obj)) {
    JsvIsInternalChecker checkerFunction = jsvGetInternalFunctionCheckerFor(obj);

    JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
    if (!arr) return 0;
    JsvIterator it;
    jsvIteratorNew(&it, obj);
    while (jsvIteratorHasElement(&it)) {
      JsVar *key = jsvIteratorGetKey(&it);
      if (!(checkerFunction && checkerFunction(key)) || (jsvIsStringEqual(key, JSPARSE_CONSTRUCTOR_VAR))) {
        /* Not sure why constructor is included in getOwnPropertyNames, but
         * not in for (i in ...) but it is, so we must explicitly override the
         * check in jsvIsInternalObjectKey! */
        JsVar *name = jsvAsArrayIndexAndUnLock(jsvCopyNameOnly(key, false, false));
        if (name) {
          jsvArrayPushAndUnLock(arr, name);
        }
      }
      jsvUnLock(key);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);

    /* Search our built-in symbol table
       Assume that ALL builtins are non-enumerable. This isn't great but
       seems to work quite well right now! */
    if (includeNonEnumerable) {
      const JswSymList *symbols = 0;

      JsVar *protoOwner = jspGetPrototypeOwner(obj);
      if (protoOwner) {
        symbols = jswGetSymbolListForObjectProto(protoOwner);
        jsvUnLock(protoOwner);
      } else if (!jsvIsObject(obj) || jsvIsRoot(obj)) {
        // get symbols, but only if we're not doing it on a basic object
        symbols = jswGetSymbolListForObject(obj);
      }

      if (symbols) {
        unsigned int i;
        for (i=0;i<symbols->symbolCount;i++)
          jsvArrayAddString(arr, &symbols->symbolChars[symbols->symbols[i].strOffset]);
      }

      if (jsvIsArray(obj) || jsvIsString(obj)) {
        jsvArrayAddString(arr, "length");
      }
    }

    return arr;
  } else {
    jsWarn("Object.keys called on non-object");
    return 0;
  }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Object",
  "name" : "create",
  "generate" : "jswrap_object_create",
  "params" : [
    ["proto","JsVar","A prototype object","propertiesObject","JsVar","An object containing properties. NOT IMPLEMENTED"]
  ],
  "return" : ["JsVar","A new object"]
}
Creates a new object with the specified prototype object and properties. properties are currently unsupported.
*/
JsVar *jswrap_object_create(JsVar *proto, JsVar *propertiesObject) {
  if (!jsvIsObject(proto) && !jsvIsNull(proto)) {
    jsWarn("Object prototype may only be an Object or null: %t", proto);
    return 0;
  }
  if (jsvIsObject(propertiesObject)) {
    jsWarn("propertiesObject is not supported yet");
  }
  JsVar *obj = jsvNewWithFlags(JSV_OBJECT);
  if (!obj) return 0;
  if (jsvIsObject(proto))
    jsvObjectSetChild(obj, JSPARSE_INHERITS_VAR, proto);
  return obj;
}


/*JSON{
  "type" : "staticmethod",
  "class" : "Object",
  "name" : "getOwnPropertyDescriptor",
  "generate" : "jswrap_object_getOwnPropertyDescriptor",
  "params" : [
    ["obj","JsVar","The object"],
    ["name","JsVar","The name of the property"]
  ],
  "return" : ["JsVar","An object with a description of the property. The values of writable/enumerable/configurable may not be entirely correct due to Espruino's implementation."]
}
Get information on the given property in the object, or undefined
*/
JsVar *jswrap_object_getOwnPropertyDescriptor(JsVar *parent, JsVar *name) {
  if (!jswrap_object_hasOwnProperty(parent, name))
    return 0;
 
  JsVar *propName = jsvAsArrayIndex(name);
  JsVar *varName = jspGetVarNamedField(parent, propName, true);
  jsvUnLock(propName);

  assert(varName); // we had it! (apparently)
  if (!varName) return 0;

  JsVar *obj = jsvNewWithFlags(JSV_OBJECT);
  if (!obj) {
    jsvUnLock(varName);
    return 0;
  }

  //jsvTrace(varName, 5);
  JsVar *var = jsvSkipName(varName);

  bool isBuiltIn = jsvIsNewChild(varName);
  JsvIsInternalChecker checkerFunction = jsvGetInternalFunctionCheckerFor(parent);

  jsvObjectSetChild(obj, "value", var);
  jsvUnLock(jsvObjectSetChild(obj, "writable", jsvNewFromBool(true)));
  jsvUnLock(jsvObjectSetChild(obj, "enumerable", jsvNewFromBool(!checkerFunction || !checkerFunction(varName))));
  jsvUnLock(jsvObjectSetChild(obj, "configurable", jsvNewFromBool(!isBuiltIn)));

  jsvUnLock(var);
  jsvUnLock(varName);
  return obj;
}

/*JSON{
  "type" : "method",
  "class" : "Object",
  "name" : "hasOwnProperty",
  "generate" : "jswrap_object_hasOwnProperty",
  "params" : [
    ["name","JsVar","The name of the property to search for"]
  ],
  "return" : ["bool","True if it exists, false if it doesn't"]
}
Return true if the object (not its prototype) has the given property.

NOTE: This currently returns false-positives for built-in functions in prototypes
*/
bool jswrap_object_hasOwnProperty(JsVar *parent, JsVar *name) {
  JsVar *propName = jsvAsArrayIndex(name);

  bool contains = false;
  if (jsvHasChildren(parent)) {
    JsVar *foundVar =  jsvFindChildFromVar(parent, propName, false);
    if (foundVar) {
      contains = true;
      jsvUnLock(foundVar);
    }
  }

  if (!contains && !jsvIsObject(parent)) {
    /* search builtin symbol table, but not for Objects, as these
     * are descended from Objects but do not themselves contain
     * an Object's properties */
    const JswSymList *symbols = jswGetSymbolListForObject(parent);
    if (symbols) {
      char str[32];
      jsvGetString(propName, str, sizeof(str));

      JsVar *v = jswBinarySearch(symbols, parent, str);
      if (v) contains = true;
      jsvUnLock(v);
    }
  }

  jsvUnLock(propName);
  return contains;
}
// --------------------------------------------------------------------------
//                                                         Misc constructors

/*JSON{
  "type" : "constructor",
  "class" : "Boolean",
  "name" : "Boolean",
  "generate" : "jswrap_boolean_constructor",
  "params" : [
    ["value","JsVar","A single value to be converted to a number"]
  ],
  "return" : ["bool","A Boolean object"]
}
Creates a number
*/
bool jswrap_boolean_constructor(JsVar *value) {
  return jsvGetBool(value);
}


// --------------------------------------------------------------------------
//                                            These should be in EventEmitter

/** A convenience function for adding event listeners */
void jswrap_object_addEventListener(JsVar *parent, const char *eventName, void (*callback)(), JsnArgumentType argTypes) {
  JsVar *n = jsvNewFromString(eventName);
  JsVar *cb = jsvNewNativeFunction(callback, argTypes);
  jswrap_object_on(parent, n, cb);
  jsvUnLock(cb);
  jsvUnLock(n);
}

/*JSON{
  "type" : "method",
  "class" : "Object",
  "name" : "on",
  "generate" : "jswrap_object_on",
  "params" : [
    ["event","JsVar","The name of the event, for instance 'data'"],
    ["listener","JsVar","The listener to call when this event is received"]
  ]
}
Register an event listener for this object, for instance ```http.on('data', function(d) {...})```. See Node.js's EventEmitter.
*/
void jswrap_object_on(JsVar *parent, JsVar *event, JsVar *listener) {
  if (!jsvIsObject(parent)) {
      jsWarn("Parent must be a proper object - not a String, Integer, etc.");
      return;
    }
  if (!jsvIsString(event)) {
      jsWarn("First argument to EventEmitter.on(..) must be a string");
      return;
    }
  if (!jsvIsFunction(listener) && !jsvIsString(listener)) {
    jsWarn("Second argument to EventEmitter.on(..) must be a function or a String (containing code)");
    return;
  }
  char eventName[16] = "#on";
  jsvGetString(event, &eventName[3], sizeof(eventName)-4);

  JsVar *eventList = jsvFindChildFromString(parent, eventName, true);
  JsVar *eventListeners = jsvSkipName(eventList);
  if (jsvIsUndefined(eventListeners)) {
    // just add
    jsvSetValueOfName(eventList, listener);
  } else {
    if (jsvIsArray(eventListeners)) {
      // we already have an array, just add to it
      jsvArrayPush(eventListeners, listener);
    } else {
      // not an array - we need to make it an array
      JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
      jsvArrayPush(arr, eventListeners);
      jsvArrayPush(arr, listener);
      jsvSetValueOfName(eventList, arr);
      jsvUnLock(arr);
    }
  }
  jsvUnLock(eventListeners);
  jsvUnLock(eventList);
  /* Special case if we're a data listener and data has already arrived then
   * we queue an event immediately. */
  if (jsvIsStringEqual(event, "data")) {
    JsVar *buf = jsvObjectGetChild(parent, STREAM_BUFFER_NAME, 0);
    if (jsvIsString(buf)) {
      jsiQueueObjectCallbacks(parent, "#ondata", &buf, 1);
      jsvRemoveNamedChild(parent, STREAM_BUFFER_NAME);
    }
    jsvUnLock(buf);
  }
}

/*JSON{
  "type" : "method",
  "class" : "Object",
  "name" : "emit",
  "generate" : "jswrap_object_emit",
  "params" : [
    ["event","JsVar","The name of the event, for instance 'data'"],
    ["args","JsVarArray","Optional arguments"]
  ]
}
Call the event listeners for this object, for instance ```http.emit('data', 'Foo')```. See Node.js's EventEmitter.
*/
void jswrap_object_emit(JsVar *parent, JsVar *event, JsVar *argArray) {
  if (!jsvIsObject(parent)) {
      jsWarn("Parent must be a proper object - not a String, Integer, etc.");
      return;
    }
  if (!jsvIsString(event)) {
    jsWarn("First argument to EventEmitter.emit(..) must be a string");
    return;
  }
  char eventName[16] = "#on";
  jsvGetString(event, &eventName[3], sizeof(eventName)-4);

  // extract data
  const int MAX_ARGS = 4;
  JsVar *args[MAX_ARGS];
  int n = 0;
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, argArray);
  while (jsvObjectIteratorHasValue(&it)) {
    if (n>=MAX_ARGS) {
      jsWarn("Too many arguments");
      break;
    }
    args[n++] = jsvObjectIteratorGetValue(&it);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);


  jsiQueueObjectCallbacks(parent, eventName, args, n);
  // unlock
  while (n--)
    jsvUnLock(args[n]);
}

/*JSON{
  "type" : "method",
  "class" : "Object",
  "name" : "removeAllListeners",
  "generate" : "jswrap_object_removeAllListeners",
  "params" : [
    ["event","JsVar","The name of the event, for instance 'data'"]
  ]
}
Removes all listeners, or those of the specified event.
*/
void jswrap_object_removeAllListeners(JsVar *parent, JsVar *event) {
  if (!jsvIsObject(parent)) {
      jsWarn("Parent must be a proper object - not a String, Integer, etc.");
      return;
    }
  if (jsvIsString(event)) {
    // remove the whole child containing listeners
    char eventName[16] = "#on";
    jsvGetString(event, &eventName[3], sizeof(eventName)-4);
    JsVar *eventList = jsvFindChildFromString(parent, eventName, true);
    if (eventList) {
      jsvRemoveChild(parent, eventList);
      jsvUnLock(eventList);
    }
  } else if (jsvIsUndefined(event)) {
    // Eep. We must remove everything beginning with '#on'
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, parent);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *key = jsvObjectIteratorGetKey(&it);
      jsvObjectIteratorNext(&it);
      if (jsvIsString(key) &&
          key->varData.str[0]=='#' &&
          key->varData.str[1]=='o' &&
          key->varData.str[2]=='n') {
        // begins with #on - we must kill it
        jsvRemoveChild(parent, key);
      }
      jsvUnLock(key);
    }
    jsvObjectIteratorFree(&it);
  } else {
    jsWarn("First argument to EventEmitter.removeAllListeners(..) must be a string, or undefined");
    return;
  }
}

// For internal use - like jswrap_object_removeAllListeners but takes a C string
void jswrap_object_removeAllListeners_cstr(JsVar *parent, const char *event) {
  JsVar *s = jsvNewFromString(event);
  if (s) {
    jswrap_object_removeAllListeners(parent, s);
    jsvUnLock(s);
  }
}

// ------------------------------------------------------------------------------

/*JSON{
  "type" : "method",
  "class" : "Function",
  "name" : "replaceWith",
  "generate" : "jswrap_function_replaceWith",
  "params" : [
    ["newFunc","JsVar","The new function to replace this function with"]
  ]
}
This replaces the function with the one in the argument - while keeping the old function's scope. This allows inner functions to be edited, and is used when edit() is called on an inner function.
*/
void jswrap_function_replaceWith(JsVar *oldFunc, JsVar *newFunc) {
  if (!jsvIsFunction(newFunc)) {
    jsWarn("First argument of replaceWith should be a function - ignoring");
    return;
  }
  // Grab scope - the one thing we want to keep
  JsVar *scope = jsvFindChildFromString(oldFunc, JSPARSE_FUNCTION_SCOPE_NAME, false);
  // so now remove all existing entries
  jsvRemoveAllChildren(oldFunc);
  // now re-add scope
  if (scope) jsvAddName(oldFunc, scope);
  jsvUnLock(scope);
  // now re-add other entries
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, newFunc);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *el = jsvObjectIteratorGetKey(&it);
    jsvObjectIteratorNext(&it);
    if (!jsvIsStringEqual(el, JSPARSE_FUNCTION_SCOPE_NAME)) {
      JsVar *copy = jsvCopy(el);
      if (copy) {
        jsvAddName(oldFunc, copy);
        jsvUnLock(copy);
      }
    }
    jsvUnLock(el);
  }
  jsvObjectIteratorFree(&it);

}

/*JSON{
  "type" : "method",
  "class" : "Function",
  "name" : "call",
  "generate" : "jswrap_function_apply_or_call",
  "params" : [
    ["this","JsVar","The value to use as the 'this' argument when executing the function"],
    ["params","JsVarArray","Optional Parameters"]
  ],
  "return" : ["JsVar","The return value of executing this function"]
}
This executes the function with the supplied 'this' argument and parameters
*/
// ... it just so happens that the way JsVarArray is parsed means that apply and call can be exactly the same function!


/*JSON{
  "type" : "method",
  "class" : "Function",
  "name" : "apply",
  "generate" : "jswrap_function_apply_or_call",
  "params" : [
    ["this","JsVar","The value to use as the 'this' argument when executing the function"],
    ["args","JsVar","Optional Array of Arguments"]
  ],
  "return" : ["JsVar","The return value of executing this function"]
}
This executes the function with the supplied 'this' argument and parameters
*/
JsVar *jswrap_function_apply_or_call(JsVar *parent, JsVar *thisArg, JsVar *argsArray) {
  unsigned int i;
  JsVar **args = 0;
  size_t argC = 0;

  if (jsvIsIterable(argsArray)) {
    argC = (size_t)jsvGetLength(argsArray);
    if (argC>64) {
      jsExceptionHere(JSET_ERROR, "Array passed to Function.apply is too big! Maximum 64 arguments, got %d", argC);
      return 0;
    }
    args = (JsVar**)alloca((size_t)argC * sizeof(JsVar*));
    for (i=0;i<argC;i++) args[i] = 0;

    JsvIterator it;
    jsvIteratorNew(&it, argsArray);
    while (jsvIteratorHasElement(&it)) {
      JsVarInt idx = jsvGetIntegerAndUnLock(jsvIteratorGetKey(&it));
      if (idx>=0 && idx<(int)argC) {
        assert(!args[idx]); // just in case there were dups
        args[idx] = jsvIteratorGetValue(&it);
      }
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  } else if (!jsvIsUndefined(argsArray)) {
    jsExceptionHere(JSET_ERROR, "Second argument to Function.apply must be iterable, got %t", argsArray);
    return 0;
  }

  JsVar *r = jspeFunctionCall(parent, 0, thisArg, false, (int)argC, args);
  for (i=0;i<argC;i++) jsvUnLock(args[i]);
  return r;
}
