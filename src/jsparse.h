/*
 * jsparse.h
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#ifndef JSPARSE_H_
#define JSPARSE_H_

#include "jsvar.h"
#include "jslex.h"


typedef struct {
  JsVarRef  root;   ///< root of symbol table

  JsVarRef zeroInt;
  JsVarRef oneInt;
  JsVarRef stringClass; ///< Built in string class
  JsVarRef objectClass; ///< Built in object class
  JsVarRef arrayClass; ///< Built in array class
  JsVarRef intClass; ///< Built in integer class
  JsVarRef doubleClass; ///< Built in double class
  JsVarRef mathClass; ///< Built in maths class
  JsVarRef jsonClass; ///< Built in json class
} JsParse;

void jspInit(JsParse *parse);
void jspKill(JsParse *parse);

// jspSoft* - 'release' or 'claim' anything we are using, but ensure that it doesn't get freed
void jspSoftInit(JsParse *parse); ///< used when recovering from or saving to flash
void jspSoftKill(JsParse *parse); ///< used when recovering from or saving to flash
bool jspIsCreatedObject(JsParse *parse, JsVar *v); ///< Is v likely to have been created by this parser?

/// if interrupting execution, this is set
bool jspIsInterrupted();
/// if interrupting execution, this is set
void jspSetInterrupted(bool interrupt);

bool jspAddNativeFunction(JsParse *parse, const char *funcDesc, JsCallback callbackPtr);
JsVar *jspEvaluateVar(JsParse *parse, JsVar *str);
JsVar *jspEvaluate(JsParse *parse, const char *str);
bool jspExecuteFunction(JsParse *parse, JsVar *func);

/** When parsing, this enum defines whether
 we are executing or not */
typedef enum  {
  EXEC_NO = 0,
  EXEC_YES = 1,
  EXEC_INTERRUPTED = 2, // true if execution has been interrupted
  EXEC_ERROR = 4,

  EXEC_RUN_MASK = 3,
  EXEC_ERROR_MASK = EXEC_INTERRUPTED|EXEC_ERROR,
} JsExecFlags;

/** This structure is used when parsing the JavaScript. It contains
 * everything that should be needed. */
typedef struct {
  JsParse *parse;
  JsLex *lex;

  // TODO: could store scopes as JsVar array for speed
  JsVarRef scopes[JSPARSE_MAX_SCOPES];
  int scopeCount;

  JsExecFlags execute;
} JsExecInfo;


bool jspParseVariableName();     ///< parse single variable name
bool jspParseEmptyFunction();    ///< parse function with no arguments
JsVar *jspParseSingleFunction(); ///< parse function with a single argument, return its value (no names!)
bool jspParseDoubleFunction(JsVar **a, JsVar **b); ///< parse function with 2 arguments, return 2 values (no names!)
bool jspParseTripleFunction(JsVar **a, JsVar **b, JsVar **c); ///< parse function with 3 arguments, return 3 values (no names!)

#endif /* JSPARSE_H_ */
