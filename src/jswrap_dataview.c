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
 * JavaScript DataView implementation
 * ----------------------------------------------------------------------------
 */
#include "jswrap_dataview.h"
#include "jsvar.h"
#include "jsvariterator.h"
#include "jsparse.h"
#include "jswrap_arraybuffer.h"

/*JSON{
  "type" : "class",
  "class" : "DataView",
  "ifndef" : "SAVE_ON_FLASH"
}
This class helps
 */

/*JSON{
  "type" : "constructor",
  "class" : "DataView",
  "name" : "DataView",
  "generate" : "jswrap_dataview_constructor",
  "params" : [
    ["buffer","JsVar","The ArrauBuffer to base this on"],
    ["byteOffset","int","(optional) The offset of this view in bytes"],
    ["byteLength","int","(optional) The length in bytes"]
  ],
  "return" : ["JsVar","A DataView object"],
  "return_object" : "DataView",
  "ifndef" : "SAVE_ON_FLASH"
}
Create a DataView object
 */
JsVar *jswrap_dataview_constructor(JsVar *buffer, int byteOffset, int byteLength) {
  if (!jsvIsArrayBuffer(buffer) ||
      buffer->varData.arraybuffer.type!=ARRAYBUFFERVIEW_ARRAYBUFFER) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting ArrayBuffer, got %t", buffer);
    return 0;
  }
  JsVar *dataview = jspNewObject(0,"DataView");
  if (dataview) {
    jsvObjectSetChild(dataview, "buffer", buffer);
    if (byteOffset)
      jsvObjectSetChildAndUnLock(dataview, "byteOffset", jsvNewFromInteger(byteOffset));
    if (byteLength)
      jsvObjectSetChildAndUnLock(dataview, "byteLength", jsvNewFromInteger(byteLength));
  }
  return dataview;
}


JsVar *jswrap_dataview_get(JsVar *dataview, JsVarDataArrayBufferViewType type, int byteOffset, bool littleEndian) {
  JsVar *buffer = jsvObjectGetChild(dataview, "buffer", 0);
  if (!jsvIsArrayBuffer(buffer)) {
    jsvUnLock(buffer);
    return 0;
  }
  byteOffset += jsvGetIntegerAndUnLock(jsvObjectGetChild(dataview, "byteOffset", 0));
  JsVarInt length = JSV_ARRAYBUFFER_GET_SIZE(type);
  // TODO: range error based on byteLength?
  if (!littleEndian) type |= ARRAYBUFFERVIEW_BIG_ENDIAN;
  JsVar *arr = jswrap_typedarray_constructor(type, buffer, byteOffset, length);
  jsvUnLock(buffer);
  if (!arr) return 0;
  JsvArrayBufferIterator it;
  jsvArrayBufferIteratorNew(&it, arr, 0);
  JsVar *value = jsvArrayBufferIteratorGetValue(&it);
  jsvArrayBufferIteratorFree(&it);
  jsvUnLock(arr);
  return value;
}
void jswrap_dataview_set(JsVar *dataview, JsVarDataArrayBufferViewType type, int byteOffset, JsVar *value, bool littleEndian) {
  JsVar *buffer = jsvObjectGetChild(dataview, "buffer", 0);
  if (!jsvIsArrayBuffer(buffer)) {
    jsvUnLock(buffer);
    return;
  }
  byteOffset += jsvGetIntegerAndUnLock(jsvObjectGetChild(dataview, "byteOffset", 0));
  JsVarInt length = JSV_ARRAYBUFFER_GET_SIZE(type);
  // TODO: range error based on byteLength?
  if (!littleEndian) type |= ARRAYBUFFERVIEW_BIG_ENDIAN;
  JsVar *arr = jswrap_typedarray_constructor(type, buffer, byteOffset, length);
  jsvUnLock(buffer);
  if (!arr) return;
  JsvArrayBufferIterator it;
  jsvArrayBufferIteratorNew(&it, arr, 0);
  jsvArrayBufferIteratorSetValue(&it, value);
  jsvArrayBufferIteratorFree(&it);
  jsvUnLock(arr);
}


// =============================================================================  GET

/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "getFloat32",
  "generate_full" : "jswrap_dataview_get(parent, ARRAYBUFFERVIEW_FLOAT32, byteOffset, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "return" : ["JsVar","the index of the value in the array, or -1"],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "getFloat64",
  "generate_full" : "jswrap_dataview_get(parent, ARRAYBUFFERVIEW_FLOAT64, byteOffset, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "return" : ["JsVar","the index of the value in the array, or -1"],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "getInt8",
  "generate_full" : "jswrap_dataview_get(parent, ARRAYBUFFERVIEW_INT8, byteOffset, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "return" : ["JsVar","the index of the value in the array, or -1"],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "getInt16",
  "generate_full" : "jswrap_dataview_get(parent, ARRAYBUFFERVIEW_INT16, byteOffset, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "return" : ["JsVar","the index of the value in the array, or -1"],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "getInt32",
  "generate_full" : "jswrap_dataview_get(parent, ARRAYBUFFERVIEW_INT32, byteOffset, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "return" : ["JsVar","the index of the value in the array, or -1"],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "getUint8",
  "generate_full" : "jswrap_dataview_get(parent, ARRAYBUFFERVIEW_UINT8, byteOffset, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "return" : ["JsVar","the index of the value in the array, or -1"],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "getUint16",
  "generate_full" : "jswrap_dataview_get(parent, ARRAYBUFFERVIEW_UINT16, byteOffset, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "return" : ["JsVar","the index of the value in the array, or -1"],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "getUint32",
  "generate_full" : "jswrap_dataview_get(parent, ARRAYBUFFERVIEW_UINT32, byteOffset, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "return" : ["JsVar","the index of the value in the array, or -1"],
  "ifndef" : "SAVE_ON_FLASH"
}
*/

// =============================================================================  SET

/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "setFloat32",
  "generate_full" : "jswrap_dataview_set(parent, ARRAYBUFFERVIEW_FLOAT32, byteOffset, value, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["value","JsVar","The value to write"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "setFloat64",
  "generate_full" : "jswrap_dataview_set(parent, ARRAYBUFFERVIEW_FLOAT64, byteOffset, value, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["value","JsVar","The value to write"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "setInt8",
  "generate_full" : "jswrap_dataview_set(parent, ARRAYBUFFERVIEW_INT8, byteOffset, value, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["value","JsVar","The value to write"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "setInt16",
  "generate_full" : "jswrap_dataview_set(parent, ARRAYBUFFERVIEW_INT16, byteOffset, value, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["value","JsVar","The value to write"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "setInt32",
  "generate_full" : "jswrap_dataview_set(parent, ARRAYBUFFERVIEW_INT32, byteOffset, value, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["value","JsVar","The value to write"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "setUint8",
  "generate_full" : "jswrap_dataview_set(parent, ARRAYBUFFERVIEW_UINT8, byteOffset, value, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["value","JsVar","The value to write"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "setUint16",
  "generate_full" : "jswrap_dataview_set(parent, ARRAYBUFFERVIEW_UINT16, byteOffset, value, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["value","JsVar","The value to write"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
/*JSON{
  "type" : "method",
  "class" : "DataView",
  "name" : "setUint32",
  "generate_full" : "jswrap_dataview_set(parent, ARRAYBUFFERVIEW_UINT32, byteOffset, value, littleEndian)",
  "params" : [
    ["byteOffset","int","The offset in bytes to read from"],
    ["value","JsVar","The value to write"],
    ["littleEndian","bool","(optional) Whether to read in little endian - if false or undefined data is read as big endian"]
  ],
  "ifndef" : "SAVE_ON_FLASH"
}
*/
