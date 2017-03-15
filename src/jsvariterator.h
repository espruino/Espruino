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

/** Iterate over the contents of var, calling callback for each. Contents may be:
 *   * numeric -> output
 *   * a string -> output each character
 *   * array/arraybuffer -> call itself on each element
 *   * object -> call itself object.count times, on object.data
 */
bool jsvIterateCallback(JsVar *var, void (*callback)(int item, void *callbackData), void *callbackData);

/** If jsvIterateCallback is called, how many times will it call the callback function? */
int jsvIterateCallbackCount(JsVar *var);

/** Write all data in array to the data pointer (of size dataSize bytes) */
unsigned int jsvIterateCallbackToBytes(JsVar *var, unsigned char *data, unsigned int dataSize);

// --------------------------------------------------------------------------------------------
typedef struct JsvStringIterator {
  size_t charIdx; ///< index of character in var
  size_t charsInVar; ///< total characters in var
  size_t varIndex; ///< index in string of the start of this var
  JsVar *var; ///< current StringExt we're looking at
  char  *ptr; ///< a pointer to string data
} JsvStringIterator;

// slight hack to enure we can use string iterator with const JsVars
#define jsvStringIteratorNewConst(it,str,startIdx) jsvStringIteratorNew(it, (JsVar*)str, startIdx)

/// Create a new String iterator from a string, starting from a specific character. NOTE: This does not keep a lock to the first element, so make sure you do or the string will be freed!
void jsvStringIteratorNew(JsvStringIterator *it, JsVar *str, size_t startIdx);

/// Clone the string iterator
JsvStringIterator jsvStringIteratorClone(JsvStringIterator *it);

/// Gets the current character (or 0)
static ALWAYS_INLINE char jsvStringIteratorGetChar(JsvStringIterator *it) {
  if (!it->ptr) return 0;
  return (char)READ_FLASH_UINT8(&it->ptr[it->charIdx]);
}

/// Gets the current (>=0) character (or -1)
int jsvStringIteratorGetCharOrMinusOne(JsvStringIterator *it);

/// Do we have a character, or are we at the end?
static ALWAYS_INLINE bool jsvStringIteratorHasChar(JsvStringIterator *it) {
  return it->charIdx < it->charsInVar;
}

/// Sets a character (will not extend the string - just overwrites)
void jsvStringIteratorSetChar(JsvStringIterator *it, char c);

/// Gets the current index in the string
static ALWAYS_INLINE size_t jsvStringIteratorGetIndex(JsvStringIterator *it) {
  return it->varIndex + it->charIdx;
}

/// Move to next character
void jsvStringIteratorNext(JsvStringIterator *it);

/// Move to next character (this one is inlined where speed is needed)
static ALWAYS_INLINE void jsvStringIteratorNextInline(JsvStringIterator *it) {
  it->charIdx++;
  if (it->charIdx >= it->charsInVar) {
    it->charIdx -= it->charsInVar;
    if (it->var && jsvGetLastChild(it->var)) {
      JsVar *next = jsvLock(jsvGetLastChild(it->var));
      jsvUnLock(it->var);
      it->var = next;
      it->ptr = &next->varData.str[0];
      it->varIndex += it->charsInVar;
      it->charsInVar = jsvGetCharactersInVar(it->var);
    } else {
      jsvUnLock(it->var);
      it->var = 0;
      it->ptr = 0;
      it->varIndex += it->charsInVar;
      it->charsInVar = 0;
    }
  }
}


/// Go to the end of the string iterator - for use with jsvStringIteratorAppend
void jsvStringIteratorGotoEnd(JsvStringIterator *it);

/// Append a character TO THE END of a string iterator
void jsvStringIteratorAppend(JsvStringIterator *it, char ch);

/// Append an entire JsVar string TO THE END of a string iterator
void jsvStringIteratorAppendString(JsvStringIterator *it, JsVar *str);

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
JsvObjectIterator jsvObjectIteratorClone(JsvObjectIterator *it);

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
JsvArrayBufferIterator jsvArrayBufferIteratorClone(JsvArrayBufferIterator *it);

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
union JsvIteratorUnion {
  JsvStringIterator str;
  JsvObjectIterator obj;
  JsvArrayBufferIterator buf;
};

/** General Purpose iterator, for Strings, Arrays, Objects, Typed Arrays */
typedef struct JsvIterator {
  enum {JSVI_STRING, JSVI_OBJECT, JSVI_ARRAYBUFFER } type;
  union JsvIteratorUnion it;
} JsvIterator;

void jsvIteratorNew(JsvIterator *it, JsVar *obj);
JsVar *jsvIteratorGetKey(JsvIterator *it);
JsVar *jsvIteratorGetValue(JsvIterator *it);
JsVarInt jsvIteratorGetIntegerValue(JsvIterator *it);
JsVarFloat jsvIteratorGetFloatValue(JsvIterator *it);
JsVar *jsvIteratorSetValue(JsvIterator *it, JsVar *value); // set the value - return it in case we need to unlock it, eg. jsvUnLock(jsvIteratorSetValue(&it, jsvNew...));
bool jsvIteratorHasElement(JsvIterator *it);
void jsvIteratorNext(JsvIterator *it);
void jsvIteratorFree(JsvIterator *it);
JsvIterator jsvIteratorClone(JsvIterator *it);



#endif /* JSVAR_H_ */
