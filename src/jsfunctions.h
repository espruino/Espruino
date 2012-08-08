/*
 * jsfunctions.h
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#ifndef JSFUNCTIONS_H_
#define JSFUNCTIONS_H_

#include "jsvar.h"
#include "jslex.h"
#include "jsparse.h"

/* This handles built-in function calls. It's easier to do it this way than to
add it to the symbol table, as that uses RAM */
JsVar *jsfHandleFunctionCall(JsExecInfo *execInfo, JsVar *a, const char *name);

typedef void (*JsfGetJSONCallbackString)(void *data, const char *string);
typedef void (*JsfGetJSONCallbackVar)(void *data, JsVar *var);
void jsfGetJSONWithCallback(JsVar *var, JsfGetJSONCallbackString callbackString, JsfGetJSONCallbackVar callbackVar, void *callbackData);

void jsfGetJSON(JsVar *var, JsVar *result);
void jsfPrintJSON(JsVar *var);

#endif /* JSFUNCTIONS_H_ */
