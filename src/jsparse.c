/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Recursive descent parser for code execution
 * ----------------------------------------------------------------------------
 */
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrapper.h"
#include "jsnative.h"
#include "jswrap_object.h" // for function_replacewith
#include "jswrap_functions.h" // insane check for eval in jspeFunctionCall

/* Info about execution when Parsing - this saves passing it on the stack
 * for each call */
JsExecInfo execInfo;

// ----------------------------------------------- Forward decls
JsVar *jspeAssignmentExpression();
JsVar *jspeExpression();
JsVar *jspeUnaryExpression();
JsVar *jspeBlock();
JsVar *jspeStatement();
JsVar *jspeFactor();
void jspEnsureIsPrototype(JsVar *instanceOf, JsVar *prototypeName);
// ----------------------------------------------- Utils
#define JSP_MATCH_WITH_CLEANUP_AND_RETURN(TOKEN, CLEANUP_CODE, RETURN_VAL) { if (!jslMatch(execInfo.lex,(TOKEN))) { CLEANUP_CODE; return RETURN_VAL; } }
#define JSP_MATCH_WITH_RETURN(TOKEN, RETURN_VAL) JSP_MATCH_WITH_CLEANUP_AND_RETURN(TOKEN, , RETURN_VAL)
#define JSP_MATCH(TOKEN) JSP_MATCH_WITH_CLEANUP_AND_RETURN(TOKEN, , 0) // Match where the user could have given us the wrong token
#define JSP_ASSERT_MATCH(TOKEN) { assert(execInfo.lex->tk==(TOKEN));jslGetNextToken(execInfo.lex); } // Match where if we have the wrong token, it's an internal error
#define JSP_SHOULD_EXECUTE (((execInfo.execute)&EXEC_RUN_MASK)==EXEC_YES)
#define JSP_SAVE_EXECUTE() JsExecFlags oldExecute = execInfo.execute
#define JSP_RESTORE_EXECUTE() execInfo.execute = (execInfo.execute&(JsExecFlags)(~EXEC_SAVE_RESTORE_MASK)) | (oldExecute&EXEC_SAVE_RESTORE_MASK);
#define JSP_HAS_ERROR (((execInfo.execute)&EXEC_ERROR_MASK)!=0)
#define JSP_SHOULDNT_PARSE (((execInfo.execute)&EXEC_NO_PARSE_MASK)!=0)

/// if interrupting execution, this is set
bool jspIsInterrupted() {
  return (execInfo.execute & EXEC_INTERRUPTED)!=0;
}

/// if interrupting execution, this is set
void jspSetInterrupted(bool interrupt) {
  if (interrupt)
    execInfo.execute = execInfo.execute | EXEC_INTERRUPTED;
  else
    execInfo.execute = execInfo.execute & (JsExecFlags)~EXEC_INTERRUPTED;
}

/// Set the error flag - set lineReported if we've already output the line number
void jspSetError(bool lineReported) {
  execInfo.execute = (execInfo.execute & (JsExecFlags)~EXEC_YES) | EXEC_ERROR;
  if (lineReported)
    execInfo.execute |= EXEC_ERROR_LINE_REPORTED;
}

bool jspHasError() {
  return JSP_HAS_ERROR;
}

void jspReplaceWith(JsVar *dst, JsVar *src) {
  // If this is an index in an array buffer, write directly into the array buffer
  if (jsvIsArrayBufferName(dst)) {
    size_t idx = (size_t)jsvGetInteger(dst);
    JsVar *arrayBuffer = jsvLock(jsvGetFirstChild(dst));
    jsvArrayBufferSet(arrayBuffer, idx, src);
    jsvUnLock(arrayBuffer);
    return;
  }
  // if destination isn't there, isn't a 'name', or is used, give an error
  if (!jsvIsName(dst)) {
    jsExceptionHere(JSET_ERROR, "Unable to assign value to non-reference %t", dst);
    return;
  }
  jsvSetValueOfName(dst, src);
  /* If dst is flagged as a new child, it means that
   * it was previously undefined, and we need to add it to
   * the given object when it is set.
   */
  if (jsvIsNewChild(dst)) {
    // Get what it should have been a child of
    JsVar *parent = jsvLock(jsvGetNextSibling(dst));
    // Remove the 'new child' flagging
    jsvUnRef(parent);
    jsvSetNextSibling(dst, 0);
    jsvUnRef(parent);
    jsvSetPrevSibling(dst, 0);
    // Add to the parent
    jsvAddName(parent, dst);
    jsvUnLock(parent);
  }
}

void jspeiInit(JsLex *lex) {
  execInfo.lex = lex;
  execInfo.scopeCount = 0;
  execInfo.execute = EXEC_YES;
  execInfo.thisVar = 0;
}

void jspeiKill() {
  execInfo.lex = 0;
  assert(execInfo.scopeCount==0);
}

bool jspeiAddScope(JsVar *scope) {
  if (execInfo.scopeCount >= JSPARSE_MAX_SCOPES) {
    jsExceptionHere(JSET_ERROR, "Maximum number of scopes exceeded");
    jspSetError(false);
    return false;
  }
  execInfo.scopes[execInfo.scopeCount++] = jsvLockAgain(scope);
  return true;
}

void jspeiRemoveScope() {
  if (execInfo.scopeCount <= 0) {
    jsExceptionHere(JSET_INTERNALERROR, "Too many scopes removed");
    jspSetError(false);
    return;
  }
  jsvUnLock(execInfo.scopes[--execInfo.scopeCount]);
}

JsVar *jspeiFindInScopes(const char *name) {
  int i;
  for (i=execInfo.scopeCount-1;i>=0;i--) {
    JsVar *ref = jsvFindChildFromString(execInfo.scopes[i], name, false);
    if (ref) return ref;
  }
  return jsvFindChildFromString(execInfo.root, name, false);
}

// TODO: get rid of these, use jspeiGetTopScope instead
JsVar *jspeiFindOnTop(const char *name, bool createIfNotFound) {
  if (execInfo.scopeCount>0)
    return jsvFindChildFromString(execInfo.scopes[execInfo.scopeCount-1], name, createIfNotFound);
  return jsvFindChildFromString(execInfo.root, name, createIfNotFound);
}
JsVar *jspeiFindNameOnTop(JsVar *childName, bool createIfNotFound) {
  if (execInfo.scopeCount>0)
    return jsvFindChildFromVar(execInfo.scopes[execInfo.scopeCount-1], childName, createIfNotFound);
  return jsvFindChildFromVar(execInfo.root, childName, createIfNotFound);
}



/** Here we assume that we have already looked in the parent itself -
 * and are now going down looking at the stuff it inherited */
JsVar *jspeiFindChildFromStringInParents(JsVar *parent, const char *name) {
  if (jsvIsObject(parent)) {
    // If an object, look for an 'inherits' var
    JsVar *inheritsFrom = jsvObjectGetChild(parent, JSPARSE_INHERITS_VAR, 0);

    // if there's no inheritsFrom, just default to 'Object.prototype'
    if (!inheritsFrom) {
      JsVar *obj = jsvObjectGetChild(execInfo.root, "Object", 0);
      if (obj) {
        inheritsFrom = jsvObjectGetChild(obj, JSPARSE_PROTOTYPE_VAR, 0);
        jsvUnLock(obj);
      }
    }

    if (inheritsFrom && inheritsFrom!=parent) {
      // we have what it inherits from (this is ACTUALLY the prototype var)
      // https://developer.mozilla.org/en-US/docs/JavaScript/Reference/Global_Objects/Object/proto
      JsVar *child = jsvFindChildFromString(inheritsFrom, name, false);
      if (!child)
        child = jspeiFindChildFromStringInParents(inheritsFrom, name);
      jsvUnLock(inheritsFrom);
      if (child) return child;
    } else
      jsvUnLock(inheritsFrom);
  } else { // Not actually an object - but might be an array/string/etc
    const char *objectName = jswGetBasicObjectName(parent);
    while (objectName) {
      JsVar *objName = jsvFindChildFromString(execInfo.root, objectName, false);
      if (objName) {
        JsVar *result = 0;
        JsVar *obj = jsvSkipNameAndUnLock(objName);
        // could be something the user has made - eg. 'Array=1'
        if (jsvHasChildren(obj)) {
          // We have found an object with this name - search for the prototype var
          JsVar *proto = jsvObjectGetChild(obj, JSPARSE_PROTOTYPE_VAR, 0);
          if (proto) {
            result = jsvFindChildFromString(proto, name, false);
            jsvUnLock(proto);
          }
        }
        jsvUnLock(obj);
        if (result) return result;
      }
      /* We haven't found anything in the actual object, we should check the 'Object' itself
        eg, we tried 'String', so now we should try 'Object'. Built-in types don't have room for
        a prototype field, so we hard-code it */
      objectName = jswGetBasicObjectPrototypeName(objectName);
    }
  }

  // no luck!
  return 0;
}

JsVar *jspeiGetScopesAsVar() {
  if (execInfo.scopeCount==0)
    return 0;
  if (execInfo.scopeCount==1)
    return jsvLockAgain(execInfo.scopes[0]);

  JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
  int i;
  for (i=0;i<execInfo.scopeCount;i++) {
      JsVar *idx = jsvMakeIntoVariableName(jsvNewFromInteger(i), execInfo.scopes[i]);
      if (!idx) { // out of memory
        jspSetError(false);
        return arr;
      }
      jsvAddName(arr, idx);
      jsvUnLock(idx);
  }
  return arr;
}

void jspeiLoadScopesFromVar(JsVar *arr) {
    execInfo.scopeCount = 0;

    if (jsvIsArray(arr)) {
      JsvObjectIterator it;
      jsvObjectIteratorNew(&it, arr);
      while (jsvObjectIteratorHasValue(&it)) {
        execInfo.scopes[execInfo.scopeCount++] = jsvObjectIteratorGetValue(&it);
        jsvObjectIteratorNext(&it);
      }
      jsvObjectIteratorFree(&it);
    } else
      execInfo.scopes[execInfo.scopeCount++] = jsvLockAgain(arr);
}
// -----------------------------------------------
bool jspCheckStackPosition() {
  if (jsuGetFreeStack() < 512) { // giving us 512 bytes leeway
    jsExceptionHere(JSET_ERROR, "Too much recursion - the stack is about to overflow");
    jspSetInterrupted(true);
    return false;
  }
  return true;
}


// Set execFlags such that we are not executing
void jspSetNoExecute() {
  execInfo.execute = (execInfo.execute & (JsExecFlags)(int)~EXEC_RUN_MASK) | EXEC_NO;
}

void jspAppendStackTrace(JsVar *stackTrace) {
  JsvStringIterator it;
  jsvStringIteratorNew(&it, stackTrace, 0);
  jsvStringIteratorGotoEnd(&it);
  jslPrintPosition((vcbprintf_callback)jsvStringIteratorPrintfCallback, &it, execInfo.lex, execInfo.lex->tokenLastStart);
  jslPrintTokenLineMarker((vcbprintf_callback)jsvStringIteratorPrintfCallback, &it, execInfo.lex, execInfo.lex->tokenLastStart);
  jsvStringIteratorFree(&it);
}

/// We had an exception (argument is the exception's value)
void jspSetException(JsVar *value) {
  // Add the exception itself to a variable in root scope
  JsVar *exception = jsvFindChildFromString(execInfo.hiddenRoot, JSPARSE_EXCEPTION_VAR, true);
  if (exception) {
    jsvSetValueOfName(exception, value);
    jsvUnLock(exception);
  }
  // Set the exception flag
  execInfo.execute = execInfo.execute | EXEC_EXCEPTION;
  // Try and do a stack trace
  if (execInfo.lex) {
    JsVar *stackTrace = jsvObjectGetChild(execInfo.hiddenRoot, JSPARSE_STACKTRACE_VAR, JSV_STRING_0);
    if (stackTrace) {
      jsvAppendPrintf(stackTrace, " at ");
      jspAppendStackTrace(stackTrace);
      jsvUnLock(stackTrace);
      // stop us from printing the trace in the same block
      execInfo.execute = execInfo.execute | EXEC_ERROR_LINE_REPORTED;
    }
  }

}

/** Return the reported exception if there was one (and clear it) */
JsVar *jspGetException() {
  JsVar *exceptionName = jsvFindChildFromString(execInfo.hiddenRoot, JSPARSE_EXCEPTION_VAR, false);
  if (exceptionName) {
    JsVar *exception = jsvSkipName(exceptionName);
    jsvRemoveChild(execInfo.hiddenRoot, exceptionName);
    jsvUnLock(exceptionName);
    return exception;
  }
  return 0;
}

/** Return a stack trace string if there was one (and clear it) */
JsVar *jspGetStackTrace() {
  JsVar *stackTraceName = jsvFindChildFromString(execInfo.hiddenRoot, JSPARSE_STACKTRACE_VAR, false);
  if (stackTraceName) {
    JsVar *stackTrace = jsvSkipName(stackTraceName);
    jsvRemoveChild(execInfo.hiddenRoot, stackTraceName);
    jsvUnLock(stackTraceName);
    return stackTrace;
  }
  return 0;
}

// ----------------------------------------------

// we return a value so that JSP_MATCH can return 0 if it fails (if we pass 0, we just parse all args)
NO_INLINE bool jspeFunctionArguments(JsVar *funcVar) {
  JSP_MATCH('(');
  while (execInfo.lex->tk!=')') {
      if (funcVar) {
        JsVar *param = jsvAddNamedChild(funcVar, 0, jslGetTokenValueAsString(execInfo.lex));
        if (!param) { // out of memory
          jspSetError(false);
          return false;
        }
        jsvMakeFunctionParameter(param); // force this to be called a function parameter
        jsvUnLock(param);
      }
      JSP_MATCH(LEX_ID);
      if (execInfo.lex->tk!=')') JSP_MATCH(',');
  }
  JSP_MATCH(')');
  return true;
}

NO_INLINE JsVar *jspeFunctionDefinition(bool parseNamedFunction) {
  // actually parse a function... We assume that the LEX_FUNCTION and name
  // have already been parsed
  JsVar *funcVar = 0;

  bool actuallyCreateFunction = JSP_SHOULD_EXECUTE || ((execInfo.execute&EXEC_PARSE_FUNCTION_DECL)!=0);
  if (actuallyCreateFunction)
    funcVar = jsvNewWithFlags(JSV_FUNCTION);

  JsVar *functionInternalName = 0;
  if (parseNamedFunction && execInfo.lex->tk==LEX_ID) {
    // you can do `var a = function foo() { foo(); };` - so cope with this
    if (funcVar) functionInternalName = jslGetTokenValueAsVar(execInfo.lex);
    // note that we don't add it to the beginning, because it would mess up our function call code
    JSP_MATCH(LEX_ID);
  }


  // Get arguments save them to the structure
  if (!jspeFunctionArguments(funcVar)) {
    jsvUnLock(functionInternalName);
    jsvUnLock(funcVar);
    // parse failed
    return 0;
  }
  // Get the code - first parse it so we know where it stops
  JslCharPos funcBegin = jslCharPosClone(&execInfo.lex->tokenStart);
  JSP_SAVE_EXECUTE();
  jspSetNoExecute();
  jsvUnLock(jspeBlock());
  JSP_RESTORE_EXECUTE();
  // Then create var and set
  if (actuallyCreateFunction) {
    // code var
    JsVar *funcCodeVar = jslNewFromLexer(&funcBegin, (size_t)(execInfo.lex->tokenLastStart+1));
    jsvUnLock(jsvAddNamedChild(funcVar, funcCodeVar, JSPARSE_FUNCTION_CODE_NAME));
    jsvUnLock(funcCodeVar);
    // scope var
    JsVar *funcScopeVar = jspeiGetScopesAsVar();
    if (funcScopeVar) {
      jsvUnLock(jsvAddNamedChild(funcVar, funcScopeVar, JSPARSE_FUNCTION_SCOPE_NAME));
      jsvUnLock(funcScopeVar);
    }
    // if we had a function name, add it to the end
    if (functionInternalName)
      jsvUnLock(jsvObjectSetChild(funcVar, JSPARSE_FUNCTION_NAME_NAME, functionInternalName));
  }
  jslCharPosFree(&funcBegin);

  return funcVar;
}

/* Parse just the brackets of a function - and throw
 * everything away */
NO_INLINE bool jspeParseFunctionCallBrackets() {
  JSP_MATCH('(');
  while (!JSP_SHOULDNT_PARSE && execInfo.lex->tk != ')') {
    jsvUnLock(jspeAssignmentExpression());
    if (execInfo.lex->tk!=')') JSP_MATCH(',');
  }
  if (!JSP_SHOULDNT_PARSE) JSP_MATCH(')');
  return 0;
}

/** Handle a function call (assumes we've parsed the function name and we're
 * on the start bracket). 'thisArg' is the value of the 'this' variable when the
 * function is executed (it's usually the parent object)
 *
 *
 * NOTE: this does not set the execInfo flags - so if execInfo==EXEC_NO, it won't execute
 *
 * If !isParsing and arg0!=0, argument 0 is set to what is supplied (same with arg1)
 *
 * functionName is used only for error reporting - and can be 0
 */
NO_INLINE JsVar *jspeFunctionCall(JsVar *function, JsVar *functionName, JsVar *thisArg, bool isParsing, int argCount, JsVar **argPtr) {
  if (JSP_SHOULD_EXECUTE && !function) {
    jsExceptionHere(JSET_ERROR, functionName ? "Function %q not found!" : "Function not found!", functionName);
    return 0;
  }

  if (JSP_SHOULD_EXECUTE) if (!jspCheckStackPosition()) return 0; // try and ensure that we won't overflow our stack

  if (JSP_SHOULD_EXECUTE && function) {
    JsVar *functionRoot;
    JsVar *returnVarName;
    JsVar *returnVar;
    JsVar *thisVar = thisArg;
    if (!jsvIsFunction(function)) {
      jsExceptionHere(JSET_ERROR, "Expecting a function to call, got %t", function);
      return 0;
    }
    if (isParsing) JSP_MATCH('(');

    /* Ok, so we have 4 options here.
     *
     * 1: we're native.
     *   a) args have been pre-parsed, which is awesome
     *   b) we have to parse our own args into an array
     * 2: we're not native
     *   a) args were pre-parsed and we have to populate the function
     *   b) we parse our own args, which is possibly better
     */
    if (jsvIsNative(function)) { // ------------------------------------- NATIVE

      unsigned int argPtrSize = 0;
      int boundArgs = 0;
      // Add 'bound' parameters if there were any
      JsvObjectIterator it;
      jsvObjectIteratorNew(&it, function);
      JsVar *param = jsvObjectIteratorGetKey(&it);
      while (jsvIsFunctionParameter(param)) {
        if ((unsigned)argCount>=argPtrSize) {
          // allocate more space on stack if needed
          unsigned int newArgPtrSize = argPtrSize?argPtrSize*4:16;
          JsVar **newArgPtr = (JsVar**)alloca(sizeof(JsVar*)*newArgPtrSize);
          memcpy(newArgPtr, argPtr, (unsigned)argCount*sizeof(JsVar*));
          argPtr = newArgPtr;
          argPtrSize = newArgPtrSize;
        }
        // if we already had arguments - shift them up...
        int i;
        for (i=argCount-1;i>=boundArgs;i--)
            argPtr[i+1] = argPtr[i];
        // add bound argument
        argPtr[boundArgs] = jsvSkipName(param);
        argCount++;
        boundArgs++;
        jsvUnLock(param);
        jsvObjectIteratorNext(&it);
        param = jsvObjectIteratorGetKey(&it);
      }
      // check if 'this' was defined
      while (param) {
        if (jsvIsStringEqual(param, JSPARSE_FUNCTION_THIS_NAME)) {
          thisVar = jsvSkipName(param);
          break;
        }
        jsvUnLock(param);
        jsvObjectIteratorNext(&it);
        param = jsvObjectIteratorGetKey(&it);
      }
      jsvUnLock(param);
      jsvObjectIteratorFree(&it);

      // Now, if we're parsing add the rest of the arguments
      int allocatedArgCount = boundArgs;
      if (isParsing) {
        while (!JSP_SHOULDNT_PARSE && execInfo.lex->tk!=')' && execInfo.lex->tk!=LEX_EOF) {
          if ((unsigned)argCount>=argPtrSize) {
            // allocate more space on stack
            unsigned int newArgPtrSize = argPtrSize?argPtrSize*4:16;
            JsVar **newArgPtr = (JsVar**)alloca(sizeof(JsVar*)*newArgPtrSize);
            memcpy(newArgPtr, argPtr, (unsigned)argCount*sizeof(JsVar*));
            argPtr = newArgPtr;
            argPtrSize = newArgPtrSize;
          }
          argPtr[argCount++] = jsvSkipNameAndUnLock(jspeAssignmentExpression());
          if (execInfo.lex->tk!=')') JSP_MATCH_WITH_CLEANUP_AND_RETURN(',',jsvUnLockMany((unsigned)argCount, argPtr);, 0);
        }

        JSP_MATCH(')');
        allocatedArgCount = argCount;
      }

      void *nativePtr = jsvGetNativeFunctionPtr(function);

      JsVar *oldThisVar = execInfo.thisVar;
      if (thisVar)
        execInfo.thisVar = jsvRef(thisVar);
      else {
        if (nativePtr==jswrap_eval) { // eval gets to use the current scope
          /* Note: proper JS has some utterly insane code that depends on whether 
           * eval is an lvalue or not:
           * 
           * http://stackoverflow.com/questions/9107240/1-evalthis-vs-evalthis-in-javascript
           * 
           * Doing this in Espruino is quite an upheaval for that one
           * slightly insane case - so it's not implemented. */          
          if (execInfo.thisVar) execInfo.thisVar = jsvRef(execInfo.thisVar);
        } else {
          execInfo.thisVar = jsvRef(execInfo.root); // 'this' should always default to root
        }
      }



      if (nativePtr) {
        returnVar = jsnCallFunction(nativePtr, function->varData.native.argTypes, thisVar, argPtr, argCount);
      } else {
        assert(0); // in case something went horribly wrong
        returnVar = 0;
      }

      // unlock values if we locked them
      jsvUnLockMany((unsigned)allocatedArgCount, argPtr);

      /* Return to old 'this' var. No need to unlock as we never locked before */
      if (execInfo.thisVar) jsvUnRef(execInfo.thisVar);
      execInfo.thisVar = oldThisVar;

    } else { // ----------------------------------------------------- NOT NATIVE

      // create a new symbol table entry for execution of this function
      // OPT: can we cache this function execution environment + param variables?
      // OPT: Probably when calling a function ONCE, use it, otherwise when recursing, make new?
      functionRoot = jsvNewWithFlags(JSV_FUNCTION);
      if (!functionRoot) { // out of memory
        jspSetError(false);
        return 0;
      }

      JsVar *functionScope = 0;
      JsVar *functionCode = 0;
      JsVar *functionInternalName = 0;

      /** NOTE: We expect that the function object will have:
       *
       *  * Parameters
       *  * Code/Scope/Name
       *
       * IN THAT ORDER.
       */
      JsvObjectIterator it;
      jsvObjectIteratorNew(&it, function);

      JsVar *param = jsvObjectIteratorGetKey(&it);
      JsVar *value = jsvObjectIteratorGetValue(&it);
      while (jsvIsFunctionParameter(param) && value) {
        JsVar *paramName = jsvCopy(param);
        if (paramName) { // could be out of memory
          jsvMakeFunctionParameter(paramName); // force this to be called a function parameter
          jsvSetValueOfName(paramName, value);
          jsvAddName(functionRoot, paramName);
          jsvUnLock(paramName);
        } else
          jspSetError(false);
        jsvUnLock(value);
        jsvUnLock(param);
        jsvObjectIteratorNext(&it);
        param = jsvObjectIteratorGetKey(&it);
        value = jsvObjectIteratorGetValue(&it);
      }
      jsvUnLock(value);
      jsvUnLock(param);
      if (isParsing) {
        int hadParams = 0;
        // grab in all parameters. We go around this loop until we've run out
        // of named parameters AND we've parsed all the supplied arguments
        while (!JSP_SHOULDNT_PARSE && execInfo.lex->tk!=')') {
          JsVar *param = jsvObjectIteratorGetKey(&it);
          bool paramDefined = jsvIsFunctionParameter(param);
          if (execInfo.lex->tk!=')' || paramDefined) {
            hadParams++;
            JsVar *value = 0;
            // ONLY parse this if it was supplied, otherwise leave 0 (undefined)
            if (execInfo.lex->tk!=')')
              value = jspeAssignmentExpression();
            // and if execute, copy it over
            value = jsvSkipNameAndUnLock(value);
            JsVar *paramName = paramDefined ? jsvCopyNameOnly(param,false,true) : jsvNewFromEmptyString();
            if (paramName) { // could be out of memory
              jsvMakeFunctionParameter(paramName); // force this to be called a function parameter
              jsvSetValueOfName(paramName, value);
              jsvAddName(functionRoot, paramName);
              jsvUnLock(paramName);
            } else
              jspSetError(false);
            jsvUnLock(value);
            if (execInfo.lex->tk!=')') JSP_MATCH(',');
          }
          jsvUnLock(param);
          if (paramDefined) jsvObjectIteratorNext(&it);
        }
        JSP_MATCH(')');
      } else {  // and NOT isParsing
        int args = 0;
        while (args<argCount) {
          JsVar *param = jsvObjectIteratorGetKey(&it);
          bool paramDefined = jsvIsFunctionParameter(param);
          JsVar *paramName = paramDefined ? jsvCopyNameOnly(param,false,true) : jsvNewFromEmptyString();
          if (paramName) {
            jsvMakeFunctionParameter(paramName); // force this to be called a function parameter
            jsvSetValueOfName(paramName, argPtr[args]);
            jsvAddName(functionRoot, paramName);
            jsvUnLock(paramName);
          } else
            jspSetError(false);
          args++;
          jsvUnLock(param);
          if (paramDefined) jsvObjectIteratorNext(&it);
        }
      }
      // Now go through what's left
      while (jsvObjectIteratorHasValue(&it)) {
        JsVar *param = jsvObjectIteratorGetKey(&it);
        if (jsvIsString(param)) {
          if (jsvIsStringEqual(param, JSPARSE_FUNCTION_SCOPE_NAME)) functionScope = jsvSkipName(param);
          else if (jsvIsStringEqual(param, JSPARSE_FUNCTION_CODE_NAME)) functionCode = jsvSkipName(param);
          else if (jsvIsStringEqual(param, JSPARSE_FUNCTION_NAME_NAME)) functionInternalName = jsvSkipName(param);
          else if (jsvIsStringEqual(param, JSPARSE_FUNCTION_THIS_NAME)) thisVar = jsvSkipName(param);
          else if (jsvIsFunctionParameter(param)) {
            JsVar *paramName = jsvCopy(param);
            // paramName is already a name (it's a function parameter)
            if (paramName) {// could be out of memory - or maybe just not supplied!
              jsvAddName(functionRoot, paramName);
              jsvUnLock(paramName);
            }
          }
        }
        jsvUnLock(param);
        jsvObjectIteratorNext(&it);
      }
      jsvObjectIteratorFree(&it);

      // setup a the function's name (if a named function)
      if (functionInternalName) {
        JsVar *name = jsvMakeIntoVariableName(jsvNewFromStringVar(functionInternalName,0,JSVAPPENDSTRINGVAR_MAXLENGTH), function);
        jsvAddName(functionRoot, name);
        jsvUnLock(name);
        jsvUnLock(functionInternalName);
      }
      // setup a return variable
      returnVarName = jsvAddNamedChild(functionRoot, 0, JSPARSE_RETURN_VAR);
      if (!returnVarName) // out of memory
        jspSetError(false);

      if (!JSP_HAS_ERROR) {
        // save old scopes
        JsVar *oldScopes[JSPARSE_MAX_SCOPES];
        int oldScopeCount;
        int i;
        oldScopeCount = execInfo.scopeCount;
        for (i=0;i<execInfo.scopeCount;i++)
          oldScopes[i] = execInfo.scopes[i];
        // if we have a scope var, load it up. We may not have one if there were no scopes apart from root
        if (functionScope) {
            jspeiLoadScopesFromVar(functionScope);
            jsvUnLock(functionScope);
        } else {
            // no scope var defined? We have no scopes at all!
            execInfo.scopeCount = 0;
        }
        // add the function's execute space to the symbol table so we can recurse
        if (jspeiAddScope(functionRoot)) {
          /* Adding scope may have failed - we may have descended too deep - so be sure
           * not to pull somebody else's scope off
           */

          JsVar *oldThisVar = execInfo.thisVar;
          if (thisVar)
            execInfo.thisVar = jsvRef(thisVar);
          else
            execInfo.thisVar = jsvRef(execInfo.root); // 'this' should always default to root


          /* we just want to execute the block, but something could
           * have messed up and left us with the wrong ScriptLex, so
           * we want to be careful here... */
          if (functionCode) {
            JsLex *oldLex;
            JsLex newLex;
            jslInit(&newLex, functionCode);

            oldLex = execInfo.lex;
            execInfo.lex = &newLex;
            JSP_SAVE_EXECUTE();
            execInfo.execute = EXEC_YES; // force execute without any previous state
            jspeBlock();
            JsExecFlags hasError = execInfo.execute&EXEC_ERROR_MASK;
            JSP_RESTORE_EXECUTE(); // because return will probably have set execute to false
            jslKill(&newLex);
            execInfo.lex = oldLex;
            if (hasError) {
              execInfo.execute |= hasError; // propogate error
              JsVar *stackTrace = jsvObjectGetChild(execInfo.hiddenRoot, JSPARSE_STACKTRACE_VAR, JSV_STRING_0);
              if (stackTrace) {
                jsvAppendPrintf(stackTrace, jsvIsString(functionName)?"in function %q called from ":
                                                           "in function called from ", functionName);
                if (execInfo.lex) {
                  jspAppendStackTrace(stackTrace);
                } else
                  jsvAppendPrintf(stackTrace, "system\n");
                jsvUnLock(stackTrace);
              }
            }
          }

          /* Return to old 'this' var. No need to unlock as we never locked before */
          if (execInfo.thisVar) jsvUnRef(execInfo.thisVar);
          execInfo.thisVar = oldThisVar;

          jspeiRemoveScope();
        }

        // Unref old scopes
        for (i=0;i<execInfo.scopeCount;i++)
            jsvUnLock(execInfo.scopes[i]);
        // restore function scopes
        for (i=0;i<oldScopeCount;i++)
            execInfo.scopes[i] = oldScopes[i];
        execInfo.scopeCount = oldScopeCount;
      }
      jsvUnLock(functionCode);

      /* get the real return var before we remove it from our function */
      returnVar = jsvSkipNameAndUnLock(returnVarName);
      if (returnVarName) // could have failed with out of memory
        jsvSetValueOfName(returnVarName, 0); // remove return value (which helps stops circular references)
      jsvUnLock(functionRoot);
    }

    // If we grabbed a new 'this' from a bound function
    // we unlock it here
    if (thisVar != thisArg)
      jsvUnLock(thisVar);

    return returnVar;
  } else if (isParsing) { // ---------------------------------- function, but not executing - just parse args and be done
    jspeParseFunctionCallBrackets();
    /* Do not return function, as it will be unlocked! */
    return 0;
  } else return 0;
}

// Find a variable (or built-in function) based on the current scopes
JsVar *jspGetNamedVariable(const char *tokenName) {
  JsVar *a = JSP_SHOULD_EXECUTE ? jspeiFindInScopes(tokenName) : 0;
  if (JSP_SHOULD_EXECUTE && !a) {
    /* Special case! We haven't found the variable, so check out
     * and see if it's one of our builtins...  */
    if (jswIsBuiltInObject(tokenName)) {
      // Check if we have a built-in function for it
      // OPT: Could we instead have jswIsBuiltInObjectWithoutConstructor?
      JsVar *obj = jswFindBuiltInFunction(0, tokenName);
      // If not, make one
      if (!obj)
        obj = jspNewBuiltin(tokenName);
      if (obj) { // not out of memory
        a = jsvAddNamedChild(execInfo.root, obj, tokenName);
        jsvUnLock(obj);
      }
    } else {
      a = jswFindBuiltInFunction(0, tokenName);
      if (!a) {
        /* Variable doesn't exist! JavaScript says we should create it
         * (we won't add it here. This is done in the assignment operator)*/
        a = jsvMakeIntoVariableName(jsvNewFromString(tokenName), 0);
      }
    }
  }
  return a;
}

JsVar *jspeFactorSingleId() {
  JsVar *a = jspGetNamedVariable(jslGetTokenValueAsString(execInfo.lex));
  JSP_MATCH_WITH_RETURN(LEX_ID, a);
  return a;
}

/// Used by jspGetNamedField / jspGetVarNamedField
static NO_INLINE JsVar *jspGetNamedFieldInParents(JsVar *object, const char* name, bool returnName) {
  // Now look in prototypes
  JsVar * child = jspeiFindChildFromStringInParents(object, name);

  /* Check for builtins via separate function
   * This way we save on RAM for built-ins because everything comes out of program code */
  if (!child) {
    child = jswFindBuiltInFunction(object, name);
  }

  /* We didn't get here if we found a child in the object itself, so
   * if we're here then we probably have the wrong name - so for example
   * with `a.b = c;` could end up setting `a.prototype.b` (bug #360)
   *
   * Also we might have got a built-in, which wouldn't have a name on it
   * anyway - so in both cases, strip the name if it is there, and create
   * a new name.
   */
  if (child && returnName) {
    // Get rid of existing name
    child = jsvSkipNameAndUnLock(child);
    // create a new name
    JsVar *nameVar = jsvNewFromString(name);
    JsVar *newChild = jsvCreateNewChild(object, nameVar, child);
    jsvUnLock(nameVar);
    jsvUnLock(child);
    child = newChild;
  }

  // If not found and is the prototype, create it
  if (!child) {
    if (jsvIsFunction(object) && strcmp(name, JSPARSE_PROTOTYPE_VAR)==0) {
      // prototype is supposed to be an object
      JsVar *proto = jsvNewWithFlags(JSV_OBJECT);
      // make sure it has a 'constructor' variable that points to the object it was part of
      jsvObjectSetChild(proto, JSPARSE_CONSTRUCTOR_VAR, object);
      child = jsvAddNamedChild(object, proto, JSPARSE_PROTOTYPE_VAR);
      jspEnsureIsPrototype(object, child);
      jsvUnLock(proto);
    } else if (strcmp(name, JSPARSE_INHERITS_VAR)==0) {
      const char *objName = jswGetBasicObjectName(object);
      if (objName) {
        child = jspNewPrototype(objName);
      }
    }
  }

  return child;
}

/** Get the named function/variable on the object - whether it's built in, or predefined.
 * If !returnName, returns the function/variable itself or undefined, but
 * if returnName, return a name (could be fake) referencing the parent.
 *
 * NOTE: ArrayBuffer/Strings are not handled here. We assume that if we're
 * passing a char* rather than a JsVar it's because we're looking up via
 * a symbol rather than a variable. To handle these use jspGetVarNamedField  */
JsVar *jspGetNamedField(JsVar *object, const char* name, bool returnName) {

  JsVar *child = 0;
  // if we're an object (or pretending to be one)
  if (jsvHasChildren(object))
    child = jsvFindChildFromString(object, name, false);

  if (!child) {
    child = jspGetNamedFieldInParents(object, name, returnName);

    // If not found and is the prototype, create it
    if (!child && jsvIsFunction(object) && strcmp(name, JSPARSE_PROTOTYPE_VAR)==0) {
      JsVar *value = jsvNewWithFlags(JSV_OBJECT); // prototype is supposed to be an object
      child = jsvAddNamedChild(object, value, JSPARSE_PROTOTYPE_VAR);
      jsvUnLock(value);
    }
  }

  if (returnName) return child;
  else return jsvSkipNameAndUnLock(child);
}

/// see jspGetNamedField - note that nameVar should have had jsvAsArrayIndex called on it first
JsVar *jspGetVarNamedField(JsVar *object, JsVar *nameVar, bool returnName) {

  JsVar *child = 0;
  // if we're an object (or pretending to be one)
  if (jsvHasChildren(object))
    child = jsvFindChildFromVar(object, nameVar, false);

  if (!child) {
    if (jsvIsArrayBuffer(object) && jsvIsInt(nameVar)) {
      // for array buffers, we actually create a NAME, and hand that back - then when we assign (or use SkipName) we pull out the correct data
      child = jsvMakeIntoVariableName(jsvNewFromInteger(jsvGetInteger(nameVar)), object);
      if (child) // turn into an 'array buffer name'
        child->flags = (child->flags & ~JSV_VARTYPEMASK) | JSV_ARRAYBUFFERNAME;
    } else if (jsvIsString(object) && jsvIsInt(nameVar)) {
        JsVarInt idx = jsvGetInteger(nameVar);
        if (idx>=0 && idx<(JsVarInt)jsvGetStringLength(object)) {
          child = jsvNewFromEmptyString();
          if (child) jsvAppendCharacter(child, jsvGetCharInString(object, (size_t)idx));
        } else if (returnName)
          child = jsvNewWithFlags(JSV_NAME_STRING_0); // just return *something* to show this is handled
    } else {
      // get the name as a string
      char name[JSLEX_MAX_TOKEN_LENGTH];
      jsvGetString(nameVar, name, JSLEX_MAX_TOKEN_LENGTH);
      // try and find it in parents
      child = jspGetNamedFieldInParents(object, name, returnName);

      // If not found and is the prototype, create it
      if (!child && jsvIsFunction(object) && jsvIsStringEqual(nameVar, JSPARSE_PROTOTYPE_VAR)) {
        JsVar *value = jsvNewWithFlags(JSV_OBJECT); // prototype is supposed to be an object
        child = jsvAddNamedChild(object, value, JSPARSE_PROTOTYPE_VAR);
        jsvUnLock(value);
      }
    }
  }

  if (returnName) return child;
  else return jsvSkipNameAndUnLock(child);
}

/// Call the named function on the object - whether it's built in, or predefined. Returns the return value of the function.
JsVar *jspCallNamedFunction(JsVar *object, char* name, int argCount, JsVar **argPtr) {
  JsVar *child = jspGetNamedField(object, name, false);
  JsVar *r = 0;
  if (jsvIsFunction(child))
    r = jspeFunctionCall(child, 0, object, false, argCount, argPtr);
  jsvUnLock(child);
  return r;
}

NO_INLINE JsVar *jspeFactorMember(JsVar *a, JsVar **parentResult) {
  /* The parent if we're executing a method call */
  JsVar *parent = 0;

  while (execInfo.lex->tk=='.' || execInfo.lex->tk=='[') {
      if (execInfo.lex->tk == '.') { // ------------------------------------- Record Access
          JSP_ASSERT_MATCH('.');
          if (JSP_SHOULD_EXECUTE) {
            // Note: name will go away when we parse something else!
            const char *name = jslGetTokenValueAsString(execInfo.lex);

            JsVar *aVar = jsvSkipName(a);
            JsVar *child = 0;
            if (aVar)
              child = jspGetNamedField(aVar, name, true);
            if (!child) {
              if (jsvHasChildren(aVar)) {
                // if no child found, create a pointer to where it could be
                // as we don't want to allocate it until it's written
                JsVar *nameVar = jslGetTokenValueAsVar(execInfo.lex);
                child = jsvCreateNewChild(aVar, nameVar, 0);
                jsvUnLock(nameVar);
              } else {
                // could have been a string...
                jsExceptionHere(JSET_ERROR, "Field or method \"%s\" does not already exist, and can't create it on %t", name, aVar);
              }
            }
            JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(parent);jsvUnLock(a);jsvUnLock(aVar);, child);

            jsvUnLock(parent);
            parent = aVar;
            jsvUnLock(a);
            a = child;
          } else {
            // Not executing, just match
            JSP_MATCH_WITH_RETURN(LEX_ID, a);
          }
      } else if (execInfo.lex->tk == '[') { // ------------------------------------- Array Access
          JsVar *index;
          JSP_ASSERT_MATCH('[');
          index = jsvSkipNameAndUnLock(jspeAssignmentExpression());
          JSP_MATCH_WITH_CLEANUP_AND_RETURN(']', jsvUnLock(parent);jsvUnLock(index);, a);
          if (JSP_SHOULD_EXECUTE) {
            index = jsvAsArrayIndexAndUnLock(index);
            JsVar *aVar = jsvSkipName(a);
            JsVar *child = 0;
            if (aVar)
              child = jspGetVarNamedField(aVar, index, true);

            if (!child) {
              if (jsvHasChildren(aVar)) {
                // if no child found, create a pointer to where it could be
                // as we don't want to allocate it until it's written
                child = jsvCreateNewChild(aVar, index, 0);
              } else {
                jsExceptionHere(JSET_ERROR, "Field or method %q does not already exist, and can't create it on %t", index, aVar);
              }
            }
            jsvUnLock(parent);
            parent = jsvLockAgainSafe(aVar);
            jsvUnLock(a);
            a = child;
            jsvUnLock(aVar);
          }
          jsvUnLock(index);
      } else {
        assert(0);
      }
  }

  if (parentResult) *parentResult = parent;
  else jsvUnLock(parent);
  return a;
}

NO_INLINE JsVar *jspeConstruct(JsVar *func, JsVar *funcName, bool hasArgs) {
  assert(JSP_SHOULD_EXECUTE);
  if (!jsvIsFunction(func)) {
    jsExceptionHere(JSET_ERROR, "Constructor should be a function, but is %t", func);
    return 0;
  }

  JsVar *thisObj = jsvNewWithFlags(JSV_OBJECT);
  if (!thisObj) return 0; // out of memory
  // Make sure the function has a 'prototype' var
  JsVar *prototypeName = jsvFindChildFromString(func, JSPARSE_PROTOTYPE_VAR, true);
  jspEnsureIsPrototype(func, prototypeName); // make sure it's an object
  JsVar *prototypeVar = jsvSkipName(prototypeName);
  jsvUnLock(jsvAddNamedChild(thisObj, prototypeVar, JSPARSE_INHERITS_VAR));
  jsvUnLock(prototypeVar);
  jsvUnLock(prototypeName);

  JsVar *a = jspeFunctionCall(func, funcName, thisObj, hasArgs, 0, 0);

  if (a) {
    jsvUnLock(thisObj);
    thisObj = a;
  } else {
    jsvUnLock(a);
  }
  return thisObj;
}

NO_INLINE JsVar *jspeFactorFunctionCall() {
  /* The parent if we're executing a method call */
  bool isConstructor = false;
  if (execInfo.lex->tk==LEX_R_NEW) {
    JSP_ASSERT_MATCH(LEX_R_NEW);
    isConstructor = true;

    if (execInfo.lex->tk==LEX_R_NEW) {
      jsExceptionHere(JSET_ERROR, "Nesting 'new' operators is unsupported");
      jspSetError(false);
      return 0;
    }
  }

  JsVar *parent = 0;
  JsVar *a = jspeFactorMember(jspeFactor(), &parent);

  while ((execInfo.lex->tk=='(' || (isConstructor && JSP_SHOULD_EXECUTE)) && !jspIsInterrupted()) {
    JsVar *funcName = a;
    JsVar *func = jsvSkipName(funcName);

    /* The constructor function doesn't change parsing, so if we're
     * not executing, just short-cut it. */
    if (isConstructor && JSP_SHOULD_EXECUTE) {
      // If we have '(' parse an argument list, otherwise don't look for any args
      bool parseArgs = execInfo.lex->tk=='(';
      a = jspeConstruct(func, funcName, parseArgs);
      isConstructor = false; // don't treat subsequent brackets as constructors
    } else
      a = jspeFunctionCall(func, funcName, parent, true, 0, 0);

    jsvUnLock(funcName);
    jsvUnLock(func);

    jsvUnLock(parent); parent=0;
    a = jspeFactorMember(a, &parent);
  }

  jsvUnLock(parent);
  return a;
}

JsVar *jspeFactorId() {
  return jspeFactorSingleId();
}


NO_INLINE JsVar *jspeFactorObject() {
  if (JSP_SHOULD_EXECUTE) {
    JsVar *contents = jsvNewWithFlags(JSV_OBJECT);
    if (!contents) { // out of memory
      jspSetError(false);
      return 0;
    }
    /* JSON-style object definition */
    JSP_MATCH_WITH_RETURN('{', contents);
    while (!JSP_SHOULDNT_PARSE && execInfo.lex->tk != '}') {
      JsVar *varName = 0;
      // we only allow strings or IDs on the left hand side of an initialisation
      if (execInfo.lex->tk==LEX_ID) {
        if (JSP_SHOULD_EXECUTE)
          varName = jslGetTokenValueAsVar(execInfo.lex);
        JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(varName), contents);
      } else if (
          execInfo.lex->tk==LEX_STR ||
          execInfo.lex->tk==LEX_FLOAT ||
          execInfo.lex->tk==LEX_INT ||
          execInfo.lex->tk==LEX_R_TRUE ||
          execInfo.lex->tk==LEX_R_FALSE ||
          execInfo.lex->tk==LEX_R_NULL ||
          execInfo.lex->tk==LEX_R_UNDEFINED) {
        varName = jspeFactor();
      } else {
        JSP_MATCH_WITH_RETURN(LEX_ID, contents);
      }
      JSP_MATCH_WITH_CLEANUP_AND_RETURN(':', jsvUnLock(varName), contents);
      if (JSP_SHOULD_EXECUTE) {
        varName = jsvAsArrayIndexAndUnLock(varName);
        JsVar *contentsName = jsvFindChildFromVar(contents, varName, true);
        if (contentsName) {
          JsVar *value = jsvSkipNameAndUnLock(jspeAssignmentExpression()); // value can be 0 (could be undefined!)
          jsvUnLock(jsvSetValueOfName(contentsName, value));
          jsvUnLock(value);
        }
      }
      jsvUnLock(varName);
      // no need to clean here, as it will definitely be used
      if (execInfo.lex->tk != '}') JSP_MATCH_WITH_RETURN(',', contents);
    }
    JSP_MATCH_WITH_RETURN('}', contents);
    return contents;
  } else {
    // Not executing so do fast skip
    return jspeBlock();
  }
}

NO_INLINE JsVar *jspeFactorArray() {
  int idx = 0;
  JsVar *contents = 0;
  if (JSP_SHOULD_EXECUTE) {
    contents = jsvNewWithFlags(JSV_ARRAY);
    if (!contents) { // out of memory
      jspSetError(false);
      return 0;
    }
  }
  /* JSON-style array */
  JSP_MATCH_WITH_RETURN('[', contents);
  while (!JSP_SHOULDNT_PARSE && execInfo.lex->tk != ']') {
    if (JSP_SHOULD_EXECUTE) {
      // OPT: Store array indices as actual ints
      JsVar *aVar = 0;
      JsVar *indexName = 0;
      if (execInfo.lex->tk != ',') { // #287 - [,] and [1,2,,4] are allowed
        aVar = jsvSkipNameAndUnLock(jspeAssignmentExpression());
        indexName = jsvMakeIntoVariableName(jsvNewFromInteger(idx),  aVar);
      }
      if (indexName) { // could be out of memory
        jsvAddName(contents, indexName);
        jsvUnLock(indexName);
      }
      jsvUnLock(aVar);
    } else {
      jsvUnLock(jspeAssignmentExpression());
    }
    // no need to clean here, as it will definitely be used
    if (execInfo.lex->tk != ']') JSP_MATCH_WITH_RETURN(',', contents);
    idx++;
  }
  JSP_MATCH_WITH_RETURN(']', contents);
  return contents;
}

NO_INLINE void jspEnsureIsPrototype(JsVar *instanceOf, JsVar *prototypeName) {
  if (!prototypeName) return;
  JsVar *prototypeVar = jsvSkipName(prototypeName);
  if (!jsvIsObject(prototypeVar)) {
    if (!jsvIsUndefined(prototypeVar))
      jsWarn("Prototype is not an Object, so setting it to {}");    
    jsvUnLock(prototypeVar);
    prototypeVar = jsvNewWithFlags(JSV_OBJECT); // prototype is supposed to be an object
    JsVar *lastName = jsvSkipToLastName(prototypeName);
    jsvSetValueOfName(lastName, prototypeVar);
    jsvUnLock(lastName);
  }
  JsVar *constructor = jsvFindChildFromString(prototypeVar, JSPARSE_CONSTRUCTOR_VAR, true);
  if (constructor) jsvSetValueOfName(constructor, instanceOf);
  jsvUnLock(constructor);

  jsvUnLock(prototypeVar);
}

NO_INLINE JsVar *jspeFactorTypeOf() {
  JSP_ASSERT_MATCH(LEX_R_TYPEOF);
  JsVar *a = jspeUnaryExpression();
  JsVar *result = 0;
  if (JSP_SHOULD_EXECUTE) {
    a = jsvSkipNameAndUnLock(a);
    result=jsvNewFromString(jsvGetTypeOf(a));
  }
  jsvUnLock(a);
  return result;
}

NO_INLINE JsVar *jspeFactorDelete() {
  JSP_ASSERT_MATCH(LEX_R_DELETE);
  JsVar *parent = 0;
  JsVar *a = jspeFactorMember(jspeFactor(), &parent);
  JsVar *result = 0;
  if (JSP_SHOULD_EXECUTE) {
    bool ok = false;
    if (jsvIsName(a) && !jsvIsNewChild(a)) {
      // if no parent, check in root?
      if (!parent && jsvIsChild(execInfo.root, a))
        parent = jsvLockAgain(execInfo.root);

      if (parent && !jsvIsFunction(parent)) {
        // else remove properly. Now arrays don't use the last element
        // for length we don't have to care
        jsvRemoveChild(parent, a);
        ok = true;
      }
    }

    result = jsvNewFromBool(ok);
  }
  jsvUnLock(a);
  jsvUnLock(parent);
  return result;
}

NO_INLINE JsVar *jspeFactor() {
  if (execInfo.lex->tk=='(') {
    JsVar *a = 0;
    JSP_ASSERT_MATCH('(');
    if (!jspCheckStackPosition()) return 0;
    a = jspeExpression();
    if (!JSP_SHOULDNT_PARSE) JSP_MATCH_WITH_RETURN(')',a);
    return a;
  } else if (execInfo.lex->tk==LEX_R_TRUE) {
    JSP_ASSERT_MATCH(LEX_R_TRUE);
    return JSP_SHOULD_EXECUTE ? jsvNewFromBool(true) : 0;
  } else if (execInfo.lex->tk==LEX_R_FALSE) {
    JSP_ASSERT_MATCH(LEX_R_FALSE);
    return JSP_SHOULD_EXECUTE ? jsvNewFromBool(false) : 0;
  } else if (execInfo.lex->tk==LEX_R_NULL) {
    JSP_ASSERT_MATCH(LEX_R_NULL);
    return JSP_SHOULD_EXECUTE ? jsvNewWithFlags(JSV_NULL) : 0;
  } else if (execInfo.lex->tk==LEX_R_UNDEFINED) {
    JSP_ASSERT_MATCH(LEX_R_UNDEFINED);
    return 0;
  } else if (execInfo.lex->tk==LEX_ID) {
    return jspeFactorId();
  } else if (execInfo.lex->tk==LEX_INT) {
    JsVar *v = 0;
    if (JSP_SHOULD_EXECUTE) {
      v = jsvNewFromLongInteger(stringToInt(jslGetTokenValueAsString(execInfo.lex)));
    }
    JSP_ASSERT_MATCH(LEX_INT);
    return v;
  } else if (execInfo.lex->tk==LEX_FLOAT) {
    JsVar *v = 0;
    if (JSP_SHOULD_EXECUTE) {
      v = jsvNewFromFloat(stringToFloat(jslGetTokenValueAsString(execInfo.lex)));
    }
    JSP_ASSERT_MATCH(LEX_FLOAT);
    return v;
  } else if (execInfo.lex->tk==LEX_STR) {
    if (JSP_SHOULD_EXECUTE) {
      JsVar *a = jslGetTokenValueAsVar(execInfo.lex);
      JSP_ASSERT_MATCH(LEX_STR);
      return a;
    } else {
      JSP_ASSERT_MATCH(LEX_STR);
      return 0;
    }
  } else if (execInfo.lex->tk=='{') {
    return jspeFactorObject();
  } else if (execInfo.lex->tk=='[') {
    return jspeFactorArray();
  } else if (execInfo.lex->tk==LEX_R_FUNCTION) {
    JSP_ASSERT_MATCH(LEX_R_FUNCTION);
    return jspeFunctionDefinition(true);
  } else if (execInfo.lex->tk==LEX_R_THIS) {
    JSP_ASSERT_MATCH(LEX_R_THIS);
    return jsvLockAgain( execInfo.thisVar ? execInfo.thisVar : execInfo.root );
  } else if (execInfo.lex->tk==LEX_R_DELETE) {
    return jspeFactorDelete();
  } else if (execInfo.lex->tk==LEX_R_TYPEOF) {
    return jspeFactorTypeOf();
  } else if (execInfo.lex->tk==LEX_R_VOID) {
    JSP_ASSERT_MATCH(LEX_R_VOID);
    jsvUnLock(jspeFactor());
    return 0;
  }
  // Nothing we can do here... just hope it's the end...
  JSP_MATCH(LEX_EOF);
  return 0;
}

NO_INLINE JsVar *__jspePostfixExpression(JsVar *a) {
  while (execInfo.lex->tk==LEX_PLUSPLUS || execInfo.lex->tk==LEX_MINUSMINUS) {
    int op = execInfo.lex->tk;
    JSP_ASSERT_MATCH(op);
    if (JSP_SHOULD_EXECUTE) {
        JsVar *one = jsvNewFromInteger(1);
        JsVar *oldValue = jsvAsNumberAndUnLock(jsvSkipName(a)); // keep the old value (but convert to number)
        JsVar *res = jsvMathsOpSkipNames(oldValue, one, op==LEX_PLUSPLUS ? '+' : '-');
        jsvUnLock(one);

        // in-place add/subtract
        jspReplaceWith(a, res);
        jsvUnLock(res);
        // but then use the old value
        jsvUnLock(a);
        a = oldValue;
    }
  }
  return a;
}

NO_INLINE JsVar *jspePostfixExpression() {
  JsVar *a;
  // TODO: should be in jspeUnaryExpression
  if (execInfo.lex->tk==LEX_PLUSPLUS || execInfo.lex->tk==LEX_MINUSMINUS) {
      int op = execInfo.lex->tk;
      JSP_ASSERT_MATCH(op);
      a = jspePostfixExpression();
      if (JSP_SHOULD_EXECUTE) {
          JsVar *one = jsvNewFromInteger(1);
          JsVar *res = jsvMathsOpSkipNames(a, one, op==LEX_PLUSPLUS ? '+' : '-');
          jsvUnLock(one);
          // in-place add/subtract
          jspReplaceWith(a, res);
          jsvUnLock(res);
      }
  } else
    a = jspeFactorFunctionCall();
  return __jspePostfixExpression(a);
}

NO_INLINE JsVar *jspeUnaryExpression() {
    if (execInfo.lex->tk=='!' || execInfo.lex->tk=='~' || execInfo.lex->tk=='-' || execInfo.lex->tk=='+') {
      short tk = execInfo.lex->tk;
      JSP_ASSERT_MATCH(tk);
      if (!JSP_SHOULD_EXECUTE) {
        return jspeUnaryExpression();
      }
      if (tk=='!') { // logical not
        return jsvNewFromBool(!jsvGetBoolAndUnLock(jsvSkipNameAndUnLock(jspeUnaryExpression())));
      } else if (tk=='~') { // bitwise not
        return jsvNewFromInteger(~jsvGetIntegerAndUnLock(jsvSkipNameAndUnLock(jspeUnaryExpression())));
      } else if (tk=='-') { // unary minus
        return jsvNegateAndUnLock(jspeUnaryExpression()); // names already skipped
      }  else if (tk=='+') { // unary plus (convert to number)
        JsVar *v = jsvSkipNameAndUnLock(jspeUnaryExpression());
        JsVar *r = jsvAsNumber(v); // names already skipped
        jsvUnLock(v);
        return r;
      }
      assert(0);
      return 0;
    } else
      return jspePostfixExpression();
}


// Get the precedence of a BinaryExpression - or return 0 if not one
unsigned int jspeGetBinaryExpressionPrecedence(int op) {
  switch (op) {
    case LEX_OROR: return 1; break;
    case LEX_ANDAND: return 2; break;
    case '|' : return 3; break;
    case '^' : return 4; break;
    case '&' : return 5; break;
    case LEX_EQUAL:
    case LEX_NEQUAL:
    case LEX_TYPEEQUAL:
    case LEX_NTYPEEQUAL: return 6;
    case LEX_LEQUAL:
    case LEX_GEQUAL:
    case '<':
    case '>':
    case LEX_R_INSTANCEOF: return 7;
    case LEX_R_IN: return (execInfo.execute&EXEC_FOR_INIT)?0:7;
    case LEX_LSHIFT:
    case LEX_RSHIFT:
    case LEX_RSHIFTUNSIGNED: return 8;
    case '+':
    case '-': return 9;
    case '*':
    case '/':
    case '%': return 10;
    default: return 0;
  }
}

NO_INLINE JsVar *__jspeBinaryExpression(JsVar *a, unsigned int lastPrecedence) {
  /* This one's a bit strange. Basically all the ops have their own precedence, it's not
   * like & and | share the same precedence. We don't want to recurse for each one,
   * so instead we do this.
   *
   * We deal with an expression in recursion ONLY if it's of higher precedence
   * than the current one, otherwise we stick in the while loop.
   */
  unsigned int precedence = jspeGetBinaryExpressionPrecedence(execInfo.lex->tk);
  while (precedence && precedence>lastPrecedence) {
    int op = execInfo.lex->tk;
    JSP_ASSERT_MATCH(op);

    // if we have short-circuit ops, then if we know the outcome
    // we don't bother to execute the other op. Even if not
    // we need to tell mathsOp it's an & or |
    if (op==LEX_ANDAND || op==LEX_OROR) {
      bool aValue = jsvGetBoolAndUnLock(jsvSkipName(a));
      if ((!aValue && op==LEX_ANDAND) ||
          (aValue && op==LEX_OROR)) {
        // use first argument (A)
        JSP_SAVE_EXECUTE();
        jspSetNoExecute();
        jsvUnLock(__jspeBinaryExpression(jspeUnaryExpression(),precedence));
        JSP_RESTORE_EXECUTE();
      } else {
        // use second argument (B)
        jsvUnLock(a);
        a = __jspeBinaryExpression(jspeUnaryExpression(),precedence);
      }
    } else { // else it's a more 'normal' logical expression - just use Maths
      JsVar *b = __jspeBinaryExpression(jspeUnaryExpression(),precedence);
      if (JSP_SHOULD_EXECUTE) {
        if (op==LEX_R_IN) {
          JsVar *av = jsvSkipName(a); // needle
          JsVar *bv = jsvSkipName(b); // haystack
          if (jsvIsArray(bv) || jsvIsObject(bv)) { // search keys, NOT values
            av = jsvAsArrayIndexAndUnLock(av);
            JsVar *varFound = jsvFindChildFromVar( bv, av, false);
            jsvUnLock(a);
            a = jsvNewFromBool(varFound!=0);
            jsvUnLock(varFound);
          } else {// else it will be undefined
            jsExceptionHere(JSET_ERROR, "Cannot use 'in' operator to search a %t", bv);
            jsvUnLock(a);
            a = 0;
          }
          jsvUnLock(av);
          jsvUnLock(bv);
        } else if (op==LEX_R_INSTANCEOF) {
          bool inst = false;
          JsVar *av = jsvSkipName(a);
          JsVar *bv = jsvSkipName(b);
          if (!jsvIsFunction(bv)) {
            jsExceptionHere(JSET_ERROR, "Expecting a function on RHS in instanceof check, got %t", bv);
          } else {
            if (jsvIsObject(av)) {
              JsVar *bproto = jspGetNamedField(bv, JSPARSE_PROTOTYPE_VAR, false);
              JsVar *proto = jsvObjectGetChild(av, JSPARSE_INHERITS_VAR, 0);
              while (proto) {
                if (proto == bproto) inst=true;
                // search prototype chain
                JsVar *childProto = jsvObjectGetChild(proto, JSPARSE_INHERITS_VAR, 0);
                jsvUnLock(proto);
                proto = childProto;
              }
              if (jspIsConstructor(bv, "Object")) inst = true;
              jsvUnLock(bproto);
            } else {
              const char *name = jswGetBasicObjectName(av);
              if (name) {
                inst = jspIsConstructor(bv, name);
              }
            }
          }
          jsvUnLock(av);
          jsvUnLock(bv);
          jsvUnLock(a);
          a = jsvNewFromBool(inst);
        } else {  // --------------------------------------------- NORMAL
          JsVar *res = jsvMathsOpSkipNames(a, b, op);
          jsvUnLock(a); a = res;
        }
      }
      jsvUnLock(b);
    }
    precedence = jspeGetBinaryExpressionPrecedence(execInfo.lex->tk);
  }
  return a;
}

JsVar *jspeBinaryExpression() {
  return __jspeBinaryExpression(jspeUnaryExpression(),0);
}

NO_INLINE JsVar *__jspeConditionalExpression(JsVar *lhs) {
  if (execInfo.lex->tk=='?') {
    JSP_ASSERT_MATCH('?');
    if (!JSP_SHOULD_EXECUTE) {
      // just let lhs pass through
      jsvUnLock(jspeAssignmentExpression());
      JSP_MATCH(':');
      jsvUnLock(jspeAssignmentExpression());
    } else {
      bool first = jsvGetBoolAndUnLock(jsvSkipName(lhs));
      jsvUnLock(lhs);
      if (first) {
        lhs = jspeAssignmentExpression();
        JSP_MATCH(':');
        JSP_SAVE_EXECUTE();
        jspSetNoExecute();
        jsvUnLock(jspeAssignmentExpression());
        JSP_RESTORE_EXECUTE();
      } else {
        JSP_SAVE_EXECUTE();
        jspSetNoExecute();
        jsvUnLock(jspeAssignmentExpression());
        JSP_RESTORE_EXECUTE();
        JSP_MATCH(':');
        lhs = jspeAssignmentExpression();
      }
    }
  }

  return lhs;
}

JsVar *jspeConditionalExpression() {
  return __jspeConditionalExpression(jspeBinaryExpression());
}

NO_INLINE JsVar *__jspeAssignmentExpression(JsVar *lhs) {
    if (execInfo.lex->tk=='=' || execInfo.lex->tk==LEX_PLUSEQUAL || execInfo.lex->tk==LEX_MINUSEQUAL ||
                                 execInfo.lex->tk==LEX_MULEQUAL || execInfo.lex->tk==LEX_DIVEQUAL || execInfo.lex->tk==LEX_MODEQUAL ||
                                 execInfo.lex->tk==LEX_ANDEQUAL || execInfo.lex->tk==LEX_OREQUAL ||
                                 execInfo.lex->tk==LEX_XOREQUAL || execInfo.lex->tk==LEX_RSHIFTEQUAL ||
                                 execInfo.lex->tk==LEX_LSHIFTEQUAL || execInfo.lex->tk==LEX_RSHIFTUNSIGNEDEQUAL) {
        JsVar *rhs;
        /* If we're assigning to this and we don't have a parent,
         * add it to the symbol table root as per JavaScript. */
        if (JSP_SHOULD_EXECUTE && lhs && !jsvGetRefs(lhs)) {
          if (jsvIsName(lhs)) {
            if (!jsvIsArrayBufferName(lhs) && !jsvIsNewChild(lhs))
              jsvAddName(execInfo.root, lhs);
          } else // TODO: Why was this here? can it happen?
            jsWarnAt("Trying to assign to an un-named type\n", execInfo.lex, execInfo.lex->tokenLastStart);
        }

        int op = execInfo.lex->tk;
        JSP_ASSERT_MATCH(op);
        rhs = jspeAssignmentExpression();
        rhs = jsvSkipNameAndUnLock(rhs); // ensure we get rid of any references on the RHS
        if (JSP_SHOULD_EXECUTE && lhs) {
            if (op=='=') {
                jspReplaceWith(lhs, rhs);
            } else {
                if (op==LEX_PLUSEQUAL) op='+';
                else if (op==LEX_MINUSEQUAL) op='-';
                else if (op==LEX_MULEQUAL) op='*';
                else if (op==LEX_DIVEQUAL) op='/';
                else if (op==LEX_MODEQUAL) op='%';
                else if (op==LEX_ANDEQUAL) op='&';
                else if (op==LEX_OREQUAL) op='|';
                else if (op==LEX_XOREQUAL) op='^';
                else if (op==LEX_RSHIFTEQUAL) op=LEX_RSHIFT;
                else if (op==LEX_LSHIFTEQUAL) op=LEX_LSHIFT;
                else if (op==LEX_RSHIFTUNSIGNEDEQUAL) op=LEX_RSHIFTUNSIGNED;
                if (op=='+' && jsvIsName(lhs)) {
                  JsVar *currentValue = jsvSkipName(lhs);
                  if (jsvIsString(currentValue) && jsvGetRefs(currentValue)==1) {
                    /* A special case for string += where this is the only use of the string,
                     * as we may be able to do a simple append (rather than clone + append)*/
                    JsVar *str = jsvAsString(rhs, false);
                    jsvAppendStringVarComplete(currentValue, str);
                    jsvUnLock(str);
                    op = 0;
                  }
                  jsvUnLock(currentValue);
                }
                if (op) {
                  /* Fallback which does a proper add */
                  JsVar *res = jsvMathsOpSkipNames(lhs,rhs,op);
                  jspReplaceWith(lhs, res);
                  jsvUnLock(res);
                }
            }
        }
        jsvUnLock(rhs);
    }
    return lhs;
}


JsVar *jspeAssignmentExpression() {
  return __jspeAssignmentExpression(jspeConditionalExpression());
}

// ',' is allowed to add multiple expressions, this is not allowed in jspeAssignmentExpression
NO_INLINE JsVar *jspeExpression() {
  while (!JSP_SHOULDNT_PARSE) {
  JsVar *a = jspeAssignmentExpression();
  if (execInfo.lex->tk!=',') return a;
    // if we get a comma, we just forget this data and parse the next bit...
    jsvUnLock(a);
    JSP_ASSERT_MATCH(',');
  }
  return 0;
}

NO_INLINE JsVar *jspeBlock() {
  JSP_MATCH('{');
  if (JSP_SHOULD_EXECUTE) {
    while (execInfo.lex->tk && execInfo.lex->tk!='}') {
      jsvUnLock(jspeStatement());
      if (JSP_HAS_ERROR) {
        if (execInfo.lex && !(execInfo.execute&EXEC_ERROR_LINE_REPORTED)) {
          execInfo.execute = (JsExecFlags)(execInfo.execute | EXEC_ERROR_LINE_REPORTED);
          JsVar *stackTrace = jsvObjectGetChild(execInfo.hiddenRoot, JSPARSE_STACKTRACE_VAR, JSV_STRING_0);
          if (stackTrace) {
            jsvAppendPrintf(stackTrace, "at ");
            jspAppendStackTrace(stackTrace);
            jsvUnLock(stackTrace);
          }
        }
      }
      if (JSP_SHOULDNT_PARSE)
        return 0;
    }
    JSP_MATCH('}');
  } else {
    // fast skip of blocks
    int brackets = 1;
    while (execInfo.lex->tk && brackets) {
      if (execInfo.lex->tk == '{') brackets++;
      if (execInfo.lex->tk == '}') brackets--;
      JSP_ASSERT_MATCH(execInfo.lex->tk);
    }
  }
  return 0;
}

NO_INLINE JsVar *jspeBlockOrStatement() {
    if (execInfo.lex->tk=='{') 
       return jspeBlock();
    else {
       JsVar *v = jspeStatement();
       if (execInfo.lex->tk==';') JSP_ASSERT_MATCH(';');
       return v;
    }
}

NO_INLINE JsVar *jspeStatementVar() {
  JsVar *lastDefined = 0;
   /* variable creation. TODO - we need a better way of parsing the left
    * hand side. Maybe just have a flag called can_create_var that we
    * set and then we parse as if we're doing a normal equals.*/
  JSP_ASSERT_MATCH(LEX_R_VAR);
   bool hasComma = true; // for first time in loop
   while (hasComma && execInfo.lex->tk == LEX_ID && !jspIsInterrupted()) {
     JsVar *a = 0;
     if (JSP_SHOULD_EXECUTE) {
       a = jspeiFindOnTop(jslGetTokenValueAsString(execInfo.lex), true);
       if (!a) { // out of memory
         jspSetError(false);
         return lastDefined;
       }
     }
     JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(a), lastDefined);
     // now do stuff defined with dots
     while (execInfo.lex->tk == '.') {
         JSP_MATCH_WITH_CLEANUP_AND_RETURN('.', jsvUnLock(a), lastDefined);
         if (JSP_SHOULD_EXECUTE) {
             JsVar *lastA = a;
             a = jsvFindChildFromString(lastA, jslGetTokenValueAsString(execInfo.lex), true);
             jsvUnLock(lastA);
         }
         JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(a), lastDefined);
     }
     // sort out initialiser
     if (execInfo.lex->tk == '=') {
         JsVar *var;
         JSP_MATCH_WITH_CLEANUP_AND_RETURN('=', jsvUnLock(a), lastDefined);
         var = jsvSkipNameAndUnLock(jspeAssignmentExpression());
         if (JSP_SHOULD_EXECUTE)
             jspReplaceWith(a, var);
         jsvUnLock(var);
     }
     jsvUnLock(lastDefined);
     lastDefined = a;
     hasComma = execInfo.lex->tk == ',';
     if (hasComma) JSP_MATCH_WITH_RETURN(',', lastDefined);
   }
   return lastDefined;
}

NO_INLINE JsVar *jspeStatementIf() {
  bool cond;
  JsVar *var;
  JSP_ASSERT_MATCH(LEX_R_IF);
  JSP_MATCH('(');
  var = jspeExpression();
  if (JSP_SHOULDNT_PARSE) return var;
  JSP_MATCH(')');
  cond = JSP_SHOULD_EXECUTE && jsvGetBoolAndUnLock(jsvSkipName(var));
  jsvUnLock(var);

  JSP_SAVE_EXECUTE();
  if (!cond) jspSetNoExecute();
  jsvUnLock(jspeBlockOrStatement());
  if (!cond) JSP_RESTORE_EXECUTE();
  if (execInfo.lex->tk==LEX_R_ELSE) {
    JSP_ASSERT_MATCH(LEX_R_ELSE);
    JSP_SAVE_EXECUTE();
    if (cond) jspSetNoExecute();
    jsvUnLock(jspeBlockOrStatement());
    if (cond) JSP_RESTORE_EXECUTE();
  }
  return 0;
}

NO_INLINE JsVar *jspeStatementSwitch() {
  JSP_ASSERT_MATCH(LEX_R_SWITCH);
  JSP_MATCH('(');
  JsVar *switchOn = jspeAssignmentExpression();
  JSP_MATCH_WITH_CLEANUP_AND_RETURN(')', jsvUnLock(switchOn), 0);
  JSP_MATCH_WITH_CLEANUP_AND_RETURN('{', jsvUnLock(switchOn), 0);
  JSP_SAVE_EXECUTE();
  bool execute = JSP_SHOULD_EXECUTE;
  bool hasExecuted = false;
  if (execute) execInfo.execute=EXEC_NO|EXEC_IN_SWITCH;
  while (execInfo.lex->tk==LEX_R_CASE) {
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_R_CASE, jsvUnLock(switchOn), 0);
    JsExecFlags oldFlags = execInfo.execute;
    if (execute) execInfo.execute=EXEC_YES|EXEC_IN_SWITCH;
    JsVar *test = jspeAssignmentExpression();
    execInfo.execute = oldFlags|EXEC_IN_SWITCH;;
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(':', jsvUnLock(switchOn);jsvUnLock(test), 0);
    bool cond = false;
    if (execute)
      cond = jsvGetBoolAndUnLock(jsvMathsOpSkipNames(switchOn, test, LEX_TYPEEQUAL));
    if (cond) hasExecuted = true;
    jsvUnLock(test);
    if (cond && (execInfo.execute&EXEC_RUN_MASK)==EXEC_NO)
      execInfo.execute=EXEC_YES|EXEC_IN_SWITCH;
    while (!JSP_SHOULDNT_PARSE && execInfo.lex->tk!=LEX_EOF && execInfo.lex->tk!=LEX_R_CASE && execInfo.lex->tk!=LEX_R_DEFAULT && execInfo.lex->tk!='}')
      jsvUnLock(jspeBlockOrStatement());
  }
  jsvUnLock(switchOn);
  if (execute && (execInfo.execute&EXEC_RUN_MASK)==EXEC_BREAK)
    execInfo.execute=EXEC_YES|EXEC_IN_SWITCH;
  JSP_RESTORE_EXECUTE();

  if (execInfo.lex->tk==LEX_R_DEFAULT) {
    JSP_ASSERT_MATCH(LEX_R_DEFAULT);
    JSP_MATCH(':');
    JSP_SAVE_EXECUTE();
    if (hasExecuted) jspSetNoExecute();
    else execInfo.execute |= EXEC_IN_SWITCH;
    while (!JSP_SHOULDNT_PARSE && execInfo.lex->tk!=LEX_EOF && execInfo.lex->tk!='}')
      jsvUnLock(jspeBlockOrStatement());
    if (execute && !hasExecuted)
        execInfo.execute = execInfo.execute & (JsExecFlags)~EXEC_BREAK;
    JSP_RESTORE_EXECUTE();
  }
  JSP_MATCH('}');
  return 0;
}

NO_INLINE JsVar *jspeStatementDoOrWhile(bool isWhile) {
#ifdef JSPARSE_MAX_LOOP_ITERATIONS
  int loopCount = JSPARSE_MAX_LOOP_ITERATIONS;
#endif
  JsVar *cond;
  bool loopCond = true; // true for do...while loops
  bool hasHadBreak = false;
  JslCharPos whileCondStart;
  // We do repetition by pulling out the string representing our statement
  // there's definitely some opportunity for optimisation here
  JSP_ASSERT_MATCH(isWhile ? LEX_R_WHILE : LEX_R_DO);
  if (isWhile) { // while loop
    JSP_MATCH('(');
    whileCondStart = jslCharPosClone(&execInfo.lex->tokenStart);
    cond = jspeAssignmentExpression();
    loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolAndUnLock(jsvSkipName(cond));
    jsvUnLock(cond);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(')',jslCharPosFree(&whileCondStart);,0);
  }
  JslCharPos whileBodyStart = jslCharPosClone(&execInfo.lex->tokenStart);
  JSP_SAVE_EXECUTE();
  // actually try and execute first bit of while loop (we'll do the rest in the actual loop later)
  if (!loopCond) jspSetNoExecute();
  execInfo.execute |= EXEC_IN_LOOP;
  jsvUnLock(jspeBlockOrStatement());

  execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
  if (execInfo.execute & EXEC_CONTINUE)
    execInfo.execute = EXEC_YES;
  else if (execInfo.execute & EXEC_BREAK) {
    execInfo.execute = EXEC_YES;
    hasHadBreak = true; // fail loop condition, so we exit
  }
  if (!loopCond) JSP_RESTORE_EXECUTE();

  if (!isWhile) { // do..while loop
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_R_WHILE,jslCharPosFree(&whileBodyStart);,0);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN('(',jslCharPosFree(&whileBodyStart);if (isWhile)jslCharPosFree(&whileCondStart);,0);
    whileCondStart = jslCharPosClone(&execInfo.lex->tokenStart);
    cond = jspeAssignmentExpression();
    loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolAndUnLock(jsvSkipName(cond));
    jsvUnLock(cond);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(')',jslCharPosFree(&whileBodyStart);jslCharPosFree(&whileCondStart);,0);
  }

  JslCharPos whileBodyEnd;
  whileBodyEnd = jslCharPosClone(&execInfo.lex->tokenStart);

  while (!hasHadBreak && loopCond
#ifdef JSPARSE_MAX_LOOP_ITERATIONS
         && loopCount-->0
#endif
         ) {
      jslSeekToP(execInfo.lex, &whileCondStart);
      cond = jspeAssignmentExpression();
      loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolAndUnLock(jsvSkipName(cond));
      jsvUnLock(cond);
      if (loopCond) {
          jslSeekToP(execInfo.lex, &whileBodyStart);
          execInfo.execute |= EXEC_IN_LOOP;
          jsvUnLock(jspeBlockOrStatement());
          execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
          if (execInfo.execute & EXEC_CONTINUE)
            execInfo.execute = EXEC_YES;
          else if (execInfo.execute & EXEC_BREAK) {
            execInfo.execute = EXEC_YES;
            hasHadBreak = true;
          }
      }
  }
  jslSeekToP(execInfo.lex, &whileBodyEnd);
  jslCharPosFree(&whileCondStart);
  jslCharPosFree(&whileBodyStart);
  jslCharPosFree(&whileBodyEnd);
#ifdef JSPARSE_MAX_LOOP_ITERATIONS
  if (loopCount<=0) {
    jsExceptionHere(JSET_ERROR, "WHILE Loop exceeded the maximum number of iterations (" STRINGIFY(JSPARSE_MAX_LOOP_ITERATIONS) ")");
  }
#endif
  return 0;
}

NO_INLINE JsVar *jspeStatementFor() {
  JSP_ASSERT_MATCH(LEX_R_FOR);
  JSP_MATCH('(');
  bool wasInLoop = (execInfo.execute&EXEC_IN_LOOP)!=0;
  execInfo.execute |= EXEC_FOR_INIT;
  // initialisation
  JsVar *forStatement = 0;
  // we could have 'for (;;)' - so don't munch up our semicolon if that's all we have
  if (execInfo.lex->tk != ';') 
    forStatement = jspeStatement();
  if (jspIsInterrupted()) {
    jsvUnLock(forStatement);
    return 0;
  }
  execInfo.execute &= (JsExecFlags)~EXEC_FOR_INIT;
  if (execInfo.lex->tk == LEX_R_IN) {
    // for (i in array)
    // where i = jsvUnLock(forStatement);
    if (JSP_SHOULD_EXECUTE && !jsvIsName(forStatement)) {
      jsvUnLock(forStatement);
      jsExceptionHere(JSET_ERROR, "FOR a IN b - 'a' must be a variable name, not %t", forStatement);
      return 0;
    }
    bool addedIteratorToScope = false;
    if (JSP_SHOULD_EXECUTE && !jsvGetRefs(forStatement)) {
      // if the variable did not exist, add it to the scope
      addedIteratorToScope = true;
      jsvAddName(execInfo.root, forStatement);
    }
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_R_IN, jsvUnLock(forStatement), 0);
    JsVar *array = jsvSkipNameAndUnLock(jspeExpression());
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(')', jsvUnLock(forStatement);jsvUnLock(array), 0);
    JslCharPos forBodyStart = jslCharPosClone(&execInfo.lex->tokenStart);
    JSP_SAVE_EXECUTE();
    jspSetNoExecute();
    execInfo.execute |= EXEC_IN_LOOP;
    jsvUnLock(jspeBlockOrStatement());
    JslCharPos forBodyEnd = jslCharPosClone(&execInfo.lex->tokenStart);
    if (!wasInLoop) execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
    JSP_RESTORE_EXECUTE();

    if (JSP_SHOULD_EXECUTE) {
      if (jsvIsIterable(array)) {
        JsvIsInternalChecker checkerFunction = jsvGetInternalFunctionCheckerFor(array);
        JsVar *foundPrototype = 0;

        JsvIterator it;
        jsvIteratorNew(&it, array);
        bool hasHadBreak = false;
        while (JSP_SHOULD_EXECUTE && jsvIteratorHasElement(&it) && !hasHadBreak) {
            JsVar *loopIndexVar = jsvIteratorGetKey(&it);
            bool ignore = false;
            if (checkerFunction && checkerFunction(loopIndexVar)) {
              ignore = true;
              if (jsvIsString(loopIndexVar) &&
                  jsvIsStringEqual(loopIndexVar, JSPARSE_INHERITS_VAR))
                foundPrototype = jsvSkipName(loopIndexVar);
            }
            if (!ignore) {
              JsVar *indexValue = jsvIsName(loopIndexVar) ?
                                    jsvCopyNameOnly(loopIndexVar, false/*no copy children*/, false/*not a name*/) :
                                    loopIndexVar;
              if (indexValue) { // could be out of memory
                assert(!jsvIsName(indexValue) && jsvGetRefs(indexValue)==0);
                jsvSetValueOfName(forStatement, indexValue);
                if (indexValue!=loopIndexVar) jsvUnLock(indexValue);

                jsvIteratorNext(&it);

                jslSeekToP(execInfo.lex, &forBodyStart);
                execInfo.execute |= EXEC_IN_LOOP;
                jsvUnLock(jspeBlockOrStatement());
                if (!wasInLoop) execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
  
                if (execInfo.execute & EXEC_CONTINUE)
                  execInfo.execute = EXEC_YES;
                else if (execInfo.execute & EXEC_BREAK) {
                  execInfo.execute = EXEC_YES;
                  hasHadBreak = true;
                }
              }
            } else
              jsvIteratorNext(&it);
            jsvUnLock(loopIndexVar);

            if (!jsvIteratorHasElement(&it) && foundPrototype) {
              jsvIteratorFree(&it);
              jsvIteratorNew(&it, foundPrototype);
              jsvUnLock(foundPrototype);
              foundPrototype = 0;
            }
        }
        assert(!foundPrototype);
        jsvIteratorFree(&it);
      } else if (!jsvIsUndefined(array)) {
        jsExceptionHere(JSET_ERROR, "FOR loop can only iterate over Arrays, Strings or Objects, not %t", array);
      }
    }
    jslSeekToP(execInfo.lex, &forBodyEnd);
    jslCharPosFree(&forBodyStart);
    jslCharPosFree(&forBodyEnd);

    if (addedIteratorToScope) {
      jsvRemoveChild(execInfo.root, forStatement);
    }
    jsvUnLock(forStatement);
    jsvUnLock(array);
  } else { // ----------------------------------------------- NORMAL FOR LOOP
#ifdef JSPARSE_MAX_LOOP_ITERATIONS
    int loopCount = JSPARSE_MAX_LOOP_ITERATIONS;
#endif
    bool loopCond = true;
    bool hasHadBreak = false;

    jsvUnLock(forStatement);
    JSP_MATCH(';');
    JslCharPos forCondStart = jslCharPosClone(&execInfo.lex->tokenStart);
    if (execInfo.lex->tk != ';') {
      JsVar *cond = jspeAssignmentExpression(); // condition
      loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolAndUnLock(jsvSkipName(cond));
      jsvUnLock(cond);
    }
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(';',jslCharPosFree(&forCondStart);,0);
    JslCharPos forIterStart = jslCharPosClone(&execInfo.lex->tokenStart);
    if (execInfo.lex->tk != ')')  { // we could have 'for (;;)'
      JSP_SAVE_EXECUTE();
      jspSetNoExecute();
      jsvUnLock(jspeExpression()); // iterator
      JSP_RESTORE_EXECUTE();
    }
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(')',jslCharPosFree(&forCondStart);jslCharPosFree(&forIterStart);,0);

    JslCharPos forBodyStart = jslCharPosClone(&execInfo.lex->tokenStart); // actual for body
    JSP_SAVE_EXECUTE();
    if (!loopCond) jspSetNoExecute();
    execInfo.execute |= EXEC_IN_LOOP;
    jsvUnLock(jspeBlockOrStatement());
    JslCharPos forBodyEnd = jslCharPosClone(&execInfo.lex->tokenStart);
    if (!wasInLoop) execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
    if (loopCond || !JSP_SHOULD_EXECUTE) {
      if (execInfo.execute & EXEC_CONTINUE)
        execInfo.execute = EXEC_YES;
      else if (execInfo.execute & EXEC_BREAK) {
        execInfo.execute = EXEC_YES;
        hasHadBreak = true;
      }
    }
    if (!loopCond) JSP_RESTORE_EXECUTE();
    if (loopCond) {
        jslSeekToP(execInfo.lex, &forIterStart);
        if (execInfo.lex->tk != ')') jsvUnLock(jspeExpression());
    }
    while (!hasHadBreak && JSP_SHOULD_EXECUTE && loopCond
#ifdef JSPARSE_MAX_LOOP_ITERATIONS
           && loopCount-->0
#endif
           ) {
        jslSeekToP(execInfo.lex, &forCondStart);
        ;
        if (execInfo.lex->tk == ';') {
          loopCond = true;
        } else {
          JsVar *cond = jspeAssignmentExpression();
          loopCond = jsvGetBoolAndUnLock(jsvSkipName(cond));
          jsvUnLock(cond);
        }
        if (JSP_SHOULD_EXECUTE && loopCond) {
            jslSeekToP(execInfo.lex, &forBodyStart);
            execInfo.execute |= EXEC_IN_LOOP;
            jsvUnLock(jspeBlockOrStatement());
            if (!wasInLoop) execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
            if (execInfo.execute & EXEC_CONTINUE)
              execInfo.execute = EXEC_YES;
            else if (execInfo.execute & EXEC_BREAK) {
              execInfo.execute = EXEC_YES;
              hasHadBreak = true;
            }
        }
        if (JSP_SHOULD_EXECUTE && loopCond && !hasHadBreak) {
            jslSeekToP(execInfo.lex, &forIterStart);
            if (execInfo.lex->tk != ')') jsvUnLock(jspeExpression());
        }
    }
    jslSeekToP(execInfo.lex, &forBodyEnd);

    jslCharPosFree(&forCondStart);
    jslCharPosFree(&forIterStart);
    jslCharPosFree(&forBodyStart);
    jslCharPosFree(&forBodyEnd);

#ifdef JSPARSE_MAX_LOOP_ITERATIONS
    if (loopCount<=0) {
      jsExceptionHere(JSET_ERROR, "FOR Loop exceeded the maximum number of iterations ("STRINGIFY(JSPARSE_MAX_LOOP_ITERATIONS)")");
    }
#endif
  }
  return 0;
}

NO_INLINE JsVar *jspeStatementTry() {
  // execute the try block
  JSP_ASSERT_MATCH(LEX_R_TRY);
  bool shouldExecuteBefore = JSP_SHOULD_EXECUTE;
  jspeBlock();
  bool hadException = shouldExecuteBefore && ((execInfo.execute & EXEC_EXCEPTION)!=0);

  bool hadCatch = false;
  if (execInfo.lex->tk == LEX_R_CATCH) {
    JSP_ASSERT_MATCH(LEX_R_CATCH);
    hadCatch = true;
    JSP_MATCH('(');
    JsVar *exceptionVar = 0;
    if (hadException)
      exceptionVar = jspeiFindOnTop(jslGetTokenValueAsString(execInfo.lex), true);
    JSP_MATCH(LEX_ID);
    JSP_MATCH(')');
    if (exceptionVar) {
      // set the exception var up properly
      JsVar *actualExceptionName = jsvFindChildFromString(execInfo.hiddenRoot, JSPARSE_EXCEPTION_VAR, false);
      if (actualExceptionName) {
        JsVar *actualException = jsvSkipName(actualExceptionName);
        jsvSetValueOfName(exceptionVar, actualException);
        jsvUnLock(actualException);
        // remove the actual exception
        jsvRemoveChild(execInfo.hiddenRoot, actualExceptionName);
        jsvUnLock(actualExceptionName);
        // remove any stack trace
        jsvRemoveNamedChild(execInfo.hiddenRoot, JSPARSE_STACKTRACE_VAR);
        // Now clear the exception flag (it's handled - we hope!)
        execInfo.execute = execInfo.execute & (JsExecFlags)~EXEC_EXCEPTION;
      }
      jsvUnLock(exceptionVar);
    }
    JSP_SAVE_EXECUTE();
    if (shouldExecuteBefore && !hadException) jspSetNoExecute();
    jspeBlock();
    JSP_RESTORE_EXECUTE();
  }
  if (execInfo.lex->tk == LEX_R_FINALLY || (!hadCatch && ((execInfo.execute&(EXEC_ERROR|EXEC_INTERRUPTED))==0))) {
    JSP_MATCH(LEX_R_FINALLY);
    // clear the exception flag - but only momentarily!
    if (hadException) execInfo.execute = execInfo.execute & (JsExecFlags)~EXEC_EXCEPTION;
    jspeBlock();
    // put the flag back!
    if (hadException && !hadCatch) execInfo.execute = execInfo.execute | EXEC_EXCEPTION;
  }
  return 0;
}

NO_INLINE JsVar *jspeStatementReturn() {
  JsVar *result = 0;
  JSP_ASSERT_MATCH(LEX_R_RETURN);
  if (execInfo.lex->tk != ';' && execInfo.lex->tk != '}') {
    // we only want the value, so skip the name if there was one
    result = jsvSkipNameAndUnLock(jspeExpression());
  }
  if (JSP_SHOULD_EXECUTE) {
    JsVar *resultVar = jspeiFindOnTop(JSPARSE_RETURN_VAR, false);
    if (resultVar) {
      jspReplaceWith(resultVar, result);
      jsvUnLock(resultVar);
      jspSetNoExecute(); // Stop anything else in this function executing
    } else {
      jsExceptionHere(JSET_SYNTAXERROR, "RETURN statement, but not in a function.\n");
    }
  }
  jsvUnLock(result);
  return 0;
}

NO_INLINE JsVar *jspeStatementThrow() {
  JsVar *result = 0;
  JSP_ASSERT_MATCH(LEX_R_THROW);
  result = jsvSkipNameAndUnLock(jspeExpression());
  if (JSP_SHOULD_EXECUTE) {
    jspSetException(result); // Stop anything else in this function executing
  }
  jsvUnLock(result);
  return 0;
}

NO_INLINE JsVar *jspeStatementFunctionDecl() {
  JsVar *funcName = 0;
  JsVar *funcVar;
  JSP_ASSERT_MATCH(LEX_R_FUNCTION);

  bool actuallyCreateFunction = JSP_SHOULD_EXECUTE || ((execInfo.execute&EXEC_PARSE_FUNCTION_DECL)!=0);
  if (actuallyCreateFunction) {
    funcName = jsvMakeIntoVariableName(jslGetTokenValueAsVar(execInfo.lex), 0);
    if (!funcName) { // out of memory
      jspSetError(false);
      return 0;
    }
  }
  JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(funcName), 0);
  funcVar = jspeFunctionDefinition(false);
  if (actuallyCreateFunction) {
    // find a function with the same name (or make one)
    // OPT: can Find* use just a JsVar that is a 'name'?
    JsVar *existingName = jspeiFindNameOnTop(funcName, true);
    JsVar *existingFunc = jsvSkipName(existingName);
    if (jsvIsFunction(existingFunc)) {
      // 'proper' replace, that keeps the original function var and swaps the children
      funcVar = jsvSkipNameAndUnLock(funcVar);
      jswrap_function_replaceWith(existingFunc, funcVar);
      jsvUnLock(funcVar);
      funcVar = existingName;
    } else {
      jspReplaceWith(existingName, funcVar);
      jsvUnLock(funcName);
      funcName = existingName;
    }
    jsvUnLock(existingFunc);
    // existingName is used - don't UnLock
  }
  jsvUnLock(funcVar);
  return funcName;
}

NO_INLINE JsVar *jspeStatement() {
    if (execInfo.lex->tk==LEX_ID ||
        execInfo.lex->tk==LEX_INT ||
        execInfo.lex->tk==LEX_FLOAT ||
        execInfo.lex->tk==LEX_STR ||
        execInfo.lex->tk==LEX_R_NEW ||
        execInfo.lex->tk==LEX_R_NULL ||
        execInfo.lex->tk==LEX_R_UNDEFINED ||
        execInfo.lex->tk==LEX_R_TRUE ||
        execInfo.lex->tk==LEX_R_FALSE ||
        execInfo.lex->tk==LEX_R_THIS ||
        execInfo.lex->tk==LEX_R_DELETE ||
        execInfo.lex->tk==LEX_R_TYPEOF ||
        execInfo.lex->tk==LEX_R_VOID ||
        execInfo.lex->tk==LEX_PLUSPLUS ||
        execInfo.lex->tk==LEX_MINUSMINUS ||
        execInfo.lex->tk=='!' ||
        execInfo.lex->tk=='-' ||
        execInfo.lex->tk=='+' ||
        execInfo.lex->tk=='~' ||
        execInfo.lex->tk=='[' ||
        execInfo.lex->tk=='(') {
        /* Execute a simple statement that only contains basic arithmetic... */
      return jspeExpression();
    } else if (execInfo.lex->tk=='{') {
      /* A block of code */
      return jspeBlock();
    } else if (execInfo.lex->tk==';') {
      /* Empty statement - to allow things like ;;; */
      JSP_ASSERT_MATCH(';');
      return 0;
    } else if (execInfo.lex->tk==LEX_R_VAR) {
      return jspeStatementVar();
    } else if (execInfo.lex->tk==LEX_R_IF) {
      return jspeStatementIf();
    } else if (execInfo.lex->tk==LEX_R_DO) {
      return jspeStatementDoOrWhile(false);
    } else if (execInfo.lex->tk==LEX_R_WHILE) {
      return jspeStatementDoOrWhile(true);
    } else if (execInfo.lex->tk==LEX_R_FOR) {
      return jspeStatementFor();
    } else if (execInfo.lex->tk==LEX_R_TRY) {
      return jspeStatementTry();
    } else if (execInfo.lex->tk==LEX_R_RETURN) {
      return jspeStatementReturn();
    } else if (execInfo.lex->tk==LEX_R_THROW) {
      return jspeStatementThrow();
    } else if (execInfo.lex->tk==LEX_R_FUNCTION) {
      return jspeStatementFunctionDecl();
    } else if (execInfo.lex->tk==LEX_R_CONTINUE) {
      JSP_ASSERT_MATCH(LEX_R_CONTINUE);
      if (JSP_SHOULD_EXECUTE) {
        if (!(execInfo.execute & EXEC_IN_LOOP))
          jsExceptionHere(JSET_SYNTAXERROR, "CONTINUE statement outside of FOR or WHILE loop");
        else
          execInfo.execute = (execInfo.execute & (JsExecFlags)~EXEC_RUN_MASK) | EXEC_CONTINUE;
      }
    } else if (execInfo.lex->tk==LEX_R_BREAK) {
      JSP_ASSERT_MATCH(LEX_R_BREAK);
      if (JSP_SHOULD_EXECUTE) {
        if (!(execInfo.execute & (EXEC_IN_LOOP|EXEC_IN_SWITCH)))
          jsExceptionHere(JSET_SYNTAXERROR, "BREAK statement outside of SWITCH, FOR or WHILE loop");
        else
          execInfo.execute = (execInfo.execute & (JsExecFlags)~EXEC_RUN_MASK) | EXEC_BREAK;
      }
    } else if (execInfo.lex->tk==LEX_R_SWITCH) {
      return jspeStatementSwitch();
    } else JSP_MATCH(LEX_EOF);
    return 0;
}

// -----------------------------------------------------------------------------
/// Create a new built-in object that jswrapper can use to check for built-in functions
JsVar *jspNewBuiltin(const char *instanceOf) {
  JsVar *objFunc = jswFindBuiltInFunction(0, instanceOf);
  if (!objFunc) return 0; // out of memory
  return objFunc;
}

/// Create a new Class of the given instance and return its prototype
NO_INLINE JsVar *jspNewPrototype(const char *instanceOf) {
  JsVar *objFuncName = jsvFindChildFromString(execInfo.root, instanceOf, true);
  if (!objFuncName) // out of memory
    return 0;

  JsVar *objFunc = jsvSkipName(objFuncName);
  if (!objFunc) {
    objFunc = jspNewBuiltin(instanceOf);
    if (!objFunc) { // out of memory
      jsvUnLock(objFuncName);
      return 0;
    }

    // set up name
    jsvSetValueOfName(objFuncName, objFunc);
  }

  JsVar *prototypeName = jsvFindChildFromString(objFunc, JSPARSE_PROTOTYPE_VAR, true);
  jspEnsureIsPrototype(objFunc, prototypeName); // make sure it's an object
  jsvUnLock(objFunc);
  jsvUnLock(objFuncName);

  return prototypeName;
}

/** Create a new object of the given instance and add it to root with name 'name'.
 * If name!=0, added to root with name, and the name is returned
 * If name==0, not added to root and Object itself returned */
NO_INLINE JsVar *jspNewObject(const char *name, const char *instanceOf) {
  JsVar *prototypeName = jspNewPrototype(instanceOf);

  JsVar *obj = jsvNewWithFlags(JSV_OBJECT);
  if (!obj) { // out of memory
    jsvUnLock(prototypeName);
    return 0;
  }
  if (name) {
    // If it's a device, set the device number up as the Object data
    // See jsiGetDeviceFromClass
    IOEventFlags device = jshFromDeviceString(name);
    if (device!=EV_NONE) {
      obj->varData.str[0] = 'D';
      obj->varData.str[1] = 'E';
      obj->varData.str[2] = 'V';
      obj->varData.str[3] = (char)device;
    }

  }
  // add __proto__
  JsVar *prototypeVar = jsvSkipName(prototypeName);
  jsvUnLock(jsvAddNamedChild(obj, prototypeVar, JSPARSE_INHERITS_VAR));
  jsvUnLock(prototypeVar);
  jsvUnLock(prototypeName);prototypeName=0;

  if (name) {
    JsVar *objName = jsvFindChildFromString(execInfo.root, name, true);
    if (objName) jsvSetValueOfName(objName, obj);
    jsvUnLock(obj);
    if (!objName) { // out of memory
      return 0;
    }
    return objName;
  } else
    return obj;
}

/** Returns true if the constructor function given is the same as that
 * of the object with the given name. */
bool jspIsConstructor(JsVar *constructor, const char *constructorName) {
  JsVar *objFunc = jsvObjectGetChild(execInfo.root, constructorName, 0);
  if (!objFunc) return false;
  bool isConstructor = objFunc == constructor;
  jsvUnLock(objFunc);
  return isConstructor;
}

// -----------------------------------------------------------------------------

void jspSoftInit() {
  execInfo.root = jsvFindOrCreateRoot();
  // Root now has a lock and a ref
  execInfo.hiddenRoot = jsvObjectGetChild(execInfo.root, JS_HIDDEN_CHAR_STR, JSV_OBJECT);
}

void jspSoftKill() {
  jsvUnLock(execInfo.hiddenRoot);
  execInfo.hiddenRoot = 0;
  jsvUnLock(execInfo.root);
  execInfo.root = 0;
  // Root is now left with just a ref
}

void jspInit() {
  jspSoftInit();
}

void jspKill() {
  jspSoftKill();
  // Unreffing this should completely kill everything attached to root
  JsVar *r = jsvFindOrCreateRoot();
  jsvUnRef(r);
  jsvUnLock(r);
}


/** Execute code form a variable and return the result. If parseTwice is set,
 * we run over the variable twice - once to pick out function declarations,
 * and once to actually execute.  */
JsVar *jspEvaluateVar(JsVar *str, JsVar *scope, bool parseTwice) {
  JsLex lex;
  JsVar *v = 0;

  assert(jsvIsString(str));
  jslInit(&lex, str);

  JSP_SAVE_EXECUTE();
  JsExecInfo oldExecInfo = execInfo;
  execInfo.lex = &lex;
  execInfo.execute = EXEC_YES;
  bool scopeAdded = false;
  if (scope) {
    // if we're adding a scope, make sure it's the *only* scope
    execInfo.scopeCount = 0;
    scopeAdded = jspeiAddScope(scope);
  }

  if (parseTwice) {
    JsExecFlags oldFlags = execInfo.execute;
    execInfo.execute = EXEC_PARSE_FUNCTION_DECL;
    while (!JSP_SHOULDNT_PARSE && execInfo.lex->tk != LEX_EOF) {
      jsvUnLock(v);
      v = jspeBlockOrStatement();
    }
    jslReset(execInfo.lex); // back to beginning
    execInfo.execute = oldFlags; // old flags
  }

  while (!JSP_SHOULDNT_PARSE && execInfo.lex->tk != LEX_EOF) {
    jsvUnLock(v);
    v = jspeBlockOrStatement();
  }
  // clean up
  if (scopeAdded)
    jspeiRemoveScope();
  jslKill(&lex);

  // restore state and execInfo
  JSP_RESTORE_EXECUTE();
  oldExecInfo.execute = execInfo.execute; // JSP_RESTORE_EXECUTE has made this ok.
  execInfo = oldExecInfo;

  // It may have returned a reference, but we just want the value...
  if (v) {
    return jsvSkipNameAndUnLock(v);
  }
  // nothing returned
  return 0;
}

JsVar *jspEvaluate(const char *str, bool parseTwice) {
  JsVar *v = 0;

  JsVar *evCode = jsvNewFromString(str);
  if (!jsvIsMemoryFull())
    v = jspEvaluateVar(evCode, 0, parseTwice);
  jsvUnLock(evCode);

  return v;
}

JsVar *jspExecuteFunction(JsVar *func, JsVar *thisArg, int argCount, JsVar **argPtr) {
  JSP_SAVE_EXECUTE();
  JsExecInfo oldExecInfo = execInfo;

  jspeiInit(0);
  JsVar *result = jspeFunctionCall(func, 0, thisArg, false, argCount, argPtr);
  // clean up
  jspeiKill();
  // restore state
  JSP_RESTORE_EXECUTE();
  oldExecInfo.execute = execInfo.execute; // JSP_RESTORE_EXECUTE has made this ok.
  execInfo = oldExecInfo;

  return result;
}


/// Evaluate a JavaScript module and return its exports
JsVar *jspEvaluateModule(JsVar *moduleContents) {
  assert(jsvIsString(moduleContents));
  JsVar *scope = jsvNewWithFlags(JSV_OBJECT);
  if (!scope) return 0; // out of mem
  JsVar *scopeExports = jsvNewWithFlags(JSV_OBJECT);
  if (!scopeExports) { jsvUnLock(scope); return 0; } // out of mem
  JsVar *exportsName = jsvAddNamedChild(scope, scopeExports, "exports");
  jsvUnLock(scopeExports);
  jsvUnLock(jsvAddNamedChild(scope, scope, "module"));

  // TODO: maybe we do want to parse twice here, to get functions defined after their use?
  JsVar *oldThisVar = execInfo.thisVar;
  execInfo.thisVar = scopeExports; // set 'this' variable to exports
  jsvUnLock(jspEvaluateVar(moduleContents, scope, false));
  execInfo.thisVar = oldThisVar;

  jsvUnLock(scope);
  return jsvSkipNameAndUnLock(exportsName);
}

/** Get the owner of the current prototype. We assume that it's
 * the first item in the array, because that's what we will
 * have added when we created it. It's safe to call this on
 * non-prototypes and non-objects.  */
JsVar *jspGetPrototypeOwner(JsVar *proto) {
  if (jsvIsObject(proto) || jsvIsArray(proto)) {
    return jsvSkipNameAndUnLock(jsvObjectGetChild(proto, JSPARSE_CONSTRUCTOR_VAR, 0));
  }
  return 0;
}
