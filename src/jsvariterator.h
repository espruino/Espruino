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

// --------------------------------------------------------------------------------------------
typedef struct JsvStringIterator {
  size_t charIdx; ///< index of character in var
  size_t charsInVar; ///< total characters in var
  size_t varIndex; ///< index in string of the start of this var
  JsVar *var; ///< current StringExt we're looking at
} JsvStringIterator;

// slight hack to enure we can use string iterator with const JsVars
#define jsvStringIteratorNewConst(it,str,startIdx) jsvStringIteratorNew(it,(JsVar*)str,startIdx)

/// Create a new String iterator from a string, starting from a specific character. NOTE: This does not keep a lock to the first element, so make sure you do or the string will be freed!
void jsvStringIteratorNew(JsvStringIterator *it, JsVar *str, size_t startIdx);

/// Clone the string iterator
static inline JsvStringIterator jsvStringIteratorClone(JsvStringIterator *it) {
  JsvStringIterator i = *it;
  if (i.var) jsvLockAgain(i.var);
  return i;
}


/// Gets the current character (or 0)
static inline char jsvStringIteratorGetChar(JsvStringIterator *it) {
  if (!it->var) return 0;
  return  it->var->varData.str[it->charIdx];
}

/// Gets the current (>=0) character (or -1)
static inline int jsvStringIteratorGetCharOrMinusOne(JsvStringIterator *it) {
  if (!it->var) return -1;
  return (int)(unsigned char)it->var->varData.str[it->charIdx];
}

/// Do we have a character, or are we at the end?
static inline bool jsvStringIteratorHasChar(JsvStringIterator *it) {
  return it->charIdx < it->charsInVar;
}

/// Sets a character (will not extend the string - just overwrites)
static inline void jsvStringIteratorSetChar(JsvStringIterator *it, char c) {
  if (jsvStringIteratorHasChar(it))
    it->var->varData.str[it->charIdx] = c;
}

/// Gets the current index in the string
static inline size_t jsvStringIteratorGetIndex(JsvStringIterator *it) {
  return it->varIndex + it->charIdx;
}

/// Move to next character
void jsvStringIteratorNext(JsvStringIterator *it);

/// Move to next character (this one is inlined where speed is needed)
static inline void jsvStringIteratorNextInline(JsvStringIterator *it) {
  it->charIdx++;
  if (it->charIdx >= it->charsInVar) {
    it->charIdx -= it->charsInVar;
    if (it->var && jsvGetLastChild(it->var)) {
      JsVar *next = jsvLock(jsvGetLastChild(it->var));
      jsvUnLock(it->var);
      it->var = next;
      it->varIndex += it->charsInVar;
      it->charsInVar = jsvGetCharactersInVar(it->var);
    } else {
      jsvUnLock(it->var);
      it->var = 0;
      it->varIndex += it->charsInVar;
      it->charsInVar = 0;
    }
  }
}


/// Go to the end of the string iterator - for use with jsvStringIteratorAppend
void jsvStringIteratorGotoEnd(JsvStringIterator *it);

/// Append a character TO THE END of a string iterator
void jsvStringIteratorAppend(JsvStringIterator *it, char ch);

static inline void jsvStringIteratorFree(JsvStringIterator *it) {
  jsvUnLock(it->var);
}

/// Special version of append designed for use with vcbprintf_callback (See jsvAppendPrintf)
void jsvStringIteratorPrintfCallback(const char *str, void *user_data);

// --------------------------------------------------------------------------------------------
typedef struct JsvArrayIterator {
  JsVar *var;
} JsvArrayIterator;

static inline void jsvArrayIteratorNew(JsvArrayIterator *it, JsVar *arr) {
  assert(jsvIsArray(arr));
  it->var = jsvGetFirstChild(arr) ? jsvLock(jsvGetFirstChild(arr)) : 0;
}

/// Clone the iterator
static inline JsvArrayIterator jsvArrayIteratorClone(JsvArrayIterator *it) {
  JsvArrayIterator i = *it;
  if (i.var) jsvLockAgain(i.var);
  return i;
}

/// Gets the current array element (or 0)
static inline JsVar *jsvArrayIteratorGetElement(JsvArrayIterator *it) {
  if (!it->var) return 0; // end of array
  return jsvSkipName(it->var); // might even be undefined
}

/// Set the current array element
static inline void jsvArrayIteratorSetElement(JsvArrayIterator *it, JsVar *value) {
  if (!it->var) return; // end of array
  jsvSetValueOfName(it->var, value);
}

/// Gets the current array element index (or 0)
static inline JsVar *jsvArrayIteratorGetIndex(JsvArrayIterator *it) {
  if (!it->var) return 0;
  return jsvLockAgain(it->var);
}

/// Do we have a character, or are we at the end?
static inline bool jsvArrayIteratorHasElement(JsvArrayIterator *it) {
  return it->var != 0;
}

/// Move to next element
static inline void jsvArrayIteratorNext(JsvArrayIterator *it) {
  if (it->var) {
    JsVarRef next = jsvGetNextSibling(it->var);
    jsvUnLock(it->var);
    it->var = next ? jsvLock(next) : 0;
  }
}

/// Remove the current element and move to next element. Needs the parent supplied (the JsVar passed to jsvArrayIteratorNew) as we don't store it
static inline void jsvArrayIteratorRemoveAndGotoNext(JsvArrayIterator *it, JsVar *parent) {
  if (it->var) {
    JsVarRef next = jsvGetNextSibling(it->var);
    jsvRemoveChild(parent, it->var);
    jsvUnLock(it->var);
    it->var = next ? jsvLock(next) : 0;
  }
}

static inline void jsvArrayIteratorFree(JsvArrayIterator *it) {
  jsvUnLock(it->var);
}
// --------------------------------------------------------------------------------------------
typedef struct JsvObjectIterator {
  JsVar *var;
} JsvObjectIterator;

static inline void jsvObjectIteratorNew(JsvObjectIterator *it, JsVar *obj) {
  assert(jsvIsObject(obj) || jsvIsFunction(obj));
  it->var = jsvGetFirstChild(obj) ? jsvLock(jsvGetFirstChild(obj)) : 0;
}

/// Clone the iterator
static inline JsvObjectIterator jsvObjectIteratorClone(JsvObjectIterator *it) {
  JsvObjectIterator i = *it;
  if (i.var) jsvLockAgain(i.var);
  return i;
}

/// Gets the current object element key (or 0)
static inline JsVar *jsvObjectIteratorGetKey(JsvObjectIterator *it) {
  if (!it->var) return 0; // end of object
  return jsvLockAgain(it->var);
}

/// Gets the current object element value (or 0)
static inline JsVar *jsvObjectIteratorGetValue(JsvObjectIterator *it) {
  if (!it->var) return 0; // end of object
  return jsvSkipName(it->var); // might even be undefined
}

/// Set the current array element
static inline void jsvObjectIteratorSetValue(JsvObjectIterator *it, JsVar *value) {
  if (!it->var) return; // end of object
  jsvSetValueOfName(it->var, value);
}


/// Do we have a key, or are we at the end?
static inline bool jsvObjectIteratorHasElement(JsvObjectIterator *it) {
  return it->var != 0;
}

/// Move to next character
static inline void jsvObjectIteratorNext(JsvObjectIterator *it) {
  if (it->var) {
    JsVarRef next = jsvGetNextSibling(it->var);
    jsvUnLock(it->var);
    it->var = next ? jsvLock(next) : 0;
  }
}

static inline void jsvObjectIteratorFree(JsvObjectIterator *it) {
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

void   jsvArrayBufferIteratorNew(JsvArrayBufferIterator *it, JsVar *arrayBuffer, size_t index);

/// Clone the iterator
static inline JsvArrayBufferIterator jsvArrayBufferIteratorClone(JsvArrayBufferIterator *it) {
  JsvArrayBufferIterator i = *it;
  i.it = jsvStringIteratorClone(&it->it);
  return i;
}

/** ArrayBuffers have the slightly odd side-effect that you can't write an element
 * once you have read it. That's why we have jsvArrayBufferIteratorGetValueAndRewind
 * which allows this, but is slower. */

JsVar *jsvArrayBufferIteratorGetValue(JsvArrayBufferIterator *it);
JsVar *jsvArrayBufferIteratorGetValueAndRewind(JsvArrayBufferIterator *it);
JsVarInt jsvArrayBufferIteratorGetIntegerValue(JsvArrayBufferIterator *it);
JsVarFloat jsvArrayBufferIteratorGetFloatValue(JsvArrayBufferIterator *it);
void   jsvArrayBufferIteratorSetValue(JsvArrayBufferIterator *it, JsVar *value);
void   jsvArrayBufferIteratorSetIntegerValue(JsvArrayBufferIterator *it, JsVarInt value);
JsVar* jsvArrayBufferIteratorGetIndex(JsvArrayBufferIterator *it);
bool   jsvArrayBufferIteratorHasElement(JsvArrayBufferIterator *it);
void   jsvArrayBufferIteratorNext(JsvArrayBufferIterator *it);
void   jsvArrayBufferIteratorFree(JsvArrayBufferIterator *it);
// --------------------------------------------------------------------------------------------
union JsvIteratorUnion {
  JsvStringIterator str;
  JsvObjectIterator obj;
  JsvArrayIterator arr;
  JsvArrayBufferIterator buf;
};

/** General Purpose iterator, for Strings, Arrays, Objects, Typed Arrays */
typedef struct JsvIterator {
  enum {JSVI_STRING, JSVI_ARRAY, JSVI_OBJECT, JSVI_ARRAYBUFFER } type;
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
