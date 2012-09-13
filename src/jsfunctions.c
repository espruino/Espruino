/*
 * jsfunctions.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#include "jsfunctions.h"
#ifdef USE_MATH
#include <math.h>
#endif

JsfHandleFunctionCallDelegate jsfHandleFunctionCallDelegate = 0;

/** If we want to add our own extra functions, we can do it by creating a
 * delegate function and linking it in here. */
void jsfSetHandleFunctionCallDelegate(JsfHandleFunctionCallDelegate delegate) {
  jsfHandleFunctionCallDelegate = delegate;
}

/** Note, if this function actually does handle a function call, it
 * MUST return something. If it needs to return undefined, that's tough :/
 */
JsVar *jsfHandleFunctionCall(JsExecInfo *execInfo, JsVar *a, const char *name) {
  if (jsfHandleFunctionCallDelegate) {
    JsVar *v = jsfHandleFunctionCallDelegate(execInfo, a, name);
    if (v!=JSFHANDLEFUNCTIONCALL_UNHANDLED) return v;
  }

  if (a==0) { // Special cases for where we're just a basic function
    if (strcmp(name,"eval")==0) {
      JsVar *v = jspParseSingleFunction();
      JsVar *result = 0;
      if (v) {
        result = jspEvaluateVar(execInfo->parse, v);
        jsvUnLock(v);
      }
      return result;
    }
  } else {
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
    if (jsvGetRef(a) == execInfo->parse->doubleClass) {
      if (strcmp(name,"doubleToIntBits")==0) {
        JsVar *v = jspParseSingleFunction(execInfo);
        JsVarFloat f = jsvGetDouble(v);
        jsvUnLock(v);
        return jsvNewFromInteger(*(JsVarInt*)&f);
      }
    }
    if (jsvGetRef(a) == execInfo->parse->mathClass) {
      if (strcmp(name,"E")==0) {
        jspParseVariableName();
        return jsvNewFromFloat(2.71828182846);
      }
      if (strcmp(name,"PI")==0) {
        jspParseVariableName();
        return jsvNewFromFloat(3.14159265359);
      }
#ifdef USE_MATH
      if (strcmp(name,"abs")==0) {
        JsVarFloat x = jsvGetDoubleAndUnLock(jspParseSingleFunction());
        return jsvNewFromFloat(abs(x));
      }
      if (strcmp(name,"acos")==0) {
        JsVarFloat x = jsvGetDoubleAndUnLock(jspParseSingleFunction());
        return jsvNewFromFloat(acos(x));
      }
      if (strcmp(name,"asin")==0) {
        JsVarFloat x = jsvGetDoubleAndUnLock(jspParseSingleFunction());
        return jsvNewFromFloat(asin(x));
      }
      if (strcmp(name,"atan")==0) {
        JsVarFloat x = jsvGetDoubleAndUnLock(jspParseSingleFunction());
        return jsvNewFromFloat(atan(x));
      }
      if (strcmp(name,"atan2")==0) {
        JsVar *x,*y;
        jspParseDoubleFunction(&y,&x);
        return jsvNewFromFloat(atan2(jsvGetDoubleAndUnLock(y),jsvGetDoubleAndUnLock(x)));
      }
      if (strcmp(name,"cos")==0) {
        JsVarFloat x = jsvGetDoubleAndUnLock(jspParseSingleFunction());
        return jsvNewFromFloat(cos(x));
      }
      if (strcmp(name,"round")==0) {
        JsVarFloat x = jsvGetDoubleAndUnLock(jspParseSingleFunction());
        return jsvNewFromInteger(lround(x));
      }
      if (strcmp(name,"sin")==0) {
        JsVarFloat x = jsvGetDoubleAndUnLock(jspParseSingleFunction());
        return jsvNewFromFloat(sin(x));
      }
      if (strcmp(name,"sqrt")==0) {
        JsVarFloat x = jsvGetDoubleAndUnLock(jspParseSingleFunction());
        return jsvNewFromFloat(sqrt(x));
      }
      if (strcmp(name,"tan")==0) {
        JsVarFloat x = jsvGetDoubleAndUnLock(jspParseSingleFunction());
        return jsvNewFromFloat(tan(x));
      }
#endif
      if (strcmp(name,"random")==0) {
        if (jspParseEmptyFunction())
          return jsvNewFromFloat((JsVarFloat)rand() / (JsVarFloat)RAND_MAX);
      }
    }
    if (jsvGetRef(a) == execInfo->parse->jsonClass) {
        if (strcmp(name,"stringify")==0) {
          JsVar *v = jspParseSingleFunction();
          JsVar *result = jsvNewFromString("");
          if (result) // could be out of memory
              jsfGetJSON(v, result);
          jsvUnLock(v);
          return result;
        }
        if (strcmp(name,"parse")==0) {
          JsVar *v = jspParseSingleFunction();
          JsVar *res = 0;
          JsVar *bracketed = jsvNewFromString("(");
          if (bracketed) { // could be out of memory
            jsvAppendStringVar(bracketed, v, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
            jsvUnLock(v);
            jsvAppendString(bracketed, ")");
            res = jspEvaluateVar(execInfo->parse, bracketed);
            jsvUnLock(bracketed);
          }
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
         JsVar *v = jsvAsString(jspParseSingleFunction(), true);
         if (!v) return 0; // out of memory
         int idx = -1;
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
         int pStart, pEnd;
         jspParseDoubleFunction(&vStart, &vEnd);
         pStart = (int)jsvGetInteger(vStart);
         pEnd = jsvIsUndefined(vEnd) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vEnd);
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
         if (!res) return 0; // out of memory
         jsvAppendStringVar(res, a, pStart, pEnd-pStart);
         return res;
       }
       if (strcmp(name,"substr")==0) {
          JsVar *vStart, *vLen, *res;
          int pStart, pLen;
          jspParseDoubleFunction(&vStart, &vLen);
          pStart = (int)jsvGetInteger(vStart);
          pLen = jsvIsUndefined(vLen) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vLen);
          jsvUnLock(vStart);
          jsvUnLock(vLen);
          if (pLen<0) pLen=0;
          res = jsvNewWithFlags(JSV_STRING);
          if (!res) return 0; // out of memory
          jsvAppendStringVar(res, a, pStart, pLen);
          return res;
        }
        if (strcmp(name,"split")==0) {
          JsVar *array;
          int last, idx, arraylen=0;
          JsVar *split = jspParseSingleFunction();
          int splitlen =  (int)jsvGetStringLength(split);
          int l = (int)jsvGetStringLength(a) - splitlen;
          last = 0;

          array = jsvNewWithFlags(JSV_ARRAY);
          if (!array) return 0; // out of memory

          for (idx=0;idx<=l;idx++) {
            if (idx==l || jsvCompareString(a, split, idx, 0, true)==0) {
              JsVar *part = jsvNewFromString("");
              if (!part) break; // out of memory
              JsVar *idxvar = jsvMakeIntoVariableName(jsvNewFromInteger(arraylen++), part);
              if (idxvar) { // could be out of memory
                jsvAppendStringVar(part, a, last, idx-(last+1));
                jsvAddName(array, idxvar);
                last = idx+splitlen;
                jsvUnLock(idxvar);
              }
              jsvUnLock(part);
            }
          }

          jsvUnLock(split);
          return array;
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
            return idx;
          }
         if (strcmp(name,"join")==0) {
           JsVar *filler = jsvAsString(jspParseSingleFunction(), true);
           if (!filler) return 0; // out of memory
           JsVar *str = jsvNewFromString("");
           if (!str) { // out of memory
             jsvUnLock(filler);
             return 0;
           }
           JsVarRef childRef = a->firstChild;
           while (childRef) {
             JsVar *child = jsvLock(childRef);
             if (child->firstChild) {
               JsVar *data = jsvAsString(jsvLock(child->firstChild), true);
               if (data) { // could be out of memory
                 jsvAppendStringVar(str, data, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
                 jsvUnLock(data);
               }
             }
             childRef = child->nextSibling;
             jsvUnLock(child);
             if (childRef)
               jsvAppendStringVar(str, filler, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
           }
           jsvUnLock(filler);
           return str;
         }
         if (strcmp(name,"push")==0) {
           JsVar *childValue = jspParseSingleFunction();
           JsVarInt newSize = jsvArrayPush(a, childValue);
           jsvUnLock(childValue);
           return jsvNewFromInteger(newSize);
         }
         if (strcmp(name,"pop")==0) {
           JsVar *childValue = jspParseSingleFunction();
           JsVar *item = jsvArrayPop(a);
           jsvUnLock(childValue);
           return item;
         }
    }
  }
  // unhandled
  return JSFHANDLEFUNCTIONCALL_UNHANDLED;
}

void jsfGetJSONWithCallback(JsVar *var, JsfGetJSONCallbackString callbackString, JsfGetJSONCallbackVar callbackVar, void *callbackData) {
  if (jsvIsUndefined(var)) { 
    callbackString(callbackData, "undefined");
  } else if (jsvIsArray(var)) { 
    int length = (int)jsvGetArrayLength(var);
    int i;
    callbackString(callbackData, "[");
    for (i=0;i<length;i++) {
      JsVar *item = jsvGetArrayItem(var, i);
      jsfGetJSONWithCallback(item, callbackString, callbackVar, callbackData);
      jsvUnLock(item);
      if (i<length-1) callbackString(callbackData, ",");
    }
    callbackString(callbackData, "]");
  } else if (jsvIsObject(var)) {
    JsVarRef childref = var->firstChild;
    callbackString(callbackData, "{");
    while (childref) {
      JsVar *child = jsvLock(childref);
      JsVar *childVar;
      callbackString(callbackData, "\"");
      callbackVar(callbackData, child); // FIXME: escape the string
      callbackString(callbackData, "\":");
      childVar = jsvLock(child->firstChild);
      childref = child->nextSibling;
      jsvUnLock(child);
      jsfGetJSONWithCallback(childVar, callbackString, callbackVar, callbackData);
      jsvUnLock(childVar);
      if (childref) callbackString(callbackData, ",");
    }
    callbackString(callbackData, "}");
  } else if (jsvIsFunction(var)) {
    JsVarRef coderef = 0;
    JsVarRef childref = var->firstChild;
    bool firstParm = true;
    callbackString(callbackData, "function (");
    while (childref) {
      JsVar *child = jsvLock(childref);
      childref = child->nextSibling;
      if (jsvIsFunctionParameter(child)) {
        if (firstParm) 
          firstParm=false;
        else
          callbackString(callbackData, ",");
        callbackVar(callbackData, child); // FIXME: escape the string
      } else if (jsvIsString(child) && jsvIsStringEqual(child, JSPARSE_FUNCTION_CODE_NAME)) {
        coderef = child->firstChild;
      }
      jsvUnLock(child);
    }
    callbackString(callbackData, ") ");
    if (coderef) {
       JsVar *codeVar = jsvLock(coderef);
       callbackVar(callbackData, codeVar);
       jsvUnLock(codeVar);
    } else callbackString(callbackData, "{}");
  } else if (jsvIsString(var)) {
    callbackString(callbackData, "\"");
    callbackVar(callbackData, var);
    callbackString(callbackData, "\"");
  } else {
    char buf[JSLEX_MAX_TOKEN_LENGTH];
    jsvGetString(var, buf, JSLEX_MAX_TOKEN_LENGTH);
    callbackString(callbackData, buf);
  }
  // TODO: functions
}

void jsfGetJSON(JsVar *var, JsVar *result) {
  assert(jsvIsString(result));
  jsfGetJSONWithCallback(var, (JsfGetJSONCallbackString)jsvAppendString, (JsfGetJSONCallbackVar)jsvAppendStringVarComplete, result);
}

void _jsfPrintJSON_str(void *data, const char *str) { jsPrint(str); }
void _jsfPrintJSON_var(void *data, JsVar *var) { jsvPrintStringVar(var); }
void jsfPrintJSON(JsVar *var) {
  jsfGetJSONWithCallback(var, _jsfPrintJSON_str, _jsfPrintJSON_var, 0);
}

