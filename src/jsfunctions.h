/*
 * jsfunctions.h
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#ifndef JSFUNCTIONS_H_
#define JSFUNCTIONS_H_

#include "jsvar.h"

/* This handles built-in function calls. It's easier to do it this way than to
add it to the symbol table, as that uses RAM */
//JsVar *jsfHandleFunctionCall(JsVar *a, JsVar *parent, const char *name);

void jsfGetJSON(JsVar *var, JsVar *result);

#endif /* JSFUNCTIONS_H_ */
