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
    ["buffer","JsVar","The `ArrayBuffer` to base this on"],
    ["byteOffset","int","(optional) The offset of this view in bytes"],
    ["byteLength","int","(optional) The length in bytes"]
  ],
  "return" : ["JsVar","A `DataView` object"],
  "return_object" : "DataView",
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "new(buffer: ArrayBuffer, byteOffset?: number, byteLength?: number): DataView;"
}
Create a `DataView` object that can be used to access the data in an
`ArrayBuffer`.

```
var b = new ArrayBuffer(8)
var v = new DataView(b)
v.setUint16(0,"0x1234")
v.setUint8(3,"0x56")
console.log("0x"+v.getUint32(0).toString(16))
// prints 0x12340056
```
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
    jsvObjectSetChildAndUnLock(dataview, "byteOffset", jsvNewFromInteger(byteOffset));
    jsvObjectSetChildAndUnLock(dataview, "byteLength", jsvNewFromInteger(
        byteLength?(unsigned int)byteLength:jsvGetArrayBufferLength(buffer)));
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "getFloat32(byteOffset: number, littleEndian?: boolean): number;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "getFloat64(byteOffset: number, littleEndian?: boolean): number;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "getInt8(byteOffset: number, littleEndian?: boolean): number;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "getInt16(byteOffset: number, littleEndian?: boolean): number;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "getInt32(byteOffset: number, littleEndian?: boolean): number;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "getUint8(byteOffset: number, littleEndian?: boolean): number;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "getUint16(byteOffset: number, littleEndian?: boolean): number;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "getUint32(byteOffset: number, littleEndian?: boolean): number;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "setFloat32(byteOffset: number, value: number, littleEndian?: boolean): void;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "setFloat64(byteOffset: number, value: number, littleEndian?: boolean): void;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "setInt8(byteOffset: number, value: number, littleEndian?: boolean): void;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "setInt16(byteOffset: number, value: number, littleEndian?: boolean): void;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "setInt32(byteOffset: number, value: number, littleEndian?: boolean): void;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "setUint8(byteOffset: number, value: number, littleEndian?: boolean): void;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "setUint16(byteOffset: number, value: number, littleEndian?: boolean): void;"
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
  "ifndef" : "SAVE_ON_FLASH",
  "typescript" : "setUint32(byteOffset: number, value: number, littleEndian?: boolean): void;"
}
*/
