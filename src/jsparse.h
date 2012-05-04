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
} JsParse;

void jspInit(JsParse *parse);
void jspKill(JsParse *parse);

bool jspAddNativeFunction(JsParse *parse, const char *funcDesc, JsCallback callbackPtr);
JsVar *jspEvaluateVar(JsParse *parse, JsVar *str);
JsVar *jspEvaluate(JsParse *parse, const char *str);

/** This structure is used when parsing the JavaScript. It contains
 * everything that should be needed. */
typedef struct {
  JsParse *parse;
  JsLex *lex;

  JsVarRef scopes[JSPARSE_MAX_SCOPES];
  int scopeCount;
} JsExecInfo;

bool jspParseEmptyFunction(JsExecInfo *execInfo);    ///< parse function with no arguments
JsVar *jspParseSingleFunction(JsExecInfo *execInfo); ///< parse function with a single argument, return its value (no names!)

#endif /* JSPARSE_H_ */
