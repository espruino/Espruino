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
  "type" : "constructor",
  "class" : "Object",
  "name" : "Object",
  "generate" : "jswrap_object_constructor",
  "params" : [
    ["value","JsVar","A single value to be converted to an object"]
  ],
  "return" : ["JsVar","An Object"]
}
Creates an Object from the supplied argument
 */
JsVar *jswrap_object_constructor(JsVar *value) {
  if (jsvIsObject(value) || jsvIsArray(value) || jsvIsFunction(value))
    return jsvLockAgain(value);
  const char *objName = jswGetBasicObjectName(value);
  JsVar *funcName = objName ? jspGetNamedVariable(objName) : 0;
  if (!funcName) return jsvNewObject();
  JsVar *func = jsvSkipName(funcName);
  JsVar *result = jspeFunctionCall(func, funcName, 0, false, 1, &value);
  jsvUnLock2(funcName, func);
  return result;
}

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
  "generate_full" : "jswrap_object_keys_or_property_names(object, false, false)",
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
  "generate_full" : "jswrap_object_keys_or_property_names(object, true, false)",
  "params" : [
    ["object","JsVar","The Object to return a list of property names for"]
  ],
  "return" : ["JsVar","An array of the Object's own properties"]
}
Returns an array of all properties (enumerable or not) found directly on a given object.
*/


void _jswrap_object_keys_or_property_names_iterator(
    const JswSymList *symbols,
    void (*callback)(void *data, JsVar *name),
    void *data) {
  if (!symbols) return;
  unsigned int i;
  unsigned char symbolCount = READ_FLASH_UINT8(&symbols->symbolCount);
  for (i=0;i<symbolCount;i++) {
    unsigned short strOffset = READ_FLASH_UINT16(&symbols->symbols[i].strOffset);
#ifndef USE_FLASH_MEMORY
    JsVar *name = jsvNewFromString(&symbols->symbolChars[strOffset]);
#else
    // On the esp8266 the string is in flash, so we have to copy it to RAM first
    // We can't use flash_strncpy here because it assumes that strings start on a word
    // boundary and that's not the case here.
    char buf[64], *b = buf, c; const char *s = &symbols->symbolChars[strOffset];
    do { c = READ_FLASH_UINT8(s++); *b++ = c; } while (c && b != buf+64);
    JsVar *name = jsvNewFromString(buf);
#endif
    //os_printf_plus("OBJ cb %s\n", buf);
    callback(data, name);
    jsvUnLock(name);
  }
}

/** This is for Object.keys and Object. However it uses a callback so doesn't allocate anything */
void jswrap_object_keys_or_property_names_cb(
    JsVar *obj,
    bool includeNonEnumerable,  ///< include 'hidden' items
    bool includePrototype, ///< include items for the prototype too (for autocomplete)
    void (*callback)(void *data, JsVar *name),
    void *data
) {
  // strings are iterable, but we shouldn't try and show keys for them
  if (jsvIsIterable(obj)) {
    JsvIsInternalChecker checkerFunction = jsvGetInternalFunctionCheckerFor(obj);

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
          callback(data, name);
          jsvUnLock(name);
        }
      }
      jsvUnLock(key);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  }

  /* Search our built-in symbol table
     Assume that ALL builtins are non-enumerable. This isn't great but
     seems to work quite well right now! */
  if (includeNonEnumerable) {
    JsVar *protoOwner = jspGetPrototypeOwner(obj);
    if (protoOwner) {
      // If protoOwner then this is the prototype (protoOwner is the object)
      const JswSymList *symbols = jswGetSymbolListForObjectProto(protoOwner);
      jsvUnLock(protoOwner);
      _jswrap_object_keys_or_property_names_iterator(symbols, callback, data);
    } else if (!jsvIsObject(obj) || jsvIsRoot(obj)) {
      // get symbols, but only if we're not doing it on a basic object
       const JswSymList *symbols = jswGetSymbolListForObject(obj);
      _jswrap_object_keys_or_property_names_iterator(symbols, callback, data);
    }

    if (includePrototype) {
      if (jsvIsObject(obj) || jsvIsFunction(obj)) {
        JsVar *proto = jsvObjectGetChild(obj, JSPARSE_INHERITS_VAR, 0);
        while (jsvIsObject(proto)) {
          const JswSymList *symbols = jswGetSymbolListForObjectProto(proto);
          _jswrap_object_keys_or_property_names_iterator(symbols, callback, data);
          JsVar *p2 = jsvObjectGetChild(proto, JSPARSE_INHERITS_VAR, 0);
          jsvUnLock(proto);
          proto = p2;
        }
      }
      // include Object/String/etc
      const JswSymList *symbols = jswGetSymbolListForObjectProto(obj);
      _jswrap_object_keys_or_property_names_iterator(symbols, callback, data);
      // if the last call wasn't an Object, add the object proto as well
      const JswSymList *objSymbols = jswGetSymbolListForObjectProto(0);
      if (objSymbols!=symbols)
        _jswrap_object_keys_or_property_names_iterator(objSymbols, callback, data);
    }

    if (jsvIsArray(obj) || jsvIsString(obj)) {
      JsVar *name = jsvNewFromString("length");
      callback(data, name);
      jsvUnLock(name);
    }
  }
}

JsVar *jswrap_object_keys_or_property_names(
    JsVar *obj,
    bool includeNonEnumerable,  ///< include 'hidden' items
    bool includePrototype ///< include items for the prototype too (for autocomplete)
    ) {
  JsVar *arr = jsvNewEmptyArray();
  if (!arr) return 0;

  jswrap_object_keys_or_property_names_cb(obj, includeNonEnumerable, includePrototype, (void (*)(void *, JsVar *))jsvArrayAddUnique, arr);

  return arr;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Object",
  "name" : "create",
  "generate" : "jswrap_object_create",
  "params" : [
    ["proto","JsVar","A prototype object"],
    ["propertiesObject","JsVar","An object containing properties. NOT IMPLEMENTED"]
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
  JsVar *obj = jsvNewObject();
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

  JsVar *obj = jsvNewObject();
  if (!obj) {
    jsvUnLock(varName);
    return 0;
  }

  //jsvTrace(varName, 5);
  JsVar *var = jsvSkipName(varName);

  bool isBuiltIn = jsvIsNewChild(varName);
  JsvIsInternalChecker checkerFunction = jsvGetInternalFunctionCheckerFor(parent);

  jsvObjectSetChild(obj, "value", var);
  jsvObjectSetChildAndUnLock(obj, "writable", jsvNewFromBool(true));
  jsvObjectSetChildAndUnLock(obj, "enumerable", jsvNewFromBool(!checkerFunction || !checkerFunction(varName)));
  jsvObjectSetChildAndUnLock(obj, "configurable", jsvNewFromBool(!isBuiltIn));

  jsvUnLock2(var, varName);
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

/*JSON{
  "type" : "staticmethod",
  "class" : "Object",
  "name" : "defineProperty",
  "generate" : "jswrap_object_defineProperty",
  "params" : [
    ["obj","JsVar","An object"],
    ["name","JsVar","The name of the property"],
    ["desc","JsVar","The property descriptor"]
  ],
  "return" : ["JsVar","The object, obj."]
}
Add a new property to the Object. 'Desc' is an object with the following fields:

* `configurable` (bool = false) - can this property be changed/deleted
* `enumerable` (bool = false) - can this property be enumerated
* `value` (anything) - the value of this property
* `writable` (bool = false) - can the value be changed with the assignment operator?
* `get` (function) - the getter function, or undefined if no getter
* `set` (function) - the setter function, or undefined if no setter
*
**Note:** `configurable`, `enumerable`, `writable`, `get`, and `set` are not implemented and will be ignored.
 */
JsVar *jswrap_object_defineProperty(JsVar *parent, JsVar *propName, JsVar *desc) {
  if (!jsvIsObject(parent)) {
    jsExceptionHere(JSET_ERROR, "First argument must be an object, got %t", parent);
    return 0;
  }
  if (!jsvIsObject(desc)) {
    jsExceptionHere(JSET_ERROR, "Property description must be an object, got %t", desc);
    return 0;
  }

  JsVar *name = jsvAsArrayIndex(propName);
  JsVar *value = jsvObjectGetChild(desc, "value", 0);
  JsVar *property = jsvFindChildFromVar(parent, name, true);
  jsvUnLock(name);
  if (property && value)
    jsvSetValueOfName(property, value);
  jsvUnLock2(property, value);

  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Object",
  "name" : "defineProperties",
  "generate" : "jswrap_object_defineProperties",
  "params" : [
    ["obj","JsVar","An object"],
    ["props","JsVar","An object whose fields represent property names, and whose values are property descriptors."]
  ],
  "return" : ["JsVar","The object, obj."]
}
Adds new properties to the Object. See `Object.defineProperty` for more information
 */
JsVar *jswrap_object_defineProperties(JsVar *parent, JsVar *props) {
  if (!jsvIsObject(parent)) {
    jsExceptionHere(JSET_ERROR, "First argument must be an object, got %t\n", parent);
    return 0;
  }
  if (!jsvIsObject(props)) {
    jsExceptionHere(JSET_ERROR, "Second argument must be an object, got %t\n", props);
    return 0;
  }

  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, props);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *name = jsvObjectIteratorGetKey(&it);
    JsVar *desc = jsvObjectIteratorGetValue(&it);
    jsvUnLock3(jswrap_object_defineProperty(parent, name, desc), name, desc);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);

  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Object",
  "name" : "getPrototypeOf",
  "generate" : "jswrap_object_getPrototypeOf",
  "params" : [
    ["object","JsVar","An object"]
  ],
  "return" : ["JsVar","The prototype"]
}
Get the prototype of the given object - this is like writing `object.__proto__`
but is the 'proper' ES6 way of doing it
 */
JsVar *jswrap_object_getPrototypeOf(JsVar *object) {
  return jspGetNamedField(object, "__proto__", false);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Object",
  "name" : "setPrototypeOf",
  "generate" : "jswrap_object_setPrototypeOf",
  "params" : [
    ["object","JsVar","An object"],
    ["prototype","JsVar","The prototype to set on the object"]
  ],
  "return" : ["JsVar","The object passed in"]
}
Set the prototype of the given object - this is like writing
`object.__proto__ = prototype` but is the 'proper' ES6 way of doing it
 */
JsVar *jswrap_object_setPrototypeOf(JsVar *object, JsVar *proto) {
  JsVar *v = jspGetNamedField(object, "__proto__", true);
  if (!jsvIsName(v)) {
    jsExceptionHere(JSET_TYPEERROR, "Can't extend this object\n");
  } else {
    jsvSetValueOfName(v, proto);
  }
  jsvUnLock(v);
  return jsvLockAgain(object);
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
Creates a boolean
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
  jsvUnLock2(cb, n);
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
  if (!jsvHasChildren(parent)) {
    jsWarn("Parent must be an object - not a String, Integer, etc.");
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

  JsVar *eventName = jsvVarPrintf(JS_EVENT_PREFIX"%v",event);
  if (!eventName) return; // no memory

  JsVar *eventList = jsvFindChildFromVar(parent, eventName, true);
  jsvUnLock(eventName);
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
      JsVar *arr = jsvNewEmptyArray();
      jsvArrayPush(arr, eventListeners);
      jsvArrayPush(arr, listener);
      jsvSetValueOfName(eventList, arr);
      jsvUnLock(arr);
    }
  }
  jsvUnLock2(eventListeners, eventList);
  /* Special case if we're a data listener and data has already arrived then
   * we queue an event immediately. */
  if (jsvIsStringEqual(event, "data")) {
    JsVar *buf = jsvObjectGetChild(parent, STREAM_BUFFER_NAME, 0);
    if (jsvIsString(buf)) {
      jsiQueueObjectCallbacks(parent, STREAM_CALLBACK_NAME, &buf, 1);
      jsvObjectRemoveChild(parent, STREAM_BUFFER_NAME);
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
  if (!jsvHasChildren(parent)) {
    jsWarn("Parent must be an object - not a String, Integer, etc.");
    return;
  }
  if (!jsvIsString(event)) {
    jsWarn("First argument to EventEmitter.emit(..) must be a string");
    return;
  }
  JsVar *eventName = jsvVarPrintf(JS_EVENT_PREFIX"%v", event);
  if (!eventName) return; // no memory

  // extract data
  const unsigned int MAX_ARGS = 4;
  JsVar *args[MAX_ARGS];
  unsigned int n = 0;
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


  JsVar *callback = jsvSkipNameAndUnLock(jsvFindChildFromVar(parent, eventName, 0));
  jsvUnLock(eventName);
  if (callback) jsiQueueEvents(parent, callback, args, (int)n);
  jsvUnLock(callback);

  // unlock
  jsvUnLockMany(n, args);
}

/*JSON{
  "type" : "method",
  "class" : "Object",
  "name" : "removeListener",
  "generate" : "jswrap_object_removeListener",
  "params" : [
    ["event","JsVar","The name of the event, for instance 'data'"],
    ["listener","JsVar","The listener to remove"]
  ]
}
Removes the specified event listener.

```
function foo(d) {
  console.log(d);
}
Serial1.on("data", foo);
Serial1.removeListener("data", foo);
```
 */
void jswrap_object_removeListener(JsVar *parent, JsVar *event, JsVar *callback) {
  if (!jsvHasChildren(parent)) {
    jsWarn("Parent must be an object - not a String, Integer, etc.");
    return;
  }
  if (jsvIsString(event)) {
    // remove the whole child containing listeners
    JsVar *eventName = jsvVarPrintf(JS_EVENT_PREFIX"%v", event);
    if (!eventName) return; // no memory
    JsVar *eventListName = jsvFindChildFromVar(parent, eventName, true);
    jsvUnLock(eventName);
    JsVar *eventList = jsvSkipName(eventListName);
    if (eventList) {
      if (eventList == callback) {
        // there's no array, it was a single item
        jsvRemoveChild(parent, eventListName);
      } else if (jsvIsArray(eventList)) {
        // it's an array, search for the index
        JsVar *idx = jsvGetArrayIndexOf(eventList, callback, true);
        if (idx) {
          jsvRemoveChild(eventList, idx);
          jsvUnLock(idx);
        }
      }
      jsvUnLock(eventList);
    }
    jsvUnLock(eventListName);
  } else {
    jsWarn("First argument to EventEmitter.removeListener(..) must be a string");
    return;
  }
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
  if (!jsvHasChildren(parent)) {
    jsWarn("Parent must be an object - not a String, Integer, etc.");
    return;
  }
  if (jsvIsString(event)) {
    // remove the whole child containing listeners
    JsVar *eventName = jsvVarPrintf(JS_EVENT_PREFIX"%v", event);
    if (!eventName) return; // no memory

    JsVar *eventList = jsvFindChildFromVar(parent, eventName, true);
    jsvUnLock(eventName);
    if (eventList) {
      jsvRemoveChild(parent, eventList);
      jsvUnLock(eventList);
    }
  } else if (jsvIsUndefined(event)) {
    // Eep. We must remove everything beginning with '#on' (JS_EVENT_PREFIX)
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, parent);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *key = jsvObjectIteratorGetKey(&it);
      jsvObjectIteratorNext(&it);
      if (jsvIsStringEqualOrStartsWith(key, JS_EVENT_PREFIX, true)) {
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
  // If old was native or vice versa...
  if (jsvIsNativeFunction(oldFunc) != jsvIsNativeFunction(newFunc)) {
    if (jsvIsNativeFunction(newFunc))
      oldFunc->flags |= JSV_NATIVE;
    else
      oldFunc->flags &= ~JSV_NATIVE;
  }
  // If old fn started with 'return' or vice versa...
  if (jsvIsFunctionReturn(oldFunc) != jsvIsFunctionReturn(newFunc)) {
    if (jsvIsFunctionReturn(newFunc))
      oldFunc->flags = (oldFunc->flags&~JSV_VARTYPEMASK) |JSV_FUNCTION_RETURN;
    else
      oldFunc->flags = (oldFunc->flags&~JSV_VARTYPEMASK) |JSV_FUNCTION;
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
  unsigned int argC = 0;

  if (jsvIsIterable(argsArray)) {
    argC = (unsigned int)jsvGetLength(argsArray);
    if (argC>64) {
      jsExceptionHere(JSET_ERROR, "Array passed to Function.apply is too big! Maximum 64 arguments, got %d", argC);
      return 0;
    }
    args = (JsVar**)alloca((size_t)argC * sizeof(JsVar*));
    for (i=0;i<argC;i++) args[i] = 0;
    // TODO: Use jsvGetArrayItems?
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
  jsvUnLockMany(argC, args);
  return r;
}

/*JSON{
  "type" : "method",
  "class" : "Function",
  "name" : "bind",
  "generate" : "jswrap_function_bind",
  "params" : [
    ["this","JsVar","The value to use as the 'this' argument when executing the function"],
    ["params","JsVarArray","Optional Default parameters that are prepended to the call"]
  ],
  "return" : ["JsVar","The 'bound' function"]
}
This executes the function with the supplied 'this' argument and parameters
 */
JsVar *jswrap_function_bind(JsVar *parent, JsVar *thisArg, JsVar *argsArray) {
  if (!jsvIsFunction(parent)) {
    jsExceptionHere(JSET_TYPEERROR, "Function.bind expects to be called on function, got %t", parent);
    return 0;
  }
  JsVar *fn;
  if (jsvIsNativeFunction(parent))
    fn = jsvNewNativeFunction(parent->varData.native.ptr, parent->varData.native.argTypes);
  else
    fn = jsvNewWithFlags(jsvIsFunctionReturn(parent) ? JSV_FUNCTION_RETURN : JSV_FUNCTION);
  if (!fn) return 0;

  // Old function info
  JsvObjectIterator fnIt;
  jsvObjectIteratorNew(&fnIt, parent);
  // add previously bound arguments
  while (jsvObjectIteratorHasValue(&fnIt)) {
    JsVar *param = jsvObjectIteratorGetKey(&fnIt);
    JsVar *defaultValue = jsvObjectIteratorGetValue(&fnIt);
    bool wasBound = jsvIsFunctionParameter(param) && defaultValue;
    if (wasBound) {
      JsVar *newParam = jsvCopy(param);
      if (newParam) { // could be out of memory
        jsvAddName(fn, newParam);
        jsvUnLock(newParam);
      }
    }
    jsvUnLock2(param, defaultValue);
    if (!wasBound) break;
    jsvObjectIteratorNext(&fnIt);
  }

  // add bound arguments
  if (argsArray) {
    JsvObjectIterator argIt;
    jsvObjectIteratorNew(&argIt, argsArray);
    while (jsvObjectIteratorHasValue(&argIt)) {
      JsVar *defaultValue = jsvObjectIteratorGetValue(&argIt);
      bool addedParam = false;
      while (!addedParam && jsvObjectIteratorHasValue(&fnIt)) {
        JsVar *param = jsvObjectIteratorGetKey(&fnIt);
        if (!jsvIsFunctionParameter(param)) {
          jsvUnLock(param);
          break;
        }
        JsVar *newParam = jsvCopyNameOnly(param, false,  true);
        jsvSetValueOfName(newParam, defaultValue);
        jsvAddName(fn, newParam);
        addedParam = true;
        jsvUnLock2(param, newParam);
        jsvObjectIteratorNext(&fnIt);
      }

      if (!addedParam) {
        JsVar *paramName = jsvNewFromEmptyString();
        if (paramName) {
          jsvMakeFunctionParameter(paramName); // force this to be called a function parameter
          jsvSetValueOfName(paramName, defaultValue);
          jsvAddName(fn, paramName);
          jsvUnLock(paramName);
        }
      }
      jsvUnLock(defaultValue);
      jsvObjectIteratorNext(&argIt);
    }
    jsvObjectIteratorFree(&argIt);
  }

  // Copy the rest of the old function's info
  while (jsvObjectIteratorHasValue(&fnIt)) {
    JsVar *param = jsvObjectIteratorGetKey(&fnIt);
    JsVar *newParam = jsvCopyNameOnly(param, true, true);
    if (newParam) { // could be out of memory
      jsvAddName(fn, newParam);
      jsvUnLock(newParam);
    }
    jsvUnLock(param);
    jsvObjectIteratorNext(&fnIt);
  }
  jsvObjectIteratorFree(&fnIt);
  // Add 'this'
  jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, thisArg); // no unlock needed

  return fn;
}
