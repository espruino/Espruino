/*
 * jsfunctions.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#include "jsfunctions.h"

JsVar* jsfMakeUndefined() {
  return jsvNewWithFlags(JSV_NULL); // TODO see about above - should really be 'undefined'
}

/** Note, if this function actually does handle a function call, it
 * MUST return something. If it needs to return undefined, that's tough :/
 */
JsVar *jsfHandleFunctionCall(JsExecInfo *execInfo, JsVar *a, const char *name) {
  if (a==0) { // Special cases for where we're just a basic function
    if (strcmp(name,"eval")==0) {
      JsVar *v = jspParseSingleFunction();
      JsVar *result = 0;
      if (v) result = jspEvaluateVar(execInfo->parse, v);
      jsvUnLock(v);
      if (!result) result = jsfMakeUndefined();
      return result;
    }
    // unhandled
    return 0;
  }
  // Is actually a method on some variable
  if (strcmp(name,"length")==0) {
    if (jsvIsArray(a)) {
      jslMatch(execInfo->lex, LEX_ID);
      return jsvNewFromInteger(jsvGetArrayLength(a));
    }
    if (jsvIsString(a)) {
      jslMatch(execInfo->lex, LEX_ID);
      return jsvNewFromInteger((JsVarInt)jsvGetStringLength(a));
    }
  }
  // --------------------------------- built-in class stuff
  if (jsvGetRef(a) == execInfo->parse->intClass) {
    if (strcmp(name,"parseInt")==0) {
      char buffer[16];
      JsVar *v = jspParseSingleFunction();
      jsvGetString(v, buffer, 16);
      jsvUnLock(v);
      return jsvNewFromInteger(stringToInt(buffer));
    }
    if (strcmp(name,"valueOf")==0) {
      // value of a single character
      int c;
      JsVar *v = jspParseSingleFunction(execInfo);
      if (!jsvIsString(v) || jsvGetStringLength(v)!=1) {
        jsvUnLock(v);
        return jsvNewFromInteger(0);
      }
      c = (int)v->varData.str[0];
      jsvUnLock(v);
      return jsvNewFromInteger(c);
    }
  }
  if (jsvGetRef(a) == execInfo->parse->mathClass) {
    if (strcmp(name,"random")==0) {
      if (jspParseEmptyFunction())
        return jsvNewFromFloat((JsVarFloat)rand() / (JsVarFloat)RAND_MAX);
    }
  }
  if (jsvGetRef(a) == execInfo->parse->jsonClass) {
      if (strcmp(name,"stringify")==0) {
        JsVar *v = jspParseSingleFunction();
        JsVar *result = jsvNewFromString("");
        jsfGetJSON(v, result);
        jsvUnLock(v);
        return result;
      }
      if (strcmp(name,"parse")==0) {
        JsVar *v = jspParseSingleFunction();
        JsVar *res;
        JsVar *bracketed = jsvNewFromString("(");
        jsvAppendStringVar(bracketed, v, 0, 0x7FFFFFFF);
        jsvUnLock(v);
        jsvAppendString(bracketed, ")");
        res = jspEvaluateVar(execInfo->parse, bracketed);
        jsvUnLock(bracketed);
        return res;
      }
      // TODO: Add JSON.parse
    }
  // ------------------------------------------ Built-in variable stuff
  if (jsvIsString(a)) {
     if (strcmp(name,"charAt")==0) {
       char buffer[2];
       size_t idx = 0;
       JsVar *v = jspParseSingleFunction();
       idx = (size_t)jsvGetInteger(v);
       jsvUnLock(v);
       // now search to try and find the char
       v = jsvLock(jsvGetRef(a));
       while (v && idx >= jsvGetMaxCharactersInVar(v)) {
         JsVarRef next;
         idx -= jsvGetMaxCharactersInVar(v);
         next = v->lastChild;
         jsvUnLock(v);
         v = jsvLock(next);
       }
       buffer[0] = 0;
       if (v) buffer[0] = v->varData.str[idx];
       buffer[1] = 0;
       jsvUnLock(v);
       return jsvNewFromString(buffer);
     }
     if (strcmp(name,"indexOf")==0) {
       // slow, but simple!
       JsVar *v = jspParseSingleFunction();
       JsVarInt idx = -1;
       int l = (int)jsvGetStringLength(a) - (int)jsvGetStringLength(v);
       for (idx=0;idx<l;idx++) {
         if (jsvCompareString(a, v, idx, 0, true)==0) {
           jsvUnLock(v);
           return jsvNewFromInteger(idx);
         }
       }
       jsvUnLock(v);
       return jsvNewFromInteger(-1);
     }
     if (strcmp(name,"substring")==0) {
       JsVar *vStart, *vEnd, *res;
       JsVarInt pStart, pEnd;
       jspParseDoubleFunction(&vStart, &vEnd);
       pStart = jsvGetInteger(vStart);
       pEnd = jsvIsUndefined(vEnd) ? 0x7FFFFFFF : jsvGetInteger(vEnd);
       jsvUnLock(vStart);
       jsvUnLock(vEnd);
       if (pStart<0) pStart=0;
       if (pEnd<0) pEnd=0;
       if (pEnd<pStart) {
         int l = pStart;
         pStart = pEnd;
         pEnd = l;
       }
       res = jsvNewWithFlags(JSV_STRING);
       jsvAppendStringVar(res, a, pStart, pEnd-(pStart+1));
       return res;
     }
     if (strcmp(name,"substr")==0) {
        JsVar *vStart, *vLen, *res;
        JsVarInt pStart, pLen;
        jspParseDoubleFunction(&vStart, &vLen);
        pStart = jsvGetInteger(vStart);
        pLen = jsvIsUndefined(vLen) ? 0x7FFFFFFF : jsvGetInteger(vLen);
        jsvUnLock(vStart);
        jsvUnLock(vLen);
        if (pLen<0) pLen=0;
        res = jsvNewWithFlags(JSV_STRING);
        jsvAppendStringVar(res, a, pStart, pLen);
        return res;
      }
   }
  if (jsvIsString(a) || jsvIsObject(a)) {
    if (strcmp(name,"clone")==0) {
      if (jspParseEmptyFunction())
        return jsvCopy(a);
    }
  }
  if (jsvIsArray(a)) {
       if (strcmp(name,"contains")==0) {
         JsVar *childValue = jspParseSingleFunction();
         JsVarRef found = jsvUnLock(jsvGetArrayIndexOf(a, childValue));
         jsvUnLock(childValue);
         return jsvNewFromBool(found!=0);
       }
       if (strcmp(name,"indexOf")==0) {
          JsVar *childValue = jspParseSingleFunction();
          JsVar *idx = jsvGetArrayIndexOf(a, childValue);
          jsvUnLock(childValue);
          if (idx==0) return jsfMakeUndefined();
          return idx;
        }
       if (strcmp(name,"join")==0) {
         JsVar *filler = jsvAsString(jspParseSingleFunction(), true);

         JsVar *str = jsvNewFromString("");
         JsVarRef childRef = a->firstChild;
         while (childRef) {
           JsVar *child = jsvLock(childRef);
           if (child->firstChild) {
             JsVar *data = jsvAsString(jsvLock(child->firstChild), true);
             jsvAppendStringVar(str, data, 0, 0x7FFFFFFF);
             jsvUnLock(data);
           }
           childRef = child->nextSibling;
           jsvUnLock(child);
           if (childRef)
             jsvAppendStringVar(str, filler, 0, 0x7FFFFFFF);
         }
         jsvUnLock(filler);
         return str;
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
        char buf[JSLEX_MAX_TOKEN_LENGTH];
        if (firstParm) 
          firstParm=false;
        else
          jsvAppendString(result, ","); 
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

