/*
 * jsvar.h
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#ifndef JSVAR_H_
#define JSVAR_H_

#include "jsutils.h"

/// Reference for variables
typedef unsigned int JsVarRef;
// We treat 0 as null

typedef long JsVarInt;
typedef double JsVarFloat;

typedef void (*JsCallback)(JsVarRef var);

typedef struct {
  JsVarRef this; ///< The reference of this variable itself (so we can get back)
  unsigned char locks; ///< When a pointer is obtained, 'locks' is increased
  int refs; ///< The number of references held to this - used for garbage collection

  char strData[JSVAR_STRING_LEN]; ///< The contents of this variable if it is a string
  JsVarInt intData; ///< The contents of this variable if it is an int
  JsVarFloat doubleData; ///< The contents of this variable if it is a double
  int flags; ///< the flags determine the type of the variable - int/double/string/etc

  JsCallback callback; ///< Callback for native functions, or 0

  JsVarRef firstChild; /// For Variable DATA + NAMES
  JsVarRef lastChild; /// For Variable DATA ONLY
  // For Variable NAMES ONLY
  JsVarRef nextSibling;
  JsVarRef prevSibling;

} JsVar;

// Init/kill vars as a whole
void jsvInit();
void jsvKill();
int jsvGetMemoryUsage(); ///< Get number of memory records (JsVars) used
void jsvShowAllocated(); ///< Show what is still allocated, for debugging memory problems

// Note that jsvNew* don't REF a variable for you, but the do LOCK it
JsVar *jsvNew(); ///< Create a new variable
JsVar *jsvNewFromString(const char *str); ///< Create a new string
JsVar *jsvNewFromLexer(struct JsLex *lex, int charFrom, int charTo); // Create a new STRING from part of the lexer
JsVar *jsvNewWithFlags(SCRIPTVAR_FLAGS flags);
JsVar *jsvNewFromInteger(JsVarInt value);
JsVar *jsvNewFromBool(bool value);
JsVar *jsvNewFromFloat(JsVarFloat value);
// Creates a new Variable name that links to the given variable...
JsVar *jsvNewVariableName(JsVarRef variable, const char *name); // variable can be 0
JsVar *jsvNewVariableNameFromLexerToken(JsVarRef variable, struct JsLex *lex); // Create a new Variable name from the lexer's last token. variable can be 0

JsVar *jsvLock(JsVarRef ref); ///< Lock this reference and return a pointer
void jsvUnLock(JsVarRef ref); ///< Unlock this reference
JsVar *jsvLockPtr(JsVar *var); ///< Lock this variable again (utility fn for jsvUnLock)
JsVarRef jsvUnLockPtr(JsVar *var); ///< Unlock this variable (utility fn for jsvUnLock)

JsVar *jsvRef(JsVar *v); ///< Reference - set this variable as used by something
void jsvUnRef(JsVar *v); ///< Unreference - set this variable as not used by anything
JsVarRef jsvRefRef(JsVarRef ref); ///< Helper fn, Reference - set this variable as used by something
JsVarRef jsvUnRefRef(JsVarRef ref); ///< Helper fn, Unreference - set this variable as not used by anything

bool jsvIsInt(JsVar *v);
bool jsvIsDouble(JsVar *v);
bool jsvIsString(JsVar *v);
bool jsvIsNumeric(JsVar *v);
bool jsvIsFunction(JsVar *v);
bool jsvIsFunctionParameter(JsVar *v);
bool jsvIsObject(JsVar *v);
bool jsvIsArray(JsVar *v);
bool jsvIsNative(JsVar *v);
bool jsvIsUndefined(JsVar *v);
bool jsvIsNull(JsVar *v);
bool jsvIsBasic(JsVar *v);
bool jsvIsName(JsVar *v); ///< NAMEs are what's used to name a variable (it is not the data itself)

/// Save this var as a string to the given buffer
void jsvGetString(JsVar *v, char *str, size_t len);
int jsvGetStringLength(JsVar *v); // Get the length of this string, IF it is a string
bool jsvIsStringEqual(JsVar *var, const char *str);

JsVarInt jsvGetInteger(JsVar *v);
JsVarFloat jsvGetDouble(JsVar *v);
bool jsvGetBool(JsVar *v);

/** If a is a name skip it and go to what it points to.
 * ALWAYS locks - so must unlock what it returns. */
JsVar *jsvSkipName(JsVar *a);
/** Same as jsvSkipName, but ensures that 'a' is unlocked if it was
 * a name, so it can be used inline */
JsVar *jsvSkipNameAndUnlock(JsVar *a);
JsVarInt jsvGetIntegerSkipName(JsVar *v);
bool jsvGetBoolSkipName(JsVar *v);

/// MATHS!
JsVar *jsvMathsOp(JsVarRef ar, JsVarRef br, int op);
JsVar *jsvMathsOpPtrSkipNames(JsVar *a, JsVar *b, int op);
JsVar *jsvMathsOpPtr(JsVar *a, JsVar *b, int op);

/// Tree related stuff
void jsvAddName(JsVarRef parent, JsVarRef nameChild); // Add a child, which is itself a name
JsVar *jsvAddNamedChild(JsVarRef parent, JsVarRef child, const char *name); // Add a child, and create a name for it. Returns a LOCKED var
JsVar *jsvSetValueOfName(JsVar *name, JsVar *src); // Set the value of a child created with jsvAddName,jsvAddNamedChild
JsVar *jsvFindChild(JsVarRef parentref, const char *name, bool createIfNotFound); // Non-recursive finding of child with name. Returns a LOCKED var

int jsvGetChildren(JsVar *v);
int jsvGetArrayLength(JsVar *v);

/** Write debug info for this Var out to the console */
void jsvTrace(JsVarRef ref, int indent);

#endif /* JSVAR_H_ */
