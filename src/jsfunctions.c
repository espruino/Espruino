/*
 * jsfunctions.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#include "jsfunctions.h"
#include "jsinteractive.h"

#ifdef USE_MATH
#ifdef ARM
#include "mconf.h"
#include "protos.h"
#else
#include <math.h>
#endif
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
JsVar *jsfHandleFunctionCall(JsExecInfo *execInfo, JsVar *parent, JsVar *parentName, const char *name) {
  if (jsfHandleFunctionCallDelegate) {
    JsVar *v = jsfHandleFunctionCallDelegate(execInfo, parent, name);
    if (v!=JSFHANDLEFUNCTIONCALL_UNHANDLED) return v;
  }

  if (parent==0) { // Special cases for where we're just a basic function
    if (strcmp(name,"eval")==0) {
      /*JS* function eval(code)
       *JS*  Evaluate a string containing JavaScript code
       */
      JsVar *v = jspParseSingleFunction();
      JsVar *result = 0;
      if (v) {
        result = jspEvaluateVar(execInfo->parse, v);
        jsvUnLock(v);
      }
      return result;
    }
    if (strcmp(name,"parseInt")==0) {
    /*JS* method parseInt(string, radix)
     *JS*  Convert a string representing a number into an integer
     */
      char buffer[JS_NUMBER_BUFFER_SIZE];
      JsVar *v, *radixVar;
      jspParseFunction(0, &v, &radixVar, 0, 0);
      if (!v) {
        jsvUnLock(radixVar);
        return 0; // undefined!
      }
      jsvGetString(v, buffer, JS_NUMBER_BUFFER_SIZE);
      jsvUnLock(v);
      int radix = 0;
      if (radixVar)
         radix = (int)jsvGetIntegerAndUnLock(radixVar);
      return jsvNewFromInteger(stringToIntWithRadix(buffer, radix));
    }
    if (strcmp(name,"parseFloat")==0) {
      /*JS* method parseFloat(string)
       *JS*  Convert a string representing a number into a float
       */
        char buffer[JS_NUMBER_BUFFER_SIZE];
        JsVar *v = jspParseSingleFunction();
        jsvGetString(v, buffer, JS_NUMBER_BUFFER_SIZE);
        jsvUnLock(v);
        return jsvNewFromFloat(atof(buffer));
    }
  } else {
    // Is actually a method on some variable
    if (strcmp(name,"length")==0) {
      if (jsvIsArray(parent)) {
      /*JS* property Array.length
       *JS*  Return the length of the array
       */
        jslMatch(execInfo->lex, LEX_ID);
        return jsvNewFromInteger(jsvGetArrayLength(parent));
      }
      if (jsvIsString(parent)) {
      /*JS* property String.length
       *JS*  Return the number of characters in the string
       */
        jslMatch(execInfo->lex, LEX_ID);
        return jsvNewFromInteger((JsVarInt)jsvGetStringLength(parent));
      }
    }
    if (strcmp(name,"toString")==0) {
      jspParseEmptyFunction();
      return jsvAsString(parent, false);
    }
    // --------------------------------- built-in class stuff
    if (parentName && jsvIsFunction(parent)) {
      if (jsvIsStringEqual(parentName, "Integer")) {
        if (strcmp(name,"parseInt")==0) {
        /*JS* method Integer.parseInt(string)
         *JS*  Convert a string representing a number into a number
         */
          char buffer[JS_NUMBER_BUFFER_SIZE];
          JsVar *v = jspParseSingleFunction();
          jsvGetString(v, buffer, JS_NUMBER_BUFFER_SIZE);
          jsvUnLock(v);
          return jsvNewFromInteger(stringToInt(buffer));
        }
        if (strcmp(name,"valueOf")==0) {
        /*JS* method Integer.valueOf(char)
         *JS*  Given a string containing a single character, return the numeric value of it
         */
          // value of a single character
          int c;
          JsVar *v = jspParseSingleFunction();
          if (!jsvIsString(v) || jsvGetStringLength(v)!=1) {
            jsvUnLock(v);
            return jsvNewFromInteger(0);
          }
          c = (int)v->varData.str[0];
          jsvUnLock(v);
          return jsvNewFromInteger(c);
        }
      }
      if (jsvIsStringEqual(parentName, "Double")) {
        if (strcmp(name,"doubleToIntBits")==0) {
        /*JS* method Double.doubleToIntBits(val)
         *JS*  Convert the floating point value given into an integer representing the bits contained in it
         */
          JsVar *v = jspParseSingleFunction();
          JsVarFloat f = jsvGetFloatAndUnLock(v);
          return jsvNewFromInteger(*(JsVarInt*)&f);
        }
      }
      if (jsvIsStringEqual(parentName, "Math")) {
        if (strcmp(name,"E")==0) {
        /*JS* property Math.E
         *JS*  The value of E - 2.71828182846
         */
          jspParseVariableName();
          return jsvNewFromFloat(2.71828182846);
        }
        if (strcmp(name,"PI")==0) {
        /*JS* property Math.PI
         *JS*  The value of PI - 3.14159265359
         */
          jspParseVariableName();
          return jsvNewFromFloat(3.14159265359);
        }
        if (strcmp(name,"abs")==0) {
        /*JS* method Math.abs(x)
         *JS*  Return the absolute value of X (as a double)
         */
          JsVarFloat x = jsvGetFloatAndUnLock(jspParseSingleFunction());
          if (x<0) x=-x;
          return jsvNewFromFloat(x);
        }
  #ifdef USE_MATH
        if (strcmp(name,"acos")==0) {
        /*JS* method Math.acos(x)
         *JS*  Arc Cosine. Takes a double between -1 and 1 and produce a value between 0 and PI.
         */
          JsVarFloat x = jsvGetFloatAndUnLock(jspParseSingleFunction());
          return jsvNewFromFloat(acos(x));
        }
        if (strcmp(name,"asin")==0) {
        /*JS* method Math.asin(x)
         *JS*  Arc Sine. Takes a double between -1 and 1 and produce a value between -PI/2 and PI/2.
         */
          JsVarFloat x = jsvGetFloatAndUnLock(jspParseSingleFunction());
          return jsvNewFromFloat(asin(x));
        }
        if (strcmp(name,"atan")==0) {
        /*JS* method Math.atan(x)
         *JS*  Arc Tangent. Takes a double between -1 and 1 and produce a value between -PI/2 and PI/2.
         */
          JsVarFloat x = jsvGetFloatAndUnLock(jspParseSingleFunction());
          return jsvNewFromFloat(atan(x));
        }
        if (strcmp(name,"atan2")==0) {
        /*JS* method Math.atan2(x,y)
         *JS*  Arc Tangent of y/x. Takes any values and produces values between 0 and PI.
         */
          JsVar *x,*y;
          jspParseFunction(0,&y,&x,0,0);
          return jsvNewFromFloat(atan2(jsvGetFloatAndUnLock(y),jsvGetFloatAndUnLock(x)));
        }
        if (strcmp(name,"cos")==0) {
        /*JS* method Math.cos(rads)
         *JS*  Cosine. Takes any value in radians and produces values between -1 and 1
         */
          JsVarFloat x = jsvGetFloatAndUnLock(jspParseSingleFunction());
          return jsvNewFromFloat(cos(x));
        }
        if (strcmp(name,"round")==0) {
          JsVarFloat x = jsvGetFloatAndUnLock(jspParseSingleFunction());
          return jsvNewFromInteger(lround(x));
        }
        if (strcmp(name,"sin")==0) {
        /*JS* method Math.sin(rads)
         *JS*  Sine. Takes any value in radians and produces values between -1 and 1
         */
          JsVarFloat x = jsvGetFloatAndUnLock(jspParseSingleFunction());
          return jsvNewFromFloat(sin(x));
        }
        if (strcmp(name,"sqrt")==0) {
        /*JS* method Math.sqrt(x)
         *JS*  Return the Square Root of X
         */
          JsVarFloat x = jsvGetFloatAndUnLock(jspParseSingleFunction());
          return jsvNewFromFloat(sqrt(x));
        }
        if (strcmp(name,"tan")==0) {
        /*JS* method Math.tan(rads)
         *JS*  Tangent. Takes any value in radians and produces the Tangent
         */
          JsVarFloat x = jsvGetFloatAndUnLock(jspParseSingleFunction());
          return jsvNewFromFloat(tan(x));
        }
  #endif
        if (strcmp(name,"random")==0) {
        /*JS* method Math.random()
         *JS*  Return a random floating point value between 0 and 1
         */
          if (jspParseEmptyFunction())
            return jsvNewFromFloat((JsVarFloat)rand() / (JsVarFloat)RAND_MAX);
        }
      }
      if (jsvIsStringEqual(parentName, "JSON")) {
          if (strcmp(name,"stringify")==0) {
        /*JS* method JSON.stringify(object)
         *JS*  Convert the given object into a JSON string which can subsequently be parsed with JSON.parse or eval
         */
            JsVar *v = jspParseSingleFunction();
            JsVar *result = jsvNewFromString("");
            if (result) // could be out of memory
                jsfGetJSON(v, result);
            jsvUnLock(v);
            return result;
          }
          if (strcmp(name,"parse")==0) {
        /*JS* method JSON.parse(string)
         *JS*  Parse the given JSON string into a JavaScript object
         */
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
        }
    }
    // ------------------------------------------ Built-in variable stuff
    if (jsvIsString(parent)) {
       if (strcmp(name,"charAt")==0) {
      /*JS* method String.charAt(idx)
       *JS*  Return a single character at the given position in the String.
       *JS*  Negative values return characters from end of string (-1 = last char)
       *JS*  Running on a non-string returns 0
       */
         char buffer[2];
         JsVar *v = jspParseSingleFunction();
         JsVarInt idx = jsvGetIntegerAndUnLock(v);
         // now search to try and find the char
         buffer[0] = jsvGetCharInString(parent, (int)idx);
         buffer[1] = 0;
         return jsvNewFromString(buffer);
       }
       if (strcmp(name,"indexOf")==0) {
      /*JS* method String.indexOf(substring)
       *JS*  Return the index of substring in this string, or -1 if not found
       */
         // slow, but simple!
         JsVar *v = jsvAsString(jspParseSingleFunction(), true);
         if (!v) return 0; // out of memory
         int idx = -1;
         int l = (int)jsvGetStringLength(parent) - (int)jsvGetStringLength(v);
         for (idx=0;idx<l;idx++) {
           if (jsvCompareString(parent, v, idx, 0, true)==0) {
             jsvUnLock(v);
             return jsvNewFromInteger(idx);
           }
         }
         jsvUnLock(v);
         return jsvNewFromInteger(-1);
       }
       if (strcmp(name,"substring")==0) {
      /*JS* method String.substring(start,end)
       */
         JsVar *vStart, *vEnd, *res;
         int pStart, pEnd;
         jspParseFunction(0, &vStart, &vEnd, 0, 0);
         pStart = (int)jsvGetIntegerAndUnLock(vStart);
         pEnd = jsvIsUndefined(vEnd) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vEnd);
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
         jsvAppendStringVar(res, parent, pStart, pEnd-pStart);
         return res;
       }
       if (strcmp(name,"substr")==0) {
      /*JS* method String.substr(start,len)
       */
          JsVar *vStart, *vLen, *res;
          int pStart, pLen;
          jspParseFunction(0, &vStart, &vLen, 0, 0);
          pStart = (int)jsvGetIntegerAndUnLock(vStart);
          pLen = jsvIsUndefined(vLen) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vLen);
          jsvUnLock(vLen);
          if (pLen<0) pLen=0;
          res = jsvNewWithFlags(JSV_STRING);
          if (!res) return 0; // out of memory
          jsvAppendStringVar(res, parent, pStart, pLen);
          return res;
        }
        if (strcmp(name,"split")==0) {
      /*JS* method String.split(separator)
       *JS*  Return an array made by splitting this string up by the separator. eg. "1,2,3".split(",")==[1,2,3]
       */
          JsVar *array;
          int last, idx, arraylen=0;
          JsVar *split = jspParseSingleFunction();
          int splitlen =  (int)jsvGetStringLength(split);
          int l = (int)jsvGetStringLength(parent) - splitlen;
          last = 0;

          array = jsvNewWithFlags(JSV_ARRAY);
          if (!array) return 0; // out of memory

          for (idx=0;idx<=l;idx++) {
            if (idx==l || jsvCompareString(parent, split, idx, 0, true)==0) {
              JsVar *part = jsvNewFromString("");
              if (!part) break; // out of memory
              JsVar *idxvar = jsvMakeIntoVariableName(jsvNewFromInteger(arraylen++), part);
              if (idxvar) { // could be out of memory
                if (idx==l) idx=l+splitlen; // if the last element, do to the end of the string
                jsvAppendStringVar(part, parent, last, idx-last);
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
    if (jsvIsString(parent) || jsvIsObject(parent)) {
      if (strcmp(name,"clone")==0) {
      /*JS* method Object.clone()
       *JS*  Copy this object in its Entirity
       */
        if (jspParseEmptyFunction())
          return jsvCopy(parent);
      }
    }
    if (jsvIsArray(parent)) {
         if (strcmp(name,"contains")==0) {
      /*JS* method Array.contains(value)
       *JS*  Return true if this array contains the given value
       */
           JsVar *childValue = jspParseSingleFunction();
           JsVarRef found = jsvUnLock(jsvGetArrayIndexOf(parent, childValue, false/*not exact*/)); // ArrayIndexOf will return 0 if not found
           jsvUnLock(childValue);
           return jsvNewFromBool(found!=0);
         }
         if (strcmp(name,"indexOf")==0) {
      /*JS* method Array.indexOf(value)
       *JS*  Return the index of the value in the array, or -1
       */
            JsVar *childValue = jspParseSingleFunction();
            JsVar *idxName = jsvGetArrayIndexOf(parent, childValue, false/*not exact*/);
            jsvUnLock(childValue);
            // but this is the name - we must turn it into a var
            if (idxName == 0) return 0; // not found!
            JsVar *idx = jsvCopyNameOnly(idxName, false/* no children */, false/* Make sure this is not a name*/);
            jsvUnLock(idxName);
            return idx;
          }
         if (strcmp(name,"join")==0) {
      /*JS* method Array.join(separator)
       *JS*  Join all elements of this array together into one string, using 'separator' between them. eg. [1,2,3].join(" ")=="1 2 3"
       */
           JsVar *filler = jspParseSingleFunction();
           if (jsvIsUndefined(filler))
             filler = jsvNewFromString(","); // the default it seems
           else
             filler = jsvAsString(filler, true);
           if (!filler) return 0; // out of memory
           JsVar *str = jsvArrayJoin(parent, filler);
           jsvUnLock(filler);
           return str;
         }
         if (strcmp(name,"push")==0) {
      /*JS* method Array.push(value)
       *JS*  Push a new value onto the end of this array
       */
           JsVar *childValue = jspParseSingleFunction();
           JsVarInt newSize = jsvArrayPush(parent, childValue);
           jsvUnLock(childValue);
           return jsvNewFromInteger(newSize);
         }
         if (strcmp(name,"pop")==0) {
      /*JS* method Array.pop()
       *JS*  Pop a new value off of the end of this array
       */
           JsVar *childValue = jspParseSingleFunction();
           JsVar *item = jsvArrayPop(parent);
           jsvUnLock(childValue);
           return item;
         }
         if (strcmp(name,"map")==0) {
      /*JS* method Array.map(function, thisArg)
       *JS*  Return an array which is made from the following: A.map(function) = [function(A[0]), function(A[1]), ...]
       *JS*  If thisArg is specified, the function is called with 'this' set to thisArg
       */
           JsVar *funcVar, *thisVar; 
           jspParseFunction(0, &funcVar, &thisVar, 0, 0);
           if (!jsvIsFunction(funcVar)) {
             jsError("Array.map's first argument should be a function");
             jsvUnLock(funcVar); jsvUnLock(thisVar);
             return 0;
           }
           if (!jsvIsUndefined(thisVar) && !jsvIsObject(thisVar)) {
             jsError("Array.map's second argument should be undefined, or an object");
             jsvUnLock(funcVar); jsvUnLock(thisVar);
             return 0;
           }
           JsVar *array = jsvNewWithFlags(JSV_ARRAY);
           if (array) { 
             JsVarRef childRef = parent->firstChild;
             while (childRef) {
               JsVar *child = jsvLock(childRef);
               if (jsvIsInt(child)) {
                 JsVar *childValue = jsvLock(child->firstChild);
                 JsVar *mapped = jspeFunctionCall(funcVar, 0, thisVar, false, childValue);
                 jsvUnLock(childValue);
                 if (mapped) {
                   JsVar *name = jsvCopyNameOnly(child, false/*linkChildren*/, true/*keepAsName*/);
                   if (name) { // out of memory?
                     name->firstChild = jsvGetRef(jsvRef(mapped));
                     jsvAddName(array, name);
                     jsvUnLock(name);
                   }
                   jsvUnLock(mapped); 
                 }
               }
               childRef = child->nextSibling;
               jsvUnLock(child);
             }
           }
           jsvUnLock(funcVar); jsvUnLock(thisVar);
           return array;
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
      childVar = child->firstChild ? jsvLock(child->firstChild) : 0;
      childref = child->nextSibling;
      jsvUnLock(child);

      jsfGetJSONWithCallback(childVar, callbackString, callbackVar, callbackData);
      jsvUnLock(childVar);
      if (childref) callbackString(callbackData, ",");
    }
    callbackString(callbackData, "}");
  } else if (jsvIsFunction(var)) {
    JsVarRef coderef = 0; // TODO: this should really be in jsvAsString
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
  } else if (jsvIsString(var) && !jsvIsName(var)) {
    // escape the string
    callbackString(callbackData, "\"");
    JsvStringIterator it;
    jsvStringIteratorNew(&it, var, 0);
    while (jsvStringIteratorHasChar(&it)) {
      char ch = jsvStringIteratorGetChar(&it);
      callbackString(callbackData, escapeCharacter(ch));
      jsvStringIteratorNext(&it);
    }
    jsvStringIteratorFree(&it);
    callbackString(callbackData, "\"");
  } else {
    JsVar *str = jsvAsString(var, false);
    if (str) {
      callbackVar(callbackData, str);
      jsvUnLock(str);
    }
  }
  // TODO: functions
}

void jsfGetJSON(JsVar *var, JsVar *result) {
  assert(jsvIsString(result));
  jsfGetJSONWithCallback(var, (JsfGetJSONCallbackString)jsvAppendString, (JsfGetJSONCallbackVar)jsvAppendStringVarComplete, result);
}

void _jsfPrintJSON_str(void *data, const char *str) { NOT_USED(data); jsiConsolePrint(str); }
void _jsfPrintJSON_var(void *data, JsVar *var) { NOT_USED(data); jsiConsolePrintStringVar(var); }
void jsfPrintJSON(JsVar *var) {
  jsfGetJSONWithCallback(var, _jsfPrintJSON_str, _jsfPrintJSON_var, 0);
}

