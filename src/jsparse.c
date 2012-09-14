#include "jsparse.h"
#include "jsfunctions.h"

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
#define JSP_RESTORE_EXECUTE() execInfo.execute = (execInfo.execute&(JsExecFlags)(~EXEC_YES)) | (oldExecute&EXEC_YES);
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


JsVar *jspeiFindChildFromStringInParents(JsVar *parent, const char *name) {
  if (jsvIsInt(parent))
      return jsvFindChildFromString(execInfo.parse->intClass, name, false);
  if (jsvIsFloat(parent))
      return jsvFindChildFromString(execInfo.parse->doubleClass, name, false);
  if (jsvIsString(parent))
      return jsvFindChildFromString(execInfo.parse->stringClass, name, false);
  if (jsvIsArray(parent))
      return jsvFindChildFromString(execInfo.parse->arrayClass, name, false);

  if (jsvIsObject(parent)) {
    // If an object, look for prototypes
    JsVar *proto = jsvSkipNameAndUnlock(jsvFindChildFromString(jsvGetRef(parent), JSPARSE_PROTOTYPE_CLASS, false));
    if (proto) {
      JsVar *child = jsvFindChildFromString(jsvGetRef(proto), name, false);
      if (child) { // we found a child!
        jsvUnLock(proto);
        return child;
      }
      // else look for prototypes in THAT object
      child = jspeiFindChildFromStringInParents(proto, name);
      jsvUnLock(proto);
      return child;
    } else {
      // look in the basic object class
      return jsvFindChildFromString(execInfo.parse->objectClass, name, false);
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
  JSP_MATCH(')');
  return true;
}

// parse function with a single argument, return its value (no names!)
JsVar *jspParseSingleFunction() {
  JsExecFlags execute = EXEC_YES;
  JsVar *v = 0;
  JSP_MATCH(LEX_ID);
  JSP_MATCH('(');
  if (execInfo.lex->tk != ')')
    v = jsvSkipNameAndUnlock(jspeBase(&execute));
  // throw away extra params
  while (execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    jsvUnLock(jspeBase(&execute));
  }
  JSP_MATCH(')');
  return v;
}

// parse function with 2 arguments, return 2 values (no names!)
bool jspParseDoubleFunction(JsVar **a, JsVar **b) {
  if (a) *a = 0;
  if (b) *b = 0;
  JsExecFlags execute = EXEC_YES;
  JSP_MATCH(LEX_ID);
  JSP_MATCH('(');
  if (b && execInfo.lex->tk != ')')
    *a = jsvSkipNameAndUnlock(jspeBase(&execute));
  if (b && execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    *b = jsvSkipNameAndUnlock(jspeBase(&execute));
  }
  // throw away extra params
  while (execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    jsvUnLock(jspeBase(&execute));
  }
  JSP_MATCH(')');
  return true;
}

// parse function with 3 arguments, return 3 values (no names!)
bool jspParseTripleFunction(JsVar **a, JsVar **b, JsVar **c) {
  if (a) *a = 0;
  if (b) *b = 0;
  if (c) *c = 0; 
  JsExecFlags execute = EXEC_YES;
  JSP_MATCH(LEX_ID);
  JSP_MATCH('(');
  if (execInfo.lex->tk != ')')
    *a = jsvSkipNameAndUnlock(jspeBase(&execute));
  if (b && execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    *b = jsvSkipNameAndUnlock(jspeBase(&execute));
  }
  if (c && execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    *c = jsvSkipNameAndUnlock(jspeBase(&execute));
  }
  // throw away extra params
  while (execInfo.lex->tk != ')') {
    JSP_MATCH(',');
    jsvUnLock(jspeBase(&execute));
  }
  JSP_MATCH(')');
  return true;
}
// -----------------------------------------------

// we return a value so that JSP_MATCH can return 0 if it fails
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
    bool success;
    JsLex lex;
    JsVar *fncode;
    JSP_SAVE_EXECUTE();
    JsExecInfo oldExecInfo = execInfo;

    // Set up Lexer
    fncode = jsvNewFromString(funcDesc);

    jslInit(&lex, fncode, 0, -1);
    jsvUnLock(fncode);

    
    jspeiInit(parse, &lex);

    // Parse
    success = jspeParseNativeFunction(callbackPtr);
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

/** Handle a function call (assumes we've parsed the function name and we're
 * on the start bracket). 'parent' is the object that contains this method,
 * if there was one (otherwise it's just a normnal function).
 */
JsVar *jspeFunctionCall(JsVar *function, JsVar *parent, bool isParsing) {
  if (JSP_SHOULD_EXECUTE && !function) {
      jsWarnAt("Function not found! Skipping.", execInfo.lex, execInfo.lex->tokenLastEnd );
  }

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
        // if we have a scope var, load it up. We may not have one if there were no scopes apart from root
        JsVar *functionScope = jsvFindChildFromString(jsvGetRef(function), JSPARSE_FUNCTION_SCOPE_NAME, false);
        if (functionScope) {
            int i;
            oldScopeCount = execInfo.scopeCount;
            for (i=0;i<execInfo.scopeCount;i++)
                oldScopes[i] = execInfo.scopes[i];
            JsVar *functionScopeVar = jsvLock(functionScope->firstChild);
            //jsvTrace(jsvGetRef(functionScopeVar),5);
            jspeiLoadScopesFromVar(functionScopeVar);
            jsvUnLock(functionScopeVar);
            jsvUnLock(functionScope);
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
              jsPrint("in function called from ");
              if (execInfo.lex)
                jsPrintPosition(execInfo.lex, execInfo.lex->tokenLastEnd);
              else
                jsPrint("system\n");
              jspSetError();
            }
          }

          jspeiRemoveScope();
        }

        if (functionScope) {
            int i;
            // Unref old scopes
            for (i=0;i<execInfo.scopeCount;i++)
                jsvUnRefRef(execInfo.scopes[i]);
            // restore function scopes
            for (i=0;i<oldScopeCount;i++)
                execInfo.scopes[i] = oldScopes[i];
            execInfo.scopeCount = oldScopeCount;
        }
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
  } else if (isParsing) {
    // function, but not executing - just parse args and be done
    JSP_MATCH('(');
    while (execInfo.lex->tk != ')') {
      JsVar *value = jspeBase();
      jsvUnLock(value);
      if (execInfo.lex->tk!=')') JSP_MATCH(',');
    }
    JSP_MATCH(')');
    /* function will be a blank scriptvarlink if we're not executing,
     * so just return it rather than an alloc/free */
    return function;
  } else return 0;
}

JsVar *jspeFactor() {
    if (execInfo.lex->tk=='(') {
        JsVar *a;
        JSP_MATCH('(');
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
        JsVar *a = JSP_SHOULD_EXECUTE ? jspeiFindInScopes(jslGetTokenValueAsString(execInfo.lex)) : 0;
        /* The parent if we're executing a method call */
        JsVar *parent = 0;

        if (JSP_SHOULD_EXECUTE && !a) {
          /* Special case! We haven't found the variable, so check out
           * and see if it's one of our builtins...  */
          a = jsfHandleFunctionCall(&execInfo, 0, jslGetTokenValueAsString(execInfo.lex));
          if (a == JSFHANDLEFUNCTIONCALL_UNHANDLED) {
            /* Variable doesn't exist! JavaScript says we should create it
             * (we won't add it here. This is done in the assignment operator)*/
            a = jsvMakeIntoVariableName(jsvNewFromString(jslGetTokenValueAsString(execInfo.lex)), 0);
            JSP_MATCH(LEX_ID);
          }
        } else {
          JSP_MATCH(LEX_ID);
        }

        while (execInfo.lex->tk=='(' || execInfo.lex->tk=='.' || execInfo.lex->tk=='[') {
            if (execInfo.lex->tk=='(') { // ------------------------------------- Function Call
              JsVar *func = 0;
              func = jsvSkipNameAndUnlock(a);
              a = jspeFunctionCall(func, parent, true);
              jsvUnLock(func);
            } else if (execInfo.lex->tk == '.') { // ------------------------------------- Record Access
                JSP_MATCH('.');
                if (JSP_SHOULD_EXECUTE) {
                  // Note: name will go away when we oarse something else!
                  const char *name = jslGetTokenValueAsString(execInfo.lex);

                  JsVar *aVar = jsvSkipName(a);
                  JsVar *child = 0;
                  if (aVar && (jsvIsObject(aVar) || jsvIsString(aVar) || jsvIsArray(aVar))) {
                      child = jsvFindChildFromString(jsvGetRef(aVar), name, false);

                      if (!child)
                        child = jspeiFindChildFromStringInParents(aVar, name);

                      if (child) {
                        // it was found - no need for name ptr now, so match!
                        JSP_MATCH_WITH_CLEANUP_AND_RETURN(LEX_ID, jsvUnLock(parent);jsvUnLock(a);, child);
                      } else { // NOT FOUND...
                        /* Check for builtins via separate function
                         * This way we save on RAM for built-ins because all comes out of program code. */
                        child = jsfHandleFunctionCall(&execInfo, aVar, name);
                        if (child == JSFHANDLEFUNCTIONCALL_UNHANDLED) {
                          child = 0;
                          // It wasn't handled... We already know this is an object so just add a new child
                          if (jsvIsObject(aVar)) {
                            child = jsvAddNamedChild(aVar, 0, name);
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
        JsVar *a = jsvNewFromString(jslGetTokenValueAsString(execInfo.lex));
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
            varName = jsvNewFromString(jslGetTokenValueAsString(execInfo.lex));
            if (!varName) { // out of memory
              jspSetError();
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
            JsVar *value = jspeBase();
            if (!value) {
              jspSetError();
              jsvUnLock(varName);
              return contents;
            }
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
            if (!indexName)  // out of memory
              jspSetError();
            else {
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
        JsVar *obj;
        JsVar *objClassOrFunc = jsvSkipNameAndUnlock(jspeiFindInScopes(jslGetTokenValueAsString(execInfo.lex)));
        if (!objClassOrFunc) {
          jsWarnAt("Prototype used in NEW is not defined", execInfo.lex, execInfo.lex->tokenStart);
        }
        JSP_MATCH(LEX_ID);
        obj = jsvNewWithFlags(JSV_OBJECT);
        if (jsvIsFunction(objClassOrFunc)) {
          jsvUnLock(jspeFunctionCall(objClassOrFunc, obj, true));
        } else {
          jsvUnLock(jsvAddNamedChild(obj, objClassOrFunc, JSPARSE_PROTOTYPE_CLASS));
          if (execInfo.lex->tk == '(') {
            JSP_MATCH('(');
            JSP_MATCH(')');
          }
        }
        jsvUnLock(objClassOrFunc);
        return obj;
      } else {
        JSP_MATCH(LEX_ID);
        if (execInfo.lex->tk == '(') {
          JSP_MATCH('(');
          JSP_MATCH(')');
        }
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
        JsVar *one = jsvLock(execInfo.parse->oneInt);
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
            JsVar *zero = jsvLock(execInfo.parse->zeroInt);
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
      JsVar *zero = jsvLock(execInfo.parse->zeroInt);
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
           execInfo.lex->tk=='<' || execInfo.lex->tk=='>') {
        int op = execInfo.lex->tk;
        JSP_MATCH(execInfo.lex->tk);
        b = jspeShift();
        if (JSP_SHOULD_EXECUTE) {
            JsVar *res = jsvMathsOpSkipNames(a, b, op);
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
        if (!loopCond) jspSetNoExecute(); 
        jsvUnLock(jspeBlockOrStatement());
        if (!loopCond) JSP_RESTORE_EXECUTE();
        jslInitFromLex(&whileBody, execInfo.lex, whileBodyStart);
        oldLex = execInfo.lex;

        while (loopCond && loopCount-->0) {
            jslReset(&whileCond);
            execInfo.lex = &whileCond;
            cond = jspeBase();
            loopCond = JSP_SHOULD_EXECUTE && jsvGetBoolSkipName(cond);
            jsvUnLock(cond);
            if (loopCond) {
                jslReset(&whileBody);
                execInfo.lex = &whileBody;
                jsvUnLock(jspeBlockOrStatement());
            }
        }
        execInfo.lex = oldLex;
        jslKill(&whileCond);
        jslKill(&whileBody);

        if (loopCount<=0) {
          jsErrorAt("WHILE Loop exceeded the maximum number of iterations", execInfo.lex, execInfo.lex->tokenLastEnd);
          jspSetError();
        }
    } else if (execInfo.lex->tk==LEX_R_FOR) {
        JSP_MATCH(LEX_R_FOR);
        JSP_MATCH('(');
        JsVar *forStatement = jspeStatement(); // initialisation
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
          jsvUnLock(jspeBlockOrStatement());
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

          while (JSP_SHOULD_EXECUTE && loopIndex) {
              JsVar *loopIndexVar = jsvLock(loopIndex);
              JsVar *indexValue = jsvCopyNameOnly(loopIndexVar, false);
              assert(jsvIsName(indexValue) && indexValue->refs==0);
              indexValue->flags &= ~JSV_NAME; // make sure this is NOT a name
              jsvSetValueOfName(forStatement, indexValue);
              jsvUnLock(indexValue);
              loopIndex = loopIndexVar->nextSibling;
              jsvUnLock(loopIndexVar);

              jslReset(&forBody);
              execInfo.lex = &forBody;
              jsvUnLock(jspeBlockOrStatement());
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
          int forBodyStart = execInfo.lex->tokenStart;
          JSP_SAVE_EXECUTE();
          if (!loopCond) jspSetNoExecute();
          jsvUnLock(jspeBlockOrStatement());
          if (!loopCond) JSP_RESTORE_EXECUTE();
          JsLex forBody;
          jslInitFromLex(&forBody, execInfo.lex, forBodyStart);
          JsLex *oldLex = execInfo.lex;
          if (loopCond) {
              jslReset(&forIter);
              execInfo.lex = &forIter;
              jsvUnLock(jspeBase());
          }
          while (JSP_SHOULD_EXECUTE && loopCond && loopCount-->0) {
              jslReset(&forCond);
              execInfo.lex = &forCond;
              cond = jspeBase();
              loopCond = jsvGetBoolSkipName(cond);
              jsvUnLock(cond);
              if (JSP_SHOULD_EXECUTE && loopCond) {
                  jslReset(&forBody);
                  execInfo.lex = &forBody;
                  jsvUnLock(jspeBlockOrStatement());
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
              jsErrorAt("FOR Loop exceeded the maximum number of iterations", execInfo.lex, execInfo.lex->tokenLastEnd);
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
    } else JSP_MATCH(LEX_EOF);
    return 0;
}

// -----------------------------------------------------------------------------

void jspSoftInit(JsParse *parse) {
  parse->root = jsvUnLock(jsvFindOrCreateRoot());

  JsVar *name;

  parse->zeroInt = jsvUnLock(jsvRef(jsvNewFromInteger(0)));
  parse->oneInt = jsvUnLock(jsvRef(jsvNewFromInteger(1)));

  name = jsvFindChildFromString(parse->root, "String", true);
  name->flags |= JSV_NATIVE;
  if (!name->firstChild) name->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  parse->stringClass = jsvRefRef(name->firstChild);
  jsvUnLock(name);

  name = jsvFindChildFromString(parse->root, "Object", true);
  name->flags |= JSV_NATIVE;
  if (!name->firstChild) name->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  parse->objectClass = jsvRefRef(name->firstChild);
  jsvUnLock(name);

  name = jsvFindChildFromString(parse->root, "Array", true);
  name->flags |= JSV_NATIVE;
  if (!name->firstChild) name->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  parse->arrayClass = jsvRefRef(name->firstChild);
  jsvUnLock(name);

  name = jsvFindChildFromString(parse->root, "Integer", true);
  name->flags |= JSV_NATIVE;
  if (!name->firstChild) name->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  parse->intClass = jsvRefRef(name->firstChild);
  jsvUnLock(name);

  name = jsvFindChildFromString(parse->root, "Double", true);
  name->flags |= JSV_NATIVE;
  if (!name->firstChild) name->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  parse->doubleClass = jsvRefRef(name->firstChild);
  jsvUnLock(name);

  name = jsvFindChildFromString(parse->root, "Math", true);
  name->flags |= JSV_NATIVE;
  if (!name->firstChild) name->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  parse->mathClass = jsvRefRef(name->firstChild);
  jsvUnLock(name);

  name = jsvFindChildFromString(parse->root, "JSON", true);
  name->flags |= JSV_NATIVE;
  if (!name->firstChild) name->firstChild = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  parse->jsonClass = jsvRefRef(name->firstChild);
  jsvUnLock(name);
}

void jspSoftKill(JsParse *parse) {
  jsvUnRefRef(parse->zeroInt);
  jsvUnRefRef(parse->oneInt);
  jsvUnRefRef(parse->stringClass);
  jsvUnRefRef(parse->objectClass);
  jsvUnRefRef(parse->arrayClass);
  jsvUnRefRef(parse->intClass);
  jsvUnRefRef(parse->doubleClass);
  jsvUnRefRef(parse->mathClass);
  jsvUnRefRef(parse->jsonClass);
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
  JsExecFlags execute = EXEC_YES;
  JsLex lex;
  JsVar *v = 0;
  JSP_SAVE_EXECUTE();
  JsExecInfo oldExecInfo = execInfo;

  jslInit(&lex, str, 0, -1);

  jspeiInit(parse, &lex);
  while (!JSP_HAS_ERROR && execInfo.lex->tk != LEX_EOF) {
    jsvUnLock(v);
    v = jspeBlockOrStatement(&execute);
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

bool jspExecuteFunction(JsParse *parse, JsVar *func) {
  JSP_SAVE_EXECUTE();
  JsExecInfo oldExecInfo = execInfo;

  jspeiInit(parse, 0);
  JsVar *resultVar = jspeFunctionCall(func, 0, false);
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
