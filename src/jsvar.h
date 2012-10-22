/*
 * jsvar.h
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#ifndef JSVAR_H_
#define JSVAR_H_

#include "jsutils.h"

/* This can be set to something like 'extern inline' to try and get GCC to inline the
functions defined with it, but it may not work. Probably need to actually define them
in the header. */
#ifndef INLINE_FUNC
#define INLINE_FUNC
#endif



typedef void (*JsCallback)(JsVarRef var) 
#ifdef SDCC
__reentrant
#endif
;

typedef union {
    char str[JSVAR_DATA_STRING_LEN]; ///< The contents of this variable if it is a string
    /* NOTE: For str above, we INTENTIONALLY OVERFLOW str (and hence data) in the case of STRING_EXTS
     * to overwrite 3 references in order to grab another 6 bytes worth of string data */
    // TODO do some magic with union/structs in order to make sure we don't intentionally write off the end of arrays
    JsVarInt integer; ///< The contents of this variable if it is an int
    JsVarFloat floating; ///< The contents of this variable if it is a double
    JsCallback callback; ///< Callback for native functions, or 0
} PACKED_FLAGS JsVarData;

typedef struct {
#ifdef LARGE_MEM
  JsVarRef this; ///< The reference of this variable itself (so we can get back)
#endif
  unsigned char locks; ///< When a pointer is obtained, 'locks' is increased
  unsigned short refs; ///< The number of references held to this - used for garbage collection
  JsVarFlags flags; ///< the flags determine the type of the variable - int/double/string/etc.

  JsVarData varData;
  /* NOTE: WE INTENTIONALLY OVERFLOW data in the case of STRING_EXTS
   * to overwrite the following 3 references in order to grab another
   * 6 bytes worth of string data */


  /* For Variable NAMES...
   * For STRING_EXT - extra characters
   * Not used for other stuff
   */
  JsVarRef nextSibling;
  JsVarRef prevSibling;

  /**
   * For OBJECT/ARRAY/FUNCTION - this is the first child
   * For NAMES ONLY - this is a link to the variable it points to
   * For STRING_EXT - extra characters
   */
  JsVarRef firstChild;

  /**
   * For OBJECT/ARRAY/FUNCTION - this is the last child
   * For STRINGS/STRING_EXT/NAME+STRING - this is a link to more string data if it is needed
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

void *jsvGetVarDataPointer();
int jsvGetVarDataSize();

// For debugging/testing ONLY - maximum # of vars we are allowed to use
void jsvSetMaxVarsUsed(int size);

// Init/kill vars as a whole
void jsvInit();
void jsvKill();
void jsvSoftInit(); ///< called when loading from flash
void jsvSoftKill(); ///< called when saving to flash
JsVar *jsvFindOrCreateRoot(); ///< Find or create the ROOT variable item - used mainly if recovering from a saved state.
int jsvGetMemoryUsage(); ///< Get number of memory records (JsVars) used
int jsvGetMemoryTotal(); ///< Get total amount of memory records
bool jsvIsMemoryFull(); ///< Get whether memory is full or not
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
JsVar *jsvMakeIntoVariableName(JsVar *var, JsVar *valueOrZero);

/// Lock this reference and return a pointer - UNSAFE for null refs
JsVar *jsvLock(JsVarRef ref);
/// Lock this pointer and return a pointer - UNSAFE for null pointer
JsVar *jsvLockAgain(JsVar *var);
/// Unlock this variable - this is SAFE for null variables
JsVarRef jsvUnLock(JsVar *var);

/// DO NOT CALL THIS DIRECTLY - this frees an unreffed/locked var
void jsvFreePtr(JsVar *var);

/// Reference - set this variable as used by something
static inline JsVar *jsvRef(JsVar *v) {
  assert(v);
  v->refs++;
  return v;
}

/// Unreference - set this variable as not used by anything
static inline void jsvUnRef(JsVar *var) {
  assert(var && var->refs>0);
  var->refs--;
  // locks are never 0 here, so why bother checking!
  assert(var->locks>0);
}

/// Helper fn, Reference - set this variable as used by something
static inline JsVarRef jsvRefRef(JsVarRef ref) {
  JsVar *v;
  assert(ref);
  v = jsvLock(ref);
  jsvRef(v);
  jsvUnLock(v);
  return ref;
}

/// Helper fn, Unreference - set this variable as not used by anything
static inline JsVarRef jsvUnRefRef(JsVarRef ref) {
  JsVar *v;
  assert(ref);
  v = jsvLock(ref);
  jsvUnRef(v);
  jsvUnLock(v);
  return 0;
}

/// Get a reference from a var - SAFE for null vars
static inline JsVarRef jsvGetRef(JsVar *var) {
    if (!var) return 0;
#ifdef LARGE_MEM
    return var->this;
#else
    return (JsVarRef)(1 + (var - (JsVar*)jsvGetVarDataPointer()));
#endif
}

/** Given a variable, return the basic object name of it */
const char *jsvGetBasicObjectName(JsVar *v);
bool jsvIsBuiltInObject(const char *name);

static inline bool jsvIsInt(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_INTEGER; }
static inline bool jsvIsFloat(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_FLOAT; }
static inline bool jsvIsBoolean(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_BOOLEAN; }
static inline bool jsvIsString(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_STRING; }
static inline bool jsvIsStringExt(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_STRING_EXT; } ///< The extra bits dumped onto the end of a string to store more data
static inline bool jsvIsNumeric(const JsVar *v) { return v && (v->flags&JSV_NUMERICMASK)!=0; }
static inline bool jsvIsFunction(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_FUNCTION; }
static inline bool jsvIsFunctionParameter(const JsVar *v) { return v && (v->flags&JSV_FUNCTION_PARAMETER_MASK) == JSV_FUNCTION_PARAMETER; }
static inline bool jsvIsObject(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_OBJECT; }
static inline bool jsvIsArray(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_ARRAY; }
static inline bool jsvIsNative(const JsVar *v) { return v && (v->flags&JSV_NATIVE)!=0; }
static inline bool jsvIsUndefined(const JsVar *v) { return v==0; }
static inline bool jsvIsNull(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_NULL; }
static inline bool jsvIsBasic(const JsVar *v) { return jsvIsNumeric(v) || jsvIsString(v);} ///< Is this *not* an array/object/etc
static inline bool jsvIsName(const JsVar *v) { return v && (v->flags & JSV_NAME)!=0; } ///< NAMEs are what's used to name a variable (it is not the data itself)
static inline bool jsvHasCharacterData(const JsVar *v) { return jsvIsString(v) || jsvIsStringExt(v) || jsvIsFunctionParameter(v); } // does the v->data union contain character data?
static inline bool jsvHasChildren(const JsVar *v) { return jsvIsFunction(v) || jsvIsObject(v) || jsvIsArray(v); }

/// This is the number of characters a JsVar can contain, NOT string length
static inline size_t jsvGetMaxCharactersInVar(const JsVar *v) {
    // see jsvCopy - we need to know about this in there too
    if (jsvIsStringExt(v)) return JSVAR_DATA_STRING_MAX_LEN;
    assert(jsvHasCharacterData(v));
    return JSVAR_DATA_STRING_LEN;
}

/** Check if two Basic Variables are equal (this IGNORES the value that is pointed to,
 * so 'a=5'=='a=7' but 'a=5'!='b=5')
 */
bool jsvIsBasicVarEqual(JsVar *a, JsVar *b);

/** Check if two things are equal. Basic vars are done by value,
 * for anything else the reference/pointer must be equal */
bool jsvIsEqual(JsVar *a, JsVar *b);


const char *jsvGetConstString(JsVar *v); ///< Get a const string representing this variable - if we can. Otherwise return 0
void jsvGetString(JsVar *v, char *str, size_t len); ///< Save this var as a string to the given buffer
JsVar *jsvAsString(JsVar *var, bool unlockVar); ///< If var is a string, lock and return it, else create a new string
size_t jsvGetStringLength(JsVar *v); ///< Get the length of this string, IF it is a string
int jsvGetLinesInString(JsVar *v); ///<  IN A STRING get the number of lines in the string (min=1)
int jsvGetCharsOnLine(JsVar *v, int line); ///<  IN A STRING Get the number of characters on a line - lines start at 1
void jsvGetLineAndCol(JsVar *v, int charIdx, int* line, int *col); ///< IN A STRING, get the line and column of the given character. Both values must be non-null
int jsvGetIndexFromLineAndCol(JsVar *v, int line, int col); ///<  IN A STRING, get a character index from a line and column
bool jsvIsStringEqual(JsVar *var, const char *str);
int jsvCompareString(JsVar *va, JsVar *vb, int starta, int startb, bool equalAtEndOfString); ///< Compare 2 strings, starting from the given character positions
int jsvCompareInteger(JsVar *va, JsVar *vb); ///< Compare 2 integers, >0 if va>vb,  <0 if va<vb. If compared with a non-integer, that gets put later
void jsvAppendString(JsVar *var, const char *str); ///< Append the given string to this one
void jsvAppendInteger(JsVar *var, JsVarInt i); ///< Append the given integer to this string as a decimal
void jsvAppendCharacter(JsVar *var, char ch); ///< Append the given character to this string
#define JSVAPPENDSTRINGVAR_MAXLENGTH (0x7FFFFFFF)
void jsvAppendStringVar(JsVar *var, JsVar *str, int stridx, int maxLength); ///< Append str to var. Both must be strings. stridx = start char or str, maxLength = max number of characters (can be JSVAPPENDSTRINGVAR_MAXLENGTH). stridx can be negative to go from end of string
void jsvAppendStringVarComplete(JsVar *var, JsVar *str); ///< Append all of str to var. Both must be strings.
char jsvGetCharInString(JsVar *v, int idx);

/// Print the contents of a string var - directly
void jsiConsolePrintStringVar(JsVar *v);

INLINE_FUNC JsVarInt jsvGetInteger(const JsVar *v);
INLINE_FUNC void jsvSetInteger(JsVar *v, JsVarInt value); ///< Set an integer value (use carefully!)
INLINE_FUNC JsVarFloat jsvGetFloat(const JsVar *v); // TODO: rename to jsvGetFloat
INLINE_FUNC bool jsvGetBool(const JsVar *v);
static inline JsVarInt jsvGetIntegerAndUnLock(JsVar *v) { JsVarInt i = jsvGetInteger(v); jsvUnLock(v); return i; }
static inline JsVarFloat jsvGetFloatAndUnLock(JsVar *v) { JsVarFloat f = jsvGetFloat(v); jsvUnLock(v); return f; }
static inline bool jsvGetBoolAndUnLock(JsVar *v) { bool b = jsvGetBool(v); jsvUnLock(v); return b; }


/** If a is a name skip it and go to what it points to - and so on.
 * ALWAYS locks - so must unlock what it returns. It MAY
 * return 0.  */
static inline JsVar *jsvSkipName(JsVar *a) {
  JsVar *pa = a;
  if (!a) return 0;
  while (jsvIsName(pa)) {
    JsVarRef n = pa->firstChild;
    if (pa!=a) jsvUnLock(pa);
    if (!n) return 0;
    pa = jsvLock(n);
  }
  if (pa==a) jsvLockAgain(pa);
  return pa;
}

/** If a is a name skip it and go to what it points to.
 * ALWAYS locks - so must unlock what it returns. It MAY
 * return 0.  */
static inline JsVar *jsvSkipOneName(JsVar *a) {
  JsVar *pa = a;
  if (!a) return 0;
  if (jsvIsName(pa)) {
    JsVarRef n = pa->firstChild;
    if (pa!=a) jsvUnLock(pa);
    if (!n) return 0;
    pa = jsvLock(n);
  }
  if (pa==a) jsvLockAgain(pa);
  return pa;
}

/** Same as jsvSkipName, but ensures that 'a' is unlocked if it was
 * a name, so it can be used INLINE_FUNC */
static inline JsVar *jsvSkipNameAndUnLock(JsVar *a) {
  JsVar *b = jsvSkipName(a);
  jsvUnLock(a);
  return b;
}

/** Same as jsvSkipOneName, but ensures that 'a' is unlocked if it was
 * a name, so it can be used INLINE_FUNC */
static inline JsVar *jsvSkipOneNameAndUnlock(JsVar *a) {
  JsVar *b = jsvSkipOneName(a);
  jsvUnLock(a);
  return b;
}


INLINE_FUNC JsVarInt jsvGetIntegerSkipName(JsVar *v);
INLINE_FUNC bool jsvGetBoolSkipName(JsVar *v);

/// MATHS!
JsVar *jsvMathsOpSkipNames(JsVar *a, JsVar *b, int op);
JsVar *jsvMathsOp(JsVar *a, JsVar *b, int op);

/// Copy this variable and return the locked copy
JsVar *jsvCopy(JsVar *src);
/** Copy only a name, not what it points to. ALTHOUGH the link to what it points to is maintained unless linkChildren=false.
    If keepAsName==false, this will be converted into a normal variable */
JsVar *jsvCopyNameOnly(JsVar *src, bool linkChildren, bool keepAsName);
/// Tree related stuff
void jsvAddName(JsVar *parent, JsVar *nameChild); // Add a child, which is itself a name
JsVar *jsvAddNamedChild(JsVar *parent, JsVar *child, const char *name); // Add a child, and create a name for it. Returns a LOCKED var. DOES NOT CHECK FOR DUPLICATES
JsVar *jsvSetNamedChild(JsVar *parent, JsVar *child, const char *name); // Add a child, and create a name for it. CHECKS FOR DUPLICATES
JsVar *jsvSetValueOfName(JsVar *name, JsVar *src); // Set the value of a child created with jsvAddName,jsvAddNamedChild
JsVar *jsvFindChildFromString(JsVarRef parentref, const char *name, bool createIfNotFound); // Non-recursive finding of child with name. Returns a LOCKED var
JsVar *jsvFindChildFromVar(JsVarRef parentref, JsVar *childName, bool addIfNotFound); // Non-recursive finding of child with name. Returns a LOCKED var
void jsvRemoveChild(JsVar *parent, JsVar *child);
void jsvRemoveAllChildren(JsVar *parent);

int jsvGetChildren(JsVar *v);
JsVarInt jsvGetArrayLength(JsVar *arr); ///< Not the same as GetChildren, as it can be a sparse array
JsVar *jsvGetArrayItem(JsVar *arr, int index); ///< Get an item at the specified index in the array (and lock it)
JsVar *jsvGetArrayIndexOf(JsVar *arr, JsVar *value, bool matchExact); ///< Get the index of the value in the array (matchExact==use pointer, not equality check)
JsVarInt jsvArrayPush(JsVar *arr, JsVar *value); ///< Adds new elements to the end of an array, and returns the new length
JsVar *jsvArrayPop(JsVar *arr); ///< Removes the last element of an array, and returns that element (or 0 if empty) includes the NAME
JsVar *jsvArrayPopFirst(JsVar *arr); ///< Removes the first element of an array, and returns that element (or 0 if empty) includes the NAME
JsVar *jsvArrayGetLast(JsVar *arr); ///< Get the last element of an array (does not remove, unlike jsvArrayPop), and returns that element (or 0 if empty) includes the NAME
JsVar *jsvArrayJoin(JsVar *arr, JsVar *filler); ///< Join all elements of an array together into a string

/** Write debug info for this Var out to the console */
void jsvTrace(JsVarRef ref, int indent);

/** Run a garbage collection sweep - return true if things have been freed */
bool jsvGarbageCollect();

/** Dotty output for the graphviz package - helps
 *  visualize the data structure  */
void jsvDottyOutput();

#endif /* JSVAR_H_ */
