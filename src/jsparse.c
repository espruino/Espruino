#include "jsparse.h"
#include "jsfunctions.h"
#include "jsinteractive.h"

/* Info about execution when Parsing - this saves passing it on the stack
 * for each call */
JsExecInfo execInfo;

// ----------------------------------------------- Forward decls
JsVar *jspeBase();
JsVar *jspeBlock();
JsVar *jspeStatement();
// ----------------------------------------------- Utils
#define JSP_MATCH_WITH_CLEANUP_AND_RETURN(TOKEN, CLEANUP_CODE, RETURN_VAL) { if (!jslMatch(execInfo.lex,(TOKEN))) { jspSetError(); CLEANUP_CODE; return RETURN_VAL; } }
#define JSP_MATCH_WITH_RETURN(TOKEN, RETURN_VAL) JSP_MATCH_WITH_CLEANUP_AND_RETURN(TOKEN, , RETURN_VAL)
#define JSP_MATCH(TOKEN) JSP_MATCH_WITH_CLEANUP_AND_RETURN(TOKEN, , 0)
#define JSP_SHOULD_EXECUTE (((execInfo.execute)&EXEC_RUN_MASK)==EXEC_YES)
#define JSP_SAVE_EXECUTE() JsExecFlags oldExecute = execInfo.execute
#define JSP_RESTORE_EXECUTE() execInfo.execute = (execInfo.execute&(JsExecFlags)(~EXEC_SAVE_RESTORE_MASK)) | (oldExecute&EXEC_SAVE_RESTORE_MASK);
#define JSP_HAS_ERROR (((execInfo.execute)&EXEC_ERROR_MASK)!=0)

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

static inline void jspSetError() {
  execInfo.execute = (execInfo.execute & (JsExecFlags)~EXEC_YES) | EXEC_ERROR;
}

///< Same as jsvSetValueOfName, but nice error message
JsVar *jspReplaceWith(JsVar *dst, JsVar *src) {
  // if desination isn't there, isn't a 'name', or is used, just return source
  if (!jsvIsName(dst)) {
    jsErrorAt("Unable to assign value to non-reference", execInfo.lex, execInfo.lex->tokenLastEnd);
    jspSetError();
    return dst;
  }
  return jsvSetValueOfName(dst, src);
}

void jspeiInit(JsParse *parse, JsLex *lex) {
  execInfo.parse = parse;
  execInfo.lex = lex;
  execInfo.scopeCount = 0;
  execInfo.execute = EXEC_YES;
}

void jspeiKill() {
  execInfo.parse = 0;
  execInfo.lex = 0;
  assert(execInfo.scopeCount==0);
}

bool jspeiAddScope(JsVarRef scope) {
  if (execInfo.scopeCount >= JSPARSE_MAX_SCOPES) {
    jsError("Maximum number of scopes exceeded");
    jspSetError();
    return false;
  }
  execInfo.scopes[execInfo.scopeCount++] = jsvRefRef(scope);
  return true;
}

void jspeiRemoveScope() {
  if (execInfo.scopeCount <= 0) {
    jsError("INTERNAL: Too many scopes removed");
    jspSetError();
    return;
  }
  jsvUnRefRef(execInfo.scopes[--execInfo.scopeCount]);
}

JsVar *jspeiFindInScopes(const char *name) {
  int i;
  for (i=execInfo.scopeCount-1;i>=0;i--) {
    JsVar *ref = jsvFindChildFromString(execInfo.scopes[i], name, false);
    if (ref) return ref;
  }
  return jsvFindChildFromString(execInfo.parse->root, name, false);
}

JsVar *jspeiFindOnTop(const char *name, bool createIfNotFound) {
  if (execInfo.scopeCount>0)
    return jsvFindChildFromString(execInfo.scopes[execInfo.scopeCount-1], name, createIfNotFound);
  return jsvFindChildFromString(execInfo.parse->root, name, createIfNotFound);
}
JsVar *jspeiFindNameOnTop(JsVar *childName, bool createIfNotFound) {
  if (execInfo.scopeCount>0)
    return jsvFindChildFromVar(execInfo.scopes[execInfo.scopeCount-1], childName, createIfNotFound);
  return jsvFindChildFromVar(execInfo.parse->root, childName, createIfNotFound);
}

/** Here we assume that we have already looked in the parent itself -
 * and are now going down looking at the stuff it inherited */
JsVar *jspeiFindChildFromStringInParents(JsVar *parent, const char *name) {
  if (jsvIsObject(parent)) {
    // If an object, look for an 'inherits' var
    JsVar *inheritsFrom = jsvSkipNameAndUnlock(jsvFindChildFromString(jsvGetRef(parent), JSPARSE_INHERITS_VAR, false));

    // if there's no inheritsFrom, just default to 'Object.prototype'
    if (!inheritsFrom) {
      JsVar *obj = jsvSkipNameAndUnlock(jsvFindChildFromString(execInfo.parse->root, "Object", false));
      if (obj) {
        inheritsFrom = jsvSkipNameAndUnlock(jsvFindChildFromString(jsvGetRef(obj), JSPARSE_PROTOTYPE_VAR, false));
        jsvUnLock(obj);
      }
    }

    if (inheritsFrom) {
      // we have what it inherits from (this is ACTUALLY the prototype var)
      // https://developer.mozilla.org/en-US/docs/JavaScript/Reference/Global_Objects/Object/proto
      JsVar *child = jsvFindChildFromString(jsvGetRef(inheritsFrom), name, false);
      jsvUnLock(inheritsFrom);
      if (child) return child;
    }
  } else { // Not actually an object - but might be an array/string/etc
    const char *objectName = jsvGetBasicObjectName(parent);
    while (objectName) {
      JsVar *objName = jsvFindChildFromString(execInfo.parse->root, objectName, false);
      if (objName) {
        JsVar *result = 0;
        JsVar *obj = jsvSkipNameAndUnlock(objName);
        if (obj) {
          // We have found an object with this name - search for the prototype var
          JsVar *proto = jsvSkipNameAndUnlock(jsvFindChildFromString(jsvGetRef(obj), JSPARSE_PROTOTYPE_VAR, false));
          if (proto) {
            result = jsvFindChildFromString(jsvGetRef(proto), name, false);
            jsvUnLock(proto);
          }
          jsvUnLock(obj);
        }
        if (result) return result;
      }
      // We haven't found anything in the actual object, we should check the 'Object' itself
      // eg, we tried 'String', so now we should try 'Object'
      if (strcmp(objectName,"Object")!=0)
        objectName = "Object";
      else
        objectName = 0;
    }
  }

  // no luck!
  return 0;
}

JsVar *jspeiGetScopesAsVar() {
  if (execInfo.scopeCount==0) return 0;
  JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
  int i;
  for (i=0;i<execInfo.scopeCount;i++) {
      //printf("%d %d\n",i,execInfo.scopes[i]);
      JsVar *scope = jsvLock(execInfo.scopes[i]);
      JsVar *idx = jsvMakeIntoVariableName(jsvNewFromInteger(i), scope);
      jsvUnLock(scope);
      if (!idx) { // out of memort
        jspSetError();
        return arr;
      }
      jsvAddName(arr, idx);
      jsvUnLock(idx);
  }
  //printf("%d\n",arr->firstChild);
  return arr;
}

void jspeiLoadScopesFromVar(JsVar *arr) {
    execInfo.scopeCount = 0;
    //printf("%d\n",arr->firstChild);
    JsVarRef childref = arr->firstChild;
    while (childref) {
      JsVar *child = jsvLock(childref);
      //printf("%d %d %d %d\n",execInfo.scopeCount,childref,child, child->firstChild);
      execInfo.scopes[execInfo.scopeCount] = jsvRefRef(child->firstChild);
      execInfo.scopeCount++;
      childref = child->nextSibling;
      jsvUnLock(child);
    }
}
// -----------------------------------------------
#ifdef ARM
extern int _end;
#endif
bool jspCheckStackPosition() {
#ifdef ARM
  void *frame = __builtin_frame_address(0);
  if ((char*)frame < ((char*)&_end)+1024/*so many bytes leeway*/) {
/*    jsiConsolePrint("frame:");
    jsiConsolePrintInt((int)frame);
    jsiConsolePrint(",end:");
    jsiConsolePrintInt((int)&_end);
    jsiConsolePrint("\n");*/
    jsErrorAt("Too much recursion - the stack is about to overflow", execInfo.lex, execInfo.lex->tokenStart );
    jspSetInterrupted(true);
    return false;
  }
#endif
  return true;
}


// Set execFlags such that we are not executing
void jspSetNoExecute() {
  execInfo.execute = (execInfo.execute & (JsExecFlags)(int)~EXEC_RUN_MASK) | EXEC_NO;
}

// parse single variable name
bool jspParseVariableName() {
  JSP_MATCH(LEX_ID);
  return true;
}

// parse function with no arguments
bool jspParseEmptyFunction() {
  JSP_MATCH(LEX_ID);
  JSP_MATCH('(');
  if (execInfo.lex->tk != ')')
    jsvUnLock(jspeBase());
  // throw away extra params
  while (!JSP_HAS_ERROR && execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    jsvUnLock(jspeBase());
  }
  JSP_MATCH(')');
  return true;
}

// parse function with a single argument, return its value (no names!)
JsVar *jspParseSingleFunction() {
  JsVar *v = 0;
  JSP_MATCH(LEX_ID);
  JSP_MATCH('(');
  if (execInfo.lex->tk != ')')
    v = jsvSkipNameAndUnlock(jspeBase());
  // throw away extra params
  while (!JSP_HAS_ERROR && execInfo.lex->tk != ')') {
    JSP_MATCH_WITH_RETURN(',', v);
    jsvUnLock(jspeBase());
  }
  JSP_MATCH_WITH_RETURN(')', v);
  return v;
}

/// parse function with max 4 arguments (can set arg to 0 to avoid parse). Usually first arg will be 0, but if we DON'T want to skip names on an arg stuff, we can say
bool jspParseFunction(JspSkipFlags skipName, JsVar **a, JsVar **b, JsVar **c, JsVar **d) {
  if (a) *a = 0;
  if (b) *b = 0;
  if (c) *c = 0; 
  if (d) *d = 0;
  JSP_MATCH(LEX_ID);
  JSP_MATCH('(');
  if (a && execInfo.lex->tk != ')') {
    *a = jspeBase();
    if (!(skipName&JSP_NOSKIP_A)) *a = jsvSkipNameAndUnlock(*a);
  }
  if (b && execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    *b = jspeBase();
    if (!(skipName&JSP_NOSKIP_B)) *b = jsvSkipNameAndUnlock(*b);
  }
  if (c && execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    *c = jspeBase();
    if (!(skipName&JSP_NOSKIP_C)) *c = jsvSkipNameAndUnlock(*c);
  }
  if (d && execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    *d = jspeBase();
    if (!(skipName&JSP_NOSKIP_D)) *d = jsvSkipNameAndUnlock(*d);
  }
  // throw away extra params
  while (!JSP_HAS_ERROR && execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    jsvUnLock(jspeBase());
  }
  JSP_MATCH(')');
  return true;
}
// ----------------------------------------------

// we return a value so that JSP_MATCH can return 0 if it fails (if we pass 0, we just parse all args)
bool jspeFunctionArguments(JsVar *funcVar) {
  JSP_MATCH('(');
  while (execInfo.lex->tk!=')') {
      if (funcVar) {
        JsVar *param = jsvAddNamedChild(funcVar, 0, jslGetTokenValueAsString(execInfo.lex));
        if (!param) { // out of memory
          jspSetError();
          return false;
        }
        param->flags = JSV_FUNCTION_PARAMETER;
        jsvUnLock(param);
      }
      JSP_MATCH(LEX_ID);
      if (execInfo.lex->tk!=')') JSP_MATCH(',');
  }
  JSP_MATCH(')');
  return true;
}

bool jspeParseNativeFunction(JsCallback callbackPtr) {
    char funcName[JSLEX_MAX_TOKEN_LENGTH];
    JsVar *funcVar;
    JsVar *base = jsvLock(execInfo.parse->root);
    JSP_MATCH(LEX_R_FUNCTION);
    // not too bothered about speed/memory here as only called on init :)
    strncpy(funcName, jslGetTokenValueAsString(execInfo.lex), JSLEX_MAX_TOKEN_LENGTH);
    JSP_MATCH(LEX_ID);
    /* Check for dots, we might want to do something like function 'String.substring' ... */
    while (execInfo.lex->tk == '.') {
      JsVar *link;
      JSP_MATCH('.');
      link = jsvFindChildFromString(jsvGetRef(base), funcName, false);
      // if it doesn't exist, make a new object class
      if (!link) {
        JsVar *obj = jsvNewWithFlags(JSV_OBJECT);
        link = jsvAddNamedChild(base, obj, funcName);
        jsvUnLock(obj);
      }
      // set base to the object (not the name)
      jsvUnLock(base);
      base = jsvSkipNameAndUnlock(link);
      // Look for another name
      strncpy(funcName, jslGetTokenValueAsString(execInfo.lex), JSLEX_MAX_TOKEN_LENGTH);
      JSP_MATCH(LEX_ID);
    }
    // So now, base points to an object where we want our function
    funcVar = jsvNewWithFlags(JSV_FUNCTION | JSV_NATIVE);
    if (!funcVar) {
      jsvUnLock(base);
      jspSetError();
      return false; // Out of memory
    }
    funcVar->varData.callback = callbackPtr;
    jspeFunctionArguments(funcVar);

    if (JSP_HAS_ERROR) { // probably out of memory while parsing
      jsvUnLock(base);
      jsvUnLock(funcVar);
      return false;
    }
    // Add the function with its name
    jsvUnLock(jsvAddNamedChild(base, funcVar, funcName));
    jsvUnLock(base);
    jsvUnLock(funcVar);
    return true;
}

bool jspAddNativeFunction(JsParse *parse, const char *funcDesc, JsCallback callbackPtr) {
    JsVar *fncode = jsvNewFromString(funcDesc);
    if (!fncode) return false; // out of memory!

    JSP_SAVE_EXECUTE();
    JsExecInfo oldExecInfo = execInfo;

    // Set up Lexer

    JsLex lex;
    jslInit(&lex, fncode, 0, -1);
    jsvUnLock(fncode);

    
    jspeiInit(parse, &lex);

    // Parse
    bool success = jspeParseNativeFunction(callbackPtr);
    if (!success) {
      jsError("Parsing Native Function failed!");
      jspSetError();
    }


    // cleanup
    jspeiKill();
    jslKill(&lex);
    JSP_RESTORE_EXECUTE();
    oldExecInfo.execute = execInfo.execute; // JSP_RESTORE_EXECUTE has made this ok.
    execInfo = oldExecInfo;

    return success;
}

JsVar *jspeFunctionDefinition() {
  int funcBegin;
  // actually parse a function... We assume that the LEX_FUNCTION and name
  // have already been parsed
  JsVar *funcVar = 0;
  if (JSP_SHOULD_EXECUTE)
    funcVar = jsvNewWithFlags(JSV_FUNCTION);
  // Get arguments save them to the structure
  if (!jspeFunctionArguments(funcVar)) {
    jsvUnLock(funcVar);
    // parse failed
    return 0;
  }
  // Get the code - first parse it so we know where it stops
  funcBegin = execInfo.lex->tokenStart;
  JSP_SAVE_EXECUTE();
  jspSetNoExecute();
  jsvUnLock(jspeBlock());
  JSP_RESTORE_EXECUTE();
  // Then create var and set
  if (JSP_SHOULD_EXECUTE) {
    // code var
    JsVar *funcCodeVar = jsvNewFromLexer(execInfo.lex, funcBegin, execInfo.lex->tokenLastEnd+1);
    jsvUnLock(jsvAddNamedChild(funcVar, funcCodeVar, JSPARSE_FUNCTION_CODE_NAME));
    jsvUnLock(funcCodeVar);
    // scope var
    JsVar *funcScopeVar = jspeiGetScopesAsVar();
    if (funcScopeVar) {
      jsvUnLock(jsvAddNamedChild(funcVar, funcScopeVar, JSPARSE_FUNCTION_SCOPE_NAME));
      jsvUnLock(funcScopeVar);
    }
  }
  return funcVar;
}

/* Parse just the brackets of a function - and throw
 * everything away */
bool jspeParseFunctionCallBrackets() {
  JSP_MATCH('(');
  while (!JSP_HAS_ERROR && execInfo.lex->tk != ')') {
    jsvUnLock(jspeBase());
    if (execInfo.lex->tk!=')') JSP_MATCH(',');
  }
  if (!JSP_HAS_ERROR) JSP_MATCH(')');
  return 0;
}

/** Handle a function call (assumes we've parsed the function name and we're
 * on the start bracket). 'parent' is the object that contains this method,
 * if there was one (otherwise it's just a normal function).
 * If !isParsing and arg0!=0, argument 0 is set to what is supplied
 *
 * functionName is used only for error reporting - and can be 0
 */
JsVar *jspeFunctionCall(JsVar *function, JsVar *functionName, JsVar *parent, bool isParsing, JsVar *arg0) {
  if (JSP_SHOULD_EXECUTE && !function) {
      jsWarnAt("Function not found! Skipping.", execInfo.lex, execInfo.lex->tokenLastEnd );
  }

  if (JSP_SHOULD_EXECUTE) jspCheckStackPosition(); // try and ensure that we won't overflow our stack

  if (JSP_SHOULD_EXECUTE && function) {
    JsVar *functionRoot;
    JsVar *returnVarName;
    JsVar *returnVar;
    JsVarRef v;
    if (!jsvIsFunction(function)) {
        jsErrorAt("Expecting a function to call", execInfo.lex, execInfo.lex->tokenLastEnd );
        jspSetError();
        return 0;
    }
    if (isParsing) JSP_MATCH('(');
    // create a new symbol table entry for execution of this function
    // OPT: can we cache this function execution environment + param variables?
    // OPT: Probably when calling a function ONCE, use it, otherwise when recursing, make new?
    functionRoot = jsvNewWithFlags(JSV_FUNCTION);
    if (!functionRoot) { // out of memory
      jspSetError();
      return 0;
    }
    JsVar *thisVar = 0;
    if (parent)
        thisVar = jsvAddNamedChild(functionRoot, parent, JSPARSE_THIS_VAR);
    if (isParsing) {
      // grab in all parameters
      v = function->firstChild;
      while (!JSP_HAS_ERROR && v) {
        JsVar *param = jsvLock(v);
        if (jsvIsFunctionParameter(param)) {
          JsVar *valueName = 0;
          // ONLY parse this if it was supplied, otherwise leave 0 (undefined)
          if (execInfo.lex->tk!=')')
            valueName = jspeBase();
          // and if execute, copy it over
          if (JSP_SHOULD_EXECUTE) {
            JsVar *value = jsvSkipName(valueName);
            // TODO: deep copy required?
            /*if (jsvIsBasic(value)) {
              // pass by value
              jsvAddNamedChild(functionRoot, v->name, value->var->deepCopy());
            } else {
              // pass by reference
              jsvAddNamedChild(functionRoot, v->name, value);
            }*/
            JsVar *newValueName = jsvMakeIntoVariableName(jsvCopy(param), value);
            if (newValueName) { // could be out of memory
              jsvAddName(functionRoot, newValueName);
            } else
              jspSetError();
            jsvUnLock(newValueName);
            jsvUnLock(value);
          }
          jsvUnLock(valueName);
          if (execInfo.lex->tk!=')') JSP_MATCH(',');
        }
        v = param->nextSibling;
        jsvUnLock(param);
      }
      // throw away extra params
      while (execInfo.lex->tk != ')') {
        JSP_MATCH(',');
        jsvUnLock(jspeBase());
      }
      JSP_MATCH(')');
    } else if (JSP_SHOULD_EXECUTE && arg0!=0) {  // and NOT isParsing 
      bool foundArg = false;
      v = function->firstChild;
      while (!foundArg && v) {
        JsVar *param = jsvLock(v);
        if (jsvIsFunctionParameter(param)) {
          JsVar *newValueName = jsvMakeIntoVariableName(jsvCopy(param), arg0);
          if (newValueName) { // could be out of memory
            jsvAddName(functionRoot, newValueName);
          } else 
            jspSetError();
          jsvUnLock(newValueName);
          foundArg = true;
        }
        v = param->nextSibling;
        jsvUnLock(param);
      }
    } 
    // setup a return variable
    returnVarName = jsvAddNamedChild(functionRoot, 0, JSPARSE_RETURN_VAR);
    if (!returnVarName) // out of memory
      jspSetError();
    //jsvTrace(jsvGetRef(functionRoot), 5); // debugging
#ifdef JSPARSE_CALL_STACK
    call_stack.push_back(function->name + " from " + l->getPosition());
#endif

    if (!JSP_HAS_ERROR) {
      if (jsvIsNative(function)) {
        assert(function->varData.callback);
        function->varData.callback(jsvGetRef(functionRoot));
      } else {
        // save old scopes
        JsVarRef oldScopes[JSPARSE_MAX_SCOPES];
        int oldScopeCount;
        int i;
        oldScopeCount = execInfo.scopeCount;
        for (i=0;i<execInfo.scopeCount;i++)
          oldScopes[i] = execInfo.scopes[i];
        // if we have a scope var, load it up. We may not have one if there were no scopes apart from root
        JsVar *functionScope = jsvFindChildFromString(jsvGetRef(function), JSPARSE_FUNCTION_SCOPE_NAME, false);
        if (functionScope) {
            JsVar *functionScopeVar = jsvLock(functionScope->firstChild);
            //jsvTrace(jsvGetRef(functionScopeVar),5);
            jspeiLoadScopesFromVar(functionScopeVar);
            jsvUnLock(functionScopeVar);
            jsvUnLock(functionScope);
        } else {
            // no scope var defined? We have no scopes at all!
            execInfo.scopeCount = 0;
        }
        // add the function's execute space to the symbol table so we can recurse
        if (jspeiAddScope(jsvGetRef(functionRoot))) {
          /* Adding scope may have failed - we may have descended too deep - so be sure
           * not to pull somebody else's scope off
           */

          /* we just want to execute the block, but something could
           * have messed up and left us with the wrong ScriptLex, so
           * we want to be careful here... */

          JsVar *functionCode = jsvFindChildFromString(jsvGetRef(function), JSPARSE_FUNCTION_CODE_NAME, false);
          if (functionCode) {
            JsLex *oldLex;
            JsVar* functionCodeVar = jsvSkipNameAndUnlock(functionCode);
            JsLex newLex;
            jslInit(&newLex, functionCodeVar, 0, -1);
            jsvUnLock(functionCodeVar);

            oldLex = execInfo.lex;
            execInfo.lex = &newLex;
            JSP_SAVE_EXECUTE();
            jspeBlock();
            bool hasError = JSP_HAS_ERROR;
            JSP_RESTORE_EXECUTE(); // because return will probably have set execute to false
            jslKill(&newLex);
            execInfo.lex = oldLex;
            if (hasError) {
              jsiConsolePrint("in function ");
              if (jsvIsString(functionName)) {
                jsiConsolePrint("\"");
                jsiConsolePrintStringVar(functionName);
                jsiConsolePrint("\" ");
              }
              jsiConsolePrint("called from ");
              if (execInfo.lex)
                jsiConsolePrintPosition(execInfo.lex, execInfo.lex->tokenLastEnd);
              else
                jsiConsolePrint("system\n");
              jspSetError();
            }
          }

          jspeiRemoveScope();
        }

        // Unref old scopes
        for (i=0;i<execInfo.scopeCount;i++)
            jsvUnRefRef(execInfo.scopes[i]);
        // restore function scopes
        for (i=0;i<oldScopeCount;i++)
            execInfo.scopes[i] = oldScopes[i];
        execInfo.scopeCount = oldScopeCount;
      }
    }
#ifdef JSPARSE_CALL_STACK
    if (!call_stack.empty()) call_stack.pop_back();
#endif

    /* Now remove 'this' var */
    if (thisVar) {
        jsvRemoveChild(functionRoot, thisVar);
        jsvUnLock(thisVar);
        thisVar = 0;
    }
    /* get the real return var before we remove it from our function */
    returnVar = jsvSkipNameAndUnlock(returnVarName);
    if (returnVarName) // could have failed with out of memory
      jsvSetValueOfName(returnVarName, 0); // remove return value (which helps stops circular references)
    jsvUnLock(functionRoot);
    if (returnVar)
      return returnVar;
    else
      return 0;
  } else if (isParsing) { // ---------------------------------- function, but not executing - just parse args and be done
    jspeParseFunctionCallBrackets();
    /* Do not return function, as it will be unlocked! */
    return 0;
  } else return 0;
}

JsVar *jspeFactorID() {
  JsVar *a = JSP_SHOULD_EXECUTE ? jspeiFindInScopes(jslGetTokenValueAsString(execInfo.lex)) : 0;
  if (JSP_SHOULD_EXECUTE && !a) {
    const char *tokenName = jslGetTokenValueAsString(execInfo.lex); // BEWARE - this won't hang around forever!
    /* Special case! We haven't found the variable, so check out
     * and see if it's one of our builtins...  */
    if (jsvIsBuiltInObject(tokenName)) {
      JsVar *obj = jsvNewWithFlags(JSV_FUNCTION); // yes, really a function :/.
      if (obj) { // out of memory?
        JsVar *root = jsvLock(execInfo.parse->root);
        a = jsvAddNamedChild(root, obj, tokenName);
        jsvUnLock(root);
        jsvUnLock(obj);
      }
    } else {
      a = jsfHandleFunctionCall(&execInfo, 0, 0, tokenName);
      if (a != JSFHANDLEFUNCTIONCALL_UNHANDLED)
        return a;
      /* Variable doesn't exist! JavaScript says we should create it
       * (we won't add it here. This is done in the assignment operator)*/
      a = jsvMakeIntoVariableName(jslGetTokenValueAsVar(execInfo.lex), 0);
    }
  }
  JSP_MATCH_WITH_RETURN(LEX_ID, a);

  return a;
}

JsVar *jspeFactor() {
    if (execInfo.lex->tk=='(') {
        JsVar *a = 0;
        JSP_MATCH('(');
        if (jspCheckStackPosition())
          a = jspeBase();
        if (!JSP_HAS_ERROR) JSP_MATCH_WITH_RETURN(')',a);
        return a;
    }
    if (execInfo.lex->tk==LEX_R_TRUE) {
        JSP_MATCH(LEX_R_TRUE);
        return JSP_SHOULD_EXECUTE ? jsvNewFromBool(true) : 0;
    }
    if (execInfo.lex->tk==LEX_R_FALSE) {
        JSP_MATCH(LEX_R_FALSE);
        return JSP_SHOULD_EXECUTE ? jsvNewFromBool(false) : 0;
    }
    if (execInfo.lex->tk==LEX_R_NULL) {
        JSP_MATCH(LEX_R_NULL);
        return JSP_SHOULD_EXECUTE ? jsvNewWithFlags(JSV_NULL) : 0;
    }
    if (execInfo.lex->tk==LEX_R_UNDEFINED) {
        JSP_MATCH(LEX_R_UNDEFINED);
        return 0;
    }
    if (execInfo.lex->tk==LEX_ID) {
        /* The parent if we're executing a method call */
        JsVar *parent = 0;
        JsVar *a = jspeFactorID();

        while (execInfo.lex->tk=='(' || execInfo.lex->tk=='.' || execInfo.lex->tk=='[') {
            if (execInfo.lex->tk=='(') { // ------------------------------------- Function Call
              JsVar *funcName = a;
              JsVar *func = jsvSkipName(funcName);
              a = jspeFunctionCall(func, funcName, parent, true, 0);
              jsvUnLock(funcName);
              jsvUnLock(func);
            } else if (execInfo.lex->tk == '.') { // ------------------------------------- Record Access
                JSP_MATCH('.');
                if (JSP_SHOULD_EXECUTE) {
                  // Note: name will go away when we oarse something else!
                  const char *name = jslGetTokenValueAsString(execInfo.lex);

                  JsVar *aVar = jsvSkipName(a);
                  JsVar *child = 0;
                  if (aVar && jsvGetBasicObjectName(aVar)) {
                      child = jsvFindChildFromString(jsvGetRef(aVar), name, false);

                      if (!child)
                        child = jspeiFindChildFromStringInParents(aVar, name);

                      if (child) {
                        // it was found - no need for name ptr now, so match!
                        JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(parent);jsvUnLock(a);, child);
                      } else { // NOT FOUND...
                        /* Check for builtins via separate function
                         * This way we save on RAM for built-ins because all comes out of program code. */
                        child = jsfHandleFunctionCall(&execInfo, aVar, a/*name*/, name);
                        if (child == JSFHANDLEFUNCTIONCALL_UNHANDLED) {
                          child = 0;
                          // It wasn't handled... We already know this is an object so just add a new child
                          if (jsvIsObject(aVar) || jsvIsFunction(aVar) || jsvIsArray(aVar)) {
                            JsVar *value = 0;
                            if (jsvIsFunction(aVar) && strcmp(name, JSPARSE_PROTOTYPE_VAR)==0)
                              value = jsvNewWithFlags(JSV_ARRAY); // prototype is supposed to be an array
                            child = jsvAddNamedChild(aVar, value, name);
                            jsvUnLock(value);
                          } else {
                            // could have been a string...
                            jsWarnAt("Using '.' operator on non-object", execInfo.lex, execInfo.lex->tokenLastEnd);
                          }
                          JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(parent);jsvUnLock(a);, child);
                        }
                      }
                  } else {
                      jsWarnAt("Using '.' operator on non-object", execInfo.lex, execInfo.lex->tokenLastEnd);
                  }
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
                JSP_MATCH('[');
                index = jspeBase();
                JSP_MATCH_WITH_CLEANUP_AND_RETURN(']', jsvUnLock(parent);jsvUnLock(index);, a);
                if (JSP_SHOULD_EXECUTE) {
                  JsVar *aVar = jsvSkipName(a);
                  if (aVar && (jsvIsArray(aVar) || jsvIsObject(aVar))) {
                      // TODO: If we set to undefined, maybe we should remove the name?
                      JsVar *indexValue = jsvSkipName(index);
                      JsVar *child = jsvFindChildFromVar(jsvGetRef(aVar), indexValue, true);
                      jsvUnLock(indexValue);

                      jsvUnLock(parent);
                      parent = aVar;
                      jsvUnLock(a);
                      a = child;
                  } else {
                      jsWarnAt("Variable is not an Array or Object", execInfo.lex, execInfo.lex->tokenLastEnd);
                      jsvUnLock(parent);
                      parent = 0;
                      jsvUnLock(a);
                      a = 0;
                  }
                }
                jsvUnLock(index);
            } else {
              assert(0);
            }
        }
        jsvUnLock(parent);
        return a;
    }
    if (execInfo.lex->tk==LEX_INT) {
        // atol works only on decimals
        // strtol handles 0x12345 as well
        //JsVarInt v = (JsVarInt)atol(jslGetTokenValueAsString(execInfo.lex));
        //JsVarInt v = (JsVarInt)strtol(jslGetTokenValueAsString(execInfo.lex),0,0); // broken on PIC
        if (JSP_SHOULD_EXECUTE) {
          JsVarInt v = stringToInt(jslGetTokenValueAsString(execInfo.lex));
          JSP_MATCH(LEX_INT);
          return jsvNewFromInteger(v);
        } else {
          JSP_MATCH(LEX_INT);
          return 0;
        }
    }
    if (execInfo.lex->tk==LEX_FLOAT) {
      if (JSP_SHOULD_EXECUTE) {
        JsVarFloat v = atof(jslGetTokenValueAsString(execInfo.lex));
        JSP_MATCH(LEX_FLOAT);
        return jsvNewFromFloat(v);
      } else {
        JSP_MATCH(LEX_FLOAT);
        return 0;
      }
    }
    if (execInfo.lex->tk==LEX_STR) {
      if (JSP_SHOULD_EXECUTE) {
        JsVar *a = jslGetTokenValueAsVar(execInfo.lex);
        JSP_MATCH_WITH_RETURN(LEX_STR, a);
        return a;
      } else {
        JSP_MATCH(LEX_STR);
        return 0;
      }
    }
    if (execInfo.lex->tk=='{') {
      if (JSP_SHOULD_EXECUTE) {
        JsVar *contents = jsvNewWithFlags(JSV_OBJECT);
        if (!contents) { // out of memory
          jspSetError();
          return 0;
        }
        /* JSON-style object definition */
        JSP_MATCH_WITH_RETURN('{', contents);
        while (!JSP_HAS_ERROR && execInfo.lex->tk != '}') {
          JsVar *varName = 0;
          if (JSP_SHOULD_EXECUTE) {
            varName = jslGetTokenValueAsVar(execInfo.lex);
            if (!varName) { // out of memory
              return contents;
            }
          }
          // we only allow strings or IDs on the left hand side of an initialisation
          if (execInfo.lex->tk==LEX_STR) {
            JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_STR, jsvUnLock(varName), contents);
          } else {
            JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(varName), contents);
          }
          JSP_MATCH_WITH_CLEANUP_AND_RETURN(':', jsvUnLock(varName), contents);
          if (JSP_SHOULD_EXECUTE) {
            JsVar *valueVar;
            JsVar *value = jspeBase(); // value can be 0 (could be undefined!)
            valueVar = jsvSkipNameAndUnlock(value);
            varName = jsvMakeIntoVariableName(varName, valueVar);
            jsvAddName(contents, varName);
            jsvUnLock(valueVar);
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
    if (execInfo.lex->tk=='[') {
        int idx = 0;
        JsVar *contents = 0;
        if (JSP_SHOULD_EXECUTE) {
          contents = jsvNewWithFlags(JSV_ARRAY);
          if (!contents) { // out of memory
            jspSetError();
            return 0;
          }
        }
        /* JSON-style array */
        JSP_MATCH_WITH_RETURN('[', contents);
        while (!JSP_HAS_ERROR && execInfo.lex->tk != ']') {
          if (JSP_SHOULD_EXECUTE) {
            // OPT: Store array indices as actual ints
            JsVar *a;
            JsVar *aVar;
            JsVar *indexName;
            a = jspeBase();
            aVar = jsvSkipNameAndUnlock(a);
            indexName = jsvMakeIntoVariableName(jsvNewFromInteger(idx),  aVar);
            if (indexName) { // could be out of memory
              jsvAddName(contents, indexName);
              jsvUnLock(indexName);
            }
            jsvUnLock(aVar);
          }
          // no need to clean here, as it will definitely be used
          if (execInfo.lex->tk != ']') JSP_MATCH_WITH_RETURN(',', contents);
          idx++;
        }
        JSP_MATCH_WITH_RETURN(']', contents);
        return contents;
    }
    if (execInfo.lex->tk==LEX_R_FUNCTION) {
      JSP_MATCH(LEX_R_FUNCTION);
      return jspeFunctionDefinition();
    }
    if (execInfo.lex->tk==LEX_R_NEW) {
      // new -> create a new object
      JSP_MATCH(LEX_R_NEW);
      if (JSP_SHOULD_EXECUTE) {
        const char *name = jslGetTokenValueAsString(execInfo.lex);
        if (strcmp(name, "Array")==0) {
          JSP_MATCH(LEX_ID);
          JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
          if (!arr) return 0; // out of memory
          if (execInfo.lex->tk == '(') {
            JsVar *arg = 0;
            bool moreThanOne = false;
            JSP_MATCH('(');
            while (execInfo.lex->tk!=')' && execInfo.lex->tk!=LEX_EOF) {
              if (arg) {
                moreThanOne = true;
                jsvArrayPush(arr, arg);
                jsvUnLock(arg);
              }
              arg = jsvSkipNameAndUnlock(jspeBase());
              if (execInfo.lex->tk!=')') JSP_MATCH(',');
            }
            JSP_MATCH(')');
            if (arg) {
              if (!moreThanOne && jsvIsInt(arg) && jsvGetInteger(arg)>=0) { // this is the size of the array
                JsVarInt count = jsvGetIntegerAndUnLock(arg);
                // we cheat - no need to fill the array - just the last element
                if (count>0) {
                  JsVar *idx = jsvMakeIntoVariableName(jsvNewFromInteger(count-1), 0);
                  if (idx) { // could be out of memory
                    jsvAddName(arr, idx);
                    jsvUnLock(idx);
                  }
                }
              } else { // just append to array
                jsvArrayPush(arr, arg);
                jsvUnLock(arg);
              }
            }
          }
          return arr;
        } else if (strcmp(name, "String")==0) {
          JsVar *a = jspParseSingleFunction();
          if (!a) return jsvNewFromString(""); // out of mem, or just no argument!
          return jsvAsString(a, true);
        } else { // not built-in, try and run constructor function
          JsVar *obj;
          JsVar *objFuncName = jspeFactorID();
          JsVar *objFunc = jsvSkipName(objFuncName);
          if (!objFunc) {
            jsWarnAt("Prototype used in NEW is not defined", execInfo.lex, execInfo.lex->tokenStart);
          }
          obj = jsvNewWithFlags(JSV_OBJECT);
          if (obj) { // could be out of memory
            if (!jsvIsFunction(objFunc)) {
              jsErrorAt("object is not a function", execInfo.lex, execInfo.lex->tokenLastEnd);
            } else {
              // Make sure the function has a 'prototype' var
              JsVar *prototypeName = jsvFindChildFromString(jsvGetRef(objFunc), JSPARSE_PROTOTYPE_VAR, true);
              jsvUnLock(jsvAddNamedChild(obj, prototypeName, JSPARSE_INHERITS_VAR));
              jsvUnLock(prototypeName);
              jsvUnLock(jspeFunctionCall(objFunc, objFuncName, obj, true, 0));
            }
          }
          jsvUnLock(objFuncName);
          jsvUnLock(objFunc);
          return obj;
        }
      } else {
        JSP_MATCH(LEX_ID);
        jspeParseFunctionCallBrackets();
        return 0;
      }
    }
    // Nothing we can do here... just hope it's the end...
    JSP_MATCH(LEX_EOF);
    return 0;
}

JsVar *jspePostfix() {
  JsVar *a = jspeFactor();
  while (execInfo.lex->tk==LEX_PLUSPLUS || execInfo.lex->tk==LEX_MINUSMINUS) {
    int op = execInfo.lex->tk;
    JSP_MATCH(execInfo.lex->tk);
    if (JSP_SHOULD_EXECUTE) {
        JsVar *one = jsvNewFromInteger(1);
        JsVar *res = jsvMathsOpSkipNames(a, one, op==LEX_PLUSPLUS ? '+' : '-');
        JsVar *oldValue;
        jsvUnLock(one);
        oldValue = jsvSkipName(a); // keep the old value
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

JsVar *jspeUnary() {
    JsVar *a;
    if (execInfo.lex->tk=='!') {
        JSP_MATCH('!'); // binary not
        a = jspePostfix();
        if (JSP_SHOULD_EXECUTE) {
            JsVar *zero = jsvNewFromInteger(0);
            JsVar *res = jsvMathsOpSkipNames(a, zero, LEX_EQUAL);
            jsvUnLock(zero);
            jsvUnLock(a); a = res;
        }
    } else
        a = jspePostfix();
    return a;
}

JsVar *jspeTerm() {
    JsVar *a = jspeUnary();
    while (execInfo.lex->tk=='*' || execInfo.lex->tk=='/' || execInfo.lex->tk=='%') {
        JsVar *b;
        int op = execInfo.lex->tk;
        JSP_MATCH(execInfo.lex->tk);
        b = jspeUnary();
        if (JSP_SHOULD_EXECUTE) {
          JsVar *res = jsvMathsOpSkipNames(a, b, op);
          jsvUnLock(a); a = res;
        }
        jsvUnLock(b);
    }
    return a;
}

JsVar *jspeExpression() {
    JsVar *a;
    bool negate = false;
    if (execInfo.lex->tk=='-') {
        JSP_MATCH('-');
        negate = true;
    }
    a = jspeTerm();
    if (negate) {
      JsVar *zero = jsvNewFromInteger(0);
      JsVar *res = jsvMathsOpSkipNames(zero, a, '-');
      jsvUnLock(zero);
      jsvUnLock(a); a = res;
    }

    while (execInfo.lex->tk=='+' || execInfo.lex->tk=='-') {
        int op = execInfo.lex->tk;
        JSP_MATCH(execInfo.lex->tk);
        JsVar *b = jspeTerm();
        if (JSP_SHOULD_EXECUTE) {
            // not in-place, so just replace
          JsVar *res = jsvMathsOpSkipNames(a, b, op);
          jsvUnLock(a); a = res;
        }
        jsvUnLock(b);
    }
    return a;
}

JsVar *jspeShift() {
  JsVar *a = jspeExpression();
  if (execInfo.lex->tk==LEX_LSHIFT || execInfo.lex->tk==LEX_RSHIFT || execInfo.lex->tk==LEX_RSHIFTUNSIGNED) {
    JsVar *b;
    int op = execInfo.lex->tk;
    JSP_MATCH(op);
    b = jspeBase();
    if (JSP_SHOULD_EXECUTE) {
      JsVar *res = jsvMathsOpSkipNames(a, b, op);
      jsvUnLock(a); a = res;
    }
    jsvUnLock(b);
  }
  return a;
}

JsVar *jspeCondition() {
    JsVar *a = jspeShift();
    JsVar *b;
    while (execInfo.lex->tk==LEX_EQUAL || execInfo.lex->tk==LEX_NEQUAL ||
           execInfo.lex->tk==LEX_TYPEEQUAL || execInfo.lex->tk==LEX_NTYPEEQUAL ||
           execInfo.lex->tk==LEX_LEQUAL || execInfo.lex->tk==LEX_GEQUAL ||
           execInfo.lex->tk=='<' || execInfo.lex->tk=='>' || (execInfo.lex->tk==LEX_R_IN && !(execInfo.execute&EXEC_FOR_INIT))) {
        int op = execInfo.lex->tk;
        JSP_MATCH(execInfo.lex->tk);
        b = jspeShift();
        if (JSP_SHOULD_EXECUTE) {
          JsVar *res = 0;
          if (op==LEX_R_IN) {
            JsVar *av = jsvSkipName(a);
            JsVar *bv = jsvSkipName(b);
            if (jsvIsArray(bv) || jsvIsObject(bv)) {
              JsVarRef found = jsvUnLock(jsvGetArrayIndexOf(bv, av, false/*not exact*/));  // ArrayIndexOf will return 0 if not found
              res = jsvNewFromBool(found!=0);
            } // else it will be undefined
            jsvUnLock(av);
            jsvUnLock(bv);
          } else {
            res = jsvMathsOpSkipNames(a, b, op);

          }
          jsvUnLock(a); a = res;
        }
        jsvUnLock(b);
    }
    return a;
}

JsVar *jspeLogic() {
    JsVar *a = jspeCondition();
    JsVar *b = 0;
    while (execInfo.lex->tk=='&' || execInfo.lex->tk=='|' || execInfo.lex->tk=='^' || execInfo.lex->tk==LEX_ANDAND || execInfo.lex->tk==LEX_OROR) {
        bool shortCircuit = false;
        bool boolean = false;
        int op = execInfo.lex->tk;
        JSP_MATCH(execInfo.lex->tk);
        
        // if we have short-circuit ops, then if we know the outcome
        // we don't bother to execute the other op. Even if not
        // we need to tell mathsOp it's an & or |
        if (op==LEX_ANDAND) {
            op = '&';
            shortCircuit = !jsvGetBoolSkipName(a);
            boolean = true;
        } else if (op==LEX_OROR) {
            op = '|';
            shortCircuit = jsvGetBoolSkipName(a);
            boolean = true;
        }
        
        JSP_SAVE_EXECUTE();
        if (shortCircuit) jspSetNoExecute(); 
        b = jspeCondition();
        if (shortCircuit) JSP_RESTORE_EXECUTE();
        if (JSP_SHOULD_EXECUTE && !shortCircuit) {
            JsVar *res;
            if (boolean) {
              JsVar *newa = jsvNewFromBool(jsvGetBoolSkipName(a));
              JsVar *newb = jsvNewFromBool(jsvGetBoolSkipName(b));
              jsvUnLock(a); a = newa;
              jsvUnLock(b); b = newb;
            }
            res = jsvMathsOpSkipNames(a, b, op);
            jsvUnLock(a); a = res;
        }
        jsvUnLock(b);
    }
    return a;
}

JsVar *jspeTernary() {
  JsVar *lhs = jspeLogic();
  if (execInfo.lex->tk=='?') {
    JSP_MATCH('?');
    if (!JSP_SHOULD_EXECUTE) {
      // just let lhs pass through
      jsvUnLock(jspeBase());
      JSP_MATCH(':');
      jsvUnLock(jspeBase());
    } else {
      bool first = jsvGetBoolSkipName(lhs);
      jsvUnLock(lhs);
      if (first) {
        lhs = jspeBase();
        JSP_MATCH(':');
        JSP_SAVE_EXECUTE();
        jspSetNoExecute();
        jsvUnLock(jspeBase());
        JSP_RESTORE_EXECUTE();
      } else {
        JSP_SAVE_EXECUTE();
        jspSetNoExecute();
        jsvUnLock(jspeBase());
        JSP_RESTORE_EXECUTE();
        JSP_MATCH(':');
        lhs = jspeBase();
      }
    }
  }

  return lhs;
}

JsVar *jspeBase() {
    JsVar *lhs = jspeTernary();
    if (execInfo.lex->tk=='=' || execInfo.lex->tk==LEX_PLUSEQUAL || execInfo.lex->tk==LEX_MINUSEQUAL) {
        int op;
        JsVar *rhs;
        /* If we're assigning to this and we don't have a parent,
         * add it to the symbol table root as per JavaScript. */
        if (JSP_SHOULD_EXECUTE && lhs && !lhs->refs) {
          if (jsvIsName(lhs)/* && jsvGetStringLength(lhs)>0*/) {
            JsVar *root = jsvLock(execInfo.parse->root);
            jsvAddName(root, lhs);
            jsvUnLock(root);
          } else // TODO: Why was this here? can it happen?
            jsWarnAt("Trying to assign to an un-named type\n", execInfo.lex, execInfo.lex->tokenLastEnd);
        }

        op = execInfo.lex->tk;
        JSP_MATCH(execInfo.lex->tk);
        rhs = jspeBase();
        rhs = jsvSkipNameAndUnlock(rhs); // ensure we get rid of any references on the RHS
        if (JSP_SHOULD_EXECUTE && lhs) {
            if (op=='=') {
                jspReplaceWith(lhs, rhs);
            } else if (op==LEX_PLUSEQUAL) {
                JsVar *res = jsvMathsOpSkipNames(lhs,rhs, '+');
                jspReplaceWith(lhs, res);
                jsvUnLock(res);
            } else if (op==LEX_MINUSEQUAL) {
                JsVar *res = jsvMathsOpSkipNames(lhs,rhs, '-');
                jspReplaceWith(lhs, res);
                jsvUnLock(res);
            } else {
              assert(0);
            }
        }
        jsvUnLock(rhs);
    }
    return lhs;
}

JsVar *jspeBlock() {
    JSP_MATCH('{');
    if (JSP_SHOULD_EXECUTE) {
      while (execInfo.lex->tk && execInfo.lex->tk!='}') {
        jsvUnLock(jspeStatement());
        if (JSP_HAS_ERROR) return 0;
      }
      JSP_MATCH('}');
    } else {
      // fast skip of blocks
      int brackets = 1;
      while (execInfo.lex->tk && brackets) {
        if (execInfo.lex->tk == '{') brackets++;
        if (execInfo.lex->tk == '}') brackets--;
        JSP_MATCH(execInfo.lex->tk);
      }
    }
    return 0;
}

JsVar *jspeBlockOrStatement() {
    if (execInfo.lex->tk=='{') 
       return jspeBlock();
    else {
       JsVar *v = jspeStatement();
       if (execInfo.lex->tk==';') JSP_MATCH(';');
       return v;
    }
}

JsVar *jspeStatement() {
    if (execInfo.lex->tk==LEX_ID ||
        execInfo.lex->tk==LEX_INT ||
        execInfo.lex->tk==LEX_FLOAT ||
        execInfo.lex->tk==LEX_STR ||
        execInfo.lex->tk=='-' ||
        execInfo.lex->tk=='(') {
        /* Execute a simple statement that only contains basic arithmetic... */
        JsVar *res = jspeBase();
        return res;
    } else if (execInfo.lex->tk=='{') {
        /* A block of code */
        jspeBlock();
    } else if (execInfo.lex->tk==';') {
        /* Empty statement - to allow things like ;;; */
        JSP_MATCH(';');
    } else if (execInfo.lex->tk==LEX_R_VAR) {
        JsVar *lastDefined = 0;
        /* variable creation. TODO - we need a better way of parsing the left
         * hand side. Maybe just have a flag called can_create_var that we
         * set and then we parse as if we're doing a normal equals.*/
        JSP_MATCH(LEX_R_VAR);
        while (execInfo.lex->tk == LEX_ID) {
          JsVar *a = 0;
          if (JSP_SHOULD_EXECUTE) {
            a = jspeiFindOnTop(jslGetTokenValueAsString(execInfo.lex), true);
            if (!a) { // out of memory
              jspSetError();
              return lastDefined;
            }
          }
          JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(a), lastDefined);
          // now do stuff defined with dots
          while (execInfo.lex->tk == '.') {
              JSP_MATCH_WITH_CLEANUP_AND_RETURN('.', jsvUnLock(a), lastDefined);
              if (JSP_SHOULD_EXECUTE) {
                  JsVar *lastA = a;
                  a = jsvFindChildFromString(jsvGetRef(lastA), jslGetTokenValueAsString(execInfo.lex), true);
                  jsvUnLock(lastA);
              }
              JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(a), lastDefined);
          }
          // sort out initialiser
          if (execInfo.lex->tk == '=') {
              JsVar *var;
              JSP_MATCH_WITH_CLEANUP_AND_RETURN('=', jsvUnLock(a), lastDefined);
              var = jspeBase();
              if (JSP_SHOULD_EXECUTE)
                  jspReplaceWith(a, var);
              jsvUnLock(var);
          }
          jsvUnLock(lastDefined);
          lastDefined = a;
          if (execInfo.lex->tk != ';' && execInfo.lex->tk != LEX_R_IN) // bodge
            JSP_MATCH_WITH_RETURN(',', lastDefined);
        }
        return lastDefined;
    } else if (execInfo.lex->tk==LEX_R_IF) {
        bool cond;
        JsVar *var;        
        JSP_MATCH(LEX_R_IF);
        JSP_MATCH('(');
        var = jspeBase();
        JSP_MATCH(')');
        cond = JSP_SHOULD_EXECUTE && jsvGetBoolSkipName(var);
        jsvUnLock(var);

        JSP_SAVE_EXECUTE();
        if (!cond) jspSetNoExecute(); 
        jsvUnLock(jspeBlockOrStatement());
        if (!cond) JSP_RESTORE_EXECUTE();
        if (execInfo.lex->tk==LEX_R_ELSE) {
            //JSP_MATCH(';'); ???
            JSP_MATCH(LEX_R_ELSE);
            JSP_SAVE_EXECUTE();
            if (cond) jspSetNoExecute();
            jsvUnLock(jspeBlockOrStatement());
            if (cond) JSP_RESTORE_EXECUTE();
        }
    } else if (execInfo.lex->tk==LEX_R_WHILE) {
        int loopCount = JSPARSE_MAX_LOOP_ITERATIONS;
        JsVar *cond;
        int whileCondStart;
        bool loopCond;
        int whileBodyStart;
        JsLex whileCond;
        JsLex whileBody;
        JsLex *oldLex;
        bool hasHadBreak = false;
        // We do repetition by pulling out the string representing our statement
        // there's definitely some opportunity for optimisation here
        JSP_MATCH(LEX_R_WHILE);
        JSP_MATCH('(');
        whileCondStart = execInfo.lex->tokenStart;
        cond = jspeBase();
        loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolSkipName(cond);
        jsvUnLock(cond);
        jslInitFromLex(&whileCond, execInfo.lex, whileCondStart);
        JSP_MATCH(')');
        whileBodyStart = execInfo.lex->tokenStart;
        JSP_SAVE_EXECUTE();
        // actually try and execute first bit of while loop (we'll do the rest in the actual loop later)
        if (!loopCond) jspSetNoExecute(); 
        execInfo.execute |= EXEC_IN_LOOP;
        jsvUnLock(jspeBlockOrStatement());
        execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
        if (execInfo.execute == EXEC_CONTINUE)
          execInfo.execute = EXEC_YES;
        if (execInfo.execute == EXEC_BREAK) {
          execInfo.execute = EXEC_YES;
          hasHadBreak = true; // fail loop condition, so we exit
        }
        if (!loopCond) JSP_RESTORE_EXECUTE();
        jslInitFromLex(&whileBody, execInfo.lex, whileBodyStart);
        oldLex = execInfo.lex;

        while (!hasHadBreak && loopCond && loopCount-->0) {
            jslReset(&whileCond);
            execInfo.lex = &whileCond;
            cond = jspeBase();
            loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolSkipName(cond);
            jsvUnLock(cond);
            if (loopCond) {
                jslReset(&whileBody);
                execInfo.lex = &whileBody;
                execInfo.execute |= EXEC_IN_LOOP;
                jsvUnLock(jspeBlockOrStatement());
                execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
                if (execInfo.execute == EXEC_CONTINUE)
                  execInfo.execute = EXEC_YES;
                if (execInfo.execute == EXEC_BREAK) {
                  execInfo.execute = EXEC_YES;
                  hasHadBreak = true;
                }
            }
        }
        execInfo.lex = oldLex;
        jslKill(&whileCond);
        jslKill(&whileBody);

        if (loopCount<=0) {
          jsErrorAt("WHILE Loop exceeded the maximum number of iterations (" STRINGIFY(JSPARSE_MAX_LOOP_ITERATIONS) ")", execInfo.lex, execInfo.lex->tokenLastEnd);
          jspSetError();
        }
    } else if (execInfo.lex->tk==LEX_R_FOR) {
        JSP_MATCH(LEX_R_FOR);
        JSP_MATCH('(');
        execInfo.execute |= EXEC_FOR_INIT;
        JsVar *forStatement = jspeStatement(); // initialisation
        execInfo.execute &= (JsExecFlags)~EXEC_FOR_INIT;
        if (execInfo.lex->tk == LEX_R_IN) {
          // for (i in array)
          // where i = jsvUnLock(forStatement);
          if (!jsvIsName(forStatement)) {
            jsvUnLock(forStatement);
            jsErrorAt("FOR a IN b - 'a' must be a variable name", execInfo.lex, execInfo.lex->tokenLastEnd);
            jspSetError();
            return 0;
          }
          bool addedIteratorToScope = false;
          if (JSP_SHOULD_EXECUTE && !forStatement->refs) {
            // if the variable did not exist, add it to the scope
            addedIteratorToScope = true;
            JsVar *root = jsvLock(execInfo.parse->root);
            jsvAddName(root, forStatement);
            jsvUnLock(root);
          }
          JSP_MATCH(LEX_R_IN);
          JsVar *array = jsvSkipNameAndUnlock(jspeExpression());
          JSP_MATCH(')');
          int forBodyStart = execInfo.lex->tokenStart;
          JSP_SAVE_EXECUTE();
          jspSetNoExecute();
          execInfo.execute |= EXEC_IN_LOOP;
          jsvUnLock(jspeBlockOrStatement());
          execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
          JSP_RESTORE_EXECUTE();
          JsLex forBody;
          jslInitFromLex(&forBody, execInfo.lex, forBodyStart);
          JsLex *oldLex = execInfo.lex;

          JsVarRef loopIndex = 0;
          if (jsvIsArray(array) || jsvIsObject(array)) {
            loopIndex = array->firstChild;
          } else {
            jsErrorAt("FOR loop can only iterate over Arrays or Objects", execInfo.lex, execInfo.lex->tokenLastEnd);
            jspSetError();
          }

          bool hasHadBreak = false;
          while (JSP_SHOULD_EXECUTE && loopIndex && !hasHadBreak) {
              JsVar *loopIndexVar = jsvLock(loopIndex);
              JsVar *indexValue = jsvCopyNameOnly(loopIndexVar, false/*no copy children*/, false/*not a name*/);
              if (indexValue) { // could be out of memory
                assert(!jsvIsName(indexValue) && indexValue->refs==0);
                jsvSetValueOfName(forStatement, indexValue);
                jsvUnLock(indexValue);

                loopIndex = loopIndexVar->nextSibling;

                jslReset(&forBody);
                execInfo.lex = &forBody;
                execInfo.execute |= EXEC_IN_LOOP;
                jsvUnLock(jspeBlockOrStatement());
                execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;

                if (execInfo.execute == EXEC_CONTINUE)
                  execInfo.execute = EXEC_YES;
                if (execInfo.execute == EXEC_BREAK) {
                  execInfo.execute = EXEC_YES;
                  hasHadBreak = true;
                }
              }
              jsvUnLock(loopIndexVar);
          }
          execInfo.lex = oldLex;
          jslKill(&forBody);

          if (addedIteratorToScope) {
            JsVar *rootScope = jsvLock(execInfo.parse->root);
            jsvRemoveChild(rootScope, forStatement);
            jsvUnLock(rootScope);
          }
          jsvUnLock(forStatement);
          jsvUnLock(array);
        } else { // NORMAL FOR LOOP
          int loopCount = JSPARSE_MAX_LOOP_ITERATIONS;
          JsVar *cond;
          bool loopCond;
          JsLex forCond;
          JsLex forIter;
          bool hasHadBreak = false;

          jsvUnLock(forStatement);
          JSP_MATCH(';');
          int forCondStart = execInfo.lex->tokenStart;
          cond = jspeBase(); // condition
          loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolSkipName(cond);
          jsvUnLock(cond);
          jslInitFromLex(&forCond, execInfo.lex, forCondStart);
          JSP_MATCH(';');
          int forIterStart = execInfo.lex->tokenStart;
          {
            JSP_SAVE_EXECUTE();
            jspSetNoExecute();
            jsvUnLock(jspeBase()); // iterator
            JSP_RESTORE_EXECUTE();
          }
          jslInitFromLex(&forIter, execInfo.lex, forIterStart);
          JSP_MATCH(')');

          int forBodyStart = execInfo.lex->tokenStart; // actual for body
          JSP_SAVE_EXECUTE();
          if (!loopCond) jspSetNoExecute();
          execInfo.execute |= EXEC_IN_LOOP;
          jsvUnLock(jspeBlockOrStatement());
          execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
          if (execInfo.execute == EXEC_CONTINUE)
            execInfo.execute = EXEC_YES;
          if (execInfo.execute == EXEC_BREAK) {
            execInfo.execute = EXEC_YES;
            hasHadBreak = true;
          }
          if (!loopCond) JSP_RESTORE_EXECUTE();
          JsLex forBody;
          jslInitFromLex(&forBody, execInfo.lex, forBodyStart);
          JsLex *oldLex = execInfo.lex;
          if (loopCond) {
              jslReset(&forIter);
              execInfo.lex = &forIter;
              jsvUnLock(jspeBase());
          }
          while (!hasHadBreak && JSP_SHOULD_EXECUTE && loopCond && loopCount-->0) {
              jslReset(&forCond);
              execInfo.lex = &forCond;
              cond = jspeBase();
              loopCond = jsvGetBoolSkipName(cond);
              jsvUnLock(cond);
              if (JSP_SHOULD_EXECUTE && loopCond) {
                  jslReset(&forBody);
                  execInfo.lex = &forBody;
                  execInfo.execute |= EXEC_IN_LOOP;
                  jsvUnLock(jspeBlockOrStatement());
                  execInfo.execute &= (JsExecFlags)~EXEC_IN_LOOP;
                  if (execInfo.execute == EXEC_CONTINUE)
                    execInfo.execute = EXEC_YES;
                  if (execInfo.execute == EXEC_BREAK) {
                    execInfo.execute = EXEC_YES;
                    hasHadBreak = true;
                  }
              }
              if (JSP_SHOULD_EXECUTE && loopCond) {
                  jslReset(&forIter);
                  execInfo.lex = &forIter;
                  jsvUnLock(jspeBase());
              }
          }
          execInfo.lex = oldLex;
          jslKill(&forCond);
          jslKill(&forIter);
          jslKill(&forBody);
          if (loopCount<=0) {
              jsErrorAt("FOR Loop exceeded the maximum number of iterations ("STRINGIFY(JSPARSE_MAX_LOOP_ITERATIONS)")", execInfo.lex, execInfo.lex->tokenLastEnd);
              jspSetError();
          }
        }
    } else if (execInfo.lex->tk==LEX_R_RETURN) {
        JsVar *result = 0;
        JSP_MATCH(LEX_R_RETURN);
        if (execInfo.lex->tk != ';') {
          // we only want the value, so skip the name if there was one
          result = jsvSkipNameAndUnlock(jspeBase());
        }
        if (JSP_SHOULD_EXECUTE) {
          JsVar *resultVar = jspeiFindOnTop(JSPARSE_RETURN_VAR, false);
          if (resultVar) {
            jspReplaceWith(resultVar, result);
            jsvUnLock(resultVar);
          } else {
            jsErrorAt("RETURN statement, but not in a function.\n", execInfo.lex, execInfo.lex->tokenLastEnd);
            jspSetError();
          }
          jspSetNoExecute(); // Stop anything else in this function executing
        }
        jsvUnLock(result);
    } else if (execInfo.lex->tk==LEX_R_FUNCTION) {
        JsVar *funcName = 0;
        JsVar *funcVar;
        JSP_MATCH(LEX_R_FUNCTION);
        if (JSP_SHOULD_EXECUTE)
          funcName = jsvMakeIntoVariableName(jsvNewFromString(jslGetTokenValueAsString(execInfo.lex)), 0);
        if (!funcName) { // out of memory
          jspSetError();
          return 0;
        }
        JSP_MATCH(LEX_ID);
        funcVar = jspeFunctionDefinition();
        if (JSP_SHOULD_EXECUTE) {
          // find a function with the same name (or make one)
          // OPT: can Find* use just a JsVar that is a 'name'?
          JsVar *existingFunc = jspeiFindNameOnTop(funcName, true);
          // replace it
          jspReplaceWith(existingFunc, funcVar);
          jsvUnLock(funcName);
          funcName = existingFunc;
        }
        jsvUnLock(funcVar);
        return funcName;
    } else if (execInfo.lex->tk==LEX_R_CONTINUE) {
      JSP_MATCH(LEX_R_CONTINUE);
      if (JSP_SHOULD_EXECUTE) {
        if (!(execInfo.execute & EXEC_IN_LOOP))
          jsErrorAt("CONTINUE statement outside of FOR or WHILE loop", execInfo.lex, execInfo.lex->tokenLastEnd);
        else
          execInfo.execute = (execInfo.execute & (JsExecFlags)~EXEC_RUN_MASK) |  EXEC_CONTINUE;
      }
    } else if (execInfo.lex->tk==LEX_R_BREAK) {
      JSP_MATCH(LEX_R_BREAK);
      if (JSP_SHOULD_EXECUTE) {
        if (!(execInfo.execute & (EXEC_IN_LOOP|EXEC_IN_SWITCH)))
          jsErrorAt("BREAK statement outside of SWITCH, FOR or WHILE loop", execInfo.lex, execInfo.lex->tokenLastEnd);
        else
          execInfo.execute = (execInfo.execute & (JsExecFlags)~EXEC_RUN_MASK) | EXEC_BREAK;
      }
    } else if (execInfo.lex->tk==LEX_R_SWITCH) {
          JSP_MATCH(LEX_R_SWITCH);
          JSP_MATCH('(');
          JsVar *switchOn = jspeBase();
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
            JsVar *test = jspeBase();
            execInfo.execute = oldFlags|EXEC_IN_SWITCH;;
            JSP_MATCH_WITH_CLEANUP_AND_RETURN(':', jsvUnLock(switchOn);jsvUnLock(test), 0);
            bool cond = false;
            if (execute)
              cond = jsvGetBoolAndUnLock(jsvMathsOpSkipNames(switchOn, test, LEX_EQUAL));
            if (cond) hasExecuted = true;
            jsvUnLock(test);
            if (cond && (execInfo.execute&EXEC_RUN_MASK)==EXEC_NO)
              execInfo.execute=EXEC_YES|EXEC_IN_SWITCH;
            while (execInfo.lex->tk!=LEX_EOF && execInfo.lex->tk!=LEX_R_CASE && execInfo.lex->tk!=LEX_R_DEFAULT && execInfo.lex->tk!='}')
              jsvUnLock(jspeBlockOrStatement());
          }
          jsvUnLock(switchOn);
          if (execute && (execInfo.execute&EXEC_RUN_MASK)==EXEC_BREAK)
            execInfo.execute=EXEC_YES|EXEC_IN_SWITCH;
          JSP_RESTORE_EXECUTE();

          if (execInfo.lex->tk==LEX_R_DEFAULT) {
            JSP_MATCH(LEX_R_DEFAULT);
            JSP_MATCH(':');
            JSP_SAVE_EXECUTE();
            if (hasExecuted) jspSetNoExecute();
            while (execInfo.lex->tk!=LEX_EOF && execInfo.lex->tk!='}')
              jsvUnLock(jspeBlockOrStatement());
            JSP_RESTORE_EXECUTE();
          }

          JSP_MATCH('}');

    } else JSP_MATCH(LEX_EOF);
    return 0;
}

// -----------------------------------------------------------------------------

void jspSoftInit(JsParse *parse) {
  parse->root = jsvUnLock(jsvFindOrCreateRoot());
}

/** Is v likely to have been created by this parser? */
bool jspIsCreatedObject(JsParse *parse, JsVar *v) {
  JsVarRef r = jsvGetRef(v);
  return
      r==parse->root;
}

void jspSoftKill(JsParse *parse) {
}

void jspInit(JsParse *parse) {
  jspSoftInit(parse);
}

void jspKill(JsParse *parse) {
  jspSoftKill(parse);
  // Unreffing this should completely kill everything attached to root
  jsvUnRefRef(parse->root);
}



JsVar *jspEvaluateVar(JsParse *parse, JsVar *str) {
  JsLex lex;
  JsVar *v = 0;
  JSP_SAVE_EXECUTE();
  JsExecInfo oldExecInfo = execInfo;

  jslInit(&lex, str, 0, -1);

  jspeiInit(parse, &lex);
  while (!JSP_HAS_ERROR && execInfo.lex->tk != LEX_EOF) {
    jsvUnLock(v);
    v = jspeBlockOrStatement();
  }
  // clean up
  jspeiKill();
  jslKill(&lex);

  // restore state
  JSP_RESTORE_EXECUTE();
  oldExecInfo.execute = execInfo.execute; // JSP_RESTORE_EXECUTE has made this ok.
  execInfo = oldExecInfo;

  // It may have returned a reference, but we just want the value...
  if (v) {
    return jsvSkipNameAndUnlock(v);
  }
  // nothing returned
  return 0;
}

JsVar *jspEvaluate(JsParse *parse, const char *str) {
  JsVar *v = 0;

  JsVar *evCode = jsvNewFromString(str);
  if (!jsvIsMemoryFull())
    v = jspEvaluateVar(parse, evCode);
  jsvUnLock(evCode);

  return v;
}

bool jspExecuteFunction(JsParse *parse, JsVar *func, JsVar *arg0) {
  JSP_SAVE_EXECUTE();
  JsExecInfo oldExecInfo = execInfo;

  jspeiInit(parse, 0);
  JsVar *resultVar = jspeFunctionCall(func, 0, 0, false, arg0);
  bool result = jsvGetBool(resultVar);
  jsvUnLock(resultVar);
  // clean up
  jspeiKill();
  // restore state
  JSP_RESTORE_EXECUTE();
  oldExecInfo.execute = execInfo.execute; // JSP_RESTORE_EXECUTE has made this ok.
  execInfo = oldExecInfo;


  return result;
}
