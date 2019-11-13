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
/** Get the prototype of the given object, or return 0 if not found, or not an object */
JsVar *jspGetPrototype(JsVar *object);
/** Get the constructor of the given object, or return 0 if ot found, or not a function */
JsVar *jspGetConstructor(JsVar *object);

/// Check that we have enough stack to recurse. Return true if all ok, error if not.
bool jspCheckStackPosition();

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

/** Evaluate the given variable as an expression (in current scope) */
JsVar *jspEvaluateExpressionVar(JsVar *str);
/** Execute code form a variable and return the result. If lineNumberOffset
 * is nonzero it's added to the line numbers that get reported for errors/debug */
JsVar *jspEvaluateVar(JsVar *str, JsVar *scope, uint16_t lineNumberOffset);
/** Execute code form a string and return the result.
 * You should only set stringIsStatic if the string will hang around for
 * the life of the interpreter, as then the interpreter will use a pointer
 * to this data, which could hang around inside the code. */
JsVar *jspEvaluate(const char *str, bool stringIsStatic);
/// Execute a JS function with the given arguments. usage: jspExecuteJSFunction("(function() { print('hi'); })",0,0,0)
JsVar *jspExecuteJSFunction(const char *jsCode, JsVar *thisArg, int argCount, JsVar **argPtr);
/// Execute a function with the given arguments
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
  EXEC_BREAK = 2,     ///< Have we had a 'break' keyword (so should skip to end of loop and exit)
  EXEC_CONTINUE = 4,  ///< Have we had a 'continue' keywrord (so should skip to end of loop and restart)
  EXEC_RETURN = 8,    ///< Have we had a 'return' keyword (so should skip to end of the function)

  EXEC_INTERRUPTED = 16, ///< true if execution has been interrupted
  EXEC_EXCEPTION = 32, ///< we had an exception, so don't execute until we hit a try/catch block
  EXEC_ERROR = 64,
  EXEC_ERROR_LINE_REPORTED = 128, ///< if an error has been reported, set this so we don't do it too much (EXEC_ERROR will STILL be set)

  EXEC_FOR_INIT = 256, ///< when in for initialiser parsing - hack to avoid getting confused about multiple use for IN
  EXEC_IN_LOOP = 512, ///< when in a loop, set this - we can then block break/continue outside it
  EXEC_IN_SWITCH = 1024, ///< when in a switch, set this - we can then block break outside it/loops

  /** If Ctrl-C is pressed, the EXEC_CTRL_C flag is set on an interrupt. The next time a SysTick
   * happens, it sets EXEC_CTRL_C_WAIT, and if we get ANOTHER SysTick and it hasn't been handled,
   * we go to a full-on EXEC_INTERRUPTED. That means we only interrupt code if we're actually stuck
   * in something, and otherwise the console just clears the line. */
  EXEC_CTRL_C = 2048, ///< If Ctrl-C was pressed, set this
  EXEC_CTRL_C_WAIT = 4096, ///< If Ctrl-C was set and SysTick happens then this is set instead

#ifdef USE_DEBUGGER
  /** When the lexer hits a newline character, it'll then drop right
   * into the debugger */
  EXEC_DEBUGGER_NEXT_LINE = 8192,
  /** Break when we execute a function */
  EXEC_DEBUGGER_STEP_INTO = 16384,
  /** Break when a function finishes execution */
  EXEC_DEBUGGER_FINISH_FUNCTION = 32768,
  EXEC_DEBUGGER_MASK = EXEC_DEBUGGER_NEXT_LINE | EXEC_DEBUGGER_STEP_INTO | EXEC_DEBUGGER_FINISH_FUNCTION,
#endif

  EXEC_RUN_MASK = EXEC_YES|EXEC_BREAK|EXEC_CONTINUE|EXEC_RETURN|EXEC_INTERRUPTED|EXEC_EXCEPTION,
  EXEC_ERROR_MASK = EXEC_INTERRUPTED|EXEC_ERROR|EXEC_EXCEPTION, ///< here, we have an error, but unless EXEC_NO_PARSE, we should continue parsing but not executing
  EXEC_NO_PARSE_MASK = EXEC_INTERRUPTED|EXEC_ERROR, ///< in these cases we should exit as fast as possible - skipping out of parsing
  EXEC_SAVE_RESTORE_MASK = EXEC_YES|EXEC_BREAK|EXEC_CONTINUE|EXEC_RETURN|EXEC_IN_LOOP|EXEC_IN_SWITCH|EXEC_ERROR_MASK, ///< the things JSP_SAVE/RESTORE_EXECUTE should keep track of
  EXEC_CTRL_C_MASK = EXEC_CTRL_C | EXEC_CTRL_C_WAIT, ///< Ctrl-C was pressed at some point
  EXEC_PERSIST = EXEC_ERROR_MASK|EXEC_CTRL_C_MASK, ///< Things we should keep track of even after executing
} JsExecFlags;

/** This structure is used when parsing the JavaScript. It contains
 * everything that should be needed. */
typedef struct {
  JsVar  *root;       //!< root of symbol table
  JsVar  *hiddenRoot; //!< root of the symbol table that's hidden

  /// JsVar array of scopes
  JsVar *scopesVar;
  /// Value of 'this' reserved word
  JsVar *thisVar;

  volatile JsExecFlags execute;
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

/** Parse using current lexer until we hit the end of
 * input or there was some problem. */
JsVar *jspParse();

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

/// Return the topmost scope (and lock it)
JsVar *jspeiGetTopScope();

#endif /* JSPARSE_H_ */
