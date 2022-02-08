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
#include "jswrap_json.h" // for jsfPrintJSON
#include "jswrap_espruino.h" // for jswrap_espruino_memoryArea
#ifndef SAVE_ON_FLASH
#include "jswrap_regexp.h" // for jswrap_regexp_constructor
#endif

/* Info about execution when Parsing - this saves passing it on the stack
 * for each call */
JsExecInfo execInfo;

// ----------------------------------------------- Forward decls
JsVar *jspeAssignmentExpression();
JsVar *jspeExpression();
JsVar *jspeUnaryExpression();
void jspeBlock();
void jspeBlockNoBrackets();
JsVar *jspeStatement();
JsVar *jspeFactor();
void jspEnsureIsPrototype(JsVar *instanceOf, JsVar *prototypeName);
#ifndef SAVE_ON_FLASH
JsVar *jspeArrowFunction(JsVar *funcVar, JsVar *a);
#endif
// ----------------------------------------------- Utils
#define JSP_MATCH_WITH_CLEANUP_AND_RETURN(TOKEN, CLEANUP_CODE, RETURN_VAL) { if (!jslMatch((TOKEN))) { CLEANUP_CODE; return RETURN_VAL; } }
#define JSP_MATCH_WITH_RETURN(TOKEN, RETURN_VAL) JSP_MATCH_WITH_CLEANUP_AND_RETURN(TOKEN, , RETURN_VAL)
#define JSP_MATCH(TOKEN) JSP_MATCH_WITH_CLEANUP_AND_RETURN(TOKEN, , 0) // Match where the user could have given us the wrong token
#define JSP_ASSERT_MATCH(TOKEN) { assert(lex->tk==(TOKEN));jslGetNextToken(); } // Match where if we have the wrong token, it's an internal error
#define JSP_SHOULD_EXECUTE (((execInfo.execute)&EXEC_RUN_MASK)==EXEC_YES)
#define JSP_SAVE_EXECUTE() JsExecFlags oldExecute = execInfo.execute
#define JSP_RESTORE_EXECUTE() execInfo.execute = (execInfo.execute&(JsExecFlags)(~EXEC_SAVE_RESTORE_MASK)) | (oldExecute&EXEC_SAVE_RESTORE_MASK);
#define JSP_HAS_ERROR (((execInfo.execute)&EXEC_ERROR_MASK)!=0)
#define JSP_SHOULDNT_PARSE (((execInfo.execute)&EXEC_NO_PARSE_MASK)!=0)

ALWAYS_INLINE void jspDebuggerLoopIfCtrlC() {
#ifdef USE_DEBUGGER
  if (execInfo.execute & EXEC_CTRL_C_WAIT && JSP_SHOULD_EXECUTE)
    jsiDebuggerLoop();
#endif
}

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
void jspeiClearScopes() {
  jsvUnLock(execInfo.scopesVar);
  execInfo.scopesVar = 0;
}

bool jspeiAddScope(JsVar *scope) {
  if (!execInfo.scopesVar)
    execInfo.scopesVar = jsvNewEmptyArray();
  if (!execInfo.scopesVar) return false;
  jsvArrayPush(execInfo.scopesVar, scope);
  return true;
}

void jspeiRemoveScope() {
  if (!execInfo.scopesVar || !jsvGetArrayLength(execInfo.scopesVar)) {
    jsExceptionHere(JSET_INTERNALERROR, "Too many scopes removed");
    jspSetError(false);
    return;
  }
  jsvUnLock(jsvArrayPop(execInfo.scopesVar));
  if (!jsvGetFirstChild(execInfo.scopesVar)) {
    jsvUnLock(execInfo.scopesVar);
    execInfo.scopesVar = 0;
  }
}

JsVar *jspeiFindInScopes(const char *name) {
  if (execInfo.scopesVar) {
    JsVar *it = jsvLockSafe(jsvGetLastChild(execInfo.scopesVar));
    while (it) {
      JsVar *scope = jsvSkipName(it);
      JsVarRef next = jsvGetPrevSibling(it);
      JsVar *ref = jsvFindChildFromString(scope, name, false);
      jsvUnLock2(it, scope);
      if (ref) return ref;
      it = jsvLockSafe(next);
    }
  }
  return jsvFindChildFromString(execInfo.root, name, false);
}
/// Return the topmost scope (and lock it)
JsVar *jspeiGetTopScope() {
  if (execInfo.scopesVar) {
    JsVar *scope = jsvGetLastArrayItem(execInfo.scopesVar);
    if (scope) return scope;
  }
  return jsvLockAgain(execInfo.root);
}
JsVar *jspeiFindOnTop(const char *name, bool createIfNotFound) {
  JsVar *scope = jspeiGetTopScope();
  JsVar *result = jsvFindChildFromString(scope, name, createIfNotFound);
  jsvUnLock(scope);
  return result;
}
JsVar *jspeiFindNameOnTop(JsVar *childName, bool createIfNotFound) {
  JsVar *scope = jspeiGetTopScope();
  JsVar *result = jsvFindChildFromVar(scope, childName, createIfNotFound);
  jsvUnLock(scope);
  return result;
}

JsVar *jspFindPrototypeFor(const char *className) {
  JsVar *obj = jsvObjectGetChild(execInfo.root, className, 0);
  if (!obj) return 0;
  JsVar *proto = jsvObjectGetChild(obj, JSPARSE_PROTOTYPE_VAR, 0);
  jsvUnLock(obj);
  return proto;
}

/** Here we assume that we have already looked in the parent itself -
 * and are now going down looking at the stuff it inherited */
JsVar *jspeiFindChildFromStringInParents(JsVar *parent, const char *name) {
  if (jsvIsObject(parent)) {
    // If an object, look for an 'inherits' var
    JsVar *inheritsFrom = jsvObjectGetChild(parent, JSPARSE_INHERITS_VAR, 0);

    // if there's no inheritsFrom, just default to 'Object.prototype'
    if (!inheritsFrom)
      inheritsFrom = jspFindPrototypeFor("Object");

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
  if (!execInfo.scopesVar) return 0; // no scopes!
  // If just one element, return it (no array)
  if (jsvGetArrayLength(execInfo.scopesVar)==1) {
    JsVar *v = jsvGetLastArrayItem(execInfo.scopesVar); // this is faster than getting by index
    return v;
  }
  // Copy this - because if we just returned it, the underlying array would get altered
  return jsvCopy(execInfo.scopesVar, true);
}

void jspeiLoadScopesFromVar(JsVar *arr) {
  jsvUnLock(execInfo.scopesVar);
  execInfo.scopesVar = 0;
  if (arr) {
    if (jsvIsArray(arr)) {
      // TODO: copy on write? would make function calls faster
      execInfo.scopesVar = jsvCopy(arr, true);
    } else {
      // just a single item,but we must package it in an array
      execInfo.scopesVar = jsvNewArray(&arr, 1);
    }
  }
}
// -----------------------------------------------
/// Check that we have enough stack to recurse. Return true if all ok, error if not.
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
  jslPrintPosition((vcbprintf_callback)jsvStringIteratorPrintfCallback, &it, lex->tokenLastStart);
  jslPrintTokenLineMarker((vcbprintf_callback)jsvStringIteratorPrintfCallback, &it, lex->tokenLastStart, 0);
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
  if (lex) {
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

    JsVar *stack = jspGetStackTrace();
    if (stack && jsvHasChildren(exception)) {
      jsvObjectSetChild(exception, "stack", stack);
    }
    jsvUnLock(stack);

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
  while (lex->tk!=')') {
    if (funcVar) {
      char buf[JSLEX_MAX_TOKEN_LENGTH+1];
      buf[0] = '\xFF';
      strcpy(&buf[1], jslGetTokenValueAsString());
      JsVar *param = jsvAddNamedChild(funcVar, 0, buf);
      if (!param) { // out of memory
        jspSetError(false);
        return false;
      }
      jsvMakeFunctionParameter(param); // force this to be called a function parameter
      jsvUnLock(param);
    }
    JSP_MATCH(LEX_ID);
    if (lex->tk!=')') JSP_MATCH(',');
  }
  JSP_MATCH(')');
  return true;
}

// Parse function, assuming we're on '{'. funcVar can be 0. returns 'true' is the function included the 'this' keyword
NO_INLINE bool jspeFunctionDefinitionInternal(JsVar *funcVar, bool expressionOnly) {
  bool forcePretokenise = false;

  if (expressionOnly) {
    if (funcVar)
      funcVar->flags = (funcVar->flags & ~JSV_VARTYPEMASK) | JSV_FUNCTION_RETURN;
  } else {
    JSP_MATCH('{');
  #ifndef SAVE_ON_FLASH
    if (lex->tk==LEX_STR) {
      if (!strcmp(jslGetTokenValueAsString(), "compiled"))
        jsWarn("Function marked with \"compiled\" uploaded in source form");
      if (lex->tk==LEX_STR && !strcmp(jslGetTokenValueAsString(), "ram")) {
        JSP_ASSERT_MATCH(LEX_STR);
        forcePretokenise = true;
      }
    }
  #endif

    /* If the function starts with return, treat it specially -
     * we don't want to store the 'return' part of it
     */
    if (funcVar && lex->tk==LEX_R_RETURN) {
      funcVar->flags = (funcVar->flags & ~JSV_VARTYPEMASK) | JSV_FUNCTION_RETURN;
      JSP_ASSERT_MATCH(LEX_R_RETURN);
    }
  }
#ifndef ESPR_NO_LINE_NUMBERS
  // Get the line number (if needed)
  JsVarInt lineNumber = 0;
  if (funcVar && lex->lineNumberOffset && !(forcePretokenise||jsfGetFlag(JSF_PRETOKENISE))) {
    // jslGetLineNumber is slow, so we only do it if we have debug info
    lineNumber = (JsVarInt)jslGetLineNumber() + (JsVarInt)lex->lineNumberOffset - 1;
  }
#endif
  // Get the code - parse it and figure out where it stops
  JslCharPos funcBegin;
  jslSkipWhiteSpace();
  jslCharPosNew(&funcBegin, lex->sourceVar, lex->tokenStart);
  int lastTokenEnd = -1;
  lex->hadThisKeyword = lex->tk == LEX_R_THIS;
  if (!expressionOnly) {
    int brackets = 0;
    while (lex->tk && (brackets || lex->tk != '}')) {
      if (lex->tk == '{') brackets++;
      if (lex->tk == '}') brackets--;
      lastTokenEnd = (int)jsvStringIteratorGetIndex(&lex->it)-1;
      JSP_ASSERT_MATCH(lex->tk);
    }
    // FIXME: we might be including whitespace after the last token
  } else {
    JsExecFlags oldExec = execInfo.execute;
    execInfo.execute = EXEC_NO;
    jsvUnLock(jspeAssignmentExpression());
    execInfo.execute = oldExec;
    lastTokenEnd = (int)lex->tokenStart;
  }
  bool hadThisKeyword = lex->hadThisKeyword;
  // Then create var and set (if there was any code!)
  if (funcVar && lastTokenEnd>0) {
    // code var
    JsVar *funcCodeVar;
    if (!forcePretokenise && jsvIsNativeString(lex->sourceVar)) {
      /* If we're parsing from a Native String (eg. E.memoryArea, E.setBootCode) then
      use another Native String to load function code straight from flash */
      int s = (int)jsvStringIteratorGetIndex(&funcBegin.it) - 1;
      funcCodeVar = jsvNewNativeString(lex->sourceVar->varData.nativeStr.ptr + s, (unsigned int)(lastTokenEnd - s));
#ifdef SPIFLASH_BASE
    } else if (!forcePretokenise && jsvIsFlashString(lex->sourceVar)) {
        /* If we're parsing from a Flash String (eg. loaded from Storage on Bangle.js) then
      use another Flash String to load function code straight from flash*/
        int s = (int)jsvStringIteratorGetIndex(&funcBegin.it) - 1;
        funcCodeVar = jsvNewFlashString(lex->sourceVar->varData.nativeStr.ptr + s, (unsigned int)(lastTokenEnd - s));
#endif
    } else {
      if (jsfGetFlag(JSF_PRETOKENISE) || forcePretokenise) {
        funcCodeVar = jslNewTokenisedStringFromLexer(&funcBegin, (size_t)lastTokenEnd);
      } else {
        funcCodeVar = jslNewStringFromLexer(&funcBegin, (size_t)lastTokenEnd);
      }
    }
    jsvUnLock2(jsvAddNamedChild(funcVar, funcCodeVar, JSPARSE_FUNCTION_CODE_NAME), funcCodeVar);
    // scope var
    JsVar *funcScopeVar = jspeiGetScopesAsVar();
    if (funcScopeVar) {
      jsvUnLock2(jsvAddNamedChild(funcVar, funcScopeVar, JSPARSE_FUNCTION_SCOPE_NAME), funcScopeVar);
    }
#ifndef ESPR_NO_LINE_NUMBERS
    // If we've got a line number, add a var for it
    if (lineNumber) {
      JsVar *funcLineNumber = jsvNewFromInteger(lineNumber);
      if (funcLineNumber) {
        jsvUnLock2(jsvAddNamedChild(funcVar, funcLineNumber, JSPARSE_FUNCTION_LINENUMBER_NAME), funcLineNumber);
      }
    }
#endif
  }

  jslCharPosFree(&funcBegin);
  if (!expressionOnly) JSP_MATCH('}');
  return hadThisKeyword;
}

// Parse function (after 'function' has occurred
NO_INLINE JsVar *jspeFunctionDefinition(bool parseNamedFunction) {
  // actually parse a function... We assume that the LEX_FUNCTION and name
  // have already been parsed
  JsVar *funcVar = 0;

  bool actuallyCreateFunction = JSP_SHOULD_EXECUTE;
  if (actuallyCreateFunction)
    funcVar = jsvNewWithFlags(JSV_FUNCTION);

  JsVar *functionInternalName = 0;
  if (parseNamedFunction && lex->tk==LEX_ID) {
    // you can do `var a = function foo() { foo(); };` - so cope with this
    if (funcVar) functionInternalName = jslGetTokenValueAsVar();
    // note that we don't add it to the beginning, because it would mess up our function call code
    JSP_ASSERT_MATCH(LEX_ID);
  }

  // Get arguments save them to the structure
  if (!jspeFunctionArguments(funcVar)) {
    jsvUnLock2(functionInternalName, funcVar);
    // parse failed
    return 0;
  }

  // Parse the actual function block
  jspeFunctionDefinitionInternal(funcVar, false);

  // if we had a function name, add it to the end (if we don't it gets confused with arguments)
  if (funcVar && functionInternalName)
    jsvObjectSetChildAndUnLock(funcVar, JSPARSE_FUNCTION_NAME_NAME, functionInternalName);

  return funcVar;
}

/* Parse just the brackets of a function - and throw
 * everything away */
NO_INLINE bool jspeParseFunctionCallBrackets() {
  assert(!JSP_SHOULD_EXECUTE);
  JSP_MATCH('(');
  while (!JSP_SHOULDNT_PARSE && lex->tk != ')') {
    jsvUnLock(jspeAssignmentExpression());
#ifndef SAVE_ON_FLASH
    if (lex->tk==LEX_ARROW_FUNCTION) {
      jsvUnLock(jspeArrowFunction(0, 0));
    }
#endif
    if (lex->tk!=')') JSP_MATCH(',');
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
    if (functionName)
      jsExceptionHere(JSET_ERROR, "Function %q not found!", functionName);
    else
      jsExceptionHere(JSET_ERROR, "Function not found!", functionName);
    return 0;
  }

  if (JSP_SHOULD_EXECUTE) if (!jspCheckStackPosition()) return 0; // try and ensure that we won't overflow our stack

  if (JSP_SHOULD_EXECUTE && function) {
    JsVar *returnVar = 0;

    if (!jsvIsFunction(function)) {
      jsExceptionHere(JSET_ERROR, "Expecting a function to call, got %t", function);
      return 0;
    }
    JsVar *thisVar = jsvLockAgainSafe(thisArg);
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
    if (jsvIsNativeFunction(function)) { // ------------------------------------- NATIVE
      unsigned int argPtrSize = 0;
      int boundArgs = 0;
      // Add 'bound' parameters if there were any
      JsvObjectIterator it;
      jsvObjectIteratorNew(&it, function);
      JsVar *param = jsvObjectIteratorGetKey(&it);
      while (jsvIsFunctionParameter(param)) {
        if ((unsigned)argCount>=argPtrSize) {
          // allocate more space on stack if needed
          unsigned int newArgPtrSize = (argPtrSize?argPtrSize:(unsigned int)argCount)*4;
          size_t newArgPtrByteSize = sizeof(JsVar*)*newArgPtrSize;
          if (jsuGetFreeStack() < 256+newArgPtrByteSize) {
            jsExceptionHere(JSET_ERROR, "Insufficient stack for this many arguments");
            jsvUnLock(thisVar);
            return 0;
          }
          JsVar **newArgPtr = (JsVar**)alloca(newArgPtrByteSize);
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
          jsvUnLock(thisVar);
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
        while (!JSP_HAS_ERROR && lex->tk!=')' && lex->tk!=LEX_EOF) {
          if ((unsigned)argCount>=argPtrSize) {
            // allocate more space on stack
            unsigned int newArgPtrSize = argPtrSize?argPtrSize*4:16;
            JsVar **newArgPtr = (JsVar**)alloca(sizeof(JsVar*)*newArgPtrSize);
            memcpy(newArgPtr, argPtr, (unsigned)argCount*sizeof(JsVar*));
            argPtr = newArgPtr;
            argPtrSize = newArgPtrSize;
          }
          argPtr[argCount++] = jsvSkipNameAndUnLock(jspeAssignmentExpression());
          if (lex->tk!=')') JSP_MATCH_WITH_CLEANUP_AND_RETURN(',',jsvUnLockMany((unsigned)argCount, argPtr);jsvUnLock(thisVar);, 0);
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



      if (nativePtr && !JSP_HAS_ERROR) {
        returnVar = jsnCallFunction(nativePtr, function->varData.native.argTypes, thisVar, argPtr, argCount);
        assert(!jsvIsName(returnVar));
      } else {
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
      JsVar *functionRoot = jsvNewWithFlags(JSV_FUNCTION);
      if (!functionRoot) { // out of memory
        jspSetError(false);
        jsvUnLock(thisVar);
        return 0;
      }

      JsVar *functionScope = 0;
      JsVar *functionCode = 0;
      JsVar *functionInternalName = 0;
#ifndef ESPR_NO_LINE_NUMBERS
      uint16_t functionLineNumber = 0;
#endif

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
        jsvAddFunctionParameter(functionRoot, jsvNewFromStringVar(param,1,JSVAPPENDSTRINGVAR_MAXLENGTH), value);
        jsvUnLock2(value, param);
        jsvObjectIteratorNext(&it);
        param = jsvObjectIteratorGetKey(&it);
        value = jsvObjectIteratorGetValue(&it);
      }
      jsvUnLock2(value, param);
      if (isParsing) {
        int hadParams = 0;
        // grab in all parameters. We go around this loop until we've run out
        // of named parameters AND we've parsed all the supplied arguments
        while (!JSP_SHOULDNT_PARSE && lex->tk!=')') {
          JsVar *param = jsvObjectIteratorGetKey(&it);
          bool paramDefined = jsvIsFunctionParameter(param);
          if (lex->tk!=')' || paramDefined) {
            hadParams++;
            JsVar *value = 0;
            // ONLY parse this if it was supplied, otherwise leave 0 (undefined)
            if (lex->tk!=')')
              value = jspeAssignmentExpression();
            // and if execute, copy it over
            value = jsvSkipNameAndUnLock(value);
            jsvAddFunctionParameter(functionRoot, paramDefined?jsvNewFromStringVar(param,1,JSVAPPENDSTRINGVAR_MAXLENGTH):0, value);
            jsvUnLock(value);
            if (lex->tk!=')') JSP_MATCH(',');
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
          jsvAddFunctionParameter(functionRoot, paramDefined?jsvNewFromStringVar(param,1,JSVAPPENDSTRINGVAR_MAXLENGTH):0, argPtr[args]);
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
          else if (jsvIsStringEqual(param, JSPARSE_FUNCTION_THIS_NAME)) {
            jsvUnLock(thisVar);
            thisVar = jsvSkipName(param);
          }
#ifndef ESPR_NO_LINE_NUMBERS
          else if (jsvIsStringEqual(param, JSPARSE_FUNCTION_LINENUMBER_NAME)) functionLineNumber = (uint16_t)jsvGetIntegerAndUnLock(jsvSkipName(param));
#endif
          else if (jsvIsFunctionParameter(param)) {
            JsVar *defaultVal = jsvSkipName(param);
            jsvAddFunctionParameter(functionRoot, jsvNewFromStringVar(param,1,JSVAPPENDSTRINGVAR_MAXLENGTH), defaultVal);
            jsvUnLock(defaultVal);
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
        jsvUnLock2(name, functionInternalName);
      }

      if (!JSP_HAS_ERROR) {
        // save old scopes and reset scope list
        JsVar *oldScopeVar = execInfo.scopesVar;
        execInfo.scopesVar = 0;
        // if we have a scope var, load it up. We may not have one if there were no scopes apart from root
        if (functionScope) {
          jspeiLoadScopesFromVar(functionScope);
          jsvUnLock(functionScope);
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
           * have messed up and left us with the wrong Lexer, so
           * we want to be careful here... */
          if (functionCode) {
#ifdef USE_DEBUGGER
            bool hadDebuggerNextLineOnly = false;

            if (execInfo.execute&EXEC_DEBUGGER_STEP_INTO) {
	      if (functionName)
		jsiConsolePrintf("Stepping into %v\n", functionName);
	      else
		jsiConsolePrintf("Stepping into function\n", functionName);
            } else {
              hadDebuggerNextLineOnly = execInfo.execute&EXEC_DEBUGGER_NEXT_LINE;
              if (hadDebuggerNextLineOnly)
                execInfo.execute &= (JsExecFlags)~EXEC_DEBUGGER_NEXT_LINE;
            }
#endif


            JsLex newLex;
            JsLex *oldLex = jslSetLex(&newLex);
            jslInit(functionCode);
#ifndef ESPR_NO_LINE_NUMBERS
            newLex.lineNumberOffset = functionLineNumber;
#endif
            JSP_SAVE_EXECUTE();
            // force execute without any previous state
#ifdef USE_DEBUGGER
            execInfo.execute = EXEC_YES | (execInfo.execute&(EXEC_CTRL_C_MASK|EXEC_ERROR_MASK|EXEC_DEBUGGER_NEXT_LINE));
#else
            execInfo.execute = EXEC_YES | (execInfo.execute&(EXEC_CTRL_C_MASK|EXEC_ERROR_MASK));
#endif
            if (jsvIsFunctionReturn(function)) {
              #ifdef USE_DEBUGGER
                // we didn't parse a statement so wouldn't trigger the debugger otherwise
                if (execInfo.execute&EXEC_DEBUGGER_NEXT_LINE && JSP_SHOULD_EXECUTE) {
                  lex->tokenLastStart = lex->tokenStart;
                  jsiDebuggerLoop();
                }
              #endif
              // implicit return - we just need an expression (optional)
              if (lex->tk != ';' && lex->tk != '}')
                returnVar = jsvSkipNameAndUnLock(jspeExpression());
            } else {
              // setup a return variable
              JsVar *returnVarName = jsvAddNamedChild(functionRoot, 0, JSPARSE_RETURN_VAR);
              // parse the whole block
              jspeBlockNoBrackets();
              /* get the real return var before we remove it from our function.
               * We can unlock below because returnVarName is still part of
               * functionRoot, so won't get freed. */
              returnVar = jsvSkipNameAndUnLock(returnVarName);
              if (returnVarName) // could have failed with out of memory
                jsvSetValueOfName(returnVarName, 0); // remove return value (which helps stops circular references)
            }
            // Store a stack trace if we had an error
            JsExecFlags hasError = execInfo.execute&EXEC_ERROR_MASK;
            JSP_RESTORE_EXECUTE(); // because return will probably have set execute to false

#ifdef USE_DEBUGGER
            bool calledDebugger = false;
            if (execInfo.execute & EXEC_DEBUGGER_MASK) {
              jsiConsolePrint("Value returned is =");
              jsfPrintJSON(returnVar, JSON_LIMIT | JSON_SOME_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES);
              jsiConsolePrintChar('\n');
              if (execInfo.execute & EXEC_DEBUGGER_FINISH_FUNCTION) {
                calledDebugger = true;
                jsiDebuggerLoop();
              }
            }
            if (hadDebuggerNextLineOnly && !calledDebugger)
              execInfo.execute |= EXEC_DEBUGGER_NEXT_LINE;
#endif

            jslKill();
            jslSetLex(oldLex);

            if (hasError) {
              execInfo.execute |= hasError; // propogate error
              JsVar *stackTrace = jsvObjectGetChild(execInfo.hiddenRoot, JSPARSE_STACKTRACE_VAR, JSV_STRING_0);
              if (stackTrace) {
                jsvAppendPrintf(stackTrace, jsvIsString(functionName)?"in function %q called from ":
                    "in function called from ", functionName);
                if (lex) {
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

        // Unlock scopes and restore old ones
        jsvUnLock(execInfo.scopesVar);
        execInfo.scopesVar = oldScopeVar;
      }
      jsvUnLock(functionCode);
      jsvUnLock(functionRoot);
    }

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
   * a new name that references the object we actually requested the
   * member from..
   */
  if (child && returnName) {
    // Get rid of existing name
    if (jsvIsName(child)) {
      JsVar *t = jsvGetValueOfName(child);
      jsvUnLock(child);
      child = t;
    }
    // create a new name
    JsVar *nameVar = jsvNewFromString(name);
    JsVar *newChild = jsvCreateNewChild(object, nameVar, child);
    jsvUnLock2(nameVar, child);
    child = newChild;
  }

  // If not found and is the prototype, create it
  if (!child) {
    if (jsvIsFunction(object) && strcmp(name, JSPARSE_PROTOTYPE_VAR)==0) {
      // prototype is supposed to be an object
      JsVar *proto = jsvNewObject();
      // make sure it has a 'constructor' variable that points to the object it was part of
      jsvObjectSetChild(proto, JSPARSE_CONSTRUCTOR_VAR, object);
      child = jsvAddNamedChild(object, proto, JSPARSE_PROTOTYPE_VAR);
      jspEnsureIsPrototype(object, child);
      jsvUnLock(proto);
    } else if (strcmp(name, JSPARSE_INHERITS_VAR)==0) {
      const char *objName = jswGetBasicObjectName(object);
      if (objName) {
        JsVar *p = jsvSkipNameAndUnLock(jspNewPrototype(objName));
        // jspNewPrototype returns a 'prototype' name that's already a child of eg. an array
        // Create a new 'name' called __proto__ that links to it
        JsVar *i = jsvNewFromString(JSPARSE_INHERITS_VAR);
        if (p) child = jsvCreateNewChild(object, i, p);
        jsvUnLock(i);
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
      JsVar *value = jsvNewObject(); // prototype is supposed to be an object
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
        char ch = jsvGetCharInString(object, (size_t)idx);
        child = jsvNewStringOfLength(1, &ch);
      } else if (returnName)
        child = jsvCreateNewChild(object, nameVar, 0); // just return *something* to show this is handled
    } else {
      // get the name as a string
      char name[JSLEX_MAX_TOKEN_LENGTH];
      jsvGetString(nameVar, name, JSLEX_MAX_TOKEN_LENGTH);
      // try and find it in parents
      child = jspGetNamedFieldInParents(object, name, returnName);

      // If not found and is the prototype, create it
      if (!child && jsvIsFunction(object) && jsvIsStringEqual(nameVar, JSPARSE_PROTOTYPE_VAR)) {
        JsVar *value = jsvNewObject(); // prototype is supposed to be an object
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

  while (lex->tk=='.' || lex->tk=='[') {
    if (lex->tk == '.') { // ------------------------------------- Record Access
      JSP_ASSERT_MATCH('.');
      if (jslIsIDOrReservedWord()) {
        if (JSP_SHOULD_EXECUTE) {
          // Note: name will go away when we parse something else!
          const char *name = jslGetTokenValueAsString();

          JsVar *aVar = jsvSkipNameWithParent(a,true,parent);
          JsVar *child = 0;
          if (aVar)
            child = jspGetNamedField(aVar, name, true);
          if (!child) {
            if (!jsvIsUndefined(aVar)) {
              // if no child found, create a pointer to where it could be
              // as we don't want to allocate it until it's written
              JsVar *nameVar = jslGetTokenValueAsVar();
              child = jsvCreateNewChild(aVar, nameVar, 0);
              jsvUnLock(nameVar);
            } else {
              // could have been a string...
              jsExceptionHere(JSET_ERROR, "Cannot read property '%s' of undefined", name);
            }
          }
          jsvUnLock(parent);
          parent = aVar;
          jsvUnLock(a);
          a = child;
        }
        // skip over current token (we checked above that it was an ID or reserved word)
        jslGetNextToken();
      } else {
        // incorrect token - force a match fail by asking for an ID
        JSP_MATCH_WITH_RETURN(LEX_ID, a);
      }
    } else if (lex->tk == '[') { // ------------------------------------- Array Access
      JsVar *index;
      JSP_ASSERT_MATCH('[');
      if (!jspCheckStackPosition()) return parent;
      index = jsvSkipNameAndUnLock(jspeAssignmentExpression());
      JSP_MATCH_WITH_CLEANUP_AND_RETURN(']', jsvUnLock2(parent, index);, a);
      if (JSP_SHOULD_EXECUTE) {
        index = jsvAsArrayIndexAndUnLock(index);
        JsVar *aVar = jsvSkipNameWithParent(a,true,parent);
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

  JsVar *thisObj = jsvNewObject();
  if (!thisObj) return 0; // out of memory
  // Make sure the function has a 'prototype' var
  JsVar *prototypeName = jsvFindChildFromString(func, JSPARSE_PROTOTYPE_VAR, true);
  jspEnsureIsPrototype(func, prototypeName); // make sure it's an object
  JsVar *prototypeVar = jsvSkipName(prototypeName);
  jsvUnLock3(jsvAddNamedChild(thisObj, prototypeVar, JSPARSE_INHERITS_VAR), prototypeVar, prototypeName);

  JsVar *a = jspeFunctionCall(func, funcName, thisObj, hasArgs, 0, 0);

  /* FIXME: we should ignore return values that aren't objects (bug #848), but then we need
   * to be aware of `new String()` and `new Uint8Array()`. Ideally we'd let through
   * arrays/etc, and then String/etc should return 'boxed' values.
   *
   * But they don't return boxed values at the moment, so let's just
   * pass the return value through. If you try and return a string from
   * a function it's broken JS code anyway.
   */
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
  if (lex->tk==LEX_R_NEW) {
    JSP_ASSERT_MATCH(LEX_R_NEW);
    isConstructor = true;

    if (lex->tk==LEX_R_NEW) {
      jsExceptionHere(JSET_ERROR, "Nesting 'new' operators is unsupported");
      jspSetError(false);
      return 0;
    }
  }

  JsVar *parent = 0;
#ifndef SAVE_ON_FLASH
  bool wasSuper = lex->tk==LEX_R_SUPER;
#endif
  JsVar *a = jspeFactorMember(jspeFactor(), &parent);
#ifndef SAVE_ON_FLASH
  if (wasSuper) {
    /* if this was 'super.something' then we need
     * to overwrite the parent, because it'll be
     * set to the prototype otherwise.
     */
    jsvUnLock(parent);
    parent = jsvLockAgainSafe(execInfo.thisVar);
  }
#endif

  while ((lex->tk=='(' || (isConstructor && JSP_SHOULD_EXECUTE)) && !jspIsInterrupted()) {
    JsVar *funcName = a;
    JsVar *func = jsvSkipName(funcName);

    /* The constructor function doesn't change parsing, so if we're
     * not executing, just short-cut it. */
    if (isConstructor && JSP_SHOULD_EXECUTE) {
      // If we have '(' parse an argument list, otherwise don't look for any args
      bool parseArgs = lex->tk=='(';
      a = jspeConstruct(func, funcName, parseArgs);
      isConstructor = false; // don't treat subsequent brackets as constructors
    } else
      a = jspeFunctionCall(func, funcName, parent, true, 0, 0);

    jsvUnLock3(funcName, func, parent);
    parent=0;
    a = jspeFactorMember(a, &parent);
  }
#ifndef SAVE_ON_FLASH
  /* If we've got something that we care about the parent of (eg. a getter/setter)
   * then we repackage it into a 'NewChild' name that references the parent before
   * we leave. Note: You can't do this on everything because normally NewChild
   * forces a new child to be blindly created. It works on Getters/Setters because
   * we *always* run those rather than adding them.
   */
  if (parent && jsvIsBasicName(a) && !jsvIsNewChild(a)) {
    JsVar *value = jsvLockSafe(jsvGetFirstChild(a));
    if (jsvIsGetterOrSetter(value)) { // no need to do this for functions since we've just executed whatever we needed to
      JsVar *nameVar = jsvCopyNameOnly(a,false,true);
      JsVar *newChild = jsvCreateNewChild(parent, nameVar, value);
      jsvUnLock2(nameVar, a);
      a = newChild;
    }
    jsvUnLock(value);
  }
#endif
  jsvUnLock(parent);
  return a;
}


NO_INLINE JsVar *jspeFactorObject() {
  if (JSP_SHOULD_EXECUTE) {
    JsVar *contents = jsvNewObject();
    if (!contents) { // out of memory
      jspSetError(false);
      return 0;
    }
    /* JSON-style object definition */
    JSP_MATCH_WITH_RETURN('{', contents);
    while (!JSP_SHOULDNT_PARSE && lex->tk != '}') {
      JsVar *varName = 0;
      // we only allow strings or IDs on the left hand side of an initialisation
      if (jslIsIDOrReservedWord()) {
        if (JSP_SHOULD_EXECUTE)
          varName = jslGetTokenValueAsVar();
        jslGetNextToken(); // skip over current token
      } else if (
          lex->tk==LEX_STR ||
          lex->tk==LEX_FLOAT ||
          lex->tk==LEX_INT ||
          lex->tk==LEX_R_TRUE ||
          lex->tk==LEX_R_FALSE ||
          lex->tk==LEX_R_NULL ||
          lex->tk==LEX_R_UNDEFINED) {
        varName = jspeFactor();
      } else {
        JSP_MATCH_WITH_RETURN(LEX_ID, contents);
      }
#ifndef SAVE_ON_FLASH
      if (lex->tk==LEX_ID && jsvIsString(varName)) {
        bool isGetter = jsvIsStringEqual(varName, "get");
        bool isSetter = jsvIsStringEqual(varName, "set");
        if (isGetter || isSetter) {
          jsvUnLock(varName);
          varName = jslGetTokenValueAsVar();
          JSP_ASSERT_MATCH(LEX_ID);
          JsVar *method = jspeFunctionDefinition(false);
          jsvAddGetterOrSetter(contents, varName, isGetter, method);
          jsvUnLock(method);
        }
      } else
#endif
      {
        JSP_MATCH_WITH_CLEANUP_AND_RETURN(':', jsvUnLock(varName), contents);
        if (JSP_SHOULD_EXECUTE) {
          varName = jsvAsArrayIndexAndUnLock(varName);
          JsVar *contentsName = jsvFindChildFromVar(contents, varName, true);
          if (contentsName) {
            JsVar *value = jsvSkipNameAndUnLock(jspeAssignmentExpression()); // value can be 0 (could be undefined!)
            jsvUnLock2(jsvSetValueOfName(contentsName, value), value);
          }
        }
      }
      jsvUnLock(varName);
      // no need to clean here, as it will definitely be used
      if (lex->tk != '}') JSP_MATCH_WITH_RETURN(',', contents);
    }
    JSP_MATCH_WITH_RETURN('}', contents);
    return contents;
  } else {
    // Not executing so do fast skip
    jspeBlock();
    return 0;
  }
}

NO_INLINE JsVar *jspeFactorArray() {
  int idx = 0;
  JsVar *contents = 0;
  if (JSP_SHOULD_EXECUTE) {
    contents = jsvNewEmptyArray();
    if (!contents) { // out of memory
      jspSetError(false);
      return 0;
    }
  }
  /* JSON-style array */
  JSP_MATCH_WITH_RETURN('[', contents);
  while (!JSP_SHOULDNT_PARSE && lex->tk != ']') {
    if (JSP_SHOULD_EXECUTE) {
      JsVar *aVar = 0;
      JsVar *indexName = 0;
      if (lex->tk != ',') { // #287 - [,] and [1,2,,4] are allowed
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
    if (lex->tk != ']') JSP_MATCH_WITH_RETURN(',', contents);
    idx++;
  }
  if (contents) jsvSetArrayLength(contents, idx, false);
  JSP_MATCH_WITH_RETURN(']', contents);
  return contents;
}

NO_INLINE void jspEnsureIsPrototype(JsVar *instanceOf, JsVar *prototypeName) {
  if (!prototypeName) return;
  JsVar *prototypeVar = jsvSkipName(prototypeName);
  if (!(jsvIsObject(prototypeVar) || jsvIsFunction(prototypeVar))) {
    if (!jsvIsUndefined(prototypeVar))
      jsExceptionHere(JSET_TYPEERROR, "Prototype should be an object, got %t", prototypeVar);
    jsvUnLock(prototypeVar);
    prototypeVar = jsvNewObject(); // prototype is supposed to be an object
    JsVar *lastName = jsvSkipToLastName(prototypeName);
    jsvSetValueOfName(lastName, prototypeVar);
    jsvUnLock(lastName);
  }
  JsVar *constructor = jsvFindChildFromString(prototypeVar, JSPARSE_CONSTRUCTOR_VAR, true);
  if (constructor) jsvSetValueOfName(constructor, instanceOf);
  jsvUnLock2(constructor, prototypeVar);
}

NO_INLINE JsVar *jspeFactorTypeOf() {
  JSP_ASSERT_MATCH(LEX_R_TYPEOF);
  JsVar *a = jspeUnaryExpression();
  JsVar *result = 0;
  if (JSP_SHOULD_EXECUTE) {
    if (!jsvIsVariableDefined(a)) {
      // so we don't get a ReferenceError when accessing an undefined var
      result=jsvNewFromString("undefined");
    } else {
      a = jsvSkipNameAndUnLock(a);
      result=jsvNewFromString(jsvGetTypeOf(a));
    }
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

#ifdef DEBUG
      if (jsvHasChildren(parent)) assert(jsvIsChild(parent, a));
#endif
      if (jsvHasChildren(parent) && jsvIsChild(parent, a)) {
        // else remove properly.
        /* we use jsvIsChild here just in case. delete probably isn't called
        that often so it pays to be safe */
        if (jsvIsArray(parent)) {
          // For arrays, we must make sure we don't change the length
          JsVarInt l = jsvGetArrayLength(parent);
          jsvRemoveChild(parent, a);
          jsvSetArrayLength(parent, l, false);
        } else {
          jsvRemoveChild(parent, a);
        }
        ok = true;
      }
    }

    result = jsvNewFromBool(ok);
  }
  jsvUnLock2(a, parent);
  return result;
}

#ifndef SAVE_ON_FLASH
JsVar *jspeTemplateLiteral() {
  JsVar *a = 0;
  if (JSP_SHOULD_EXECUTE) {
    JsVar *template = jslGetTokenValueAsVar();
    a = jsvNewFromEmptyString();
    if (a && template) {
      JsvStringIterator it, dit;
      jsvStringIteratorNew(&it, template, 0);
      jsvStringIteratorNew(&dit, a, 0);
      while (jsvStringIteratorHasChar(&it)) {
        char ch = jsvStringIteratorGetCharAndNext(&it);
        if (ch=='$') {
          ch = jsvStringIteratorGetChar(&it);
          if (ch=='{') {
            // Now parse out the expression
            jsvStringIteratorNext(&it);
            int brackets = 1;
            JsVar *expr = jsvNewFromEmptyString();
            if (!expr) break;
            JsvStringIterator eit;
            jsvStringIteratorNew(&eit, expr, 0);
            while (jsvStringIteratorHasChar(&it)) {
              ch = jsvStringIteratorGetCharAndNext(&it);
              if (ch=='{') brackets++;
              if (ch=='}') {
                brackets--;
                if (!brackets) break;
              }
              jsvStringIteratorAppend(&eit, ch);
            }
            jsvStringIteratorFree(&eit);
            JsVar *result = jspEvaluateExpressionVar(expr);
            jsvUnLock(expr);
            result = jsvAsStringAndUnLock(result);
            jsvStringIteratorAppendString(&dit, result, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
            jsvUnLock(result);
          } else {
            jsvStringIteratorAppend(&dit, '$');
          }
        } else {
          jsvStringIteratorAppend(&dit, ch);
        }
      }
      jsvStringIteratorFree(&it);
      jsvStringIteratorFree(&dit);
    }
    jsvUnLock(template);
  }
  JSP_ASSERT_MATCH(LEX_TEMPLATE_LITERAL);
  return a;
}
#endif


NO_INLINE JsVar *jspeAddNamedFunctionParameter(JsVar *funcVar, JsVar *name) {
  if (!funcVar) funcVar = jsvNewWithFlags(JSV_FUNCTION);
  char buf[JSLEX_MAX_TOKEN_LENGTH+1];
  buf[0] = '\xFF';
  size_t l = jsvGetString(name, &buf[1], JSLEX_MAX_TOKEN_LENGTH);
  buf[l+1] = 0; // zero terminate since jsvGetString doesn't add one
  JsVar *param = jsvAddNamedChild(funcVar, 0, buf);
  jsvMakeFunctionParameter(param);
  jsvUnLock(param);
  return funcVar;
}

#ifndef SAVE_ON_FLASH
// parse an arrow function
NO_INLINE JsVar *jspeArrowFunction(JsVar *funcVar, JsVar *a) {
  assert(!a || jsvIsName(a));
  JSP_ASSERT_MATCH(LEX_ARROW_FUNCTION);
  funcVar = jspeAddNamedFunctionParameter(funcVar, a);

  bool expressionOnly = lex->tk!='{';
  bool fnIncludesThis = jspeFunctionDefinitionInternal(funcVar, expressionOnly);
  /* Arrow functions store the value of 'this' when they were defined. In order
  to differentiate between normal functions we usually have to store 'this' even
  if 'this' was just the global object.
  Very few arrow functions actually use 'this' though - usually they are just used
  as a shorthand, and so we end up wasting a whole extra var for every single
  arrow function.
  So... while parsing the function's body we check of the 'this' keyword is used.
  If it isn't, we just don't include it.
   */
  if (fnIncludesThis)
    jsvObjectSetChild(funcVar, JSPARSE_FUNCTION_THIS_NAME, execInfo.thisVar);

  return funcVar;
}

// parse expressions with commas, maybe followed by an arrow function (bracket already matched)
NO_INLINE JsVar *jspeExpressionOrArrowFunction() {
  JsVar *a = 0;
  JsVar *funcVar = 0;
  bool allNames = true;
  while (lex->tk!=')' && !JSP_SHOULDNT_PARSE) {
    if (allNames && a) {
      // we never get here if this isn't a name and a string
      funcVar = jspeAddNamedFunctionParameter(funcVar, a);
    }
    jsvUnLock(a);
    a = jspeAssignmentExpression();
    /* if we're not executing, `a` will always be undefined so
      don't do the check for allNames - just assume all is good. We'll
      properly check when we execute. */
    if (JSP_SHOULD_EXECUTE && !(jsvIsName(a) && jsvIsString(a))) allNames = false;
    if (lex->tk!=')') JSP_MATCH_WITH_CLEANUP_AND_RETURN(',', jsvUnLock2(a,funcVar), 0);
  }
  JSP_MATCH_WITH_CLEANUP_AND_RETURN(')', jsvUnLock2(a,funcVar), 0);
  // if all names inside brackets and an arrow is found, create a function
  if (allNames && lex->tk==LEX_ARROW_FUNCTION) {
    funcVar = jspeArrowFunction(funcVar, a);
    jsvUnLock(a);
    return funcVar;
  } else {
    jsvUnLock(funcVar);
    return a;
  }
}

/// Parse an ES6 class, expects LEX_R_CLASS already parsed
NO_INLINE JsVar *jspeClassDefinition(bool parseNamedClass) {
  JsVar *classFunction = 0;
  JsVar *classPrototype = 0;
  JsVar *classInternalName = 0;

  bool actuallyCreateClass = JSP_SHOULD_EXECUTE;
  if (actuallyCreateClass) {
    classFunction = jsvNewWithFlags(JSV_FUNCTION);
    JsVar *scopeVar = jspeiGetScopesAsVar();
    if (scopeVar)
      jsvUnLock2(jsvAddNamedChild(classFunction, scopeVar, JSPARSE_FUNCTION_SCOPE_NAME), scopeVar);
  }

  if (parseNamedClass && lex->tk==LEX_ID) {
    if (classFunction)
      classInternalName = jslGetTokenValueAsVar();
    JSP_ASSERT_MATCH(LEX_ID);
  }
  if (classFunction) {
    JsVar *prototypeName = jsvFindChildFromString(classFunction, JSPARSE_PROTOTYPE_VAR, true);
    jspEnsureIsPrototype(classFunction, prototypeName); // make sure it's an object
    classPrototype = jsvSkipName(prototypeName);
    jsvUnLock(prototypeName);
  }
  if (lex->tk==LEX_R_EXTENDS) {
    JSP_ASSERT_MATCH(LEX_R_EXTENDS);
    JsVar *extendsFrom = actuallyCreateClass ? jsvSkipNameAndUnLock(jspGetNamedVariable(jslGetTokenValueAsString())) : 0;
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID,jsvUnLock4(extendsFrom,classFunction,classInternalName,classPrototype),0);
    if (classPrototype) {
      if (jsvIsFunction(extendsFrom)) {
        JsVar *extendsFromProto = jsvObjectGetChild(extendsFrom, JSPARSE_PROTOTYPE_VAR, 0);
        if (extendsFromProto) {
          jsvObjectSetChild(classPrototype, JSPARSE_INHERITS_VAR, extendsFromProto);
          // link in default constructor if ours isn't supplied
          jsvObjectSetChildAndUnLock(classFunction, JSPARSE_FUNCTION_CODE_NAME, jsvNewFromString("if(this.__proto__.__proto__.constructor)this.__proto__.__proto__.constructor.apply(this,arguments)"));
          jsvUnLock(extendsFromProto);
        }
      } else
        jsExceptionHere(JSET_SYNTAXERROR, "'extends' argument should be a function, got %t", extendsFrom);
    }
    jsvUnLock(extendsFrom);
  }
  JSP_MATCH_WITH_CLEANUP_AND_RETURN('{',jsvUnLock3(classFunction,classInternalName,classPrototype),0);

  while ((lex->tk==LEX_ID || lex->tk==LEX_R_STATIC) && !jspIsInterrupted()) {
    bool isStatic = lex->tk==LEX_R_STATIC;
    if (isStatic) JSP_ASSERT_MATCH(LEX_R_STATIC);

    JsVar *funcName = jslGetTokenValueAsVar();
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID,jsvUnLock4(funcName,classFunction,classInternalName,classPrototype),0);
#ifndef SAVE_ON_FLASH
    bool isGetter = false, isSetter = false;
    if (lex->tk==LEX_ID) {
      isGetter = jsvIsStringEqual(funcName, "get");
      isSetter = jsvIsStringEqual(funcName, "set");
      if (isGetter || isSetter) {
        jsvUnLock(funcName);
        funcName = jslGetTokenValueAsVar();
        JSP_ASSERT_MATCH(LEX_ID);
      }
    }
#endif
    JsVar *method = jspeFunctionDefinition(false);
    if (classFunction && classPrototype) {
      JsVar *obj = isStatic ? classFunction : classPrototype;
      if (jsvIsStringEqual(funcName, "constructor")) {
        jswrap_function_replaceWith(classFunction, method);
#ifndef SAVE_ON_FLASH
      } else if (isGetter || isSetter) {
        jsvAddGetterOrSetter(obj, funcName, isGetter, method);
#endif
      } else {
        funcName = jsvMakeIntoVariableName(funcName, 0);
        jsvSetValueOfName(funcName, method);
        jsvAddName(obj, funcName);
      }

    }
    jsvUnLock2(method,funcName);
  }
  jsvUnLock(classPrototype);
  // If we had a name, add it to the end (or it gets confused with the constructor arguments)
  if (classInternalName)
    jsvObjectSetChildAndUnLock(classFunction, JSPARSE_FUNCTION_NAME_NAME, classInternalName);

  JSP_MATCH_WITH_CLEANUP_AND_RETURN('}',jsvUnLock(classFunction),0);
  return classFunction;
}

#endif

NO_INLINE JsVar *jspeFactor() {
  if (lex->tk==LEX_ID) {
    JsVar *a = jspGetNamedVariable(jslGetTokenValueAsString());
    JSP_ASSERT_MATCH(LEX_ID);
#ifndef SAVE_ON_FLASH
    if (lex->tk==LEX_TEMPLATE_LITERAL)
      jsExceptionHere(JSET_SYNTAXERROR, "Tagged template literals not supported");
    else if (lex->tk==LEX_ARROW_FUNCTION &&
             (jsvIsName(a) || (a==0 && !JSP_SHOULD_EXECUTE))) {
      // 'a' needs to be a name, *or* we're not executing so 0 gets returned anyway
      JsVar *funcVar = jspeArrowFunction(0,a);
      jsvUnLock(a);
      a=funcVar;
    }
#endif
    return a;
  } else if (lex->tk==LEX_INT) {
    JsVar *v = 0;
    if (JSP_SHOULD_EXECUTE) {
      v = jsvNewFromLongInteger(stringToInt(jslGetTokenValueAsString()));
    }
    JSP_ASSERT_MATCH(LEX_INT);
    return v;
  } else if (lex->tk==LEX_FLOAT) {
    JsVar *v = 0;
    if (JSP_SHOULD_EXECUTE) {
      v = jsvNewFromFloat(stringToFloat(jslGetTokenValueAsString()));
    }
    JSP_ASSERT_MATCH(LEX_FLOAT);
    return v;
  } else if (lex->tk=='(') {
    JSP_ASSERT_MATCH('(');
    if (!jspCheckStackPosition()) return 0;
#ifdef SAVE_ON_FLASH
    // Just parse a normal expression (which can include commas)
    JsVar *a = jspeExpression();
    if (!JSP_SHOULDNT_PARSE) JSP_MATCH_WITH_RETURN(')',a);
    return a;
#else
    return jspeExpressionOrArrowFunction();
#endif

  } else if (lex->tk==LEX_R_TRUE) {
    JSP_ASSERT_MATCH(LEX_R_TRUE);
    return JSP_SHOULD_EXECUTE ? jsvNewFromBool(true) : 0;
  } else if (lex->tk==LEX_R_FALSE) {
    JSP_ASSERT_MATCH(LEX_R_FALSE);
    return JSP_SHOULD_EXECUTE ? jsvNewFromBool(false) : 0;
  } else if (lex->tk==LEX_R_NULL) {
    JSP_ASSERT_MATCH(LEX_R_NULL);
    return JSP_SHOULD_EXECUTE ? jsvNewWithFlags(JSV_NULL) : 0;
  } else if (lex->tk==LEX_R_UNDEFINED) {
    JSP_ASSERT_MATCH(LEX_R_UNDEFINED);
    return 0;
  } else if (lex->tk==LEX_STR) {
    JsVar *a = 0;
    if (JSP_SHOULD_EXECUTE)
      a = jslGetTokenValueAsVar();
    JSP_ASSERT_MATCH(LEX_STR);
    return a;
#ifndef SAVE_ON_FLASH
  } else if (lex->tk==LEX_TEMPLATE_LITERAL) {
    return jspeTemplateLiteral();
#endif
  } else if (lex->tk==LEX_REGEX) {
    JsVar *a = 0;
#ifdef SAVE_ON_FLASH
    jsExceptionHere(JSET_SYNTAXERROR, "RegEx are not supported in this version of Espruino\n");
#else
    JsVar *regex = jslGetTokenValueAsVar();
    size_t regexEnd = 0, regexLen = 0;
    JsvStringIterator it;
    jsvStringIteratorNew(&it, regex, 0);
    while (jsvStringIteratorHasChar(&it)) {
      regexLen++;
      if (jsvStringIteratorGetCharAndNext(&it)=='/')
        regexEnd = regexLen;
    }
    jsvStringIteratorFree(&it);
    JsVar *flags = 0;
    if (regexEnd < regexLen)
      flags = jsvNewFromStringVar(regex, regexEnd, JSVAPPENDSTRINGVAR_MAXLENGTH);
    JsVar *regexSource = jsvNewFromStringVar(regex, 1, regexEnd-2);
    a = jswrap_regexp_constructor(regexSource, flags);
    jsvUnLock3(regex, flags, regexSource);
#endif
    JSP_ASSERT_MATCH(LEX_REGEX);
    return a;
  } else if (lex->tk=='{') {
    if (!jspCheckStackPosition()) return 0;
    return jspeFactorObject();
  } else if (lex->tk=='[') {
    if (!jspCheckStackPosition()) return 0;
    return jspeFactorArray();
  } else if (lex->tk==LEX_R_FUNCTION) {
    if (!jspCheckStackPosition()) return 0;
    JSP_ASSERT_MATCH(LEX_R_FUNCTION);
    return jspeFunctionDefinition(true);
#ifndef SAVE_ON_FLASH
  } else if (lex->tk==LEX_R_CLASS) {
    if (!jspCheckStackPosition()) return 0;
    JSP_ASSERT_MATCH(LEX_R_CLASS);
    return jspeClassDefinition(true);
  } else if (lex->tk==LEX_R_SUPER) {
    JSP_ASSERT_MATCH(LEX_R_SUPER);
    /* This is kind of nasty, since super appears to do
      three different things.

      * In the constructor it references the extended class's constructor
      * in a method it references the constructor's prototype.
      * in a static method it references the extended class's constructor (but this is different)
     */

    if (jsvIsObject(execInfo.thisVar)) {
      // 'this' is an object - must be calling a normal method
      JsVar *proto1 = jsvObjectGetChild(execInfo.thisVar, JSPARSE_INHERITS_VAR, 0); // if we're in a method, get __proto__ first
      JsVar *proto2 = jsvIsObject(proto1) ? jsvObjectGetChild(proto1, JSPARSE_INHERITS_VAR, 0) : 0; // still in method, get __proto__.__proto__
      jsvUnLock(proto1);
      if (!proto2) {
        jsExceptionHere(JSET_SYNTAXERROR, "Calling 'super' outside of class");
        return 0;
      }
      // If we're doing super() we want the constructor
      if (lex->tk=='(') {
        JsVar *constr = jsvObjectGetChild(proto2, JSPARSE_CONSTRUCTOR_VAR, 0);
        jsvUnLock(proto2);
        return constr;
      }
      // But if we're doing something else - eg 'super.' or 'super[' then it needs to reference the prototype
      return proto2;
    } else if (jsvIsFunction(execInfo.thisVar)) {
      // 'this' is a function - must be calling a static method
      JsVar *proto1 = jsvObjectGetChild(execInfo.thisVar, JSPARSE_PROTOTYPE_VAR, 0);
      JsVar *proto2 = jsvIsObject(proto1) ? jsvObjectGetChild(proto1, JSPARSE_INHERITS_VAR, 0) : 0;
      jsvUnLock(proto1);
      if (!proto2) {
        jsExceptionHere(JSET_SYNTAXERROR, "Calling 'super' outside of class");
        return 0;
      }
      JsVar *constr = jsvObjectGetChild(proto2, JSPARSE_CONSTRUCTOR_VAR, 0);
      jsvUnLock(proto2);
      return constr;
    }
    jsExceptionHere(JSET_SYNTAXERROR, "Calling 'super' outside of class");
    return 0;
#endif
  } else if (lex->tk==LEX_R_THIS) {
    JSP_ASSERT_MATCH(LEX_R_THIS);
    return jsvLockAgain( execInfo.thisVar ? execInfo.thisVar : execInfo.root );
  } else if (lex->tk==LEX_R_DELETE) {
    if (!jspCheckStackPosition()) return 0;
    return jspeFactorDelete();
  } else if (lex->tk==LEX_R_TYPEOF) {
    if (!jspCheckStackPosition()) return 0;
    return jspeFactorTypeOf();
  } else if (lex->tk==LEX_R_VOID) {
    if (!jspCheckStackPosition()) return 0;
    JSP_ASSERT_MATCH(LEX_R_VOID);
    jsvUnLock(jspeUnaryExpression());
    return 0;
  }
  JSP_MATCH(LEX_EOF);
  jsExceptionHere(JSET_SYNTAXERROR, "Unexpected end of Input\n");
  return 0;
}

NO_INLINE JsVar *__jspePostfixExpression(JsVar *a) {
  while (lex->tk==LEX_PLUSPLUS || lex->tk==LEX_MINUSMINUS) {
    int op = lex->tk;
    JSP_ASSERT_MATCH(op);
    if (JSP_SHOULD_EXECUTE) {
      JsVar *one = jsvNewFromInteger(1);
      JsVar *oldValue = jsvAsNumberAndUnLock(jsvSkipName(a)); // keep the old value (but convert to number)
      JsVar *res = jsvMathsOpSkipNames(oldValue, one, op==LEX_PLUSPLUS ? '+' : '-');
      jsvUnLock(one);

      // in-place add/subtract
      jsvReplaceWith(a, res);
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
  if (lex->tk==LEX_PLUSPLUS || lex->tk==LEX_MINUSMINUS) {
    int op = lex->tk;
    JSP_ASSERT_MATCH(op);
    a = jspePostfixExpression();
    if (JSP_SHOULD_EXECUTE) {
      JsVar *one = jsvNewFromInteger(1);
      JsVar *res = jsvMathsOpSkipNames(a, one, op==LEX_PLUSPLUS ? '+' : '-');
      jsvUnLock(one);
      // in-place add/subtract
      jsvReplaceWith(a, res);
      jsvUnLock(res);
    }
  } else
    a = jspeFactorFunctionCall();
  return __jspePostfixExpression(a);
}

NO_INLINE JsVar *jspeUnaryExpression() {
  if (lex->tk=='!' || lex->tk=='~' || lex->tk=='-' || lex->tk=='+') {
    short tk = lex->tk;
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
  unsigned int precedence = jspeGetBinaryExpressionPrecedence(lex->tk);
  while (precedence && precedence>lastPrecedence) {
    int op = lex->tk;
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
          if (jsvHasChildren(bv)) { // search keys, NOT values
            av = jsvAsArrayIndexAndUnLock(av);
            JsVar *varFound = jspGetVarNamedField( bv, av, true);
            jsvUnLock2(a,varFound);
            a = jsvNewFromBool(varFound!=0);
          } else { // else maybe it's a fake object...
            const JswSymList *syms = jswGetSymbolListForObjectProto(bv);
            if (syms) {
              JsVar *varFound = 0;
              char nameBuf[JSLEX_MAX_TOKEN_LENGTH];
              if (jsvGetString(av, nameBuf, sizeof(nameBuf)) < sizeof(nameBuf))
                varFound = jswBinarySearch(syms, bv, nameBuf);
              bool found = varFound!=0;
              jsvUnLock2(a, varFound);
              if (!found && jsvIsArrayBuffer(bv)) {
                JsVarFloat f = jsvGetFloat(av); // if not a number this will be NaN, f==floor(f) fails
                if (f==floor(f) && f>=0 && f<jsvGetArrayBufferLength(bv))
                  found = true;
              }
              a = jsvNewFromBool(found);
            } else { // not built-in, just assume we can't do it
              jsExceptionHere(JSET_ERROR, "Cannot use 'in' operator to search a %t", bv);
              jsvUnLock(a);
              a = 0;
            }
          }
          jsvUnLock2(av, bv);
        } else if (op==LEX_R_INSTANCEOF) {
          bool inst = false;
          JsVar *av = jsvSkipName(a);
          JsVar *bv = jsvSkipName(b);
          if (!jsvIsFunction(bv)) {
            jsExceptionHere(JSET_ERROR, "Expecting a function on RHS in instanceof check, got %t", bv);
          } else {
            if (jsvIsObject(av) || jsvIsFunction(av)) {
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
            }
            if (!inst) {
              const char *name = jswGetBasicObjectName(av);
              if (name) {
                inst = jspIsConstructor(bv, name);
              }
              // Hack for built-ins that should also be instances of Object
              if (!inst && (jsvIsArray(av) || jsvIsArrayBuffer(av)) &&
                  jspIsConstructor(bv, "Object"))
                inst = true;
            }
          }
          jsvUnLock3(av, bv, a);
          a = jsvNewFromBool(inst);
        } else {  // --------------------------------------------- NORMAL
          JsVar *res = jsvMathsOpSkipNames(a, b, op);
          jsvUnLock(a); a = res;
        }
      }
      jsvUnLock(b);
    }
    precedence = jspeGetBinaryExpressionPrecedence(lex->tk);
  }
  return a;
}

JsVar *jspeBinaryExpression() {
  return __jspeBinaryExpression(jspeUnaryExpression(),0);
}

NO_INLINE JsVar *__jspeConditionalExpression(JsVar *lhs) {
  if (lex->tk=='?') {
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
  if (lex->tk=='=' || lex->tk==LEX_PLUSEQUAL || lex->tk==LEX_MINUSEQUAL ||
      lex->tk==LEX_MULEQUAL || lex->tk==LEX_DIVEQUAL || lex->tk==LEX_MODEQUAL ||
      lex->tk==LEX_ANDEQUAL || lex->tk==LEX_OREQUAL ||
      lex->tk==LEX_XOREQUAL || lex->tk==LEX_RSHIFTEQUAL ||
      lex->tk==LEX_LSHIFTEQUAL || lex->tk==LEX_RSHIFTUNSIGNEDEQUAL) {
    JsVar *rhs;

    int op = lex->tk;
    JSP_ASSERT_MATCH(op);
    rhs = jspeAssignmentExpression();
    rhs = jsvSkipNameAndUnLock(rhs); // ensure we get rid of any references on the RHS

    if (JSP_SHOULD_EXECUTE && lhs) {
      if (op=='=') {
        jsvReplaceWithOrAddToRoot(lhs, rhs);
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
          if (jsvIsBasicString(currentValue) && jsvGetRefs(currentValue)==1 && rhs!=currentValue) {
            /* A special case for string += where this is the only use of the string
             * and we're not appending to ourselves. In this case we can do a
             * simple append (rather than clone + append)*/
            JsVar *str = jsvAsString(rhs);
            jsvAppendStringVarComplete(currentValue, str);
            jsvUnLock(str);
            op = 0;
          }
          jsvUnLock(currentValue);
        }
        if (op) {
          /* Fallback which does a proper add */
          JsVar *res = jsvMathsOpSkipNames(lhs,rhs,op);
          jsvReplaceWith(lhs, res);
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
    if (lex->tk!=',') return a;
    // if we get a comma, we just forget this data and parse the next bit...
    jsvCheckReferenceError(a);
    jsvUnLock(a);
    JSP_ASSERT_MATCH(',');
  }
  return 0;
}

/** Parse a block `{ ... }` */
NO_INLINE void jspeSkipBlock() {
  // fast skip of blocks
  int brackets = 1;
  while (lex->tk && brackets) {
    if (lex->tk == '{') brackets++;
    else if (lex->tk == '}') {
      brackets--;
      if (!brackets) return;
    }
    JSP_ASSERT_MATCH(lex->tk);
  }
}

/** Parse a block `{ ... }` but assume brackets are already parsed */
NO_INLINE void jspeBlockNoBrackets() {
  if (JSP_SHOULD_EXECUTE) {
    while (lex->tk && lex->tk!='}') {
      JsVar *a = jspeStatement();
      jsvCheckReferenceError(a);
      jsvUnLock(a);
      if (JSP_HAS_ERROR) {
        if (lex && !(execInfo.execute&EXEC_ERROR_LINE_REPORTED)) {
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
        return;
      if (!JSP_SHOULD_EXECUTE) {
        jspeSkipBlock();
        return;
      }
    }
  } else {
    jspeSkipBlock();
  }
  return;
}

/** Parse a block `{ ... }` */
NO_INLINE void jspeBlock() {
  JSP_MATCH_WITH_RETURN('{',);
  jspeBlockNoBrackets();
  if (!JSP_SHOULDNT_PARSE) JSP_MATCH_WITH_RETURN('}',);
  return;
}

NO_INLINE JsVar *jspeBlockOrStatement() {
  if (lex->tk=='{') {
    jspeBlock();
    return 0;
  } else {
    JsVar *v = jspeStatement();
    if (lex->tk==';') JSP_ASSERT_MATCH(';');
    return v;
  }
}

/** Parse using current lexer until we hit the end of
 * input or there was some problem. */
NO_INLINE JsVar *jspParse() {
  JsVar *v = 0;
  while (!JSP_SHOULDNT_PARSE && lex->tk != LEX_EOF) {
    jsvUnLock(v);
    v = jspeBlockOrStatement();
    jsvCheckReferenceError(v);
  }
  return v;
}

NO_INLINE JsVar *jspeStatementVar() {
  JsVar *lastDefined = 0;
  /* variable creation. TODO - we need a better way of parsing the left
   * hand side. Maybe just have a flag called can_create_var that we
   * set and then we parse as if we're doing a normal equals.*/
  assert(lex->tk==LEX_R_VAR || lex->tk==LEX_R_LET || lex->tk==LEX_R_CONST);
  jslGetNextToken();
  ///TODO: Correctly implement CONST and LET - we just treat them like 'var' at the moment
  bool hasComma = true; // for first time in loop
  while (hasComma && lex->tk == LEX_ID && !jspIsInterrupted()) {
    JsVar *a = 0;
    if (JSP_SHOULD_EXECUTE) {
      a = jspeiFindOnTop(jslGetTokenValueAsString(), true);
      if (!a) { // out of memory
        jspSetError(false);
        return lastDefined;
      }
    }
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(a), lastDefined);
    // sort out initialiser
    if (lex->tk == '=') {
      JsVar *var;
      JSP_MATCH_WITH_CLEANUP_AND_RETURN('=', jsvUnLock(a), lastDefined);
      var = jsvSkipNameAndUnLock(jspeAssignmentExpression());
      if (JSP_SHOULD_EXECUTE)
        jsvReplaceWith(a, var);
      jsvUnLock(var);
    }
    jsvUnLock(lastDefined);
    lastDefined = a;
    hasComma = lex->tk == ',';
    if (hasComma) JSP_MATCH_WITH_RETURN(',', lastDefined);
  }
  return lastDefined;
}

NO_INLINE JsVar *jspeStatementIf() {
  bool cond;
  JsVar *var, *result = 0;
  JSP_ASSERT_MATCH(LEX_R_IF);
  JSP_MATCH('(');
  var = jspeExpression();
  if (JSP_SHOULDNT_PARSE) return var;
  JSP_MATCH(')');
  cond = JSP_SHOULD_EXECUTE && jsvGetBoolAndUnLock(jsvSkipName(var));
  jsvUnLock(var);

  JSP_SAVE_EXECUTE();
  if (!cond) jspSetNoExecute();
  JsExecFlags hasError = 0;
  JsVar *a = jspeBlockOrStatement();
  hasError |= execInfo.execute&EXEC_ERROR_MASK;
  if (!cond) {
    jsvUnLock(a);
    JSP_RESTORE_EXECUTE();
    execInfo.execute |= hasError;
  } else {
    result = a;
  }
  if (lex->tk==LEX_R_ELSE) {
    JSP_ASSERT_MATCH(LEX_R_ELSE);
    JSP_SAVE_EXECUTE();
    if (cond) jspSetNoExecute();
    JsVar *a = jspeBlockOrStatement();
    hasError |= execInfo.execute&EXEC_ERROR_MASK;
    if (cond) {
      jsvUnLock(a);
      JSP_RESTORE_EXECUTE();
      execInfo.execute |= hasError;
    } else {
      result = a;
    }
  }
  return result;
}

NO_INLINE JsVar *jspeStatementSwitch() {
  JSP_ASSERT_MATCH(LEX_R_SWITCH);
  JSP_MATCH('(');
  JsVar *switchOn = jspeExpression();
  JSP_SAVE_EXECUTE();
  bool execute = JSP_SHOULD_EXECUTE;
  JSP_MATCH_WITH_CLEANUP_AND_RETURN(')', jsvUnLock(switchOn), 0);
  // shortcut if not executing...
  if (!execute) { jsvUnLock(switchOn); jspeBlock(); return 0; }
  JSP_MATCH_WITH_CLEANUP_AND_RETURN('{', jsvUnLock(switchOn), 0);

  bool executeDefault = true;
  if (execute) execInfo.execute=EXEC_NO|EXEC_IN_SWITCH;
  while (lex->tk==LEX_R_CASE) {
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_R_CASE, jsvUnLock(switchOn), 0);
    JsExecFlags oldFlags = execInfo.execute;
    if (execute) execInfo.execute=EXEC_YES|EXEC_IN_SWITCH;
    JsVar *test = jspeAssignmentExpression();
    execInfo.execute = oldFlags|EXEC_IN_SWITCH;;
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(':', jsvUnLock2(switchOn, test), 0);
    bool cond = false;
    if (execute)
      cond = jsvGetBoolAndUnLock(jsvMathsOpSkipNames(switchOn, test, LEX_TYPEEQUAL));
    if (cond) executeDefault = false;
    jsvUnLock(test);
    if (cond && (execInfo.execute&EXEC_RUN_MASK)==EXEC_NO)
      execInfo.execute=EXEC_YES|EXEC_IN_SWITCH;
    while (!JSP_SHOULDNT_PARSE && lex->tk!=LEX_EOF && lex->tk!=LEX_R_CASE && lex->tk!=LEX_R_DEFAULT && lex->tk!='}')
      jsvUnLock(jspeBlockOrStatement());
    oldExecute |= execInfo.execute & (EXEC_ERROR_MASK|EXEC_RETURN); // copy across any errors/exceptions/returns
  }
  jsvUnLock(switchOn);
  if (execute && (execInfo.execute&EXEC_RUN_MASK)==EXEC_BREAK) {
    execInfo.execute=EXEC_YES|EXEC_IN_SWITCH;
  } else {
    executeDefault = true;
  }
  JSP_RESTORE_EXECUTE();

  if (lex->tk==LEX_R_DEFAULT) {
    JSP_ASSERT_MATCH(LEX_R_DEFAULT);
    JSP_MATCH(':');
    JSP_SAVE_EXECUTE();
    if (!executeDefault) jspSetNoExecute();
    else execInfo.execute |= EXEC_IN_SWITCH;
    while (!JSP_SHOULDNT_PARSE && lex->tk!=LEX_EOF && lex->tk!='}' && lex->tk!=LEX_R_CASE)
      jsvUnLock(jspeBlockOrStatement());
    oldExecute |= execInfo.execute & (EXEC_ERROR_MASK|EXEC_RETURN); // copy across any errors/exceptions/returns
    execInfo.execute = execInfo.execute & (JsExecFlags)~EXEC_BREAK;
    JSP_RESTORE_EXECUTE();
  }
  if (lex->tk==LEX_R_CASE) {
    jsExceptionHere(JSET_SYNTAXERROR, "Espruino doesn't support CASE after DEFAULT");
    return 0;
  }
  JSP_MATCH('}');
  return 0;
}

// Check whether we received a break/continue while parsing previously. Return true if we had a 'break;
static NO_INLINE bool jspeCheckBreakContinue() {
  if (execInfo.execute & EXEC_CONTINUE)
    execInfo.execute = (execInfo.execute & ~EXEC_RUN_MASK) | EXEC_YES;
  else if (execInfo.execute & EXEC_BREAK) {
    execInfo.execute = (execInfo.execute & ~EXEC_RUN_MASK) | EXEC_YES;
    return true;
  }
  return false;
}

NO_INLINE JsVar *jspeStatementDoOrWhile(bool isWhile) {
  JsVar *cond;
  bool loopCond = true; // true for do...while loops
  bool hasHadBreak = false;
  JslCharPos whileCondStart;
  // We do repetition by pulling out the string representing our statement
  // there's definitely some opportunity for optimisation here

  bool wasInLoop = (execInfo.execute&EXEC_IN_LOOP)!=0;
  JslCharPos whileBodyStart;
  if (isWhile) { // while loop
    JSP_ASSERT_MATCH(LEX_R_WHILE);
    jslCharPosFromLex(&whileCondStart);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN('(',jslCharPosFree(&whileCondStart);,0);
    cond = jspeExpression();
    loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolAndUnLock(jsvSkipName(cond));
    jsvUnLock(cond);
    jslCharPosFromLex(&whileBodyStart);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(')',jslCharPosFree(&whileBodyStart);jslCharPosFree(&whileCondStart);,0);
  } else {
    jslCharPosFromLex(&whileBodyStart);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_R_DO, jslCharPosFree(&whileBodyStart);,0);
  }
  JSP_SAVE_EXECUTE();
  // actually try and execute first bit of while loop (we'll do the rest in the actual loop later)
  if (!loopCond) jspSetNoExecute();
  execInfo.execute |= EXEC_IN_LOOP;
  jsvUnLock(jspeBlockOrStatement());
  if (!wasInLoop) execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;

  hasHadBreak |= jspeCheckBreakContinue();
  if (!loopCond) JSP_RESTORE_EXECUTE();

  if (!isWhile) { // do..while loop
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_R_WHILE,jslCharPosFree(&whileBodyStart);,0);
    jslCharPosFromLex(&whileCondStart);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN('(',jslCharPosFree(&whileBodyStart);jslCharPosFree(&whileCondStart);,0);
    cond = jspeExpression();
    loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolAndUnLock(jsvSkipName(cond));
    jsvUnLock(cond);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(')',jslCharPosFree(&whileBodyStart);jslCharPosFree(&whileCondStart);,0);
  }

  JslCharPos whileBodyEnd;
  jslCharPosNew(&whileBodyEnd, lex->sourceVar, lex->tokenStart);

  int loopCount = 0;
  while (!hasHadBreak && loopCond
#ifdef JSPARSE_MAX_LOOP_ITERATIONS
      && loopCount<JSPARSE_MAX_LOOP_ITERATIONS
#endif
  ) {
    if (isWhile || loopCount) { // don't check the start condition a second time if we're in a do..while loop
      jslSeekToP(&whileCondStart);
      cond = jspeExpression();
      loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolAndUnLock(jsvSkipName(cond));
      jsvUnLock(cond);
    }
    if (loopCond) {
      jslSeekToP(&whileBodyStart);
      execInfo.execute |= EXEC_IN_LOOP;
      jspDebuggerLoopIfCtrlC();
      jsvUnLock(jspeBlockOrStatement());
      if (!wasInLoop) execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
      hasHadBreak |= jspeCheckBreakContinue();
    }
    loopCount++;
  }
  jslSeekToP(&whileBodyEnd);
  jslCharPosFree(&whileCondStart);
  jslCharPosFree(&whileBodyStart);
  jslCharPosFree(&whileBodyEnd);
#ifdef JSPARSE_MAX_LOOP_ITERATIONS
  if (loopCount > JSPARSE_MAX_LOOP_ITERATIONS) {
    jsExceptionHere(JSET_ERROR, "WHILE Loop exceeded the maximum number of iterations (" STRINGIFY(JSPARSE_MAX_LOOP_ITERATIONS) ")");
  }
#endif
  return 0;
}

NO_INLINE JsVar *jspGetBuiltinPrototype(JsVar *obj) {
  if (jsvIsArray(obj)) {
    JsVar *v = jspFindPrototypeFor("Array");
    if (v) return v;
  }
  if (jsvIsObject(obj) || jsvIsArray(obj)) {
    JsVar *v = jspFindPrototypeFor("Object");
    if (v==obj) { // don't return ourselves
      jsvUnLock(v);
      v = 0;
    }
    return v;
  }
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
  if (lex->tk != ';')
    forStatement = jspeStatement();
  if (jspIsInterrupted()) {
    jsvUnLock(forStatement);
    return 0;
  }
  execInfo.execute &= (JsExecFlags)~EXEC_FOR_INIT;
#ifndef SAVE_ON_FLASH_EXTREME
  if (lex->tk == LEX_R_IN || lex->tk == LEX_R_OF) {
    bool isForOf = lex->tk == LEX_R_OF;
    // for (i in array)  or   for (i of array)
    // where i = forStatement
    if (JSP_SHOULD_EXECUTE && !jsvIsName(forStatement)) {
      jsvUnLock(forStatement);
      jsExceptionHere(JSET_ERROR, "for(a %s b) - 'a' must be a variable name, not %t", isForOf?"of":"in", forStatement);
      return 0;
    }

    JSP_ASSERT_MATCH(lex->tk); // skip over in/of
    JsVar *array = jsvSkipNameAndUnLock(jspeExpression());

    JslCharPos forBodyStart;
    jslCharPosFromLex(&forBodyStart);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(')', jsvUnLock2(forStatement, array);jslCharPosFree(&forBodyStart), 0);

    // Simply scan over the loop the first time without executing to figure out where it ends
    // OPT: we could skip the first parse and actually execute the first time
    JSP_SAVE_EXECUTE();
    jspSetNoExecute();
    execInfo.execute |= EXEC_IN_LOOP;
    jsvUnLock(jspeBlockOrStatement());
    JslCharPos forBodyEnd;
    jslCharPosNew(&forBodyEnd, lex->sourceVar, lex->tokenStart);
    if (!wasInLoop) execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
    JSP_RESTORE_EXECUTE();
    // Now start executing properly
    if (JSP_SHOULD_EXECUTE) {
      if (jsvIsIterable(array)) {
        JsvIsInternalChecker checkerFunction = jsvGetInternalFunctionCheckerFor(array);
        JsVar *foundPrototype = 0;
        if (!isForOf) // for..in
          foundPrototype = jspGetBuiltinPrototype(array);

        JsvIterator it;
        jsvIteratorNew(&it, array, isForOf ?
            /* for of */ JSIF_EVERY_ARRAY_ELEMENT :
            /* for in */ JSIF_DEFINED_ARRAY_ElEMENTS);
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
            JsVar *iteratorValue;
            if (isForOf) { // for (... of ...)
              iteratorValue = jsvIteratorGetValue(&it);
            } else { // for (... in ...)
              iteratorValue = jsvIsName(loopIndexVar) ?
                  jsvCopyNameOnly(loopIndexVar, false/*no copy children*/, false/*not a name*/) :
                  loopIndexVar;
              assert(jsvGetRefs(iteratorValue)==0);
            }
            if (isForOf || iteratorValue) { // could be out of memory
              assert(!jsvIsName(iteratorValue));
              jsvReplaceWithOrAddToRoot(forStatement, iteratorValue);
              if (iteratorValue!=loopIndexVar) jsvUnLock(iteratorValue);

              jslSeekToP(&forBodyStart);
              execInfo.execute |= EXEC_IN_LOOP;
              jspDebuggerLoopIfCtrlC();
              jsvUnLock(jspeBlockOrStatement());
              if (!wasInLoop) execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;

              hasHadBreak |= jspeCheckBreakContinue();
            }
          }
          jsvIteratorNext(&it);
          jsvUnLock(loopIndexVar);
          // if using for..in we'll skip down the prototype chain when we reach the end of the current one
          if (!jsvIteratorHasElement(&it) && !isForOf && foundPrototype) {
            jsvIteratorFree(&it);
            JsVar *iterable = foundPrototype;
            jsvIteratorNew(&it, iterable, JSIF_DEFINED_ARRAY_ElEMENTS);
            checkerFunction = jsvGetInternalFunctionCheckerFor(iterable);
            foundPrototype = jspGetBuiltinPrototype(iterable);
            jsvUnLock(iterable);
          }
        }
        assert(!foundPrototype);
        jsvIteratorFree(&it);
      } else if (!jsvIsUndefined(array)) {
        jsExceptionHere(JSET_ERROR, "FOR loop can only iterate over Arrays, Strings or Objects, not %t", array);
      }
    }
    jslSeekToP(&forBodyEnd);
    jslCharPosFree(&forBodyStart);
    jslCharPosFree(&forBodyEnd);

    jsvUnLock2(forStatement, array);
#else // SAVE_ON_FLASH_EXTREME
  if (false) {
#endif // SAVE_ON_FLASH_EXTREME
  } else { // ----------------------------------------------- NORMAL FOR LOOP
#ifdef JSPARSE_MAX_LOOP_ITERATIONS
    int loopCount = JSPARSE_MAX_LOOP_ITERATIONS;
#endif
    bool loopCond = true;
    bool hasHadBreak = false;

    jsvUnLock(forStatement);
    JslCharPos forCondStart;
    jslCharPosFromLex(&forCondStart);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(';',jslCharPosFree(&forCondStart);,0);

    if (lex->tk != ';') {
      JsVar *cond = jspeExpression(); // condition
      loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolAndUnLock(jsvSkipName(cond));
      jsvUnLock(cond);
    }
    JslCharPos forIterStart;
    jslCharPosFromLex(&forIterStart);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(';',jslCharPosFree(&forCondStart);jslCharPosFree(&forIterStart);,0);
    if (lex->tk != ')')  { // we could have 'for (;;)'
      JSP_SAVE_EXECUTE();
      jspSetNoExecute();
      jsvUnLock(jspeExpression()); // iterator
      JSP_RESTORE_EXECUTE();
    }
    JslCharPos forBodyStart;
    jslSkipWhiteSpace();
    jslCharPosFromLex(&forBodyStart); // actual for body
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(')',jslCharPosFree(&forCondStart);jslCharPosFree(&forIterStart);jslCharPosFree(&forBodyStart);,0);

    JSP_SAVE_EXECUTE();
    if (!loopCond) jspSetNoExecute();
    execInfo.execute |= EXEC_IN_LOOP;
    jsvUnLock(jspeBlockOrStatement());
    JslCharPos forBodyEnd;
    jslSkipWhiteSpace();
    jslCharPosNew(&forBodyEnd, lex->sourceVar, lex->tokenStart);
    if (!wasInLoop) execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
    if (loopCond || !JSP_SHOULD_EXECUTE) {
      hasHadBreak |= jspeCheckBreakContinue();
    }
    if (!loopCond) JSP_RESTORE_EXECUTE();
    if (loopCond) {
      jslSeekToP(&forIterStart);
      if (lex->tk != ')') jsvUnLock(jspeExpression());
    }
    while (!hasHadBreak && JSP_SHOULD_EXECUTE && loopCond
#ifdef JSPARSE_MAX_LOOP_ITERATIONS
        && loopCount-->0
#endif
    ) {
      jslSeekToP(&forCondStart);
      ;
      if (lex->tk == ';') {
        loopCond = true;
      } else {
        JsVar *cond = jspeExpression();
        loopCond = jsvGetBoolAndUnLock(jsvSkipName(cond));
        jsvUnLock(cond);
      }
      if (JSP_SHOULD_EXECUTE && loopCond) {
        jslSeekToP(&forBodyStart);
        execInfo.execute |= EXEC_IN_LOOP;
        jspDebuggerLoopIfCtrlC();
        jsvUnLock(jspeBlockOrStatement());
        if (!wasInLoop) execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
        hasHadBreak |= jspeCheckBreakContinue();
      }
      if (JSP_SHOULD_EXECUTE && loopCond && !hasHadBreak) {
        jslSeekToP(&forIterStart);
        if (lex->tk != ')') jsvUnLock(jspeExpression());
      }
    }
    jslSeekToP(&forBodyEnd);

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
  if (lex->tk == LEX_R_CATCH) {
    JSP_ASSERT_MATCH(LEX_R_CATCH);
    hadCatch = true;
    JSP_MATCH('(');
    JsVar *scope = 0;
    JsVar *exceptionVar = 0;
    if (hadException) {
      scope = jsvNewObject();
      if (scope)
        exceptionVar = jsvFindChildFromString(scope, jslGetTokenValueAsString(), true);
    }
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID,jsvUnLock2(scope,exceptionVar),0);
    JSP_MATCH_WITH_CLEANUP_AND_RETURN(')',jsvUnLock2(scope,exceptionVar),0);
    if (exceptionVar) {
      // set the exception var up properly
      JsVar *exception = jspGetException();
      if (exception) {
        jsvSetValueOfName(exceptionVar, exception);
        jsvUnLock(exception);
      }
      // Now clear the exception flag (it's handled - we hope!)
      execInfo.execute = execInfo.execute & (JsExecFlags)~(EXEC_EXCEPTION|EXEC_ERROR_LINE_REPORTED);
      jsvUnLock(exceptionVar);
    }

    if (shouldExecuteBefore && !hadException) {
      JSP_SAVE_EXECUTE();
      jspSetNoExecute();
      jspeBlock();
      JSP_RESTORE_EXECUTE();
    } else {
      if (!scope || jspeiAddScope(scope)) {
        jspeBlock();
        if (scope) jspeiRemoveScope();
      }
    }
    jsvUnLock(scope);
  }
  if (lex->tk == LEX_R_FINALLY || (!hadCatch && ((execInfo.execute&(EXEC_ERROR|EXEC_INTERRUPTED))==0))) {
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
  if (lex->tk != ';' && lex->tk != '}') {
    // we only want the value, so skip the name if there was one
    result = jsvSkipNameAndUnLock(jspeExpression());
  }
  if (JSP_SHOULD_EXECUTE) {
    JsVar *resultVar = jspeiFindInScopes(JSPARSE_RETURN_VAR);
    if (resultVar) {
      jsvReplaceWith(resultVar, result);
      jsvUnLock(resultVar);
      execInfo.execute |= EXEC_RETURN; // Stop anything else in this function executing
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

NO_INLINE JsVar *jspeStatementFunctionDecl(bool isClass) {
  JsVar *funcName = 0;
  JsVar *funcVar;

#ifndef SAVE_ON_FLASH
  JSP_ASSERT_MATCH(isClass ? LEX_R_CLASS : LEX_R_FUNCTION);
#else
  JSP_ASSERT_MATCH(LEX_R_FUNCTION);
#endif

  bool actuallyCreateFunction = JSP_SHOULD_EXECUTE;
  if (actuallyCreateFunction) {
    funcName = jsvMakeIntoVariableName(jslGetTokenValueAsVar(), 0);
    if (!funcName) { // out of memory
      return 0;
    }
  }
  JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(funcName), 0);
#ifndef SAVE_ON_FLASH
  funcVar = isClass ? jspeClassDefinition(false) : jspeFunctionDefinition(false);
#else
  funcVar = jspeFunctionDefinition(false);
#endif
  if (actuallyCreateFunction) {
    // find a function with the same name (or make one)
    // OPT: can Find* use just a JsVar that is a 'name'?
    JsVar *existingName = jspeiFindNameOnTop(funcName, true);
    JsVar *existingFunc = jsvSkipName(existingName);
    if (jsvIsFunction(existingFunc)) {
      // 'proper' replace, that keeps the original function var and swaps the children
      funcVar = jsvSkipNameAndUnLock(funcVar);
      jswrap_function_replaceWith(existingFunc, funcVar);
    } else {
      jsvReplaceWith(existingName, funcVar);
    }
    jsvUnLock(funcName);
    funcName = existingName;
    jsvUnLock(existingFunc);
    // existingName is used - don't UnLock
  }
  jsvUnLock(funcVar);
  return funcName;
}

NO_INLINE JsVar *jspeStatement() {
#ifdef USE_DEBUGGER
  if (execInfo.execute&EXEC_DEBUGGER_NEXT_LINE &&
      lex->tk!=';' &&
      JSP_SHOULD_EXECUTE) {
    lex->tokenLastStart = lex->tokenStart;
    jsiDebuggerLoop();
  }
#endif
  if (lex->tk==LEX_ID ||
      lex->tk==LEX_INT ||
      lex->tk==LEX_FLOAT ||
      lex->tk==LEX_STR ||
      lex->tk==LEX_TEMPLATE_LITERAL ||
      lex->tk==LEX_REGEX ||
      lex->tk==LEX_R_NEW ||
      lex->tk==LEX_R_NULL ||
      lex->tk==LEX_R_UNDEFINED ||
      lex->tk==LEX_R_TRUE ||
      lex->tk==LEX_R_FALSE ||
      lex->tk==LEX_R_THIS ||
      lex->tk==LEX_R_DELETE ||
      lex->tk==LEX_R_TYPEOF ||
      lex->tk==LEX_R_VOID ||
      lex->tk==LEX_R_SUPER ||
      lex->tk==LEX_PLUSPLUS ||
      lex->tk==LEX_MINUSMINUS ||
      lex->tk=='!' ||
      lex->tk=='-' ||
      lex->tk=='+' ||
      lex->tk=='~' ||
      lex->tk=='[' ||
      lex->tk=='(') {
    /* Execute a simple statement that only contains basic arithmetic... */
    return jspeExpression();
  } else if (lex->tk=='{') {
    /* A block of code */
    if (!jspCheckStackPosition()) return 0;
    jspeBlock();
    return 0;
  } else if (lex->tk==';') {
    /* Empty statement - to allow things like ;;; */
    JSP_ASSERT_MATCH(';');
    return 0;
  } else if (lex->tk==LEX_R_VAR ||
            lex->tk==LEX_R_LET ||
            lex->tk==LEX_R_CONST) {
    return jspeStatementVar();
  } else if (lex->tk==LEX_R_IF) {
    return jspeStatementIf();
  } else if (lex->tk==LEX_R_DO) {
    return jspeStatementDoOrWhile(false);
  } else if (lex->tk==LEX_R_WHILE) {
    return jspeStatementDoOrWhile(true);
  } else if (lex->tk==LEX_R_FOR) {
    return jspeStatementFor();
  } else if (lex->tk==LEX_R_TRY) {
    return jspeStatementTry();
  } else if (lex->tk==LEX_R_RETURN) {
    return jspeStatementReturn();
  } else if (lex->tk==LEX_R_THROW) {
    return jspeStatementThrow();
  } else if (lex->tk==LEX_R_FUNCTION) {
    return jspeStatementFunctionDecl(false/* function */);
#ifndef SAVE_ON_FLASH
  } else if (lex->tk==LEX_R_CLASS) {
      return jspeStatementFunctionDecl(true/* class */);
#endif
  } else if (lex->tk==LEX_R_CONTINUE) {
    JSP_ASSERT_MATCH(LEX_R_CONTINUE);
    if (JSP_SHOULD_EXECUTE) {
      if (!(execInfo.execute & EXEC_IN_LOOP))
        jsExceptionHere(JSET_SYNTAXERROR, "CONTINUE statement outside of FOR or WHILE loop");
      else
        execInfo.execute = (execInfo.execute & (JsExecFlags)~EXEC_RUN_MASK) | EXEC_CONTINUE;
    }
  } else if (lex->tk==LEX_R_BREAK) {
    JSP_ASSERT_MATCH(LEX_R_BREAK);
    if (JSP_SHOULD_EXECUTE) {
      if (!(execInfo.execute & (EXEC_IN_LOOP|EXEC_IN_SWITCH)))
        jsExceptionHere(JSET_SYNTAXERROR, "BREAK statement outside of SWITCH, FOR or WHILE loop");
      else
        execInfo.execute = (execInfo.execute & (JsExecFlags)~EXEC_RUN_MASK) | EXEC_BREAK;
    }
  } else if (lex->tk==LEX_R_SWITCH) {
    return jspeStatementSwitch();
  } else if (lex->tk==LEX_R_DEBUGGER) {
    JSP_ASSERT_MATCH(LEX_R_DEBUGGER);
#ifdef USE_DEBUGGER
    if (JSP_SHOULD_EXECUTE)
      jsiDebuggerLoop();
#endif
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

/// Create a new Class of the given instance and return its prototype (as a name 'prototype')
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
  jsvUnLock2(objFunc, objFuncName);

  return prototypeName;
}

/** Create a new object of the given instance and add it to root with name 'name'.
 * If name!=0, added to root with name, and the name is returned
 * If name==0, not added to root and Object itself returned */
NO_INLINE JsVar *jspNewObject(const char *name, const char *instanceOf) {
  JsVar *prototypeName = jspNewPrototype(instanceOf);

  JsVar *obj = jsvNewObject();
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
  jsvUnLock3(jsvAddNamedChild(obj, prototypeVar, JSPARSE_INHERITS_VAR), prototypeVar, prototypeName);prototypeName=0;

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

/** Get the prototype of the given object, or return 0 if not found, or not an object */
JsVar *jspGetPrototype(JsVar *object) {
  if (!jsvIsObject(object)) return 0;
  JsVar *proto = jsvObjectGetChild(object, JSPARSE_INHERITS_VAR, 0);
  if (jsvIsObject(proto))
    return proto;
  jsvUnLock(proto);
  return 0;
}

/** Get the constructor of the given object, or return 0 if not found, or not a function */
JsVar *jspGetConstructor(JsVar *object) {
  JsVar *proto = jspGetPrototype(object);
  if (proto) {
    JsVar *constr = jsvObjectGetChild(proto, JSPARSE_CONSTRUCTOR_VAR, 0);
    if (jsvIsFunction(constr)) {
      jsvUnLock(proto);
      return constr;
    }
    jsvUnLock2(constr, proto);
  }
  return 0;
}

// -----------------------------------------------------------------------------

void jspSoftInit() {
  execInfo.root = jsvFindOrCreateRoot();
  // Root now has a lock and a ref
  execInfo.hiddenRoot = jsvObjectGetChild(execInfo.root, JS_HIDDEN_CHAR_STR, JSV_OBJECT);
  execInfo.execute = EXEC_YES;
}

void jspSoftKill() {
  jsvUnLock(execInfo.scopesVar);
  execInfo.scopesVar = 0;
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

/** Evaluate the given variable as an expression (in current scope) */
JsVar *jspEvaluateExpressionVar(JsVar *str) {
  JsLex lex;

  assert(jsvIsString(str));
  JsLex *oldLex = jslSetLex(&lex);
  jslInit(str);
#ifndef ESPR_NO_LINE_NUMBERS
  lex.lineNumberOffset = oldLex->lineNumberOffset;
#endif

  // actually do the parsing
  JsVar *v = jspeExpression();
  jslKill();
  jslSetLex(oldLex);

  return jsvSkipNameAndUnLock(v);
}

/** Execute code form a variable and return the result. If lineNumberOffset
 * is nonzero it's added to the line numbers that get reported for errors/debug */
JsVar *jspEvaluateVar(JsVar *str, JsVar *scope, uint16_t lineNumberOffset) {
  JsLex lex;

  assert(jsvIsString(str));
  JsLex *oldLex = jslSetLex(&lex);
  jslInit(str);
#ifndef ESPR_NO_LINE_NUMBERS
  lex.lineNumberOffset = lineNumberOffset;
#endif


  JsExecInfo oldExecInfo = execInfo;
  execInfo.execute = EXEC_YES;
  if (scope) {
    // if we're adding a scope, make sure it's the *only* scope
    execInfo.scopesVar = 0;
    if (scope!=execInfo.root) jspeiAddScope(scope); // it's searched by default anyway
  }

  // actually do the parsing
  JsVar *v = jspParse();
  // clean up
  if (scope) jspeiClearScopes();
  jslKill();
  jslSetLex(oldLex);

  // restore state and execInfo (keep error flags & ctrl-c)
  oldExecInfo.execute |= execInfo.execute & EXEC_PERSIST;
  execInfo = oldExecInfo;

  // It may have returned a reference, but we just want the value...
  return jsvSkipNameAndUnLock(v);
}

JsVar *jspEvaluate(const char *str, bool stringIsStatic) {

  /* using a memory area is more efficient, but the interpreter
   * may use substrings from it for function code. This means that
   * if the string goes away, everything gets corrupted - hence
   * the option here.
   */
  JsVar *evCode;
  if (stringIsStatic)
    evCode = jsvNewNativeString((char*)str, strlen(str));
  else
    evCode = jsvNewFromString(str);
  if (!evCode) return 0;

  JsVar *v = 0;
  if (!jsvIsMemoryFull())
    v = jspEvaluateVar(evCode, 0, 0);
  jsvUnLock(evCode);

  return v;
}

JsVar *jspExecuteJSFunction(const char *jsCode, JsVar *thisArg, int argCount, JsVar **argPtr) {
  JsVar *fn = jspEvaluate(jsCode,true);
  JsVar *result = jspExecuteFunction(fn,thisArg,argCount,argPtr);
  jsvUnLock(fn);
  return result;
}

JsVar *jspExecuteFunction(JsVar *func, JsVar *thisArg, int argCount, JsVar **argPtr) {
  JsExecInfo oldExecInfo = execInfo;
  execInfo.scopesVar = 0;
  execInfo.execute = EXEC_YES;
  execInfo.thisVar = 0;
  JsVar *result = jspeFunctionCall(func, 0, thisArg, false, argCount, argPtr);
  // clean up
  jspeiClearScopes();
  // restore state and execInfo (keep error flags & ctrl-c)
  oldExecInfo.execute |= execInfo.execute&EXEC_PERSIST;
  jspeiClearScopes();
  execInfo = oldExecInfo;

  return result;
}


/// Evaluate a JavaScript module and return its exports
JsVar *jspEvaluateModule(JsVar *moduleContents) {
  assert(jsvIsString(moduleContents) || jsvIsFunction(moduleContents));
  if (jsvIsFunction(moduleContents)) {
    moduleContents = jsvObjectGetChild(moduleContents,JSPARSE_FUNCTION_CODE_NAME,0);
    if (!jsvIsString(moduleContents)) {
      jsvUnLock(moduleContents);
      return 0;
    }
  } else
    jsvLockAgain(moduleContents);
  JsVar *scope = jsvNewObject();
  JsVar *scopeExports = jsvNewObject();
  if (!scope || !scopeExports) { // out of mem
    jsvUnLock3(scope, scopeExports, moduleContents);
    return 0;
  }
  JsVar *exportsName = jsvAddNamedChild(scope, scopeExports, "exports");
  jsvUnLock2(scopeExports, jsvAddNamedChild(scope, scope, "module"));

  JsExecFlags oldExecute = execInfo.execute;
  JsVar *oldThisVar = execInfo.thisVar;
  execInfo.thisVar = scopeExports; // set 'this' variable to exports
  jsvUnLock(jspEvaluateVar(moduleContents, scope, 0));
  execInfo.thisVar = oldThisVar;
  execInfo.execute = oldExecute; // make sure we fully restore state after parsing a module

  jsvUnLock2(moduleContents, scope);
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
