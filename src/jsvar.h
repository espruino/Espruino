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
 * Variables
 * ----------------------------------------------------------------------------
 */
#ifndef JSVAR_H_
#define JSVAR_H_

#include "jsutils.h"

/** To avoid confusion - JsVarRefCounter should be big enough
 * to store as many refs as can possibly be created - so it's
 * safe just to set it to the same size as JsVarRef. However
 * it is NOT a reference itself.
 */
typedef JsVarRef JsVarRefCounter;

typedef enum {
  ARRAYBUFFERVIEW_UNDEFINED = 0,

  ARRAYBUFFERVIEW_MASK_SIZE = 15,
  ARRAYBUFFERVIEW_SIGNED = 16,
  ARRAYBUFFERVIEW_FLOAT = 32,
  ARRAYBUFFERVIEW_CLAMPED = 64, // As in Uint8ClampedArray - clamp to the acceptable bounds
  ARRAYBUFFERVIEW_ARRAYBUFFER = 1 | 128, ///< Basic ArrayBuffer type

  ARRAYBUFFERVIEW_UINT8   = 1,
  ARRAYBUFFERVIEW_INT8    = 1 | ARRAYBUFFERVIEW_SIGNED,
  ARRAYBUFFERVIEW_UINT16  = 2,
  ARRAYBUFFERVIEW_INT16   = 2 | ARRAYBUFFERVIEW_SIGNED,
  ARRAYBUFFERVIEW_UINT32  = 4,
  ARRAYBUFFERVIEW_INT32   = 4 | ARRAYBUFFERVIEW_SIGNED,
  ARRAYBUFFERVIEW_FLOAT32 = 4 | ARRAYBUFFERVIEW_FLOAT,
  ARRAYBUFFERVIEW_FLOAT64 = 8 | ARRAYBUFFERVIEW_FLOAT,
} PACKED_FLAGS JsVarDataArrayBufferViewType;
#define JSV_ARRAYBUFFER_GET_SIZE(T) (size_t)((T)&ARRAYBUFFERVIEW_MASK_SIZE)
#define JSV_ARRAYBUFFER_IS_SIGNED(T) (((T)&ARRAYBUFFERVIEW_SIGNED)!=0)
#define JSV_ARRAYBUFFER_IS_FLOAT(T) (((T)&ARRAYBUFFERVIEW_FLOAT)!=0)
#define JSV_ARRAYBUFFER_IS_CLAMPED(T) (((T)&ARRAYBUFFERVIEW_CLAMPED)!=0)

#define JSV_ARRAYBUFFER_MAX_LENGTH 65535

typedef struct {
  unsigned short byteOffset;
  unsigned short length;
  JsVarDataArrayBufferViewType type;
} PACKED_FLAGS JsVarDataArrayBufferView;

/// Data for native functions
typedef struct {
  void (*ptr)(void); ///< Function pointer - this may not be the real address - see jsvGetNativeFunctionPtr
  unsigned short argTypes; ///< Actually a list of JsnArgumentType
} PACKED_FLAGS JsVarDataNative;

/// References
typedef struct {
  /* padding for data. Must be big enough for an int */
  char pad[JSVAR_DATA_STRING_LEN];

  /* For Variable NAMES (e.g. Object/Array keys) these store actual next/previous pointers for a linked list or 0.
   *   - if nextSibling==prevSibling==!0 then they point to the object that should contain this name if it ever gets set to anything that's not undefined
   * For STRING_EXT - extra characters
   * Not used for other stuff
   */
#ifndef JSVARREF_PACKED_BITS
  JsVarRef nextSibling;
  JsVarRef prevSibling;

  JsVarRefCounter refs; ///< The number of references held to this - used for automatic garbage collection. NOT USED for STRINGEXT though (it is just extra characters)

  /**
   * For OBJECT/ARRAY/FUNCTION - this is the first child
   * For NAMES and REF - this is a link to the variable it points to
   * For STRING_EXT - extra character data (NOT a link)
   * For ARRAYBUFFER - a link to a string containing the data for the array buffer
   * For CHILD_OF - a link to the variable pointed to
   */
  JsVarRef firstChild;

  /**
   * For OBJECT/ARRAY/FUNCTION - this is the last child
   * For STRINGS/STRING_EXT/NAME+STRING - this is a link to more string data if it is needed
   * For REF - this is the 'parent' that the firstChild is a member of
   * For CHILD_OF - a link to the object that should contain the variable
   */
  JsVarRef lastChild;

#else // JSVARREF_PACKED_BITS
  // see declaration of JSVARREF_PACKED_BITS in jsutils.h for more info
  unsigned char nextSibling;
  unsigned char prevSibling;
  unsigned char refs;
  unsigned char firstChild;
  unsigned char lastChild;

  unsigned char pack; // extra packed bits if JSVARREF_PACKED_BITS
#endif
} PACKED_FLAGS JsVarDataRef;


/// Union that contains all the different types of data
typedef union {
    char str[JSVAR_DATA_STRING_MAX_LEN]; ///< The contents of this variable if it is a string
    /* NOTE: For str above, we INTENTIONALLY OVERFLOW str (and hence data) in the case of STRING_EXTS
     * to overwrite 3 references in order to grab another 6 bytes worth of string data */
    // TODO do some magic with union/structs in order to make sure we don't intentionally write off the end of arrays
    JsVarInt integer; ///< The contents of this variable if it is an int
    JsVarFloat floating; ///< The contents of this variable if it is a double
    JsVarDataArrayBufferView arraybuffer; ///< information for array buffer views.
    JsVarDataNative native; ///< A native function
    JsVarDataRef ref; ///< References
} PACKED_FLAGS JsVarData;

typedef struct {
  /** The actual variable data, as well as references (see below). Put first so word aligned */
  JsVarData varData;

  /** the flags determine the type of the variable - int/double/string/etc. */
  JsVarFlags flags;
} PACKED_FLAGS __attribute__((aligned(4))) JsVar;

/* We have a few different types:
 *
 *  OBJECT/ARRAY - uses firstChild/lastChild to link to NAMEs.
 *  BUILT-IN OBJECT - as above, but we use varData to store the name as well. This means built in object names must be LESS THAN 8 CHARACTERS
 *  FUNCTION - uses firstChild/lastChild to link to NAMEs, and callback is used
 *  NAME - use nextSibling/prevSibling linking to other NAMEs, and firstChild to link to a Variable of some kind
 *  STRING - use firstChild to link to other STRINGs if String value is too long
 *  INT/DOUBLE - firstChild never used
 */

/* For 'normal' JsVars used on Espruino Board (Linux are different to allow more storage):
 *
 * Both INT and STRING can also be names:
 *
 * | Byte  | Name    | STRING | STR_EXT  | NAME_STR | NAME_INT | INT  | DOUBLE | OBJ/FUNC/ARRAY | ARRAYBUFFER | FLAT_STR | NATIVE_FUNC |
 * |-------|---------|--------|----------|----------|----------|------|--------|----------------|-------------|----------|-------------|
 * | 0 - 3 | varData | data   | data     |  data    | data     | data | data   | nativePtr      | size        | size     | nativePtr   |
 * | 4 - 5 | next    | -      | data     |  next    | next     | -    | data   |                | format      | -        | argTypes    |
 * | 6 - 7 | prev    | -      | data     |  prev    | prev     | -    | data   |                | format      | -        | -           |
 * | 8 - 9 | refs    | refs   | data     |  refs    | refs     | refs | refs   | refs           | refs        | refs     | refs        |
 * | 10-11 | first   | -      | data     |  child   | child    |  -   |  -     | first          | stringPtr   | -        | -           |
 * | 12-13 | last    | nextPtr| nextPtr  |  nextPtr |  -       |  -   |  r?    | last           | -           | nextPtr  | -           |
 * | 14-15 | Flags   | Flags  | Flags    |  Flags   | Flags    | Flags| Flags  | Flags          | Flags       | Flags    | Flags       |
 *
 * For DOUBLE on 12 byte JsVar systems, the ref count it stored in 'lastChild' instead.
 * NAME_INT_INT/NAME_INT_BOOL are the same as NAME_INT, except 'child' contains the value rather than a pointer
 * NAME_STRING_INT is the same as NAME_STRING, except 'child' contains the value rather than a pointer
 * FLAT_STRING uses the variable blocks that follow it as flat storage for all the data
 * NATIVE_FUNCTION's nativePtr is a pointer to code if there is no child called JSPARSE_FUNCTION_CODE_NAME, but if there is one, it's an index into that child
 *
 * For Objects that represent hardware devices, 'nativePtr' is actually set to a special string that
 * contains the device number. See jsiGetDeviceFromClass/jspNewObject
 */

#ifndef JSVARREF_PACKED_BITS
static ALWAYS_INLINE JsVarRef jsvGetFirstChild(const JsVar *v) { return v->varData.ref.firstChild; }
static ALWAYS_INLINE JsVarRefSigned jsvGetFirstChildSigned(const JsVar *v) { return (JsVarRefSigned)v->varData.ref.firstChild; }
static ALWAYS_INLINE JsVarRef jsvGetLastChild(const JsVar *v) { return v->varData.ref.lastChild; }
static ALWAYS_INLINE JsVarRef jsvGetNextSibling(const JsVar *v) { return v->varData.ref.nextSibling; }
static ALWAYS_INLINE JsVarRef jsvGetPrevSibling(const JsVar *v) { return v->varData.ref.prevSibling; }
static ALWAYS_INLINE void jsvSetFirstChild(JsVar *v, JsVarRef r) { v->varData.ref.firstChild = r; }
static ALWAYS_INLINE void jsvSetLastChild(JsVar *v, JsVarRef r) { v->varData.ref.lastChild = r; }
static ALWAYS_INLINE void jsvSetNextSibling(JsVar *v, JsVarRef r) { v->varData.ref.nextSibling = r; }
static ALWAYS_INLINE void jsvSetPrevSibling(JsVar *v, JsVarRef r) { v->varData.ref.prevSibling = r; }
#else
// for packed bits, functions are not inlined to save space
JsVarRef jsvGetFirstChild(const JsVar *v);
JsVarRefSigned jsvGetFirstChildSigned(const JsVar *v);
JsVarRef jsvGetLastChild(const JsVar *v);
JsVarRef jsvGetNextSibling(const JsVar *v);
JsVarRef jsvGetPrevSibling(const JsVar *v);
void jsvSetFirstChild(JsVar *v, JsVarRef r);
void jsvSetLastChild(JsVar *v, JsVarRef r);
void jsvSetNextSibling(JsVar *v, JsVarRef r);
void jsvSetPrevSibling(JsVar *v, JsVarRef r);
#endif

#if JSVARREF_SIZE==1
// For 12 byte JsVars we have a problem as doubles will overwrite the ref count - so in this case we must use 'lastChild' instead
static ALWAYS_INLINE JsVarRefCounter jsvGetRefs(JsVar *v) {
  return (JsVarRefCounter)(((v->flags&JSV_VARTYPEMASK)==JSV_FLOAT)?v->varData.ref.lastChild:v->varData.ref.refs);
}
static ALWAYS_INLINE void jsvSetRefs(JsVar *v, JsVarRefCounter refs) {
  if ((v->flags&JSV_VARTYPEMASK)==JSV_FLOAT)
    v->varData.ref.lastChild = refs;
  else
    v->varData.ref.refs = refs;
}
#else
static ALWAYS_INLINE JsVarRefCounter jsvGetRefs(JsVar *v) { return v->varData.ref.refs; }
static ALWAYS_INLINE void jsvSetRefs(JsVar *v, JsVarRefCounter refs) { v->varData.ref.refs = refs; }
#endif
static ALWAYS_INLINE unsigned char jsvGetLocks(JsVar *v) { return (unsigned char)((v->flags>>JSV_LOCK_SHIFT) & JSV_LOCK_MAX); }

// For debugging/testing ONLY - maximum # of vars we are allowed to use
void jsvSetMaxVarsUsed(unsigned int size);

// Init/kill vars as a whole
void jsvInit();
void jsvKill();
void jsvSoftInit(); ///< called when loading from flash
void jsvSoftKill(); ///< called when saving to flash
JsVar *jsvFindOrCreateRoot(); ///< Find or create the ROOT variable item - used mainly if recovering from a saved state.
unsigned int jsvGetMemoryUsage(); ///< Get number of memory records (JsVars) used
unsigned int jsvGetMemoryTotal(); ///< Get total amount of memory records
bool jsvIsMemoryFull(); ///< Get whether memory is full or not
void jsvShowAllocated(); ///< Show what is still allocated, for debugging memory problems
/// Try and allocate more memory - only works if RESIZABLE_JSVARS is defined
void jsvSetMemoryTotal(unsigned int jsNewVarCount);


// Note that jsvNew* don't REF a variable for you, but the do LOCK it
JsVar *jsvNewWithFlags(JsVarFlags flags); ///< Create a new variable with the given flags
JsVar *jsvNewFlatStringOfLength(unsigned int byteLength); ///< Try and create a special flat string
JsVar *jsvNewFromString(const char *str); ///< Create a new string
JsVar *jsvNewStringOfLength(unsigned int byteLength); ///< Create a new string of the given length - full of 0s
static ALWAYS_INLINE JsVar *jsvNewFromEmptyString() { JsVar *v = jsvNewWithFlags(JSV_STRING_0); return v; } ;///< Create a new empty string
static ALWAYS_INLINE JsVar *jsvNewNull() { return jsvNewWithFlags(JSV_NULL); } ;///< Create a new null variable
/** Create a new variable from a substring. argument must be a string. stridx = start char or str, maxLength = max number of characters (can be JSVAPPENDSTRINGVAR_MAXLENGTH)  */
JsVar *jsvNewFromStringVar(const JsVar *str, size_t stridx, size_t maxLength);
JsVar *jsvNewFromInteger(JsVarInt value);
JsVar *jsvNewFromBool(bool value);
JsVar *jsvNewFromFloat(JsVarFloat value);
// Create an integer (or float) from this value, depending on whether it'll fit in 32 bits or not.
JsVar *jsvNewFromLongInteger(long long value);
// Turns var into a Variable name that links to the given value... No locking so no need to unlock var
JsVar *jsvMakeIntoVariableName(JsVar *var, JsVar *valueOrZero);
void jsvMakeFunctionParameter(JsVar *v);
JsVar *jsvNewFromPin(int pin);
JsVar *jsvNewArray(JsVar **elements, int elementCount); ///< Create an array containing the given elements
JsVar *jsvNewNativeFunction(void (*ptr)(void), unsigned short argTypes); ///< Create an array containing the given elements
JsVar *jsvNewArrayBufferFromString(JsVar *str, unsigned int lengthOrZero); ///< Create a new ArrayBuffer backed by the given string. If length is not specified, it will be worked out

void *jsvGetNativeFunctionPtr(const JsVar *function); ///< Get the actual pointer from a native function - this may not be the contents of varData.native.ptr

/// Get a reference from a var - SAFE for null vars
ALWAYS_INLINE JsVarRef jsvGetRef(JsVar *var);

/// SCARY - only to be used for vital stuff like load/save
ALWAYS_INLINE JsVar *_jsvGetAddressOf(JsVarRef ref);

/// Lock this reference and return a pointer - UNSAFE for null refs
ALWAYS_INLINE JsVar *jsvLock(JsVarRef ref);

/// Lock this pointer and return a pointer - UNSAFE for null pointer
ALWAYS_INLINE JsVar *jsvLockAgain(JsVar *var);

/// Lock this pointer and return a pointer - SAFE for null pointer
ALWAYS_INLINE JsVar *jsvLockAgainSafe(JsVar *var);

/// Unlock this variable - this is SAFE for null variables
ALWAYS_INLINE void jsvUnLock(JsVar *var);


/// Reference - set this variable as used by something
JsVar *jsvRef(JsVar *v);

/// Unreference - set this variable as not used by anything
void jsvUnRef(JsVar *var);

/// Helper fn, Reference - set this variable as used by something
JsVarRef jsvRefRef(JsVarRef ref);

/// Helper fn, Unreference - set this variable as not used by anything
JsVarRef jsvUnRefRef(JsVarRef ref);

static ALWAYS_INLINE bool jsvIsRoot(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_ROOT; }
static ALWAYS_INLINE bool jsvIsPin(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_PIN; }
static ALWAYS_INLINE bool jsvIsSimpleInt(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_INTEGER; } // is just a very basic integer value
static ALWAYS_INLINE bool jsvIsInt(const JsVar *v) { return v && ((v->flags&JSV_VARTYPEMASK)==JSV_INTEGER || (v->flags&JSV_VARTYPEMASK)==JSV_PIN || (v->flags&JSV_VARTYPEMASK)==JSV_NAME_INT || (v->flags&JSV_VARTYPEMASK)==JSV_NAME_INT_INT || (v->flags&JSV_VARTYPEMASK)==JSV_NAME_INT_BOOL); }
static ALWAYS_INLINE bool jsvIsFloat(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_FLOAT; }
static ALWAYS_INLINE bool jsvIsBoolean(const JsVar *v) { return v && ((v->flags&JSV_VARTYPEMASK)==JSV_BOOLEAN || (v->flags&JSV_VARTYPEMASK)==JSV_NAME_INT_BOOL); }
static ALWAYS_INLINE bool jsvIsString(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)>=_JSV_STRING_START && (v->flags&JSV_VARTYPEMASK)<=_JSV_STRING_END; }
static ALWAYS_INLINE bool jsvIsStringExt(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)>=JSV_STRING_EXT_0 && (v->flags&JSV_VARTYPEMASK)<=JSV_STRING_EXT_MAX; } ///< The extra bits dumped onto the end of a string to store more data
static ALWAYS_INLINE bool jsvIsFlatString(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_FLAT_STRING; }
static ALWAYS_INLINE bool jsvIsNumeric(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)>=_JSV_NUMERIC_START && (v->flags&JSV_VARTYPEMASK)<=_JSV_NUMERIC_END; }
static ALWAYS_INLINE bool jsvIsFunction(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_FUNCTION; }
static ALWAYS_INLINE bool jsvIsFunctionParameter(const JsVar *v) { return v && (v->flags&JSV_NATIVE) && jsvIsString(v); }
static ALWAYS_INLINE bool jsvIsObject(const JsVar *v) { return v && (((v->flags&JSV_VARTYPEMASK)==JSV_OBJECT) || ((v->flags&JSV_VARTYPEMASK)==JSV_ROOT)); }
static ALWAYS_INLINE bool jsvIsArray(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_ARRAY; }
static ALWAYS_INLINE bool jsvIsArrayBuffer(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_ARRAYBUFFER; }
static ALWAYS_INLINE bool jsvIsArrayBufferName(const JsVar *v) { return v && (v->flags&(JSV_VARTYPEMASK))==JSV_ARRAYBUFFERNAME; }
static ALWAYS_INLINE bool jsvIsNative(const JsVar *v) { return v && (v->flags&JSV_NATIVE)!=0; }
static ALWAYS_INLINE bool jsvIsNativeFunction(const JsVar *v) { return v && (v->flags&(JSV_NATIVE|JSV_VARTYPEMASK))==(JSV_NATIVE|JSV_FUNCTION); }
static ALWAYS_INLINE bool jsvIsUndefined(const JsVar *v) { return v==0; }
static ALWAYS_INLINE bool jsvIsNull(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_NULL; }
static ALWAYS_INLINE bool jsvIsBasic(const JsVar *v) { return jsvIsNumeric(v) || jsvIsString(v);} ///< Is this *not* an array/object/etc
static ALWAYS_INLINE bool jsvIsName(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)>=_JSV_NAME_START && (v->flags&JSV_VARTYPEMASK)<=_JSV_NAME_END; } ///< NAMEs are what's used to name a variable (it is not the data itself)
/// Names with values have firstChild set to a value - AND NOT A REFERENCE
static ALWAYS_INLINE bool jsvIsNameWithValue(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)>=_JSV_NAME_WITH_VALUE_START && (v->flags&JSV_VARTYPEMASK)<=_JSV_NAME_WITH_VALUE_END; }
static ALWAYS_INLINE bool jsvIsNameInt(const JsVar *v) { return v && ((v->flags&JSV_VARTYPEMASK)==JSV_NAME_INT_INT || ((v->flags&JSV_VARTYPEMASK)>=JSV_NAME_STRING_INT_0 && (v->flags&JSV_VARTYPEMASK)<=JSV_NAME_STRING_INT_MAX)); }
static ALWAYS_INLINE bool jsvIsNameIntInt(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_NAME_INT_INT; }
static ALWAYS_INLINE bool jsvIsNameIntBool(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_NAME_INT_BOOL; }
/// What happens when we access a variable that doesn't exist. We get a NAME where the next + previous siblings point to the object that may one day contain them
static ALWAYS_INLINE bool jsvIsNewChild(const JsVar *v) { return jsvIsName(v) && jsvGetNextSibling(v) && jsvGetNextSibling(v)==jsvGetPrevSibling(v); }
/// See jsvIsNewChild - for fields that don't exist yet
JsVar *jsvCreateNewChild(JsVar *parent, JsVar *index, JsVar *child);
/// Are var.varData.ref.* (excl pad) used for data (so we expect them not to be empty)
static ALWAYS_INLINE bool jsvIsRefUsedForData(const JsVar *v) { return jsvIsStringExt(v) || jsvIsFloat(v) || jsvIsNativeFunction(v) || jsvIsArrayBuffer(v) || jsvIsArrayBufferName(v); }

/// Can the given variable be converted into an integer without loss of precision
static ALWAYS_INLINE bool jsvIsIntegerish(const JsVar *v) { return jsvIsInt(v) || jsvIsPin(v) || jsvIsBoolean(v) || jsvIsNull(v); }

static ALWAYS_INLINE bool jsvIsIterable(const JsVar *v) {
  return jsvIsArray(v) || jsvIsObject(v) || jsvIsFunction(v) ||
         jsvIsString(v) || jsvIsArrayBuffer(v);
}

/** Does this string contain only Numeric characters (with optional whitespace and/or '-'/'+' at the front)? NOT '.'/'e' and similar (allowDecimalPoint is for '.' only) */
bool jsvIsStringNumericInt(const JsVar *var, bool allowDecimalPoint);
/** Does this string contain only Numeric characters? This is for arrays
 * and makes the assertion that int_to_string(string_to_int(var))==var */
bool jsvIsStringNumericStrict(const JsVar *var);

// TODO: maybe isName shouldn't include ArrayBufferName?
bool jsvHasCharacterData(const JsVar *v); ///< does the v->data union contain character data?
bool jsvHasStringExt(const JsVar *v);
/// Does this variable use firstChild/lastChild to point to multiple children
bool jsvHasChildren(const JsVar *v);
/// Is this variable a type that uses firstChild to point to a single Variable (ie. it doesn't have multiple children)
bool jsvHasSingleChild(const JsVar *v);

/// Does this variable have a 'ref' argument? Stringexts use it for extra character data
static ALWAYS_INLINE bool jsvHasRef(const JsVar *v) { return !jsvIsStringExt(v); }

/// This is the number of characters a JsVar can contain, NOT string length
static ALWAYS_INLINE size_t jsvGetMaxCharactersInVar(const JsVar *v) {
  // see jsvCopy - we need to know about this in there too
  if (jsvIsStringExt(v)) return JSVAR_DATA_STRING_MAX_LEN;
  assert(jsvHasCharacterData(v));
  return JSVAR_DATA_STRING_LEN;
}

/// This is the number of characters a JsVar can contain, NOT string length
static ALWAYS_INLINE size_t jsvGetCharactersInVar(const JsVar *v) {
  unsigned int f = v->flags&JSV_VARTYPEMASK;
  if (f == JSV_FLAT_STRING)
    return (size_t)v->varData.integer;
  assert(f >= JSV_NAME_STRING_INT_0);
  assert((JSV_NAME_STRING_INT_0 < JSV_NAME_STRING_0) &&
         (JSV_NAME_STRING_0 < JSV_STRING_0) &&
         (JSV_STRING_0 < JSV_STRING_EXT_0)); // this relies on ordering
  if (f<=JSV_NAME_STRING_MAX) {
    if (f<=JSV_NAME_STRING_INT_MAX)
      return f-JSV_NAME_STRING_INT_0;
    else
      return f-JSV_NAME_STRING_0;
  } else {
    if (f<=JSV_STRING_MAX) return f-JSV_STRING_0;
    assert(f <= JSV_STRING_EXT_MAX);
    return f - JSV_STRING_EXT_0;
  }
}

/// This is the number of characters a JsVar can contain, NOT string length
static ALWAYS_INLINE void jsvSetCharactersInVar(JsVar *v, size_t chars) {
  unsigned int f = v->flags&JSV_VARTYPEMASK;
  JsVarFlags m = (JsVarFlags)(v->flags&~JSV_VARTYPEMASK);
  assert(f >= JSV_NAME_STRING_INT_0);
  assert((JSV_NAME_STRING_INT_0 < JSV_NAME_STRING_0) &&
         (JSV_NAME_STRING_0 < JSV_STRING_0) &&
         (JSV_STRING_0 < JSV_STRING_EXT_0)); // this relies on ordering
  if (f<=JSV_NAME_STRING_MAX) {
      if (f<=JSV_NAME_STRING_INT_MAX)
        v->flags = (JsVarFlags)(m | (JSV_NAME_STRING_INT_0+chars));
      else
        v->flags = (JsVarFlags)(m | (JSV_NAME_STRING_0+chars));
    } else {
      if (f<=JSV_STRING_MAX) {
        v->flags = (JsVarFlags)(m | (JSV_STRING_0+chars));
      } else {
        assert(f <= JSV_STRING_EXT_MAX);
        v->flags = (JsVarFlags)(m | (JSV_STRING_EXT_0+chars));
      }
    }
}

/** Check if two Basic Variables are equal (this IGNORES the value that is pointed to,
 * so 'a=5'=='a=7' but 'a=5'!='b=5')
 */
bool jsvIsBasicVarEqual(JsVar *a, JsVar *b);

/** Check if two things are equal. Basic vars are done by value,
 * for anything else the reference/pointer must be equal */
bool jsvIsEqual(JsVar *a, JsVar *b);


const char *jsvGetConstString(const JsVar *v); ///< Get a const string representing this variable - if we can. Otherwise return 0
const char *jsvGetTypeOf(const JsVar *v); ///< Return the 'type' of the JS variable (eg. JS's typeof operator)
JsVar *jsvGetValueOf(JsVar *v); ///< Return the JsVar, or if it's an object and has a valueOf function, call that
size_t jsvGetString(const JsVar *v, char *str, size_t len); ///< Save this var as a string to the given buffer, and return how long it was (return val doesn't include terminating 0)
size_t jsvGetStringChars(const JsVar *v, size_t startChar, char *str, size_t len); ///< Get len bytes of string data from this string. Does not error if string len is not equal to len
void jsvSetString(JsVar *v, char *str, size_t len); ///< Set the Data in this string. This must JUST overwrite - not extend or shrink
JsVar *jsvAsString(JsVar *var, bool unlockVar); ///< If var is a string, lock and return it, else create a new string
JsVar *jsvAsFlatString(JsVar *var); ///< Create a flat string from the given variable (or return it if it is already a flat string)
bool jsvIsEmptyString(JsVar *v); ///< Returns true if the string is empty - faster than jsvGetStringLength(v)==0
size_t jsvGetStringLength(JsVar *v); ///< Get the length of this string, IF it is a string
size_t jsvGetFlatStringBlocks(JsVar *v); ///< return the number of blocks used by the given flat string
char *jsvGetFlatStringPointer(JsVar *v); ///< Get a pointer to the data in this flat string
size_t jsvGetLinesInString(JsVar *v); ///<  IN A STRING get the number of lines in the string (min=1)
size_t jsvGetCharsOnLine(JsVar *v, size_t line); ///<  IN A STRING Get the number of characters on a line - lines start at 1
void jsvGetLineAndCol(JsVar *v, size_t charIdx, size_t* line, size_t *col); ///< IN A STRING, get the line and column of the given character. Both values must be non-null
size_t jsvGetIndexFromLineAndCol(JsVar *v, size_t line, size_t col); ///<  IN A STRING, get a character index from a line and column


/**
  jsvIsStringEqualOrStartsWith(A, B, false) is a proper A==B
  jsvIsStringEqualOrStartsWith(A, B, true) is A.startsWith(B)
*/
bool jsvIsStringEqualOrStartsWith(JsVar *var, const char *str, bool isStartsWith);
bool jsvIsStringEqual(JsVar *var, const char *str);
int jsvCompareString(JsVar *va, JsVar *vb, size_t starta, size_t startb, bool equalAtEndOfString); ///< Compare 2 strings, starting from the given character positions
int jsvCompareInteger(JsVar *va, JsVar *vb); ///< Compare 2 integers, >0 if va>vb,  <0 if va<vb. If compared with a non-integer, that gets put later
void jsvAppendString(JsVar *var, const char *str); ///< Append the given string to this one
bool jsvAppendStringBuf(JsVar *var, const char *str, size_t length); ///< Append the given string to this one - but does not use null-terminated strings. returns false on failure (from out of memory)
void jsvAppendPrintf(JsVar *var, const char *fmt, ...); ///< Append the formatted string to a variable (see vcbprintf)
JsVar *jsvVarPrintf( const char *fmt, ...); ///< Create a var from the formatted string
static ALWAYS_INLINE void jsvAppendCharacter(JsVar *var, char ch) { jsvAppendStringBuf(var, &ch, 1); }; ///< Append the given character to this string
#define JSVAPPENDSTRINGVAR_MAXLENGTH (0x7FFFFFFF)
void jsvAppendStringVar(JsVar *var, const JsVar *str, size_t stridx, size_t maxLength); ///< Append str to var. Both must be strings. stridx = start char or str, maxLength = max number of characters (can be JSVAPPENDSTRINGVAR_MAXLENGTH)
void jsvAppendStringVarComplete(JsVar *var, const JsVar *str); ///< Append all of str to var. Both must be strings.
char jsvGetCharInString(JsVar *v, size_t idx);
int jsvGetStringIndexOf(JsVar *str, char ch); ///< Get the index of a character in a string, or -1

JsVarInt jsvGetInteger(const JsVar *v);
void jsvSetInteger(JsVar *v, JsVarInt value); ///< Set an integer value (use carefully!)
JsVarFloat jsvGetFloat(const JsVar *v); ///< Get the floating point representation of this var
bool jsvGetBool(const JsVar *v);
long long jsvGetLongInteger(const JsVar *v);
JsVar *jsvAsNumber(JsVar *var); ///< Convert the given variable to a number

static ALWAYS_INLINE JsVar *jsvAsNumberAndUnLock(JsVar *v) { JsVar *n = jsvAsNumber(v); jsvUnLock(v); return n; }
static ALWAYS_INLINE JsVarInt _jsvGetIntegerAndUnLock(JsVar *v) { JsVarInt i = jsvGetInteger(v); jsvUnLock(v); return i; }
static ALWAYS_INLINE JsVarFloat _jsvGetFloatAndUnLock(JsVar *v) { JsVarFloat f = jsvGetFloat(v); jsvUnLock(v); return f; }
static ALWAYS_INLINE bool _jsvGetBoolAndUnLock(JsVar *v) { bool b = jsvGetBool(v); jsvUnLock(v); return b; }
#ifdef SAVE_ON_FLASH
JsVarInt jsvGetIntegerAndUnLock(JsVar *v);
JsVarFloat jsvGetFloatAndUnLock(JsVar *v);
bool jsvGetBoolAndUnLock(JsVar *v);
#else
#define jsvGetIntegerAndUnLock _jsvGetIntegerAndUnLock
#define jsvGetFloatAndUnLock _jsvGetFloatAndUnLock
#define jsvGetBoolAndUnLock _jsvGetBoolAndUnLock
#endif
long long jsvGetLongIntegerAndUnLock(JsVar *v);




/** Get the item at the given location in the array buffer and return the result */
size_t jsvGetArrayBufferLength(JsVar *arrayBuffer);
/** Get the String the contains the data for this arrayBuffer */
JsVar *jsvGetArrayBufferBackingString(JsVar *arrayBuffer);
/** Get the item at the given location in the array buffer and return the result */
JsVar *jsvArrayBufferGet(JsVar *arrayBuffer, size_t index);
/** Set the item at the given location in the array buffer */
void jsvArrayBufferSet(JsVar *arrayBuffer, size_t index, JsVar *value);
/** Given an integer name that points to an arraybuffer or an arraybufferview, evaluate it and return the result */
JsVar *jsvArrayBufferGetFromName(JsVar *name);

/** Return an array containing the arguments of the given function */
JsVar *jsvGetFunctionArgumentLength(JsVar *function);

/** If a is a name skip it and go to what it points to - and so on.
 * ALWAYS locks - so must unlock what it returns. It MAY
 * return 0.  */
JsVar *jsvSkipName(JsVar *a);

/** If a is a name skip it and go to what it points to.
 * ALWAYS locks - so must unlock what it returns. It MAY
 * return 0.  */
JsVar *jsvSkipOneName(JsVar *a);

/** If a is a's child is a name skip it and go to what it points to.
 * ALWAYS locks - so must unlock what it returns.  */
JsVar *jsvSkipToLastName(JsVar *a);

/** Same as jsvSkipName, but ensures that 'a' is unlocked */
JsVar *jsvSkipNameAndUnLock(JsVar *a);

/** Same as jsvSkipOneName, but ensures that 'a' is unlocked */
JsVar *jsvSkipOneNameAndUnLock(JsVar *a);

/** Given a JsVar meant to be an index to an array, convert it to
 * the actual variable type we'll use to access the array. For example
 * a["0"] is actually translated to a[0]
 */
JsVar *jsvAsArrayIndex(JsVar *index);

/** Same as jsvAsArrayIndex, but ensures that 'index' is unlocked */
JsVar *jsvAsArrayIndexAndUnLock(JsVar *a);

/** Try and turn the supplied variable into a name. If not, make a new one. This locks again. */
JsVar *jsvAsName(JsVar *var);

/// MATHS!
JsVar *jsvMathsOpSkipNames(JsVar *a, JsVar *b, int op);
bool jsvMathsOpTypeEqual(JsVar *a, JsVar *b);
JsVar *jsvMathsOp(JsVar *a, JsVar *b, int op);
/// Negates an integer/double value
JsVar *jsvNegateAndUnLock(JsVar *v);

/** If the given element is found, return the path to it as a string of
 * the form 'foo.bar', else return 0. If we would have returned a.b and
 * ignoreParent is a, don't! */
JsVar *jsvGetPathTo(JsVar *root, JsVar *element, int maxDepth, JsVar *ignoreParent);

/// Copy this variable and return the locked copy
JsVar *jsvCopy(JsVar *src);
/** Copy only a name, not what it points to. ALTHOUGH the link to what it points to is maintained unless linkChildren=false.
    If keepAsName==false, this will be converted into a normal variable */
JsVar *jsvCopyNameOnly(JsVar *src, bool linkChildren, bool keepAsName);
/// Tree related stuff
void jsvAddName(JsVar *parent, JsVar *nameChild); // Add a child, which is itself a name
JsVar *jsvAddNamedChild(JsVar *parent, JsVar *child, const char *name); // Add a child, and create a name for it. Returns a LOCKED var. DOES NOT CHECK FOR DUPLICATES
JsVar *jsvSetNamedChild(JsVar *parent, JsVar *child, const char *name); // Add a child, and create a name for it. Returns a LOCKED name var. CHECKS FOR DUPLICATES
JsVar *jsvSetValueOfName(JsVar *name, JsVar *src); // Set the value of a child created with jsvAddName,jsvAddNamedChild. Returns the UNLOCKED name argument
JsVar *jsvFindChildFromString(JsVar *parent, const char *name, bool createIfNotFound); // Non-recursive finding of child with name. Returns a LOCKED var
JsVar *jsvFindChildFromVar(JsVar *parent, JsVar *childName, bool addIfNotFound); // Non-recursive finding of child with name. Returns a LOCKED var

/// Remove a child - note that the child MUST ACTUALLY BE A CHILD! and should be a name, not a value.
void jsvRemoveChild(JsVar *parent, JsVar *child);
void jsvRemoveAllChildren(JsVar *parent);
void jsvRemoveNamedChild(JsVar *parent, const char *name);

/// Get the named child of an object. If createChild!=0 then create the child
JsVar *jsvObjectGetChild(JsVar *obj, const char *name, JsVarFlags createChild);
/// Set the named child of an object, and return the child (so you can choose to unlock it if you want)
JsVar *jsvObjectSetChild(JsVar *obj, const char *name, JsVar *child);

int jsvGetChildren(JsVar *v); ///< number of children of a variable. also see jsvGetArrayLength and jsvGetLength
JsVar *jsvGetFirstName(JsVar *v); ///< Get the first child's name from an object,array or function
/// Check if the given name is a child of the parent
bool jsvIsChild(JsVar *parent, JsVar *child);
JsVarInt jsvGetArrayLength(const JsVar *arr); ///< Not the same as GetChildren, as it can be a sparse array
JsVarInt jsvSetArrayLength(JsVar *arr, JsVarInt length, bool truncate); ///< set an array's length, optionally truncating if the array becomes shorter
JsVarInt jsvGetLength(JsVar *src); ///< General purpose length function. Does the 'right' thing
size_t jsvCountJsVarsUsed(JsVar *v); ///< Count the amount of JsVars used. Mostly useful for debugging
JsVar *jsvGetArrayItem(const JsVar *arr, JsVarInt index); ///< Get an item at the specified index in the array (and lock it)
JsVar *jsvGetArrayIndexOf(JsVar *arr, JsVar *value, bool matchExact); ///< Get the index of the value in the array (matchExact==use pointer, not equality check)
JsVarInt jsvArrayAddToEnd(JsVar *arr, JsVar *value, JsVarInt initialValue); ///< Adds new elements to the end of an array, and returns the new length. initialValue is the item index when no items are currently in the array.
JsVarInt jsvArrayPush(JsVar *arr, JsVar *value); ///< Adds a new element to the end of an array, and returns the new length
JsVarInt jsvArrayPushAndUnLock(JsVar *arr, JsVar *value); ///< Adds a new element to the end of an array, unlocks it, and returns the new length
JsVar *jsvArrayPop(JsVar *arr); ///< Removes the last element of an array, and returns that element (or 0 if empty). includes the NAME
JsVar *jsvArrayPopFirst(JsVar *arr); ///< Removes the first element of an array, and returns that element (or 0 if empty) includes the NAME. DOES NOT RENUMBER.
void jsvArrayAddString(JsVar *arr, const char *text); ///< Adds a new String element to the end of an array (IF it was not already there)
JsVar *jsvArrayJoin(JsVar *arr, JsVar *filler); ///< Join all elements of an array together into a string
void jsvArrayInsertBefore(JsVar *arr, JsVar *beforeIndex, JsVar *element); ///< Insert a new element before beforeIndex, DOES NOT UPDATE INDICES
static ALWAYS_INLINE bool jsvArrayIsEmpty(JsVar *arr) { assert(jsvIsArray(arr)); return !jsvGetFirstChild(arr); } ///< Return true is array is empty

/** Write debug info for this Var out to the console */
void jsvTrace(JsVar *var, int indent);

/** Run a garbage collection sweep - return true if things have been freed */
bool jsvGarbageCollect();

/** Remove whitespace to the right of a string - on MULTIPLE LINES */
JsVar *jsvStringTrimRight(JsVar *srcString);

typedef bool (*JsvIsInternalChecker)(JsVar*);

/** If v is the key of a function, return true if it is internal and shouldn't be visible to the user */
bool jsvIsInternalFunctionKey(JsVar *v);

/// If v is the key of an object, return true if it is internal and shouldn't be visible to the user
bool jsvIsInternalObjectKey(JsVar *v);

/// Get the correct checker function for the given variable. see jsvIsInternalFunctionKey/jsvIsInternalObjectKey
JsvIsInternalChecker jsvGetInternalFunctionCheckerFor(JsVar *v);

/// See jsvReadConfigObject
typedef struct {
  char *name;
  JsVarFlags type;
  void *ptr;
} jsvConfigObject;

/** Using 'configs', this reads 'object' into the given pointers, returns true on success.
 *  If object is not undefined and not an object, an error is raised.
 *  If there are fields that are not  in the list of configs, an error is raised
 */
bool jsvReadConfigObject(JsVar *object, jsvConfigObject *configs, int nConfigs);

// Create a new typed array of the given type and length
JsVar *jsvNewTypedArray(JsVarDataArrayBufferViewType type, JsVarInt length);

#endif /* JSVAR_H_ */
