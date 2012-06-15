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
  JsVarRef mathClass; ///< Built in maths class
  JsVarRef jsonClass; ///< Built in json class
} JsParse;

void jspInit(JsParse *parse);
void jspKill(JsParse *parse);

bool jspAddNativeFunction(JsParse *parse, const char *funcDesc, JsCallback callbackPtr);
JsVar *jspEvaluateVar(JsParse *parse, JsVar *str);
JsVar *jspEvaluate(JsParse *parse, const char *str);

/** When parsing, this enum defines whether
 we are executing or not */
typedef enum  {
  EXEC_NO = 0,
  EXEC_YES = 1,
  EXEC_RUN_MASK = 1,
  // EXEC_ERROR = 2 // maybe?
} JsExecFlags;

/** This structure is used when parsing the JavaScript. It contains
 * everything that should be needed. */
typedef struct {
  JsParse *parse;
  JsLex *lex;

  JsVarRef scopes[JSPARSE_MAX_SCOPES];
  int scopeCount;

  JsExecFlags execute;
} JsExecInfo;

bool jspParseEmptyFunction();    ///< parse function with no arguments
JsVar *jspParseSingleFunction(); ///< parse function with a single argument, return its value (no names!)
void jspParseDoubleFunction(JsVar **a, JsVar **b); ///< parse function with 2 arguments, return 2 values (no names!)

#endif /* JSPARSE_H_ */
