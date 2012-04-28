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
  JsVarRef  root;   /// root of symbol table

  //CScriptLex *l;             /// current lexer
  //std::vector<CScriptVar*> scopes; /// stack of scopes when parsing

  JsVarRef zeroInt;
  JsVarRef oneInt;
  JsVarRef stringClass; /// Built in string class
  JsVarRef objectClass; /// Built in object class
  JsVarRef arrayClass; /// Built in array class
} JsParse;

void jspInit(JsParse *parse);
void jspKill(JsParse *parse);

bool jspAddNativeFunction(JsParse *parse, const char *funcDesc, JsCallback callbackPtr);
JsVar *jspEvaluateVar(JsParse *parse, JsVar *str);
JsVar *jspEvaluate(JsParse *parse, const char *str);

#endif /* JSPARSE_H_ */
