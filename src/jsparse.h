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
bool jspExecuteFunction(JsParse *parse, JsVar *func, JsVar *arg0);

/** When parsing, this enum defines whether
 we are executing or not */
typedef enum  {
  EXEC_NO = 0,
  EXEC_YES = 1,
  EXEC_BREAK = 2,
  EXEC_CONTINUE = 4,

  EXEC_INTERRUPTED = 8, // true if execution has been interrupted

  EXEC_ERROR = 16,
  EXEC_FOR_INIT = 32, // when in for initialiser parsing - hack to avoid getting confused about multiple use for IN
  EXEC_IN_LOOP = 64, // when in a loop, set this - we can then block break/continue outside it
  EXEC_IN_SWITCH = 128, // when in a switch, set this - we can then block break outside it/loops

  EXEC_RUN_MASK = EXEC_YES|EXEC_BREAK|EXEC_CONTINUE|EXEC_INTERRUPTED,
  EXEC_ERROR_MASK = EXEC_INTERRUPTED|EXEC_ERROR,
  EXEC_SAVE_RESTORE_MASK = EXEC_YES|EXEC_IN_LOOP|EXEC_IN_SWITCH, // the things JSP_SAVE/RESTORE_EXECUTE should keep track of
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

/// flags for jspParseFunction
typedef enum {
  JSP_NOSKIP_A = 1,
  JSP_NOSKIP_B = 2,
  JSP_NOSKIP_C = 4,
  JSP_NOSKIP_D = 4,
} JspSkipFlags;

/// parse function with max 4 arguments (can set arg to 0 to avoid parse). Usually first arg will be 0, but if we DON'T want to skip names on an arg stuff, we can say
bool jspParseFunction(JspSkipFlags skipName, JsVar **a, JsVar **b, JsVar **c, JsVar **d);

bool jspParseVariableName();     ///< parse single variable name
bool jspParseEmptyFunction();    ///< parse function with no arguments
JsVar *jspParseSingleFunction(); ///< parse function with a single argument, return its value (no names!)

/** Handle a function call (assumes we've parsed the function name and we're
 * on the start bracket). 'parent' is the object that contains this method,
 * if there was one (otherwise it's just a normal function).
 * If !isParsing and arg0!=0, argument 0 is set to what is supplied
 */
JsVar *jspeFunctionCall(JsVar *function, JsVar *parent, bool isParsing, JsVar *arg0); 

#endif /* JSPARSE_H_ */
