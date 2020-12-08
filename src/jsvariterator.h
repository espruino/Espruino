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
#ifndef JSVARITERATOR_H_
#define JSVARITERATOR_H_

#include "jsvar.h"
#ifdef SPIFLASH_BASE
#include "jshardware.h"
#endif

/// Callback function to be used with jsvIterateCallback
typedef void (*jsvIterateCallbackFn)(int item, void *callbackData);
/// Callback function to be used with jsvIterateBufferCallback
typedef void (*jsvIterateBufferCallbackFn)(unsigned char *data, unsigned int len, void *callbackData);



/** Iterate over the contents of var, calling callback for each. Contents may be:
 *   * numeric -> output
 *   * a string -> output each character
 *   * array/arraybuffer -> call itself on each element
 *   * object -> call itself object.count times, on object.data
 */
bool jsvIterateCallback(JsVar *var, jsvIterateCallbackFn callback, void *callbackData);

// Like jsvIterateCallback, but iterates over just bytes and calls with a pointer and length wherever it can
bool jsvIterateBufferCallback(
    JsVar *data,
    jsvIterateBufferCallbackFn callback,
    void *callbackData
  );

/** If jsvIterateCallback is called, how many times will it call the callback function? */
uint32_t jsvIterateCallbackCount(JsVar *var);

/** Write all data in array to the data pointer (of size dataSize bytes) */
unsigned int jsvIterateCallbackToBytes(JsVar *var, unsigned char *data, unsigned int dataSize);

// --------------------------------------------------------------------------------------------
typedef struct JsvStringIterator {
  size_t charIdx; ///< index of character in var
  size_t charsInVar; ///< total characters in var
  size_t varIndex; ///< index in string of the start of this var
  JsVar *var; ///< current StringExt we're looking at
  char  *ptr; ///< a pointer to string data
#ifdef SPIFLASH_BASE // when using flash strings, we need somewhere to put the data
  char flashStringBuffer[16];
#endif
} JsvStringIterator;

// slight hack to ensure we can use string iterator with const JsVars
#define jsvStringIteratorNewConst(it,str,startIdx) jsvStringIteratorNew(it, (JsVar*)str, startIdx)

/// Create a new String iterator from a string, starting from a specific character. NOTE: This does not keep a lock to the first element, so make sure you do or the string will be freed!
void jsvStringIteratorNew(JsvStringIterator *it, JsVar *str, size_t startIdx);

/// Clone the string iterator
void jsvStringIteratorClone(JsvStringIterator *dstit, JsvStringIterator *it);

/// Gets the current character (or 0)
static ALWAYS_INLINE char jsvStringIteratorGetChar(JsvStringIterator *it) {
  if (!it->ptr) return 0;
  return (char)READ_FLASH_UINT8(&it->ptr[it->charIdx]);
}

/// Gets the current character (or 0) and increment iterator. Not inlined for speed
char jsvStringIteratorGetCharAndNext(JsvStringIterator *it);

/// Gets the current (>=0) character (or -1)
int jsvStringIteratorGetCharOrMinusOne(JsvStringIterator *it);

/// Do we have a character, or are we at the end?
static ALWAYS_INLINE bool jsvStringIteratorHasChar(JsvStringIterator *it) {
  return it->charIdx < it->charsInVar;
}

/// Sets a character (will not extend the string - just overwrites)
void jsvStringIteratorSetChar(JsvStringIterator *it, char c);

/// Sets a character (will not extend the string - just overwrites) and moves on to next character
void jsvStringIteratorSetCharAndNext(JsvStringIterator *it, char c);

/// Gets the current index in the string
static ALWAYS_INLINE size_t jsvStringIteratorGetIndex(JsvStringIterator *it) {
  return it->varIndex + it->charIdx;
}

/// Move to next character
void jsvStringIteratorNext(JsvStringIterator *it);

/// Returns a pointer to the next block of data and its length, and moves on to the data after
void jsvStringIteratorGetPtrAndNext(JsvStringIterator *it, unsigned char **data, unsigned int *len);

#ifdef SPIFLASH_BASE
// For 'Flash Strings' only - loads each block from flash memory as required
static void jsvStringIteratorLoadFlashString(JsvStringIterator *it) {
  it->varIndex += it->charIdx;
  it->charIdx = 0;
  uint32_t l = (uint32_t)it->var->varData.nativeStr.len;
  if (it->varIndex >= l) {
    it->ptr = 0; // past end of string
    it->charsInVar = 0;
  } else {
    it->charsInVar = l - it->varIndex;
    if (it->charsInVar > sizeof(it->flashStringBuffer))
      it->charsInVar = sizeof(it->flashStringBuffer);
    jshFlashRead(it->flashStringBuffer, (uint32_t)it->varIndex+(uint32_t)(size_t)it->var->varData.nativeStr.ptr, (uint32_t)it->charsInVar);
    it->ptr = (char*)it->flashStringBuffer;
  }
}
#endif

/// Ensures that the correct JsVar is loaded with data for the Iterator. ONLY FOR INTERNAL USE
static ALWAYS_INLINE void jsvStringIteratorLoadInline(JsvStringIterator *it) {
  it->charIdx -= it->charsInVar;
  it->varIndex += it->charsInVar;
#ifdef SPIFLASH_BASE
  if (jsvIsFlashString(it->var))
    return jsvStringIteratorLoadFlashString(it);
#endif
  if (it->var && jsvGetLastChild(it->var)) {
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
    it->varIndex += it->charIdx;
    it->charIdx = 0;
  }
}

/// Move to next character (this one is inlined where speed is needed)
static ALWAYS_INLINE void jsvStringIteratorNextInline(JsvStringIterator *it) {
  it->charIdx++;
  if (it->charIdx >= it->charsInVar) {
    jsvStringIteratorLoadInline(it);
  }
}

/// Go to the end of the string iterator - for use with jsvStringIteratorAppend
void jsvStringIteratorGotoEnd(JsvStringIterator *it);

/// Go to the given position in the string iterator. Needs the string again in case we're going back and need to start from the beginning
void jsvStringIteratorGoto(JsvStringIterator *it, JsVar *str, size_t startIdx);

/// Append a character TO THE END of a string iterator
void jsvStringIteratorAppend(JsvStringIterator *it, char ch);

/// Append an entire JsVar string TO THE END of a string iterator
void jsvStringIteratorAppendString(JsvStringIterator *it, JsVar *str, size_t startIdx, int maxLength);

static ALWAYS_INLINE void jsvStringIteratorFree(JsvStringIterator *it) {
  jsvUnLock(it->var);
}

/// Special version of append designed for use with vcbprintf_callback (See jsvAppendPrintf)
void jsvStringIteratorPrintfCallback(const char *str, void *user_data);

// --------------------------------------------------------------------------------------------
typedef struct JsvObjectIterator {
  JsVar *var;
} JsvObjectIterator;

void jsvObjectIteratorNew(JsvObjectIterator *it, JsVar *obj);

/// Clone the iterator
void jsvObjectIteratorClone(JsvObjectIterator *dstit, JsvObjectIterator *it);

/// Gets the current object element key (or 0)
static ALWAYS_INLINE JsVar *jsvObjectIteratorGetKey(JsvObjectIterator *it) {
  if (!it->var) return 0; // end of object
  return jsvLockAgain(it->var);
}

/// Gets the current object element value (or 0)
static ALWAYS_INLINE JsVar *jsvObjectIteratorGetValue(JsvObjectIterator *it) {
  if (!it->var) return 0; // end of object
  return jsvSkipName(it->var); // might even be undefined
}

/// Do we have a key, or are we at the end?
static ALWAYS_INLINE bool jsvObjectIteratorHasValue(JsvObjectIterator *it) {
  return it->var != 0;
}

/// Set the current array element
void jsvObjectIteratorSetValue(JsvObjectIterator *it, JsVar *value);

/// Move to next item
void jsvObjectIteratorNext(JsvObjectIterator *it);

/// Remove the current element and move to next element. Needs the parent supplied (the JsVar passed to jsvObjectIteratorNew) as we don't store it
void jsvObjectIteratorRemoveAndGotoNext(JsvObjectIterator *it, JsVar *parent);

static ALWAYS_INLINE void jsvObjectIteratorFree(JsvObjectIterator *it) {
  jsvUnLock(it->var);
}
// --------------------------------------------------------------------------------------------
typedef struct JsvArrayBufferIterator {
  JsvStringIterator it;
  JsVarDataArrayBufferViewType type;
  size_t byteLength;
  size_t byteOffset;
  size_t index;
  bool hasAccessedElement;
} JsvArrayBufferIterator;

/* TODO: can we add it->getIntegerValue/etc that get set by jsvArrayBufferIteratorNew?
   It's be way faster, especially for byte arrays
*/

void   jsvArrayBufferIteratorNew(JsvArrayBufferIterator *it, JsVar *arrayBuffer, size_t index);

/// Clone the iterator
void jsvArrayBufferIteratorClone(JsvArrayBufferIterator *dstit, JsvArrayBufferIterator *it);

/** ArrayBuffers have the slightly odd side-effect that you can't write an element
 * once you have read it. That's why we have jsvArrayBufferIteratorGetValueAndRewind
 * which allows this, but is slower. */
JsVar *jsvArrayBufferIteratorGetValue(JsvArrayBufferIterator *it);
JsVar *jsvArrayBufferIteratorGetValueAndRewind(JsvArrayBufferIterator *it);
JsVarInt jsvArrayBufferIteratorGetIntegerValue(JsvArrayBufferIterator *it); 
JsVarFloat jsvArrayBufferIteratorGetFloatValue(JsvArrayBufferIterator *it);
void   jsvArrayBufferIteratorSetValue(JsvArrayBufferIterator *it, JsVar *value);
void   jsvArrayBufferIteratorSetValueAndRewind(JsvArrayBufferIterator *it, JsVar *value);
void   jsvArrayBufferIteratorSetIntegerValue(JsvArrayBufferIterator *it, JsVarInt value);
void   jsvArrayBufferIteratorSetByteValue(JsvArrayBufferIterator *it, char c); ///< special case for when we know we're writing to a byte array
JsVar* jsvArrayBufferIteratorGetIndex(JsvArrayBufferIterator *it);
bool   jsvArrayBufferIteratorHasElement(JsvArrayBufferIterator *it);
void   jsvArrayBufferIteratorNext(JsvArrayBufferIterator *it);
void   jsvArrayBufferIteratorFree(JsvArrayBufferIterator *it);
// --------------------------------------------------------------------------------------------
typedef struct {
  JsvObjectIterator it;
  JsVar *var; // underlying array when using JSVI_FULLARRAY
  JsVarInt index; // index when using JSVI_FULLARRAY
} JsvIteratorObj;

union JsvIteratorUnion {
  JsvStringIterator str;
  JsvIteratorObj obj;
  JsvArrayBufferIterator buf;
};

/** General Purpose iterator, for Strings, Arrays, Objects, Typed Arrays */
typedef struct JsvIterator {
  enum {
    JSVI_NONE,
    JSVI_STRING,
    JSVI_OBJECT,
    JSVI_ARRAYBUFFER,
    JSVI_FULLARRAY, // iterate over ALL array items - including not defined
  } type;
  union JsvIteratorUnion it;
} JsvIterator;

typedef enum {
  JSIF_DEFINED_ARRAY_ElEMENTS = 0, ///< iterate only over defined array elements in sparse arrays
  JSIF_EVERY_ARRAY_ELEMENT = 1, ///< iterate over every element in arrays, even if not defined
} JsvIteratorFlags;

/** Create a new iterator for any type of variable.
If iterating over an array and everyArrayElement is false, any
array elements that haven't been specified will be skipped */
void jsvIteratorNew(JsvIterator *it, JsVar *obj, JsvIteratorFlags flags);
JsVar *jsvIteratorGetKey(JsvIterator *it);
JsVar *jsvIteratorGetValue(JsvIterator *it);
JsVarInt jsvIteratorGetIntegerValue(JsvIterator *it);
JsVarFloat jsvIteratorGetFloatValue(JsvIterator *it);
JsVar *jsvIteratorSetValue(JsvIterator *it, JsVar *value); // set the value - return it in case we need to unlock it, eg. jsvUnLock(jsvIteratorSetValue(&it, jsvNew...));
bool jsvIteratorHasElement(JsvIterator *it);
void jsvIteratorNext(JsvIterator *it);
void jsvIteratorFree(JsvIterator *it);
void jsvIteratorClone(JsvIterator *dstit, JsvIterator *it);



#endif /* JSVAR_H_ */
