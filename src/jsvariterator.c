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
 * Iterators for Variables
 * ----------------------------------------------------------------------------
 */
#include "jsvariterator.h"

/**
 * Iterate over the contents of the content of a variable, calling callback for each.
 * Contents may be:
 * * numeric -> output
 * * a string -> output each character
 * * array/arraybuffer -> call itself on each element
 * object -> call itself object.count times, on object.data
 */
bool jsvIterateCallback(
    JsVar *data,                                    // The data to iterate over.
	void (*callback)(int item, void *callbackData), // The callback function invoke.
	void *callbackData                              // Data to be passed to the callback function
  ) {
  bool ok = true;
  // Handle the data being a single numeric.
  if (jsvIsNumeric(data)) {
    callback((int)jsvGetInteger(data), callbackData);
  }
  // Handle the data being an object.
  else if (jsvIsObject(data)) {
    JsVar *countVar = jsvObjectGetChild(data, "count", 0);
    JsVar *dataVar = jsvObjectGetChild(data, "data", 0);
    if (countVar && dataVar && jsvIsNumeric(countVar)) {
      int n = (int)jsvGetInteger(countVar);
      while (ok && n-- > 0) {
        ok = jsvIterateCallback(dataVar, callback, callbackData);
      }
    } else {
      jsExceptionHere(JSET_TYPEERROR, "If specifying an object, it must be of the form {data : ..., count : N} - got %j", data);
    }
    jsvUnLock2(countVar, dataVar);
  }
  // Handle the data being a string
  else if (jsvIsString(data)) {
    JsvStringIterator it;
    jsvStringIteratorNew(&it, data, 0);
    while (jsvStringIteratorHasChar(&it) && ok) {
      char ch = jsvStringIteratorGetChar(&it);
      callback(ch, callbackData);
      jsvStringIteratorNext(&it);
    }
    jsvStringIteratorFree(&it);
  }
  // Handle the data being an array buffer
  else if (jsvIsArrayBuffer(data)) {
    JsvArrayBufferIterator it;
    jsvArrayBufferIteratorNew(&it, data, 0);
    if (JSV_ARRAYBUFFER_GET_SIZE(it.type) == 1 && !JSV_ARRAYBUFFER_IS_SIGNED(it.type)) {
      // faster for single byte arrays.
      while (jsvArrayBufferIteratorHasElement(&it)) {
        callback((int)(unsigned char)jsvStringIteratorGetChar(&it.it), callbackData);
        jsvArrayBufferIteratorNext(&it);
      }
    } else {
      while (jsvArrayBufferIteratorHasElement(&it)) {
        callback((int)jsvArrayBufferIteratorGetIntegerValue(&it), callbackData);
        jsvArrayBufferIteratorNext(&it);
      }
    }
    jsvArrayBufferIteratorFree(&it);
  }
  // Handle the data being iterable
  else if (jsvIsIterable(data)) {
    JsvIterator it;
    jsvIteratorNew(&it, data);
    while (jsvIteratorHasElement(&it) && ok) {
      JsVar *el = jsvIteratorGetValue(&it);
      ok = jsvIterateCallback(el, callback, callbackData);
      jsvUnLock(el);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  } else {
    jsExceptionHere(JSET_TYPEERROR, "Expecting a number or something iterable, got %t", data);
    ok = false;
  }
  return ok;
}


/**
 * An iterable callback that counts how many times it was called.
 * This is a function that can be supplied to `jsvIterateCallback`.
 */
static void jsvIterateCallbackCountCb(
    int n,     //!< The current item being iterated.  Not used.
	void *data //!< A pointer to an int that counts how many times we were called.
  ) {
  NOT_USED(n);
  int *count = (int*)data;
  (*count)++;
}


/**
 * Determine how many items are in this variable that will be iterated over.
 * \return The number of iterations we will call for this variable.
 */
int jsvIterateCallbackCount(JsVar *var) {
  // Actually iterate over the variable where the callback function merely increments a counter
  // that is initially zero.  The result will be the number of times the callback for iteration
  // was invoked and hence the iteration count of the variable.
  int count = 0;
  jsvIterateCallback(var, jsvIterateCallbackCountCb, (void *)&count);
  return count;
}


typedef struct { unsigned char *buf; unsigned int idx, length; } JsvIterateCallbackToBytesData;
static void jsvIterateCallbackToBytesCb(int data, void *userData) {
  JsvIterateCallbackToBytesData *cbData = (JsvIterateCallbackToBytesData*)userData;
  if (cbData->idx < cbData->length)
    cbData->buf[cbData->idx] = (unsigned char)data;
  cbData->idx++;
}
/** Write all data in array to the data pointer (of size dataSize bytes) */
unsigned int jsvIterateCallbackToBytes(JsVar *var, unsigned char *data, unsigned int dataSize) {
  JsvIterateCallbackToBytesData cbData;
  cbData.buf = (unsigned char *)data;
  cbData.idx = 0;
  cbData.length = dataSize;
  jsvIterateCallback(var, jsvIterateCallbackToBytesCb, (void*)&cbData);
  return cbData.idx;
}

// --------------------------------------------------------------------------------------------

void jsvStringIteratorNew(JsvStringIterator *it, JsVar *str, size_t startIdx) {
  assert(jsvHasCharacterData(str));
  it->var = jsvLockAgain(str);
  it->varIndex = 0;
  it->charsInVar = jsvGetCharactersInVar(str);
  it->charIdx = startIdx;
  if (jsvIsFlatString(str)) {
    it->ptr = jsvGetFlatStringPointer(it->var);
  } else if (jsvIsNativeString(str)) {
    it->ptr = (char*)it->var->varData.nativeStr.ptr;
  } else{
    it->ptr = &it->var->varData.str[0];
  }
  while (it->charIdx>0 && it->charIdx >= it->charsInVar) {
    it->charIdx -= it->charsInVar;
    it->varIndex += it->charsInVar;
    if (it->var) {
      if (jsvGetLastChild(it->var)) {
        JsVar *next = jsvLock(jsvGetLastChild(it->var));
        jsvUnLock(it->var);
        it->var = next;
        it->ptr = &next->varData.str[0];
        it->charsInVar = jsvGetCharactersInVar(it->var);
      } else {
        jsvUnLock(it->var);
        it->var = 0;
        it->ptr = 0;
        it->charsInVar = 0;
        it->varIndex = startIdx - it->charIdx;
        return; // at end of string - get out of loop
      }
    }
  }
}

JsvStringIterator jsvStringIteratorClone(JsvStringIterator *it) {
  JsvStringIterator i = *it;
  if (i.var) jsvLockAgain(i.var);
  return i;
}

/// Gets the current (>=0) character (or -1)
int jsvStringIteratorGetCharOrMinusOne(JsvStringIterator *it) {
  if (!it->ptr || it->charIdx>=it->charsInVar) return -1;
  return (int)(unsigned char)READ_FLASH_UINT8(&it->ptr[it->charIdx]);
}

void jsvStringIteratorSetChar(JsvStringIterator *it, char c) {
  if (jsvStringIteratorHasChar(it))
    it->ptr[it->charIdx] = c;
}

void jsvStringIteratorNext(JsvStringIterator *it) {
  jsvStringIteratorNextInline(it);
}

void jsvStringIteratorGotoEnd(JsvStringIterator *it) {
  assert(it->var);
  while (jsvGetLastChild(it->var)) {
    JsVar *next = jsvLock(jsvGetLastChild(it->var));
    jsvUnLock(it->var);
    it->var = next;
    it->varIndex += it->charsInVar;
    it->charsInVar = jsvGetCharactersInVar(it->var);
  }
  it->ptr = &it->var->varData.str[0];
  if (it->charsInVar) it->charIdx = it->charsInVar-1;
  else it->charIdx = 0;
}

void jsvStringIteratorAppend(JsvStringIterator *it, char ch) {
  if (!it->var) return;
  if (it->charsInVar>0) {
    assert(it->charIdx+1 == it->charsInVar /* check at end */);
    it->charIdx++;
  } else
    assert(it->charIdx == 0);
  /* Note: jsvGetMaxCharactersInVar will return the wrong length when
   * applied to flat strings, but we don't care because the length will
   * be smaller than charIdx, which will force a new string to be
   * appended onto the end  */
  if (it->charIdx >= jsvGetMaxCharactersInVar(it->var)) {
    assert(!jsvGetLastChild(it->var));
    JsVar *next = jsvNewWithFlags(JSV_STRING_EXT_0);
    if (!next) {
      jsvUnLock(it->var);
      it->var = 0;
      it->ptr = 0;
      it->charIdx = 0;
      return; // out of memory
    }
    // we don't ref, because  StringExts are never reffed as they only have one owner (and ALWAYS have an owner)
    jsvSetLastChild(it->var, jsvGetRef(next));
    jsvUnLock(it->var);
    it->var = next;
    it->ptr = &next->varData.str[0];
    it->varIndex += it->charIdx;
    it->charIdx = 0; // it's new, so empty
  }
  it->ptr[it->charIdx] = ch;
  it->charsInVar = it->charIdx+1;
  jsvSetCharactersInVar(it->var, it->charsInVar);
}

void jsvStringIteratorAppendString(JsvStringIterator *it, JsVar *str) {
  JsvStringIterator sit;
  jsvStringIteratorNew(&sit, str, 0);
  while (jsvStringIteratorHasChar(&sit)) {
    jsvStringIteratorAppend(it, jsvStringIteratorGetChar(&sit));
    jsvStringIteratorNext(&sit);
  }
  jsvStringIteratorFree(&sit);
}

// --------------------------------------------------------------------------------------------

void jsvObjectIteratorNew(JsvObjectIterator *it, JsVar *obj) {
  assert(jsvIsArray(obj) || jsvIsObject(obj) || jsvIsFunction(obj));
  it->var = jsvGetFirstChild(obj) ? jsvLock(jsvGetFirstChild(obj)) : 0;
}

/// Clone the iterator
JsvObjectIterator jsvObjectIteratorClone(JsvObjectIterator *it) {
  JsvObjectIterator i = *it;
  if (i.var) jsvLockAgain(i.var);
  return i;
}

/// Move to next item
void jsvObjectIteratorNext(JsvObjectIterator *it) {
  if (it->var) {
    JsVarRef next = jsvGetNextSibling(it->var);
    jsvUnLock(it->var);
    it->var = next ? jsvLock(next) : 0;
  }
}

void jsvObjectIteratorSetValue(JsvObjectIterator *it, JsVar *value) {
  if (!it->var) return; // end of object
  jsvSetValueOfName(it->var, value);
}

void jsvObjectIteratorRemoveAndGotoNext(JsvObjectIterator *it, JsVar *parent) {
  if (it->var) {
    JsVarRef next = jsvGetNextSibling(it->var);
    jsvRemoveChild(parent, it->var);
    jsvUnLock(it->var);
    it->var = next ? jsvLock(next) : 0;
  }
}

// --------------------------------------------------------------------------------------------
void   jsvArrayBufferIteratorNew(JsvArrayBufferIterator *it, JsVar *arrayBuffer, size_t index) {
  assert(jsvIsArrayBuffer(arrayBuffer));
  it->index = index;
  it->type = arrayBuffer->varData.arraybuffer.type;
  it->byteLength = arrayBuffer->varData.arraybuffer.length * JSV_ARRAYBUFFER_GET_SIZE(it->type);
  it->byteOffset = arrayBuffer->varData.arraybuffer.byteOffset;
  JsVar *arrayBufferData = jsvGetArrayBufferBackingString(arrayBuffer);

  it->byteLength += it->byteOffset; // because we'll check if we have more bytes using this
  it->byteOffset = it->byteOffset + index*JSV_ARRAYBUFFER_GET_SIZE(it->type);
  if (it->byteOffset>=(it->byteLength+1-JSV_ARRAYBUFFER_GET_SIZE(it->type))) {
    jsvUnLock(arrayBufferData);
    it->type = ARRAYBUFFERVIEW_UNDEFINED;
    return;
  }
  jsvStringIteratorNew(&it->it, arrayBufferData, (size_t)it->byteOffset);
  jsvUnLock(arrayBufferData);
  it->hasAccessedElement = false;
}

/// Clone the iterator
ALWAYS_INLINE JsvArrayBufferIterator jsvArrayBufferIteratorClone(JsvArrayBufferIterator *it) {
  JsvArrayBufferIterator i = *it;
  i.it = jsvStringIteratorClone(&it->it);
  return i;
}

static void jsvArrayBufferIteratorGetValueData(JsvArrayBufferIterator *it, char *data) {
  if (it->type == ARRAYBUFFERVIEW_UNDEFINED) return;
  assert(!it->hasAccessedElement); // we just haven't implemented this case yet
  int i,dataLen = (int)JSV_ARRAYBUFFER_GET_SIZE(it->type);
  if (it->type & ARRAYBUFFERVIEW_BIG_ENDIAN) {
    for (i=dataLen-1;i>=0;i--) {
       data[i] = jsvStringIteratorGetChar(&it->it);
       if (dataLen!=1) jsvStringIteratorNext(&it->it);
     }
  } else {
    for (i=0;i<dataLen;i++) {
      data[i] = jsvStringIteratorGetChar(&it->it);
      if (dataLen!=1) jsvStringIteratorNext(&it->it);
    }
  }
  if (dataLen!=1) it->hasAccessedElement = true;
}

static JsVarInt jsvArrayBufferIteratorDataToInt(JsvArrayBufferIterator *it, char *data) {
  unsigned int dataLen = JSV_ARRAYBUFFER_GET_SIZE(it->type);
  JsVarInt v = 0;
  if (dataLen==1) v = *(int8_t*)data;
  else if (dataLen==2) v = *(short*)data;
  else if (dataLen==4) return *(int*)data;
  else assert(0);
  if ((!JSV_ARRAYBUFFER_IS_SIGNED(it->type)))
    v = v & (JsVarInt)((1UL << (8*dataLen))-1);
  return v;
}

static JsVarFloat jsvArrayBufferIteratorDataToFloat(JsvArrayBufferIterator *it, char *data) {
  unsigned int dataLen = JSV_ARRAYBUFFER_GET_SIZE(it->type);
  JsVarFloat v = 0;
  if (dataLen==4) v = *(float*)data;
  else if (dataLen==8) v = *(double*)data;
  else assert(0);
  return v;
}

JsVar *jsvArrayBufferIteratorGetValue(JsvArrayBufferIterator *it) {
  if (it->type == ARRAYBUFFERVIEW_UNDEFINED) return 0;
  char data[8];
  jsvArrayBufferIteratorGetValueData(it, data);
  if (JSV_ARRAYBUFFER_IS_FLOAT(it->type)) {
    return jsvNewFromFloat(jsvArrayBufferIteratorDataToFloat(it, data));
  } else {
    JsVarInt i = jsvArrayBufferIteratorDataToInt(it, data);
    if (it->type == ARRAYBUFFERVIEW_UINT32)
      return jsvNewFromLongInteger((long long)(uint32_t)i);
    return jsvNewFromInteger(i);
  }
}

JsVar *jsvArrayBufferIteratorGetValueAndRewind(JsvArrayBufferIterator *it) {
  JsvStringIterator oldIt = jsvStringIteratorClone(&it->it);
  JsVar *v = jsvArrayBufferIteratorGetValue(it);
  jsvStringIteratorFree(&it->it);
  it->it = oldIt;
  it->hasAccessedElement = false;
  return v;
}

JsVarInt jsvArrayBufferIteratorGetIntegerValue(JsvArrayBufferIterator *it) {
  if (it->type == ARRAYBUFFERVIEW_UNDEFINED) return 0;
  char data[8];
  jsvArrayBufferIteratorGetValueData(it, data);
  if (JSV_ARRAYBUFFER_IS_FLOAT(it->type)) {
    return (JsVarInt)jsvArrayBufferIteratorDataToFloat(it, data);
  } else {
    return jsvArrayBufferIteratorDataToInt(it, data);
  }
}

JsVarFloat jsvArrayBufferIteratorGetFloatValue(JsvArrayBufferIterator *it) {
  if (it->type == ARRAYBUFFERVIEW_UNDEFINED) return 0;
  char data[8];
  jsvArrayBufferIteratorGetValueData(it, data);
  if (JSV_ARRAYBUFFER_IS_FLOAT(it->type)) {
    return jsvArrayBufferIteratorDataToFloat(it, data);
  } else {
    return (JsVarFloat)jsvArrayBufferIteratorDataToInt(it, data);
  }
}

static void jsvArrayBufferIteratorIntToData(char *data, unsigned int dataLen, int type, JsVarInt v) {
  if (JSV_ARRAYBUFFER_IS_CLAMPED(type)) {
    assert(dataLen==1 && !JSV_ARRAYBUFFER_IS_SIGNED(type)); // all we support right now
    if (v<0) v=0;
    if (v>255) v=255;
  }
  // we don't care about sign when writing - as it gets truncated
  if (dataLen==1) { data[0] = (char)v; }
  else if (dataLen==2) { *(short*)data = (short)v; }
  else if (dataLen==4) { *(int*)data = (int)v; }
  else if (dataLen==8) { *(long long*)data = (long long)v; }
  else assert(0);
}

static void jsvArrayBufferIteratorFloatToData(char *data,  unsigned int dataLen, int type, JsVarFloat v) {
  NOT_USED(type);
  if (dataLen==4) { *(float*)data = (float)v; }
  else if (dataLen==8) { *(double*)data = (double)v; }
  else assert(0);
}

void jsvArrayBufferIteratorSetIntegerValue(JsvArrayBufferIterator *it, JsVarInt v) {
  if (it->type == ARRAYBUFFERVIEW_UNDEFINED) return;
  assert(!it->hasAccessedElement); // we just haven't implemented this case yet
  char data[8];
  unsigned int i,dataLen = JSV_ARRAYBUFFER_GET_SIZE(it->type);

  if (JSV_ARRAYBUFFER_IS_FLOAT(it->type)) {
    jsvArrayBufferIteratorFloatToData(data, dataLen, it->type, (JsVarFloat)v);
  } else {
    jsvArrayBufferIteratorIntToData(data, dataLen, it->type, v);
  }

  for (i=0;i<dataLen;i++) {
    jsvStringIteratorSetChar(&it->it, data[i]);
    if (dataLen!=1) jsvStringIteratorNext(&it->it);
  }
  if (dataLen!=1) it->hasAccessedElement = true;
}

void jsvArrayBufferIteratorSetValue(JsvArrayBufferIterator *it, JsVar *value) {
  if (it->type == ARRAYBUFFERVIEW_UNDEFINED) return;
  assert(!it->hasAccessedElement); // we just haven't implemented this case yet
  char data[8];
  int i,dataLen = (int)JSV_ARRAYBUFFER_GET_SIZE(it->type);

  if (JSV_ARRAYBUFFER_IS_FLOAT(it->type)) {
    jsvArrayBufferIteratorFloatToData(data, (unsigned)dataLen, it->type, jsvGetFloat(value));
  } else {
    jsvArrayBufferIteratorIntToData(data, (unsigned)dataLen, it->type, jsvGetInteger(value));
  }

  if (it->type & ARRAYBUFFERVIEW_BIG_ENDIAN) {
    for (i=dataLen-1;i>=0;i--) {
      jsvStringIteratorSetChar(&it->it, data[i]);
      if (dataLen!=1) jsvStringIteratorNext(&it->it);
    }
  } else {
    for (i=0;i<dataLen;i++) {
      jsvStringIteratorSetChar(&it->it, data[i]);
      if (dataLen!=1) jsvStringIteratorNext(&it->it);
    }
  }
  if (dataLen!=1) it->hasAccessedElement = true;
}

void jsvArrayBufferIteratorSetByteValue(JsvArrayBufferIterator *it, char c) {
  if (JSV_ARRAYBUFFER_GET_SIZE(it->type)!=1) {
    assert(0);
    return;
  }
  jsvStringIteratorSetChar(&it->it, c);
}

void jsvArrayBufferIteratorSetValueAndRewind(JsvArrayBufferIterator *it, JsVar *value) {
  JsvStringIterator oldIt = jsvStringIteratorClone(&it->it);
  jsvArrayBufferIteratorSetValue(it, value);
  jsvStringIteratorFree(&it->it);
  it->it = oldIt;
  it->hasAccessedElement = false;
}


JsVar* jsvArrayBufferIteratorGetIndex(JsvArrayBufferIterator *it) {
  return jsvNewFromInteger((JsVarInt)it->index);
}

bool jsvArrayBufferIteratorHasElement(JsvArrayBufferIterator *it) {
  if (it->type == ARRAYBUFFERVIEW_UNDEFINED) return false;
  if (it->hasAccessedElement) return true;
  return (it->byteOffset+JSV_ARRAYBUFFER_GET_SIZE(it->type)) <= it->byteLength;
}

void   jsvArrayBufferIteratorNext(JsvArrayBufferIterator *it) {
  it->index++;
  it->byteOffset += JSV_ARRAYBUFFER_GET_SIZE(it->type);
  if (!it->hasAccessedElement) {
    unsigned int dataLen = JSV_ARRAYBUFFER_GET_SIZE(it->type);
    while (dataLen--)
      jsvStringIteratorNext(&it->it);
  } else
    it->hasAccessedElement = false;
}

void   jsvArrayBufferIteratorFree(JsvArrayBufferIterator *it) {
  if (it->type == ARRAYBUFFERVIEW_UNDEFINED) return;
  jsvStringIteratorFree(&it->it);
}
// --------------------------------------------------------------------------------------------
/* General Purpose iterator, for Strings, Arrays, Objects, Typed Arrays */

void jsvIteratorNew(JsvIterator *it, JsVar *obj) {
  if (jsvIsArray(obj) || jsvIsObject(obj) || jsvIsFunction(obj)) {
    it->type = JSVI_OBJECT;
    jsvObjectIteratorNew(&it->it.obj, obj);
  } else if (jsvIsArrayBuffer(obj)) {
    it->type = JSVI_ARRAYBUFFER;
    jsvArrayBufferIteratorNew(&it->it.buf, obj, 0);
  } else if (jsvHasCharacterData(obj)) {
    it->type = JSVI_STRING;
    jsvStringIteratorNew(&it->it.str, obj, 0);
  } else assert(0);
}

JsVar *jsvIteratorGetKey(JsvIterator *it) {
  switch (it->type) {
  case JSVI_OBJECT : return jsvObjectIteratorGetKey(&it->it.obj);
  case JSVI_STRING : return jsvMakeIntoVariableName(jsvNewFromInteger((JsVarInt)jsvStringIteratorGetIndex(&it->it.str)), 0); // some things expect a veriable name
  case JSVI_ARRAYBUFFER : return jsvMakeIntoVariableName(jsvArrayBufferIteratorGetIndex(&it->it.buf), 0); // some things expect a veriable name
  default: assert(0); return 0;
  }
}

JsVar *jsvIteratorGetValue(JsvIterator *it) {
  switch (it->type) {
  case JSVI_OBJECT : return jsvObjectIteratorGetValue(&it->it.obj);
  case JSVI_STRING : { char buf[2] = {jsvStringIteratorGetChar(&it->it.str),0}; return jsvNewFromString(buf); }
  case JSVI_ARRAYBUFFER : return jsvArrayBufferIteratorGetValueAndRewind(&it->it.buf);
  default: assert(0); return 0;
  }
}

JsVarInt jsvIteratorGetIntegerValue(JsvIterator *it) {
  switch (it->type) {
  case JSVI_OBJECT : {
    // fast path for arrays of ints
    if (jsvIsNameInt(it->it.obj.var)) return (JsVarInt)jsvGetFirstChildSigned(it->it.obj.var);
    return jsvGetIntegerAndUnLock(jsvObjectIteratorGetValue(&it->it.obj));
  }
  case JSVI_STRING : return (JsVarInt)jsvStringIteratorGetChar(&it->it.str);
  case JSVI_ARRAYBUFFER : return jsvArrayBufferIteratorGetIntegerValue(&it->it.buf);
  default: assert(0); return 0;
  }
}

JsVarFloat jsvIteratorGetFloatValue(JsvIterator *it) {
  switch (it->type) {
  case JSVI_OBJECT : return jsvGetFloatAndUnLock(jsvObjectIteratorGetValue(&it->it.obj));
  case JSVI_STRING : return (JsVarFloat)jsvStringIteratorGetChar(&it->it.str);
  case JSVI_ARRAYBUFFER : return jsvArrayBufferIteratorGetFloatValue(&it->it.buf);
  default: assert(0); return 0;
  }
}

JsVar *jsvIteratorSetValue(JsvIterator *it, JsVar *value) {
  switch (it->type) {
  case JSVI_OBJECT : jsvObjectIteratorSetValue(&it->it.obj, value); break;
  case JSVI_STRING : jsvStringIteratorSetChar(&it->it.str, (char)(jsvIsString(value) ? value->varData.str[0] : (char)jsvGetInteger(value))); break;
  case JSVI_ARRAYBUFFER : jsvArrayBufferIteratorSetValueAndRewind(&it->it.buf, value); break;
  default: assert(0); break;
  }
  return value;
}

bool jsvIteratorHasElement(JsvIterator *it) {
  switch (it->type) {
  case JSVI_OBJECT : return jsvObjectIteratorHasValue(&it->it.obj);
  case JSVI_STRING : return jsvStringIteratorHasChar(&it->it.str);
  case JSVI_ARRAYBUFFER : return jsvArrayBufferIteratorHasElement(&it->it.buf);
  default: assert(0); return 0;
  }
}

void jsvIteratorNext(JsvIterator *it) {
  switch (it->type) {
  case JSVI_OBJECT : jsvObjectIteratorNext(&it->it.obj); break;
  case JSVI_STRING : jsvStringIteratorNext(&it->it.str); break;
  case JSVI_ARRAYBUFFER : jsvArrayBufferIteratorNext(&it->it.buf); break;
  default: assert(0); break;
  }
}

void jsvIteratorFree(JsvIterator *it) {
  switch (it->type) {
  case JSVI_OBJECT : jsvObjectIteratorFree(&it->it.obj); break;
  case JSVI_STRING : jsvStringIteratorFree(&it->it.str); break;
  case JSVI_ARRAYBUFFER : jsvArrayBufferIteratorFree(&it->it.buf); break;
  default: assert(0); break;
  }
}

JsvIterator jsvIteratorClone(JsvIterator *it) {
  JsvIterator newit;
  newit.type = it->type;
  switch (it->type) {
  case JSVI_OBJECT : newit.it.obj = jsvObjectIteratorClone(&it->it.obj); break;
  case JSVI_STRING : newit.it.str = jsvStringIteratorClone(&it->it.str); break;
  case JSVI_ARRAYBUFFER : newit.it.buf = jsvArrayBufferIteratorClone(&it->it.buf); break;
  default: assert(0); break;
  }
  return newit;
}
