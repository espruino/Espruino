/*
 * jsfunctions.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#include "jsfunctions.h"

JsVar *jsfHandleFunctionCall(JsVar *a, const char *name) {
  if (strcmp(name,"length")==0) {
    if (jsvIsArray(a)) {
      return jsvNewFromInteger(jsvGetArrayLength(a));
    }
    if (jsvIsString(a)) {
      return jsvNewFromInteger(jsvGetStringLength(a));
    }
  }
  // unhandled
  return 0;
}

void jsfGetJSON(JsVar *var, JsVar *result) {
  assert(jsvIsString(result));
  if (jsvIsUndefined(var)) { 
    jsvAppendString(result,"undefined");
  } else if (jsvIsArray(var)) { 
    int length = (int)jsvGetArrayLength(var);
    int i;
    jsvAppendString(result,"[");
    for (i=0;i<length;i++) {
      JsVar *item = jsvGetArrayItem(var, i);
      jsfGetJSON(item, result);
      jsvUnLock(item);
      if (i<length-1) jsvAppendString(result,",");
    }
    jsvAppendString(result,"]");
  } else if (jsvIsObject(var)) {
    JsVarRef childref = var->firstChild;
    jsvAppendString(result,"{");
    while (childref) {
      char buf[JSLEX_MAX_TOKEN_LENGTH];
      JsVar *child = jsvLock(childref);
      JsVar *childVar;
      // TODO: ability to append one string to another
      jsvGetString(child, buf, JSLEX_MAX_TOKEN_LENGTH);
      jsvAppendString(result, "\"");
      jsvAppendString(result, buf); // FIXME: escape the string
      jsvAppendString(result, "\":");
      childVar = jsvLock(child->firstChild);
      childref = child->nextSibling;
      jsvUnLock(child);
      jsfGetJSON(childVar, result);
      jsvUnLock(childVar);
      if (childref) jsvAppendString(result,",");
    }
    jsvAppendString(result,"}");
  } else if (jsvIsFunction(var)) {
    JsVarRef coderef = 0;
    JsVarRef childref = var->firstChild;
    bool firstParm = true;
    jsvAppendString(result,"function (");
    while (childref) {
      JsVar *child = jsvLock(childref);
      childref = child->nextSibling;
      if (jsvIsFunctionParameter(child)) {
        if (firstParm) 
          firstParm=false;
        else
          jsvAppendString(result, ","); 
        char buf[JSLEX_MAX_TOKEN_LENGTH];
        // TODO: ability to append one string to another      
        jsvGetString(child, buf, JSLEX_MAX_TOKEN_LENGTH);
        jsvAppendString(result, buf); // FIXME: escape the string
      } else if (jsvIsString(child) && jsvIsStringEqual(child, JSPARSE_FUNCTION_CODE_NAME)) {
        coderef = child->firstChild;
      }
      jsvUnLock(child);
    }
    jsvAppendString(result,") ");
    if (coderef) {
       JsVar *codeVar = jsvLock(coderef);
       // TODO: ability to append one string to another
       char buf[JSLEX_MAX_TOKEN_LENGTH];
       jsvGetString(codeVar, buf, JSLEX_MAX_TOKEN_LENGTH);
       jsvAppendString(result, buf);
       jsvUnLock(codeVar);
    } else jsvAppendString(result,"{}");
  } else {
    char buf[JSLEX_MAX_TOKEN_LENGTH];
    jsvGetString(var, buf, JSLEX_MAX_TOKEN_LENGTH);
    jsvAppendString(result, buf);
  }
  // TODO: functions
}

