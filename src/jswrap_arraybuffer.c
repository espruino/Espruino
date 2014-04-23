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
 * JavaScript methods for ArrayBuffers
 * ----------------------------------------------------------------------------
 */
#include "jswrap_arraybuffer.h"
#include "jsparse.h"
#include "jsinteractive.h"

/*JSON{ "type":"class",
        "class" : "ArrayBuffer",
        "check" : "jsvIsArrayBuffer(var) && var->varData.arraybuffer.type==ARRAYBUFFERVIEW_ARRAYBUFFER",
        "not_real_object" : "Don't treat this as a real object - it's handled differently internally",
        "description" : ["This is the built-in JavaScript class for array buffers." ]
}*/

/*JSON{ "type":"class",
        "class" : "ArrayBufferView",
        "description" : ["This is the built-in JavaScript class that is the prototype for Uint8Array / Float32Array / etc" ]
}*/

/*JSON{ "type":"class",
        "class" : "Uint8Array", "prototype" : "ArrayBufferView",
        "check" : "jsvIsArrayBuffer(var) && var->varData.arraybuffer.type==ARRAYBUFFERVIEW_UINT8",
        "not_real_object" : "Don't treat this as a real object - it's handled differently internally",
        "description" : ["This is the built-in JavaScript class for a typed array.",
                         "Instantiate this in order to efficiently store arrays of data (Espruino's normal arrays store data in a map, which is inefficient for non-sparse arrays). " ]
}*/
/*JSON{ "type":"class",
        "class" : "Int8Array", "prototype" : "ArrayBufferView",
        "check" : "jsvIsArrayBuffer(var) && var->varData.arraybuffer.type==ARRAYBUFFERVIEW_INT8",
        "not_real_object" : "Don't treat this as a real object - it's handled differently internally",
        "description" : ["This is the built-in JavaScript class for a typed array.",
                         "Instantiate this in order to efficiently store arrays of data (Espruino's normal arrays store data in a map, which is inefficient for non-sparse arrays). " ]
}*/
/*JSON{ "type":"class",
        "class" : "Uint16Array", "prototype" : "ArrayBufferView",
        "check" : "jsvIsArrayBuffer(var) && var->varData.arraybuffer.type==ARRAYBUFFERVIEW_UINT16",
        "not_real_object" : "Don't treat this as a real object - it's handled differently internally",
        "description" : ["This is the built-in JavaScript class for a typed array.",
                         "Instantiate this in order to efficiently store arrays of data (Espruino's normal arrays store data in a map, which is inefficient for non-sparse arrays). " ]
}*/
/*JSON{ "type":"class",
        "class" : "Int16Array", "prototype" : "ArrayBufferView",
        "check" : "jsvIsArrayBuffer(var) && var->varData.arraybuffer.type==ARRAYBUFFERVIEW_INT16",
        "not_real_object" : "Don't treat this as a real object - it's handled differently internally",
        "description" : ["This is the built-in JavaScript class for a typed array.",
                         "Instantiate this in order to efficiently store arrays of data (Espruino's normal arrays store data in a map, which is inefficient for non-sparse arrays). " ]
}*/
/*JSON{ "type":"class",
        "class" : "Uint32Array", "prototype" : "ArrayBufferView",
        "check" : "jsvIsArrayBuffer(var) && var->varData.arraybuffer.type==ARRAYBUFFERVIEW_UINT32",
        "not_real_object" : "Don't treat this as a real object - it's handled differently internally",
        "description" : ["This is the built-in JavaScript class for a typed array.",
                         "Instantiate this in order to efficiently store arrays of data (Espruino's normal arrays store data in a map, which is inefficient for non-sparse arrays). " ]
}*/
/*JSON{ "type":"class",
        "class" : "Int32Array", "prototype" : "ArrayBufferView",
        "check" : "jsvIsArrayBuffer(var) && var->varData.arraybuffer.type==ARRAYBUFFERVIEW_INT32",
        "not_real_object" : "Don't treat this as a real object - it's handled differently internally",
        "description" : ["This is the built-in JavaScript class for a typed array.",
                         "Instantiate this in order to efficiently store arrays of data (Espruino's normal arrays store data in a map, which is inefficient for non-sparse arrays). " ]
}*/
/*JSON{ "type":"class",
        "class" : "Float32Array", "prototype" : "ArrayBufferView",
        "check" : "jsvIsArrayBuffer(var) && var->varData.arraybuffer.type==ARRAYBUFFERVIEW_FLOAT32",
        "not_real_object" : "Don't treat this as a real object - it's handled differently internally",
        "description" : ["This is the built-in JavaScript class for a typed array.",
                         "Instantiate this in order to efficiently store arrays of data (Espruino's normal arrays store data in a map, which is inefficient for non-sparse arrays). " ]
}*/
/*JSON{ "type":"class",
        "class" : "Float64Array", "prototype" : "ArrayBufferView",
        "check" : "jsvIsArrayBuffer(var) && var->varData.arraybuffer.type==ARRAYBUFFERVIEW_FLOAT64",
        "not_real_object" : "Don't treat this as a real object - it's handled differently internally",
        "description" : ["This is the built-in JavaScript class for a typed array.",
                         "Instantiate this in order to efficiently store arrays of data (Espruino's normal arrays store data in a map, which is inefficient for non-sparse arrays). " ]
}*/


/*JSON{ "type":"constructor", "class": "ArrayBuffer",  "name": "ArrayBuffer",
         "description" : "Create an Array Buffer object",
         "generate" : "jswrap_arraybuffer_constructor",
         "params" : [ [ "byteLength", "int", "The length in Bytes" ] ],
         "return" : [ "JsVar", "An ArrayBuffer object" ]

}*/
JsVar *jswrap_arraybuffer_constructor(JsVarInt byteLength) {
  if (byteLength <= 0 || byteLength>65535) {
    jsError("Invalid length for ArrayBuffer\n");
    return 0;
  }
  if (byteLength > JSV_ARRAYBUFFER_MAX_LENGTH) {
    jsError("ArrayBuffer too long\n");
    return 0;
  }
  JsVar *arrData = jsvNewStringOfLength((unsigned int)byteLength);
  if (!arrData) return 0;
  JsVar *v = jsvNewArrayBufferFromString(arrData, (unsigned int)byteLength);
  jsvUnLock(arrData);
  return v;
}


/*
 * Potential invocations:
 * Uint8Array Uint8Array(unsigned long length);
 * Uint8Array Uint8Array(TypedArray array);
 * Uint8Array Uint8Array(sequence<type> array);
 * Uint8Array Uint8Array(ArrayBuffer buffer, optional unsigned long byteOffset, optional unsigned long length);
 */

/*JSON{ "type":"constructor", "class": "Uint8Array",  "name": "Uint8Array",
         "description" : "Create a typed array based on the given input. Either an existing Array Buffer, an Integer as a Length, or a simple array. If an ArrayBuffer view (eg. Uint8Array rather than ArrayBuffer) is given, it will be completely copied rather than referenced.",
         "generate_full" : "jswrap_typedarray_constructor(ARRAYBUFFERVIEW_UINT8, arr, byteOffset, length)",
         "params" : [ [ "arr", "JsVar", "The array or typed array to base this off, or an integer which is the array length" ],
                      [ "byteOffset", "int", "The byte offset in the ArrayBuffer  (ONLY IF the first argument was an ArrayBuffer)" ],
                      [ "length", "int", "The length (ONLY IF the first argument was an ArrayBuffer)" ] ],
         "return" : [ "JsVar", "A typed array" ]
}*/
/*JSON{ "type":"constructor", "class": "Int8Array",  "name": "Int8Array",
         "description" : "Create a typed array based on the given input. Either an existing Array Buffer, an Integer as a Length, or a simple array. If an ArrayBuffer view (eg. Uint8Array rather than ArrayBuffer) is given, it will be completely copied rather than referenced.",
         "generate_full" : "jswrap_typedarray_constructor(ARRAYBUFFERVIEW_INT8, arr, byteOffset, length)",
         "params" : [ [ "arr", "JsVar", "The array or typed array to base this off, or an integer which is the array length" ],
                      [ "byteOffset", "int", "The byte offset in the ArrayBuffer  (ONLY IF the first argument was an ArrayBuffer)" ],
                      [ "length", "int", "The length (ONLY IF the first argument was an ArrayBuffer)" ] ],
         "return" : [ "JsVar", "A typed array" ]
}*/
/*JSON{ "type":"constructor", "class": "Uint16Array",  "name": "Uint16Array",
         "description" : "Create a typed array based on the given input. Either an existing Array Buffer, an Integer as a Length, or a simple array. If an ArrayBuffer view (eg. Uint8Array rather than ArrayBuffer) is given, it will be completely copied rather than referenced.",
         "generate_full" : "jswrap_typedarray_constructor(ARRAYBUFFERVIEW_UINT16, arr, byteOffset, length)",
         "params" : [ [ "arr", "JsVar", "The array or typed array to base this off, or an integer which is the array length" ],
                      [ "byteOffset", "int", "The byte offset in the ArrayBuffer  (ONLY IF the first argument was an ArrayBuffer)" ],
                      [ "length", "int", "The length (ONLY IF the first argument was an ArrayBuffer)" ] ],
         "return" : [ "JsVar", "A typed array" ]
}*/
/*JSON{ "type":"constructor", "class": "Int16Array",  "name": "Int16Array",
         "description" : "Create a typed array based on the given input. Either an existing Array Buffer, an Integer as a Length, or a simple array. If an ArrayBuffer view (eg. Uint8Array rather than ArrayBuffer) is given, it will be completely copied rather than referenced.",
         "generate_full" : "jswrap_typedarray_constructor(ARRAYBUFFERVIEW_INT16, arr, byteOffset, length)",
         "params" : [ [ "arr", "JsVar", "The array or typed array to base this off, or an integer which is the array length" ],
                      [ "byteOffset", "int", "The byte offset in the ArrayBuffer  (ONLY IF the first argument was an ArrayBuffer)" ],
                      [ "length", "int", "The length (ONLY IF the first argument was an ArrayBuffer)" ] ],
         "return" : [ "JsVar", "A typed array" ]
}*/
/*JSON{ "type":"constructor", "class": "Uint32Array",  "name": "Uint32Array",
         "description" : "Create a typed array based on the given input. Either an existing Array Buffer, an Integer as a Length, or a simple array. If an ArrayBuffer view (eg. Uint8Array rather than ArrayBuffer) is given, it will be completely copied rather than referenced.",
         "generate_full" : "jswrap_typedarray_constructor(ARRAYBUFFERVIEW_UINT32, arr, byteOffset, length)",
         "params" : [ [ "arr", "JsVar", "The array or typed array to base this off, or an integer which is the array length" ],
                      [ "byteOffset", "int", "The byte offset in the ArrayBuffer  (ONLY IF the first argument was an ArrayBuffer)" ],
                      [ "length", "int", "The length (ONLY IF the first argument was an ArrayBuffer)" ] ],
         "return" : [ "JsVar", "A typed array" ]
}*/
/*JSON{ "type":"constructor", "class": "Int32Array",  "name": "Int32Array",
         "description" : "Create a typed array based on the given input. Either an existing Array Buffer, an Integer as a Length, or a simple array. If an ArrayBuffer view (eg. Uint8Array rather than ArrayBuffer) is given, it will be completely copied rather than referenced.",
         "generate_full" : "jswrap_typedarray_constructor(ARRAYBUFFERVIEW_INT32, arr, byteOffset, length)",
         "params" : [ [ "arr", "JsVar", "The array or typed array to base this off, or an integer which is the array length" ],
                      [ "byteOffset", "int", "The byte offset in the ArrayBuffer  (ONLY IF the first argument was an ArrayBuffer)" ],
                      [ "length", "int", "The length (ONLY IF the first argument was an ArrayBuffer)" ] ],
         "return" : [ "JsVar", "A typed array" ]
}*/
/*JSON{ "type":"constructor", "class": "Float32Array",  "name": "Float32Array",
         "description" : "Create a typed array based on the given input. Either an existing Array Buffer, an Integer as a Length, or a simple array. If an ArrayBuffer view (eg. Uint8Array rather than ArrayBuffer) is given, it will be completely copied rather than referenced.",
         "generate_full" : "jswrap_typedarray_constructor(ARRAYBUFFERVIEW_FLOAT32, arr, byteOffset, length)",
         "params" : [ [ "arr", "JsVar", "The array or typed array to base this off, or an integer which is the array length" ],
                      [ "byteOffset", "int", "The byte offset in the ArrayBuffer  (ONLY IF the first argument was an ArrayBuffer)" ],
                      [ "length", "int", "The length (ONLY IF the first argument was an ArrayBuffer)" ] ],
         "return" : [ "JsVar", "A typed array" ]
}*/
/*JSON{ "type":"constructor", "class": "Float64Array",  "name": "Float64Array",
         "description" : "Create a typed array based on the given input. Either an existing Array Buffer, an Integer as a Length, or a simple array. If an ArrayBuffer view (eg. Uint8Array rather than ArrayBuffer) is given, it will be completely copied rather than referenced.",
         "generate_full" : "jswrap_typedarray_constructor(ARRAYBUFFERVIEW_FLOAT64, arr, byteOffset, length)",
         "params" : [ [ "arr", "JsVar", "The array or typed array to base this off, or an integer which is the array length" ],
                      [ "byteOffset", "int", "The byte offset in the ArrayBuffer  (ONLY IF the first argument was an ArrayBuffer)" ],
                      [ "length", "int", "The length (ONLY IF the first argument was an ArrayBuffer)" ] ],
         "return" : [ "JsVar", "A typed array" ]
}*/



JsVar *jswrap_typedarray_constructor(JsVarDataArrayBufferViewType type, JsVar *arr, JsVarInt byteOffset, JsVarInt length) {
  JsVar *arrayBuffer = 0;
  // Only allow use of byteOffset/length if we're passing an ArrayBuffer - NOT A VIEW.
  bool copyData = false;
  if (jsvIsArrayBuffer(arr) && arr->varData.arraybuffer.type==ARRAYBUFFERVIEW_ARRAYBUFFER) {
    arrayBuffer = jsvLockAgain(arr);
  } else if (jsvIsNumeric(arr)) {
    length = jsvGetInteger(arr);
    byteOffset = 0;
    arrayBuffer = jswrap_arraybuffer_constructor((int)JSV_ARRAYBUFFER_GET_SIZE(type)*length);
  } else if (jsvIsArray(arr) || jsvIsArrayBuffer(arr)) {
    length = (JsVarInt)jsvGetLength(arr);
    byteOffset = 0;
    arrayBuffer = jswrap_arraybuffer_constructor((int)JSV_ARRAYBUFFER_GET_SIZE(type)*length);
    copyData = true; // so later on we'll populate this
  }
  if (!arrayBuffer) {
    jsError("Unsupported first argument of type %t\n", arr);
    return 0;
  }
  if (length==0) length = (JsVarInt)(jsvGetArrayBufferLength(arrayBuffer) / JSV_ARRAYBUFFER_GET_SIZE(type));
  JsVar *typedArr = jsvNewWithFlags(JSV_ARRAYBUFFER);
  if (typedArr) {
    typedArr->varData.arraybuffer.type = type;
    typedArr->varData.arraybuffer.byteOffset = (unsigned short)byteOffset;
    typedArr->varData.arraybuffer.length = (unsigned short)length;
    typedArr->firstChild = jsvGetRef(jsvRef(arrayBuffer));

    if (copyData) {
      // if we were given an array, populate this ArrayBuffer
      JsvIterator it;
      jsvIteratorNew(&it, arr);
      while (jsvIteratorHasElement(&it)) {
        JsVar *idx = jsvIteratorGetKey(&it);
        if (jsvIsInt(idx)) {
          JsVar *val = jsvIteratorGetValue(&it);
          // TODO: This is horrible! We need to try and iterate properly...
          jsvArrayBufferSet(typedArr, (size_t)jsvGetInteger(idx), val);
          jsvUnLock(val);
        }
        jsvUnLock(idx);
        jsvIteratorNext(&it);
      }
      jsvIteratorFree(&it);
    }
  }
  jsvUnLock(arrayBuffer);
  return typedArr;
}


/*JSON{ "type":"property", "class": "ArrayBufferView",  "name": "buffer",
         "description" : "The buffer this view references",
         "generate_full" : "jsvLock(parent->firstChild)",
         "return" : [ "JsVar", "An ArrayBuffer object" ]
}*/
/*JSON{ "type":"property", "class": "ArrayBufferView",  "name": "byteLength",
         "description" : "The length, in bytes, of the view",
         "generate_full" : "(JsVarInt)(parent->varData.arraybuffer.length * JSV_ARRAYBUFFER_GET_SIZE(parent->varData.arraybuffer.type))",
         "return" : [ "int", "The Length" ]
}*/
/*JSON{ "type":"property", "class": "ArrayBufferView",  "name": "byteOffset",
         "description" : "The offset, in bytes, to the first byte of the view within the ArrayBuffer",
         "generate_full" : "parent->varData.arraybuffer.byteOffset",
         "return" : [ "int", "The byte Offset" ]
}*/

/*JSON{ "type":"method", "class": "ArrayBufferView",  "name": "set",
         "description" : "Copy the contents of `array` into this one, mapping `this[x+offset]=array[x];`",
         "generate" : "jswrap_arraybufferview_set",
         "params" : [ [ "arr", "JsVar", "Floating point index to access" ], ["offset","int32","The offset in this array at which to write the values (optional)"] ]
}*/
void jswrap_arraybufferview_set(JsVar *parent, JsVar *arr, int offset) {
  if (!(jsvIsString(arr) || jsvIsArray(arr) || jsvIsArrayBuffer(arr))) {
    jsError("Expecting first argument to be an array, not %t", arr);
    return;
  }
  JsvIterator itsrc;
  jsvIteratorNew(&itsrc, arr);
  JsvArrayBufferIterator itdst;
  jsvArrayBufferIteratorNew(&itdst, parent, (size_t)offset);

  bool useInts = JSV_ARRAYBUFFER_IS_FLOAT(itdst.type) || jsvIsString(arr);

  while (jsvIteratorHasElement(&itsrc) && jsvArrayBufferIteratorHasElement(&itdst)) {
    if (useInts) {
      jsvArrayBufferIteratorSetIntegerValue(&itdst, jsvIteratorGetIntegerValue(&itsrc));
    } else {
      JsVar *value = jsvIteratorGetValue(&itsrc);
      jsvArrayBufferIteratorSetValue(&itdst, value);
      jsvUnLock(value);
    }
    jsvArrayBufferIteratorNext(&itdst);
    jsvIteratorNext(&itsrc);
  }
  jsvArrayBufferIteratorFree(&itdst);
  jsvIteratorFree(&itsrc);
}


// 'special' ArrayBufferView.map as it needs to return an ArrayBuffer
/*JSON{ "type":"method", "class": "ArrayBufferView", "name" : "map",
         "description" : ["Return an array which is made from the following: ```A.map(function) = [function(A[0]), function(A[1]), ...]```",
                          "**Note:** This returns an ArrayBuffer of the same type it was called on. To get an Array, use `Array.prototype.map`" ],
         "generate" : "jswrap_arraybufferview_map",
         "params" : [ [ "function", "JsVar", "Function used to map one item to another"] ,
                      [ "thisArg", "JsVar", "if specified, the function is called with 'this' set to thisArg (optional)"] ],
         "return" : ["JsVar", "An array containing the results"]
}*/
JsVar *jswrap_arraybufferview_map(JsVar *parent, JsVar *funcVar, JsVar *thisVar) {
  if (!jsvIsArrayBuffer(parent)) {
    jsError("ArrayBufferView.map can only be called on an ArrayBufferView");
    return 0;
  }
  if (!jsvIsFunction(funcVar)) {
    jsError("ArrayBufferView.map's first argument should be a function");
    return 0;
  }
  if (!jsvIsUndefined(thisVar) && !jsvIsObject(thisVar)) {
    jsError("ArrayBufferView.map's second argument should be undefined, or an object");
    return 0;
  }

  // create ArrayBuffer result
  JsVarDataArrayBufferViewType arrayBufferType = parent->varData.arraybuffer.type;
  JsVar *arrayBufferLength = jsvNewFromInteger((JsVarInt)jsvGetArrayBufferLength(parent));
  JsVar *array = jswrap_typedarray_constructor(arrayBufferType, arrayBufferLength, 0, 0);
  jsvUnLock(arrayBufferLength);
  if (!array) return 0;

  // now iterate
  JsvIterator it; // TODO: if we really are limited to ArrayBuffers, this could be an ArrayBufferIterator.
  jsvIteratorNew(&it, parent);
  JsvArrayBufferIterator itdst;
  jsvArrayBufferIteratorNew(&itdst, array, 0);

  while (jsvIteratorHasElement(&it)) {
    JsVar *index = jsvIteratorGetKey(&it);
    if (jsvIsInt(index)) {
      JsVarInt idxValue = jsvGetInteger(index);

      JsVar *args[3], *mapped;
      args[0] = jsvIteratorGetValue(&it);
      args[1] = jsvNewFromInteger(idxValue); // child is a variable name, create a new variable for the index
      args[2] = parent;
      mapped = jspeFunctionCall(funcVar, 0, thisVar, false, 3, args);
      jsvUnLock(args[0]);
      jsvUnLock(args[1]);
      if (mapped) {
        jsvArrayBufferIteratorSetValue(&itdst, mapped);
        jsvUnLock(mapped);
      }
    }
    jsvUnLock(index);
    jsvIteratorNext(&it);
    jsvArrayBufferIteratorNext(&itdst);
  }
  jsvIteratorFree(&it);
  jsvArrayBufferIteratorFree(&itdst);

  return array;
}


// -----------------------------------------------------------------------------------------------------
//                                                                      Steal Array's methods for this
// -----------------------------------------------------------------------------------------------------

/*JSON{ "type":"method", "class": "ArrayBufferView", "name" : "indexOf",
         "description" : "Return the index of the value in the array, or -1",
         "generate" : "jswrap_array_indexOf",
         "params" : [ [ "value", "JsVar", "The value to check for"] ],
         "return" : ["JsVar", "the index of the value in the array, or -1"]
}*/
/*JSON{ "type":"method", "class": "ArrayBufferView", "name" : "join",
         "description" : "Join all elements of this array together into one string, using 'separator' between them. eg. ```[1,2,3].join(' ')=='1 2 3'```",
         "generate" : "jswrap_array_join",
         "params" : [ [ "separator", "JsVar", "The separator"] ],
         "return" : ["JsVar", "A String representing the Joined array"]
}*/
/*JSON{ "type":"method", "class": "ArrayBufferView", "name" : "sort", "ifndef" : "SAVE_ON_FLASH",
         "description" : "Do an in-place quicksort of the array",
         "generate" : "jswrap_array_sort",
         "params" : [ [ "var", "JsVar", "A function to use to compare array elements (or undefined)"] ],
         "return" : [ "JsVar", "This array object" ]
}*/
/*JSON{ "type":"method", "class": "ArrayBufferView", "name" : "forEach",
         "description" : "Executes a provided function once per array element.",
         "generate" : "jswrap_array_forEach",
         "params" : [ [ "function", "JsVar", "Function to be executed"] ,
                      [ "thisArg", "JsVar", "if specified, the function is called with 'this' set to thisArg (optional)"] ]
}*/
/*JSON{ "type":"method", "class": "ArrayBufferView", "name" : "reduce",
         "description" : "Execute `previousValue=initialValue` and then `previousValue = callback(previousValue, currentValue, index, array)` for each element in the array, and finally return previousValue.",
         "generate" : "jswrap_array_reduce",
         "params" : [ [ "callback", "JsVar", "Function used to reduce the array"] ,
                      [ "initialValue", "JsVar", "if specified, the initial value to pass to the function"] ],
         "return" : ["JsVar", "The value returned by the last function called"]
}*/

