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

typedef struct {
  JsVarRef this; ///< The reference of this variable itself (so we can get back)
  unsigned char locks; ///< When a pointer is obtained, 'locks' is increased
  int refs; ///< The number of references held to this - used for garbage collection

  char strData[JSVAR_STRING_LEN]; ///< The contents of this variable if it is a string
  JsVarInt intData; ///< The contents of this variable if it is an int
  JsVarFloat doubleData; ///< The contents of this variable if it is a double
  int flags; ///< the flags determine the type of the variable - int/double/string/etc

  int callback; ///< Callback for native functions or 0

  JsVarRef firstChild; /// For Variable DATA + NAMES
  JsVarRef lastChild; /// For Variable DATA ONLY
  // For Variable NAMES ONLY
  JsVarRef nextSibling;
  JsVarRef prevSibling;

} JsVar;

// Init/kill vars as a whole
void jsvInit();
void jsvKill();

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
JsVarInt jsvGetIntegerSkipName(JsVar *v);
bool jsvGetBoolSkipName(JsVar *v);

/// MATHS!
JsVar *jsvMathsOp(JsVarRef ar, JsVarRef br, int op);
JsVar *jsvMathsOpPtrSkipNames(JsVar *a, JsVar *b, int op);
JsVar *jsvMathsOpPtr(JsVar *a, JsVar *b, int op);

/// Tree related stuff
void jsvAddName(JsVarRef parent, JsVarRef nameChild); // Add a child, which is itself a name
JsVar *jsvAddNamedChild(JsVarRef parent, JsVarRef child, const char *name); // Add a child, and create a name for it. Returns a LOCKED var
JsVar *jsvFindChild(JsVarRef parentref, const char *name, bool createIfNotFound); // Non-recursive finding of child with name. Returns a LOCKED var

int jsvGetChildren(JsVar *v);
int jsvGetArrayLength(JsVar *v);

/** Write debug info for this Var out to the console */
void jsvTrace(JsVarRef ref, int indent);

/*class CScriptVarLink
{
public:
  std::string name;
  CScriptVarLink *nextSibling;
  CScriptVarLink *prevSibling;
  CScriptVar *var;
  bool owned;

  CScriptVarLink(CScriptVar *var, const std::string &name = TINYJS_TEMP_NAME);
  CScriptVarLink(const CScriptVarLink &link); ///< Copy constructor
  ~CScriptVarLink();
  void replaceWith(CScriptVar *newVar); ///< Replace the Variable pointed to
  void replaceWith(CScriptVarLink *newVar); ///< Replace the Variable pointed to (just dereferences)
  int getIntName(); ///< Get the name as an integer (for arrays)
  void setIntName(int n); ///< Set the name as an integer (for arrays)
};

/// Variable class (containing a doubly-linked list of children)
class CScriptVar
{
public:
    CScriptVar(); ///< Create undefined
    CScriptVar(const std::string &varData, int varFlags); ///< User defined
    CScriptVar(const std::string &str); ///< Create a string
    CScriptVar(double varData);
    CScriptVar(int val);
    ~CScriptVar(void);

    CScriptVar *getReturnVar(); ///< If this is a function, get the result value (for use by native functions)
    void setReturnVar(CScriptVar *var); ///< Set the result value. Use this when setting complex return data as it avoids a deepCopy()
    CScriptVar *getParameter(const std::string &name); ///< If this is a function, get the parameter with the given name (for use by native functions)

    CScriptVarLink *findChild(const std::string &childName); ///< Tries to find a child with the given name, may return 0
    CScriptVarLink *findChildOrCreate(const std::string &childName, int varFlags=SCRIPTVAR_UNDEFINED); ///< Tries to find a child with the given name, or will create it with the given flags
    CScriptVarLink *findChildOrCreateByPath(const std::string &path); ///< Tries to find a child with the given path (separated by dots)
    CScriptVarLink *addChild(const std::string &childName, CScriptVar *child=NULL);
    CScriptVarLink *addChildNoDup(const std::string &childName, CScriptVar *child=NULL); ///< add a child overwriting any with the same name
    void removeChild(CScriptVar *child);
    void removeLink(CScriptVarLink *link); ///< Remove a specific link (this is faster than finding via a child)
    void removeAllChildren();
    CScriptVar *getArrayIndex(int idx); ///< The the value at an array index
    void setArrayIndex(int idx, CScriptVar *value); ///< Set the value at an array index
    int getArrayLength(); ///< If this is an array, return the number of items in it (else 0)
    int getChildren(); ///< Get the number of children

    int getInt();
    bool getBool() { return getInt() != 0; }
    double getDouble();
    const std::string &getString();
    std::string getParsableString(); ///< get Data as a parsable javascript string
    void setInt(int num);
    void setDouble(double val);
    void setString(const std::string &str);
    void setUndefined();
    void setArray();
    bool equals(CScriptVar *v);

    bool isInt() { return (flags&SCRIPTVAR_INTEGER)!=0; }
    bool isDouble() { return (flags&SCRIPTVAR_DOUBLE)!=0; }
    bool isString() { return (flags&SCRIPTVAR_STRING)!=0; }
    bool isNumeric() { return (flags&SCRIPTVAR_NUMERICMASK)!=0; }
    bool isFunction() { return (flags&SCRIPTVAR_FUNCTION)!=0; }
    bool isObject() { return (flags&SCRIPTVAR_OBJECT)!=0; }
    bool isArray() { return (flags&SCRIPTVAR_ARRAY)!=0; }
    bool isNative() { return (flags&SCRIPTVAR_NATIVE)!=0; }
    bool isUndefined() { return (flags & SCRIPTVAR_VARTYPEMASK) == SCRIPTVAR_UNDEFINED; }
    bool isNull() { return (flags & SCRIPTVAR_NULL)!=0; }
    bool isBasic() { return firstChild==0; } ///< Is this *not* an array/object/etc

    CScriptVar *mathsOp(CScriptVar *b, int op); ///< do a maths op with another script variable
    void copyValue(CScriptVar *val); ///< copy the value from the value given
    CScriptVar *deepCopy(); ///< deep copy this node and return the result

    void trace(std::string indentStr = "", const std::string &name = ""); ///< Dump out the contents of this using trace
    std::string getFlagsAsString(); ///< For debugging - just dump a string version of the flags
    void getJSON(std::ostringstream &destination, const std::string linePrefix=""); ///< Write out all the JS code needed to recreate this script variable to the stream (as JSON)
    void setCallback(JSCallback callback, void *userdata); ///< Set the callback for native functions

    CScriptVarLink *firstChild;
    CScriptVarLink *lastChild;

    /// For memory management/garbage collection
    CScriptVar *ref(); ///< Add reference to this variable
    void unref(); ///< Remove a reference, and delete this variable if required
    int getRefs(); ///< Get the number of references to this script variable
protected:
    int refs; ///< The number of references held to this - used for garbage collection

    std::string data; ///< The contents of this variable if it is a string
    long intData; ///< The contents of this variable if it is an int
    double doubleData; ///< The contents of this variable if it is a double
    int flags; ///< the flags determine the type of the variable - int/double/string/etc
    JSCallback jsCallback; ///< Callback for native functions
    void *jsCallbackUserData; ///< user data passed as second argument to native functions

    void init(); ///< initialisation of data members

    // Copy the basic data and flags from the variable given, with no
    / children. Should be used internally only - by copyValue and deepCopy
    void copySimpleData(CScriptVar *val);

    friend class CTinyJS;
};
*/

#endif /* JSVAR_H_ */
