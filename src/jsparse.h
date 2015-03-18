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
 * Recursive descent parser for code execution
 * ----------------------------------------------------------------------------
 */
#ifndef JSPARSE_H_
#define JSPARSE_H_

#include "jsvar.h"
#include "jslex.h"

void jspInit();
void jspKill();

// jspSoft* - 'release' or 'claim' anything we are using, but ensure that it doesn't get freed
void jspSoftInit(); ///< used when recovering from or saving to flash
void jspSoftKill(); ///< used when recovering from or saving to flash
/** Returns true if the constructor function given is the same as that
 * of the object with the given name. */
bool jspIsConstructor(JsVar *constructor, const char *constructorName);

/// Create a new built-in object that jswrapper can use to check for built-in functions
JsVar *jspNewBuiltin(const char *name);

/// Create a new Class of the given instance and return its prototype
NO_INLINE JsVar *jspNewPrototype(const char *instanceOf);

/** Create a new object of the given instance and add it to root with name 'name'.
 * If name!=0, added to root with name, and the name is returned
 * If name==0, not added to root and Object itself returned */
JsVar *jspNewObject(const char *name, const char *instanceOf);

/// if interrupting execution, this is set
bool jspIsInterrupted();
/// if interrupting execution, this is set
void jspSetInterrupted(bool interrupt);
/// Has there been an error during parsing
bool jspHasError();
/// Set the error flag - set lineReported if we've already output the line number
void jspSetError(bool lineReported);
/// We had an exception (argument is the exception's value)
void jspSetException(JsVar *value);
/** Return the reported exception if there was one (and clear it) */
JsVar *jspGetException();
/** Return a stack trace string if there was one (and clear it) */
JsVar *jspGetStackTrace();

/** Execute code form a variable and return the result. If parseTwice is set,
 * we run over the variable twice - once to pick out function declarations,
 * and once to actually execute. */
JsVar *jspEvaluateVar(JsVar *str, JsVar *scope, bool parseTwice);
/** Execute code form a string and return the result. */
JsVar *jspEvaluate(const char *str, bool parseTwice);
JsVar *jspExecuteFunction(JsVar *func, JsVar *thisArg, int argCount, JsVar **argPtr);

/// Evaluate a JavaScript module and return its exports
JsVar *jspEvaluateModule(JsVar *moduleContents);

/** Get the owner of the current prototype. We assume that it's
 * the first item in the array, because that's what we will
 * have added when we created it. It's safe to call this on
 * non-prototypes and non-objects.  */
JsVar *jspGetPrototypeOwner(JsVar *proto);

/** When parsing, this enum defines whether
 we are executing or not */
typedef enum  {
  EXEC_NO = 0,
  EXEC_YES = 1,
  EXEC_BREAK = 2,
  EXEC_CONTINUE = 4,

  EXEC_INTERRUPTED = 8, // true if execution has been interrupted
  EXEC_EXCEPTION = 16, // we had an exception, so don't execute until we hit a try/catch block
  EXEC_ERROR = 32,
  EXEC_ERROR_LINE_REPORTED = 64, // if an error has been reported, set this so we don't do it too much (EXEC_ERROR will STILL be set)

  EXEC_FOR_INIT = 128, // when in for initialiser parsing - hack to avoid getting confused about multiple use for IN
  EXEC_IN_LOOP = 256, // when in a loop, set this - we can then block break/continue outside it
  EXEC_IN_SWITCH = 512, // when in a switch, set this - we can then block break outside it/loops

  /** If Ctrl-C is pressed, the EXEC_CTRL_C flag is set on an interrupt. The next time a SysTick
   * happens, it sets EXEC_CTRL_C_WAIT, and if we get ANOTHER SysTick and it hasn't been handled,
   * we go to a full-on EXEC_INTERRUPTED. That means we only interrupt code if we're actually stuck
   * in something, and otherwise the console just clears the line. */
  EXEC_CTRL_C = 1024, // If Ctrl-C was pressed, set this
  EXEC_CTRL_C_WAIT = 2048, // If Ctrl-C was set and SysTick happens then this is set instead

  /** Parse function declarations, even if we're not executing. This
   * is used when we want to do two passes, to effectively 'hoist' function
   * declarations to the top so they can be called before they're defined.
   * NOTE: This is only needed to call a function before it is defined IF
   * code is being executed as it is being parsed. If it's in a function
   * then you're fine anyway. */
  EXEC_PARSE_FUNCTION_DECL = 4096,

  EXEC_RUN_MASK = EXEC_YES|EXEC_BREAK|EXEC_CONTINUE|EXEC_INTERRUPTED|EXEC_EXCEPTION,
  EXEC_ERROR_MASK = EXEC_INTERRUPTED|EXEC_ERROR|EXEC_EXCEPTION, // here, we have an error, but unless EXEC_NO_PARSE, we should continue parsing but not executing
  EXEC_NO_PARSE_MASK = EXEC_INTERRUPTED|EXEC_ERROR, // in these cases we should exit as fast as possible - skipping out of parsing
  EXEC_SAVE_RESTORE_MASK = EXEC_YES|EXEC_IN_LOOP|EXEC_IN_SWITCH|EXEC_CONTINUE|EXEC_BREAK|EXEC_ERROR_MASK, // the things JSP_SAVE/RESTORE_EXECUTE should keep track of
  EXEC_CTRL_C_MASK = EXEC_CTRL_C | EXEC_CTRL_C_WAIT, // Ctrl-C was pressed at some point
} JsExecFlags;

/** This structure is used when parsing the JavaScript. It contains
 * everything that should be needed. */
typedef struct {
  JsVar  *root;   ///< root of symbol table
  JsVar  *hiddenRoot;   ///< root of the symbol table that's hidden
  JsLex *lex;

  // TODO: could store scopes as JsVar array for speed
  JsVar *scopes[JSPARSE_MAX_SCOPES];
  int scopeCount;
  /// Value of 'this' reserved word
  JsVar *thisVar;

  JsExecFlags execute;
} JsExecInfo;

/* Info about execution when Parsing - this saves passing it on the stack
 * for each call */
extern JsExecInfo execInfo;

/// flags for jspParseFunction
typedef enum {
  JSP_NOSKIP_A = 1,
  JSP_NOSKIP_B = 2,
  JSP_NOSKIP_C = 4,
  JSP_NOSKIP_D = 8,
  JSP_NOSKIP_E = 16,
  JSP_NOSKIP_F = 32,
  JSP_NOSKIP_G = 64,
  JSP_NOSKIP_H = 128,
} JspSkipFlags;

bool jspParseEmptyFunction();    ///< parse function with no arguments

/** Handle a function call (assumes we've parsed the function name and we're
 * on the start bracket). 'thisArg' is the value of the 'this' variable when the
 * function is executed (it's usually the parent object).
 *
 * NOTE: this does not set the execInfo flags - so if execInfo==EXEC_NO, it won't execute
 *
 * If !isParsing and arg0!=0, argument 0 is set to what is supplied (same with arg1)
 *
 * functionName is used only for error reporting - and can be 0
 */
JsVar *jspeFunctionCall(JsVar *function, JsVar *functionName, JsVar *thisArg, bool isParsing, int argCount, JsVar **argPtr);


// Find a variable (or built-in function) based on the current scopes
JsVar *jspGetNamedVariable(const char *tokenName);

/** Get the named function/variable on the object - whether it's built in, or predefined.
 * If !returnName, returns the function/variable itself or undefined, but
 * if returnName, return a name (could be fake) referencing the parent.
 *
 * NOTE: ArrayBuffer/Strings are not handled here. We assume that if we're
 * passing a char* rather than a JsVar it's because we're looking up via
 * a symbol rather than a variable. To handle these use jspGetVarNamedField  */
JsVar *jspGetNamedField(JsVar *object, const char* name, bool returnName);
JsVar *jspGetVarNamedField(JsVar *object, JsVar *nameVar, bool returnName);

/** Call the function named on the given object. For example you might call:
 *
 *  JsVar *str = jspCallNamedFunction(var, "toString", 0, 0);
 */
JsVar *jspCallNamedFunction(JsVar *object, char* name, int argCount, JsVar **argPtr);


// These are exported for the Web IDE's compiler. See exportPtrs in jswrap_process.c
JsVar *jspeiFindInScopes(const char *name);
void jspReplaceWith(JsVar *dst, JsVar *src);

#endif /* JSPARSE_H_ */
