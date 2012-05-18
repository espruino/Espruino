/*
 * jsvar.h
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#ifndef JSVAR_H_
#define JSVAR_H_

#include "jsutils.h"

#ifndef INLINE_FUNC
#define INLINE_FUNC
#endif



typedef void (*JsCallback)(JsVarRef var) 
#ifdef SDCC
__reentrant
#endif
;

typedef union {
    char str[JSVAR_STRING_LEN]; ///< The contents of this variable if it is a string
    JsVarInt integer; ///< The contents of this variable if it is an int
    JsVarFloat floating; ///< The contents of this variable if it is a double
    JsCallback callback; ///< Callback for native functions, or 0
} PACKED_FLAGS JsVarData;

typedef struct {
  JsVarRef this; ///< The reference of this variable itself (so we can get back)
  unsigned char locks; ///< When a pointer is obtained, 'locks' is increased
  unsigned short refs; ///< The number of references held to this - used for garbage collection
  JsVarFlags flags; ///< the flags determine the type of the variable - int/double/string/etc.

  JsVarData varData;
  // For Variable NAMES ONLY (could this be part of the union?)
  JsVarRef nextSibling;
  JsVarRef prevSibling;

  /**
   * For OBJECT/ARRAY/FUNCTION - this is the first child
   * For NAMES ONLY - this is a link to the variable it points to
   */
  JsVarRef firstChild;

  /**
   * For OBJECT/ARRAY/FUNCTION - this is the last child
   * For STRINGS/NAMES - this is a link to more string data if it is needed
   */
  JsVarRef lastChild;
} PACKED_FLAGS JsVar;


/* We have a few different types:
 *
 *  OBJECT/ARRAY - uses firstChild/lastChild to link to NAMEs
 *  FUNCTION - uses firstChild/lastChild to link to NAMEs, and callback is used
 *  NAME - use nextSibling/prevSibling linking to other NAMEs, and firstChild to link to a Variable of some kind
 *  STRING - use firstChild to link to other STRINGs if String value is too long
 *  INT/DOUBLE - firstChild never used
 */

// Init/kill vars as a whole
void jsvInit();
void jsvKill();
int jsvGetMemoryUsage(); ///< Get number of memory records (JsVars) used
void jsvShowAllocated(); ///< Show what is still allocated, for debugging memory problems

// Note that jsvNew* don't REF a variable for you, but the do LOCK it
JsVar *jsvNew(); ///< Create a new variable
JsVar *jsvNewFromString(const char *str); ///< Create a new string
JsVar *jsvNewFromLexer(struct JsLex *lex, int charFrom, int charTo); // Create a new STRING from part of the lexer
JsVar *jsvNewWithFlags(JsVarFlags flags);
JsVar *jsvNewFromInteger(JsVarInt value);
JsVar *jsvNewFromBool(bool value);
JsVar *jsvNewFromFloat(JsVarFloat value);
// Turns var into a Variable name that links to the given value... No locking so no need to unlock var
JsVar *jsvMakeIntoVariableName(JsVar *var, JsVarRef valueOrZero);

INLINE_FUNC JsVar *jsvLock(JsVarRef ref); ///< Lock this reference and return a pointer
INLINE_FUNC JsVarRef jsvUnLock(JsVar *var); ///< Unlock this variable - this is SAFE for null variables

INLINE_FUNC JsVar *jsvRef(JsVar *v); ///< Reference - set this variable as used by something
INLINE_FUNC void jsvUnRef(JsVar *v); ///< Unreference - set this variable as not used by anything
INLINE_FUNC JsVarRef jsvRefRef(JsVarRef ref); ///< Helper fn, Reference - set this variable as used by something
INLINE_FUNC JsVarRef jsvUnRefRef(JsVarRef ref); ///< Helper fn, Unreference - set this variable as not used by anything
INLINE_FUNC JsVarRef jsvGetRef(JsVar *var); ///< Get a reference from a var - SAFE for null vars

INLINE_FUNC bool jsvIsInt(JsVar *v);
INLINE_FUNC bool jsvIsDouble(JsVar *v);
INLINE_FUNC bool jsvIsString(JsVar *v);
INLINE_FUNC bool jsvIsStringExt(JsVar *v); ///< The extra bits dumped onto the end of a string to store more data
INLINE_FUNC bool jsvIsNumeric(JsVar *v);
INLINE_FUNC bool jsvIsFunction(JsVar *v);
INLINE_FUNC bool jsvIsFunctionParameter(JsVar *v);
INLINE_FUNC bool jsvIsObject(JsVar *v);
INLINE_FUNC bool jsvIsArray(JsVar *v);
INLINE_FUNC bool jsvIsNative(JsVar *v);
INLINE_FUNC bool jsvIsUndefined(JsVar *v);
INLINE_FUNC bool jsvIsNull(JsVar *v);
INLINE_FUNC bool jsvIsBasic(JsVar *v);
INLINE_FUNC bool jsvIsName(JsVar *v); ///< NAMEs are what's used to name a variable (it is not the data itself)

/** Check if two Basic Variables are equal (this IGNORES the value that is pointed to,
 * so 'a=5'=='a=7' but 'a=5'!='b=5')
 */
bool jsvIsBasicVarEqual(JsVar *a, JsVar *b);

/// Save this var as a string to the given buffer
void jsvGetString(JsVar *v, char *str, size_t len);
int jsvGetStringLength(JsVar *v); ///< Get the length of this string, IF it is a string
bool jsvIsStringEqual(JsVar *var, const char *str);
void jsvAppendString(JsVar *var, const char *str); ///< Append the given string to this one

INLINE_FUNC JsVarInt jsvGetInteger(JsVar *v);
INLINE_FUNC JsVarFloat jsvGetDouble(JsVar *v); // TODO: rename to jsvGetFloat
INLINE_FUNC bool jsvGetBool(JsVar *v);

/** If a is a name skip it and go to what it points to.
 * ALWAYS locks - so must unlock what it returns. */
INLINE_FUNC JsVar *jsvSkipName(JsVar *a);
/** Same as jsvSkipName, but ensures that 'a' is unlocked if it was
 * a name, so it can be used INLINE_FUNC */
INLINE_FUNC JsVar *jsvSkipNameAndUnlock(JsVar *a);
INLINE_FUNC JsVarInt jsvGetIntegerSkipName(JsVar *v);
INLINE_FUNC bool jsvGetBoolSkipName(JsVar *v);

/// MATHS!
JsVar *jsvMathsOp(JsVarRef ar, JsVarRef br, int op);
JsVar *jsvMathsOpPtrSkipNames(JsVar *a, JsVar *b, int op);
JsVar *jsvMathsOpPtr(JsVar *a, JsVar *b, int op);

/// Copy this variable and return the locked copy
JsVar *jsvCopy(JsVar *src);
/// Tree related stuff
void jsvAddName(JsVarRef parent, JsVarRef nameChild); // Add a child, which is itself a name
JsVar *jsvAddNamedChild(JsVarRef parent, JsVarRef child, const char *name); // Add a child, and create a name for it. Returns a LOCKED var
JsVar *jsvSetValueOfName(JsVar *name, JsVar *src); // Set the value of a child created with jsvAddName,jsvAddNamedChild
JsVar *jsvFindChildFromString(JsVarRef parentref, const char *name, bool createIfNotFound); // Non-recursive finding of child with name. Returns a LOCKED var
JsVar *jsvFindChildFromVar(JsVarRef parentref, JsVar *childName, bool addIfNotFound); // Non-recursive finding of child with name. Returns a LOCKED var

int jsvGetChildren(JsVar *v);
JsVarInt jsvGetArrayLength(JsVar *v); ///< Not the same as GetChildren, as it can be a sparse array
JsVar *jsvGetArrayItem(JsVar *arr, int index); ///< Get an item at the specified index in the array (and lock it)
JsVar *jsvGetArrayIndexOf(JsVar *arr, JsVar *value); ///< Get the index of the value in the array

/** Write debug info for this Var out to the console */
void jsvTrace(JsVarRef ref, int indent);

#endif /* JSVAR_H_ */
