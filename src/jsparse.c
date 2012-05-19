#include "jsparse.h"
#include "jsfunctions.h"

typedef enum  {
  EXEC_NO = 0,
  EXEC_YES = 1,
  EXEC_RUN_MASK = 1,
  // EXEC_ERROR = 2 // maybe?
} JsExecFlags;

/* Info about execution when Parsing - this saves passing it on the stack
 * for each call */
JsExecInfo execInfo;

// ----------------------------------------------- Forward decls
JsVar *jspeBase(JsExecFlags *execute);
JsVar *jspeBlock(JsExecFlags *execute);
JsVar *jspeStatement(JsExecFlags *execute);
// ----------------------------------------------- Utils
#define JSP_MATCH(TOKEN) {if (!jslMatch(execInfo.lex,(TOKEN))) return 0;}
#define JSP_SHOULD_EXECUTE(execute) (((*execute)&EXEC_RUN_MASK)==EXEC_YES)

///< Same as jsvSetValueOfName, but nice error message
JsVar *jspReplaceWith(JsVar *dst, JsVar *src) {
  assert(dst);
  // if desination isn't there, isn't a 'name', or is used, just return source
  if (!jsvIsName(dst)) {
    jsErrorAt("Unable to assign value to non-reference", execInfo.lex, execInfo.lex->tokenLastEnd);
    return dst;
  }
  return jsvSetValueOfName(dst, src);
}

void jspeiInit(JsParse *parse, JsLex *lex) {
  execInfo.parse = parse;
  execInfo.lex = lex;
  execInfo.scopeCount = 0;
}

void jspeiKill() {
  execInfo.parse = 0;
  execInfo.lex = 0;
  assert(execInfo.scopeCount==0);
}

void jspeiAddScope(JsVarRef scope) {
  if (execInfo.scopeCount >= JSPARSE_MAX_SCOPES) {
    jsError("Maximum number of scopes exceeded");
    return;
  }
  execInfo.scopes[execInfo.scopeCount++] = jsvRefRef(scope);
}

void jspeiRemoveScope() {
  if (execInfo.scopeCount <= 0) {
    jsError("INTERNAL: Too many scopes removed");
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
// -----------------------------------------------

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
// -----------------------------------------------

// we return a value so that JSP_MATCH can return 0 if it fails
bool jspeFunctionArguments(JsVar *funcVar) {
  JSP_MATCH('(');
  while (execInfo.lex->tk!=')') {
      if (funcVar) {
        JsVar *param = jsvAddNamedChild(jsvGetRef(funcVar), 0, jslGetTokenValueAsString(execInfo.lex));
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
        link = jsvAddNamedChild(jsvGetRef(base), jsvGetRef(obj), funcName);
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
    funcVar->varData.callback = callbackPtr;
    jspeFunctionArguments(funcVar);

    // Add the function with its name
    jsvUnLock(jsvAddNamedChild(jsvGetRef(base), jsvGetRef(funcVar), funcName));
    jsvUnLock(base);
    jsvUnLock(funcVar);
    return true;
}

bool jspAddNativeFunction(JsParse *parse, const char *funcDesc, JsCallback callbackPtr) {
    bool success;
    JsExecInfo oldExecInfo = execInfo;
    // Set up Lexer
    JsVar *fncode = jsvNewFromString(funcDesc);
    JsLex lex;
    jslInit(&lex, fncode, 0, -1);
    jsvUnLock(fncode);

    
    jspeiInit(parse, &lex);

    // Parse
    success = jspeParseNativeFunction(callbackPtr);
    if (!success) {
      jsError("Parsing Native Function failed!");
    }


    // cleanup
    jspeiKill();
    jslKill(&lex);
    execInfo = oldExecInfo;


    return success;
}

JsVar *jspeFunctionDefinition(JsExecFlags *execute) {
  int funcBegin;
  JsExecFlags noexecute = EXEC_NO;
  // actually parse a function... We assume that the LEX_FUNCTION and name
  // have already been parsed
  JsVar *funcVar = 0;
  if (JSP_SHOULD_EXECUTE(execute))
    funcVar = jsvNewWithFlags(JSV_FUNCTION);
  // Get arguments save them to the structure
  if (!jspeFunctionArguments(funcVar)) {
    // parse failed
    return 0;
  }
  // Get the code - first parse it so we know where it stops
  funcBegin = execInfo.lex->tokenStart;
  jsvUnLock(jspeBlock(&noexecute));
  // Then create var and set
  if (JSP_SHOULD_EXECUTE(execute)) {
    JsVar *funcCodeVar = jsvNewFromLexer(execInfo.lex, funcBegin, execInfo.lex->tokenLastEnd+1);
    jsvUnLock(jsvAddNamedChild(jsvGetRef(funcVar), jsvGetRef(funcCodeVar), JSPARSE_FUNCTION_CODE_NAME));
    jsvUnLock(funcCodeVar);
  }
  return funcVar;
}

/** Handle a function call (assumes we've parsed the function name and we're
 * on the start bracket). 'parent' is the object that contains this method,
 * if there was one (otherwise it's just a normnal function).
 */
JsVar *jspeFunctionCall(JsExecFlags *execute, JsVar *function, JsVar *parent) {
  if (JSP_SHOULD_EXECUTE(execute) && !function) {
      jsWarnAt("Function not found! Skipping.", execInfo.lex, execInfo.lex->tokenLastEnd );
  }

  if (JSP_SHOULD_EXECUTE(execute) && function) {
    JsVar *functionRoot;
    JsVar *returnVarName;
    JsVar *returnVar;
    JsVarRef v;
    if (!jsvIsFunction(function)) {
        jsErrorAt("Expecting a function to call", execInfo.lex, execInfo.lex->tokenLastEnd );
        return 0;
    }
    JSP_MATCH('(');
    // create a new symbol table entry for execution of this function
    // OPT: can we cache this function execution environment + param variables?
    // OPT: Probably when calling a function ONCE, use it, otherwise when recursing, make new?
    functionRoot = jsvNewWithFlags(JSV_FUNCTION);
    if (parent)
      jsvUnLock(jsvAddNamedChild(jsvGetRef(functionRoot), jsvGetRef(parent), JSPARSE_THIS_VAR));
    // grab in all parameters
    v = function->firstChild;
    while (v) {
        JsVar *param = jsvLock(v);
        if (jsvIsFunctionParameter(param)) {
          JsVar *valueName = 0;
          // ONLY parse this if it was supplied, otherwise leave 0 (undefined)
          if (execInfo.lex->tk!=')')
            valueName = jspeBase(execute);
          // and if execute, copy it over
          if (JSP_SHOULD_EXECUTE(execute)) {
            JsVar *value = jsvSkipName(valueName);
            // TODO: deep copy required?
            /*if (jsvIsBasic(value)) {
              // pass by value
              jsvAddNamedChild(jsvGetRef(functionRoot), v->name, value->var->deepCopy());
            } else {
              // pass by reference
              jsvAddNamedChild(jsvGetRef(functionRoot), v->name, value);
            }*/
            JsVar *newValueName = jsvMakeIntoVariableName(jsvCopy(param), jsvGetRef(value));
            jsvAddName(jsvGetRef(functionRoot), jsvGetRef(newValueName));
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
      jsvUnLock(jspeBase(execute));
    }
    JSP_MATCH(')');
    // setup a return variable
    returnVarName = jsvAddNamedChild(jsvGetRef(functionRoot), 0, JSPARSE_RETURN_VAR);
    // add the function's execute space to the symbol table so we can recurse
    jspeiAddScope(jsvGetRef(functionRoot));
    //jsvTrace(jsvGetRef(functionRoot), 5); // debugging
#ifdef JSPARSE_CALL_STACK
    call_stack.push_back(function->name + " from " + l->getPosition());
#endif

    if (jsvIsNative(function)) {
        assert(function->varData.callback);
        function->varData.callback(jsvGetRef(functionRoot));
    } else {
        /* we just want to execute the block, but something could
         * have messed up and left us with the wrong ScriptLex, so
         * we want to be careful here... */

        JsVar *functionCode = jsvFindChildFromString(jsvGetRef(function), JSPARSE_FUNCTION_CODE_NAME, false);
        if (functionCode) {
          JsLex *oldLex;
          JsExecFlags oldExecute = *execute;
          JsVar* functionCodeVar = jsvSkipNameAndUnlock(functionCode);
          JsLex newLex;
          jslInit(&newLex, functionCodeVar, 0, -1);
          jsvUnLock(functionCodeVar);

          oldLex = execInfo.lex;
          execInfo.lex = &newLex;
          jspeBlock(execute);
          // TODO: what about an error flag in execute?
          *execute = oldExecute; // because return will probably have called this, and set execute to false
          jslKill(&newLex);
          execInfo.lex = oldLex;
        }
    }
#ifdef JSPARSE_CALL_STACK
    if (!call_stack.empty()) call_stack.pop_back();
#endif
    jspeiRemoveScope(execInfo);
    /* get the real return var before we remove it from our function */
    returnVar = jsvSkipNameAndUnlock(returnVarName);
    jsvUnLock(functionRoot);
    if (returnVar)
      return returnVar;
    else
      return 0;
  } else {
    // function, but not executing - just parse args and be done
    JSP_MATCH('(');
    while (execInfo.lex->tk != ')') {
      JsVar *value = jspeBase(execute);
      jsvUnLock(value);
      if (execInfo.lex->tk!=')') JSP_MATCH(',');
    }
    JSP_MATCH(')');
    /* function will be a blank scriptvarlink if we're not executing,
     * so just return it rather than an alloc/free */
    return function;
  }
}

JsVar *jspeFactor(JsExecFlags *execute) {
    if (execInfo.lex->tk=='(') {
        JsVar *a;
        JSP_MATCH('(');
        a = jspeBase(execute);
        JSP_MATCH(')');
        return a;
    }
    if (execInfo.lex->tk==LEX_R_TRUE) {
        JSP_MATCH(LEX_R_TRUE);
        return jsvNewFromBool(true);
    }
    if (execInfo.lex->tk==LEX_R_FALSE) {
        JSP_MATCH(LEX_R_FALSE);
        return jsvNewFromBool(false);
    }
    if (execInfo.lex->tk==LEX_R_NULL) {
        JSP_MATCH(LEX_R_NULL);
        return jsvNewWithFlags(JSV_NULL);
    }
    if (execInfo.lex->tk==LEX_R_UNDEFINED) {
        JSP_MATCH(LEX_R_UNDEFINED);
        return 0;
    }
    if (execInfo.lex->tk==LEX_ID) {
        JsVar *a = JSP_SHOULD_EXECUTE(execute) ? jspeiFindInScopes(jslGetTokenValueAsString(execInfo.lex)) : 0;
        /* The parent if we're executing a method call */
        JsVar *parent = 0;

        if (JSP_SHOULD_EXECUTE(execute) && !a) {
          /* Special case! We haven't found the variable, so check out
           * and see if it's one of our builtins...  */
          a = jsfHandleFunctionCall(&execInfo, 0, jslGetTokenValueAsString(execInfo.lex));
          if (!a) {
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
              a = jspeFunctionCall(execute, func, parent);
              jsvUnLock(func);
            } else if (execInfo.lex->tk == '.') { // ------------------------------------- Record Access
                JSP_MATCH('.');
                if (JSP_SHOULD_EXECUTE(execute)) {
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
                        JSP_MATCH(LEX_ID);
                      } else { // NOT FOUND...
                        /* Check for builtins via separate function
                         * This way we save on RAM for built-ins because all comes out of program code. */
                        child = jsfHandleFunctionCall(&execInfo, aVar, name);
                        if (!child) {
                          // It wasn't handled... We already know this is an object so just add a new child
                          if (jsvIsObject(aVar)) {
                            child = jsvAddNamedChild(jsvGetRef(aVar), 0, name);
                          } else {
                            // could have been a string...
                            jsWarnAt("Using '.' operator on non-object", execInfo.lex, execInfo.lex->tokenLastEnd);
                          }
                          JSP_MATCH(LEX_ID);
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
                  JSP_MATCH(LEX_ID);
                }
            } else if (execInfo.lex->tk == '[') { // ------------------------------------- Array Access
                JsVar *index;
                JSP_MATCH('[');
                index = jspeBase(execute);
                JSP_MATCH(']');
                if (JSP_SHOULD_EXECUTE(execute)) {
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
            } else assert(0);
        }
        jsvUnLock(parent);
        return a;
    }
    if (execInfo.lex->tk==LEX_INT) {
        // atol works only on decimals
        // strtol handles 0x12345 as well
        //JsVarInt v = (JsVarInt)atol(jslGetTokenValueAsString(execInfo.lex));
        //JsVarInt v = (JsVarInt)strtol(jslGetTokenValueAsString(execInfo.lex),0,0); // broken on PIC
        JsVarInt v = stringToInt(jslGetTokenValueAsString(execInfo.lex));
        JSP_MATCH(LEX_INT);
        return jsvNewFromInteger(v);
    }
    if (execInfo.lex->tk==LEX_FLOAT) {
        JsVarFloat v = atof(jslGetTokenValueAsString(execInfo.lex));
        JSP_MATCH(LEX_FLOAT);
        return jsvNewFromFloat(v);
    }
    if (execInfo.lex->tk==LEX_STR) {
        JsVar *a = jsvNewFromString(jslGetTokenValueAsString(execInfo.lex));
        JSP_MATCH(LEX_STR);
        return a;
    }
    if (execInfo.lex->tk=='{') {
        JsVar *contents = jsvNewWithFlags(JSV_OBJECT);
        /* JSON-style object definition */
        JSP_MATCH('{');
        while (execInfo.lex->tk != '}') {
          JsVar *varName = 0;
          if (JSP_SHOULD_EXECUTE(execute))
            varName = jsvNewFromString(jslGetTokenValueAsString(execInfo.lex));
          // we only allow strings or IDs on the left hand side of an initialisation
          if (execInfo.lex->tk==LEX_STR) {
            JSP_MATCH(LEX_STR);
          } else {
            JSP_MATCH(LEX_ID);
          }
          JSP_MATCH(':');
          if (JSP_SHOULD_EXECUTE(execute)) {
            JsVar *valueVar;
            JsVar *value = jspeBase(execute);
            assert(value);
            valueVar = jsvSkipNameAndUnlock(value);
            varName = jsvMakeIntoVariableName(varName, jsvGetRef(valueVar));
            jsvAddName(jsvGetRef(contents), jsvGetRef(varName));
            jsvUnLock(valueVar);
          }
          jsvUnLock(varName);
          // no need to clean here, as it will definitely be used
          if (execInfo.lex->tk != '}') JSP_MATCH(',');
        }
        JSP_MATCH('}');
        return contents;
    }
    if (execInfo.lex->tk=='[') {
        int idx = 0;
        JsVar *contents = jsvNewWithFlags(JSV_ARRAY);
        /* JSON-style array */
        JSP_MATCH('[');
        while (execInfo.lex->tk != ']') {
          if (JSP_SHOULD_EXECUTE(execute)) {
            // OPT: Store array indices as actual ints
            JsVar *a;
            JsVar *aVar;
            JsVar *indexName;
            a = jspeBase(execute);
            aVar = jsvSkipNameAndUnlock(a);
            indexName = jsvMakeIntoVariableName(jsvNewFromInteger(idx),  jsvGetRef(aVar));
            jsvAddName(jsvGetRef(contents), jsvGetRef(indexName));
            jsvUnLock(indexName);
            jsvUnLock(aVar);
          }
          // no need to clean here, as it will definitely be used
          if (execInfo.lex->tk != ']') JSP_MATCH(',');
          idx++;
        }
        JSP_MATCH(']');
        return contents;
    }
    if (execInfo.lex->tk==LEX_R_FUNCTION) {
      JSP_MATCH(LEX_R_FUNCTION);
      return jspeFunctionDefinition(execute);
    }
    if (execInfo.lex->tk==LEX_R_NEW) {
      // new -> create a new object
      JSP_MATCH(LEX_R_NEW);
      if (JSP_SHOULD_EXECUTE(execute)) {
        JsVar *objClassOrFunc = jsvSkipNameAndUnlock(jspeiFindInScopes(jslGetTokenValueAsString(execInfo.lex)));
        if (!objClassOrFunc) {
          jsWarnAt("Prototype used in NEW is not defined", execInfo.lex, execInfo.lex->tokenStart);
        }
        JSP_MATCH(LEX_ID);
        JsVar *obj = jsvNewWithFlags(JSV_OBJECT);
        if (jsvIsFunction(objClassOrFunc)) {
          jsvUnLock(jspeFunctionCall(execute, objClassOrFunc, obj));
        } else {
          jsvUnLock(jsvAddNamedChild(jsvGetRef(obj), jsvGetRef(objClassOrFunc), JSPARSE_PROTOTYPE_CLASS));
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

JsVar *jspeUnary(JsExecFlags *execute) {
    JsVar *a;
    if (execInfo.lex->tk=='!') {
        JSP_MATCH('!'); // binary not
        a = jspeFactor(execute);
        if (JSP_SHOULD_EXECUTE(execute)) {
            JsVar *zero = jsvLock(execInfo.parse->zeroInt);
            JsVar *res = jsvMathsOpPtrSkipNames(a, zero, LEX_EQUAL);
            jsvUnLock(zero);
            jsvUnLock(a); a = res;
        }
    } else
        a = jspeFactor(execute);
    return a;
}

JsVar *jspeTerm(JsExecFlags *execute) {
    JsVar *a = jspeUnary(execute);
    while (execInfo.lex->tk=='*' || execInfo.lex->tk=='/' || execInfo.lex->tk=='%') {
        JsVar *b;
        int op = execInfo.lex->tk;
        JSP_MATCH(execInfo.lex->tk);
        b = jspeUnary(execute);
        if (JSP_SHOULD_EXECUTE(execute)) {
          JsVar *res = jsvMathsOpPtrSkipNames(a, b, op);
          jsvUnLock(a); a = res;
        }
        jsvUnLock(b);
    }
    return a;
}

JsVar *jspeExpression(JsExecFlags *execute) {
    JsVar *a;
    bool negate = false;
    if (execInfo.lex->tk=='-') {
        JSP_MATCH('-');
        negate = true;
    }
    a = jspeTerm(execute);
    if (negate) {
      JsVar *zero = jsvLock(execInfo.parse->zeroInt);
      JsVar *res = jsvMathsOpPtrSkipNames(zero, a, '-');
      jsvUnLock(zero);
      jsvUnLock(a); a = res;
    }

    while (execInfo.lex->tk=='+' || execInfo.lex->tk=='-' ||
        execInfo.lex->tk==LEX_PLUSPLUS || execInfo.lex->tk==LEX_MINUSMINUS) {
        int op = execInfo.lex->tk;
        JSP_MATCH(execInfo.lex->tk);
        if (op==LEX_PLUSPLUS || op==LEX_MINUSMINUS) {
            if (JSP_SHOULD_EXECUTE(execute)) {
                JsVar *one = jsvLock(execInfo.parse->oneInt);
                JsVar *res = jsvMathsOpPtrSkipNames(a, one, op==LEX_PLUSPLUS ? '+' : '-');
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
        } else {
            JsVar *b = jspeTerm(execute);
            if (JSP_SHOULD_EXECUTE(execute)) {
                // not in-place, so just replace
              JsVar *res = jsvMathsOpPtrSkipNames(a, b, op);
              jsvUnLock(a); a = res;
            }
            jsvUnLock(b);
        }
    }
    return a;
}

JsVar *jspeShift(JsExecFlags *execute) {
  JsVar *a = jspeExpression(execute);
  if (execInfo.lex->tk==LEX_LSHIFT || execInfo.lex->tk==LEX_RSHIFT || execInfo.lex->tk==LEX_RSHIFTUNSIGNED) {
    JsVar *b;
    int op = execInfo.lex->tk;
    JSP_MATCH(op);
    b = jspeBase(execute);
    if (JSP_SHOULD_EXECUTE(execute)) {
      JsVar *res = jsvMathsOpPtrSkipNames(a, b, op);
      jsvUnLock(a); a = res;
    }
    jsvUnLock(b);
  }
  return a;
}

JsVar *jspeCondition(JsExecFlags *execute) {
    JsVar *a = jspeShift(execute);
    JsVar *b;
    while (execInfo.lex->tk==LEX_EQUAL || execInfo.lex->tk==LEX_NEQUAL ||
           execInfo.lex->tk==LEX_TYPEEQUAL || execInfo.lex->tk==LEX_NTYPEEQUAL ||
           execInfo.lex->tk==LEX_LEQUAL || execInfo.lex->tk==LEX_GEQUAL ||
           execInfo.lex->tk=='<' || execInfo.lex->tk=='>') {
        int op = execInfo.lex->tk;
        JSP_MATCH(execInfo.lex->tk);
        b = jspeShift(execute);
        if (JSP_SHOULD_EXECUTE(execute)) {
            JsVar *res = jsvMathsOpPtrSkipNames(a, b, op);
            jsvUnLock(a); a = res;
        }
        jsvUnLock(b);
    }
    return a;
}

JsVar *jspeLogic(JsExecFlags *execute) {
    JsVar *a = jspeCondition(execute);
    JsVar *b = 0;
    while (execInfo.lex->tk=='&' || execInfo.lex->tk=='|' || execInfo.lex->tk=='^' || execInfo.lex->tk==LEX_ANDAND || execInfo.lex->tk==LEX_OROR) {
        JsExecFlags noexecute = EXEC_NO;
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
        b = jspeCondition(shortCircuit ? &noexecute : execute);
        if (JSP_SHOULD_EXECUTE(execute) && !shortCircuit) {
            JsVar *res;
            if (boolean) {
              JsVar *newa = jsvNewFromBool(jsvGetBoolSkipName(a));
              JsVar *newb = jsvNewFromBool(jsvGetBoolSkipName(b));
              jsvUnLock(a); a = newa;
              jsvUnLock(b); b = newb;
            }
            res = jsvMathsOpPtrSkipNames(a, b, op);
            jsvUnLock(a); a = res;
        }
        jsvUnLock(b);
    }
    return a;
}

JsVar *jspeTernary(JsExecFlags *execute) {
  JsVar *lhs = jspeLogic(execute);
  JsExecFlags noexecute = EXEC_NO;
  if (execInfo.lex->tk=='?') {
    JSP_MATCH('?');
    if (!JSP_SHOULD_EXECUTE(execute)) {
      jsvUnLock(lhs);
      jsvUnLock(jspeBase(&noexecute));
      JSP_MATCH(':');
      jsvUnLock(jspeBase(&noexecute));
    } else {
      bool first = jsvGetBoolSkipName(lhs);
      jsvUnLock(lhs);
      if (first) {
        lhs = jspeBase(execute);
        JSP_MATCH(':');
        jsvUnLock(jspeBase(&noexecute));
      } else {
        jsvUnLock(jspeBase(&noexecute));
        JSP_MATCH(':');
        lhs = jspeBase(execute);
      }
    }
  }

  return lhs;
}

JsVar *jspeBase(JsExecFlags *execute) {
    JsVar *lhs = jspeTernary(execute);
    if (execInfo.lex->tk=='=' || execInfo.lex->tk==LEX_PLUSEQUAL || execInfo.lex->tk==LEX_MINUSEQUAL) {
        int op;
        JsVar *rhs;
        /* If we're assigning to this and we don't have a parent,
         * add it to the symbol table root as per JavaScript. */
        if (JSP_SHOULD_EXECUTE(execute) && lhs && !lhs->refs) {
          if (jsvIsName(lhs)/* && jsvGetStringLength(lhs)>0*/) {
            jsvAddName(execInfo.parse->root, jsvGetRef(lhs));
          } else // TODO: Why was this here? can it happen?
            jsWarnAt("Trying to assign to an un-named type\n", execInfo.lex, execInfo.lex->tokenLastEnd);
        }

        op = execInfo.lex->tk;
        JSP_MATCH(execInfo.lex->tk);
        rhs = jspeBase(execute);
        rhs = jsvSkipNameAndUnlock(rhs); // ensure we get rid of any references on the RHS
        if (JSP_SHOULD_EXECUTE(execute) && lhs) {
            if (op=='=') {
                jspReplaceWith(lhs, rhs);
            } else if (op==LEX_PLUSEQUAL) {
                JsVar *res = jsvMathsOpPtrSkipNames(lhs,rhs, '+');
                jspReplaceWith(lhs, res);
                jsvUnLock(res);
            } else if (op==LEX_MINUSEQUAL) {
                JsVar *res = jsvMathsOpPtrSkipNames(lhs,rhs, '-');
                jspReplaceWith(lhs, res);
                jsvUnLock(res);
            } else assert(0);
        }
        jsvUnLock(rhs);
    }
    return lhs;
}

JsVar *jspeBlock(JsExecFlags *execute) {
    JSP_MATCH('{');
    if (JSP_SHOULD_EXECUTE(execute)) {
      while (execInfo.lex->tk && execInfo.lex->tk!='}')
        jsvUnLock(jspeStatement(execute));
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

JsVar *jspeStatement(JsExecFlags *execute) {
    if (execInfo.lex->tk==LEX_ID ||
        execInfo.lex->tk==LEX_INT ||
        execInfo.lex->tk==LEX_FLOAT ||
        execInfo.lex->tk==LEX_STR ||
        execInfo.lex->tk=='-' ||
        execInfo.lex->tk=='(') {
        /* Execute a simple statement that only contains basic arithmetic... */
        JsVar *res = jspeBase(execute);
        if (execInfo.lex->tk==';') JSP_MATCH(';');
        return res;
    } else if (execInfo.lex->tk=='{') {
        /* A block of code */
        jspeBlock(execute);
    } else if (execInfo.lex->tk==';') {
        /* Empty statement - to allow things like ;;; */
        JSP_MATCH(';');
    } else if (execInfo.lex->tk==LEX_R_VAR) {
        /* variable creation. TODO - we need a better way of parsing the left
         * hand side. Maybe just have a flag called can_create_var that we
         * set and then we parse as if we're doing a normal equals.*/
        JSP_MATCH(LEX_R_VAR);
        while (execInfo.lex->tk != ';') {
          JsVar *a = 0;
          if (JSP_SHOULD_EXECUTE(execute))
            a = jspeiFindOnTop(jslGetTokenValueAsString(execInfo.lex), true);
          JSP_MATCH(LEX_ID);
          // now do stuff defined with dots
          while (execInfo.lex->tk == '.') {
              JSP_MATCH('.');
              if (JSP_SHOULD_EXECUTE(execute)) {
                  JsVar *lastA = a;
                  a = jsvFindChildFromString(jsvGetRef(lastA), jslGetTokenValueAsString(execInfo.lex), true);
                  jsvUnLock(lastA);
              }
              JSP_MATCH(LEX_ID);
          }
          // sort out initialiser
          if (execInfo.lex->tk == '=') {
              JsVar *var;
              JSP_MATCH('=');
              var = jspeBase(execute);
              if (JSP_SHOULD_EXECUTE(execute))
                  jspReplaceWith(a, var);
              jsvUnLock(var);
          }
          jsvUnLock(a);
          if (execInfo.lex->tk != ';')
            JSP_MATCH(',');
        }
        JSP_MATCH(';');
    } else if (execInfo.lex->tk==LEX_R_IF) {
        JsExecFlags noexecute = EXEC_NO; // because we need to be abl;e to write to it
        bool cond;
        JsVar *var;        
        JSP_MATCH(LEX_R_IF);
        JSP_MATCH('(');
        var = jspeBase(execute);
        JSP_MATCH(')');
        cond = JSP_SHOULD_EXECUTE(execute) && jsvGetBoolSkipName(var);
        jsvUnLock(var);
        jsvUnLock(jspeStatement(cond ? execute : &noexecute));
        if (execInfo.lex->tk==LEX_R_ELSE) {
            JSP_MATCH(LEX_R_ELSE);
            jsvUnLock(jspeStatement(cond ? &noexecute : execute));
        }
    } else if (execInfo.lex->tk==LEX_R_WHILE) {
        int loopCount = JSPARSE_MAX_LOOP_ITERATIONS;
        JsExecFlags noexecute = EXEC_NO;
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
        cond = jspeBase(execute);
        loopCond = JSP_SHOULD_EXECUTE(execute) && jsvGetBoolSkipName(cond);
        jsvUnLock(cond);
        jslInitFromLex(&whileCond, execInfo.lex, whileCondStart);
        JSP_MATCH(')');
        whileBodyStart = execInfo.lex->tokenStart;
        jsvUnLock(jspeStatement(loopCond ? execute : &noexecute));
        jslInitFromLex(&whileBody, execInfo.lex, whileBodyStart);
        oldLex = execInfo.lex;

        while (loopCond && loopCount-->0) {
            jslReset(&whileCond);
            execInfo.lex = &whileCond;
            cond = jspeBase(execute);
            loopCond = JSP_SHOULD_EXECUTE(execute) && jsvGetBoolSkipName(cond);
            jsvUnLock(cond);
            if (loopCond) {
                jslReset(&whileBody);
                execInfo.lex = &whileBody;
                jsvUnLock(jspeStatement(execute));
            }
        }
        execInfo.lex = oldLex;
        jslKill(&whileCond);
        jslKill(&whileBody);

        if (loopCount<=0) {
          jsErrorAt("WHILE Loop exceeded the maximum number of iterations", execInfo.lex, execInfo.lex->tokenLastEnd);
        }
    } else if (execInfo.lex->tk==LEX_R_FOR) {
        int loopCount = JSPARSE_MAX_LOOP_ITERATIONS;
        JsExecFlags noexecute = EXEC_NO;
        int forCondStart;
        JsVar *cond;
        bool loopCond;
        JsLex forCond;
        int forIterStart;
        JsLex forIter;
        int forBodyStart;
        JsLex forBody;
        JsLex *oldLex;
        
        JSP_MATCH(LEX_R_FOR);
        JSP_MATCH('(');
        jsvUnLock(jspeStatement(execute)); // initialisation
        //JSP_MATCH(';');
        forCondStart = execInfo.lex->tokenStart;
        cond = jspeBase(execute); // condition
        loopCond = JSP_SHOULD_EXECUTE(execute) && jsvGetBoolSkipName(cond);
        jsvUnLock(cond);
        jslInitFromLex(&forCond, execInfo.lex, forCondStart);
        JSP_MATCH(';');
        forIterStart = execInfo.lex->tokenStart;
        jsvUnLock(jspeBase(&noexecute)); // iterator
        jslInitFromLex(&forIter, execInfo.lex, forIterStart);
        JSP_MATCH(')');
        forBodyStart = execInfo.lex->tokenStart;
        jsvUnLock(jspeStatement(loopCond ? execute : &noexecute));
        jslInitFromLex(&forBody, execInfo.lex, forBodyStart);
        oldLex = execInfo.lex;
        if (loopCond) {
            jslReset(&forIter);
            execInfo.lex = &forIter;
            jsvUnLock(jspeBase(execute));
        }
        while (JSP_SHOULD_EXECUTE(execute) && loopCond && loopCount-->0) {
            jslReset(&forCond);
            execInfo.lex = &forCond;
            cond = jspeBase(execute);
            loopCond = jsvGetBoolSkipName(cond);
            jsvUnLock(cond);
            if (JSP_SHOULD_EXECUTE(execute) && loopCond) {
                jslReset(&forBody);
                execInfo.lex = &forBody;
                jsvUnLock(jspeStatement(execute));
            }
            if (JSP_SHOULD_EXECUTE(execute) && loopCond) {
                jslReset(&forIter);
                execInfo.lex = &forIter;
                jsvUnLock(jspeBase(execute));
            }
        }
        execInfo.lex = oldLex;
        jslKill(&forCond);
        jslKill(&forIter);
        jslKill(&forBody);
        if (loopCount<=0) {
            jsErrorAt("FOR Loop exceeded the maximum number of iterations", execInfo.lex, execInfo.lex->tokenLastEnd);
        }
    } else if (execInfo.lex->tk==LEX_R_RETURN) {
        JsVar *result = 0;
        JSP_MATCH(LEX_R_RETURN);
        if (execInfo.lex->tk != ';') {
          // we only want the value, so skip the name if there was one
          result = jsvSkipNameAndUnlock(jspeBase(execute));
        }
        if (JSP_SHOULD_EXECUTE(execute)) {
          JsVar *resultVar = jspeiFindOnTop(JSPARSE_RETURN_VAR, false);
          if (resultVar) {
            jspReplaceWith(resultVar, result);
            jsvUnLock(resultVar);
          } else
            jsErrorAt("RETURN statement, but not in a function.\n", execInfo.lex, execInfo.lex->tokenLastEnd);
          *execute = (*execute & (JsExecFlags)(int)~EXEC_RUN_MASK) | EXEC_NO;
        }
        jsvUnLock(result);
        JSP_MATCH(';');
    } else if (execInfo.lex->tk==LEX_R_FUNCTION) {
        JsVar *funcName = 0;
        JsVar *funcVar;
        JSP_MATCH(LEX_R_FUNCTION);
        if (JSP_SHOULD_EXECUTE(execute))
          funcName = jsvMakeIntoVariableName(jsvNewFromString(jslGetTokenValueAsString(execInfo.lex)), 0);
        JSP_MATCH(LEX_ID);
        funcVar = jspeFunctionDefinition(execute);
        if (JSP_SHOULD_EXECUTE(execute)) {
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

void jspInit(JsParse *parse) {
  parse->root = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));

  parse->zeroInt = jsvUnLock(jsvRef(jsvNewFromInteger(0)));
  jsvUnLock(jsvAddNamedChild(parse->root, parse->zeroInt, "#zero#"));
  jsvUnRefRef(parse->zeroInt);
  parse->oneInt = jsvUnLock(jsvRef(jsvNewFromInteger(1)));
  jsvUnLock(jsvAddNamedChild(parse->root, parse->oneInt, "#one#"));
  jsvUnRefRef(parse->oneInt);
  parse->stringClass = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  jsvUnLock(jsvAddNamedChild(parse->root, parse->stringClass, "String"));
  jsvUnRefRef(parse->stringClass);
  parse->objectClass = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  jsvUnLock(jsvAddNamedChild(parse->root, parse->objectClass, "Object"));
  jsvUnRefRef(parse->objectClass);
  parse->arrayClass = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  jsvUnLock(jsvAddNamedChild(parse->root, parse->arrayClass, "Array"));
  jsvUnRefRef(parse->arrayClass);
  parse->intClass = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  jsvUnLock(jsvAddNamedChild(parse->root, parse->intClass, "Integer"));
  jsvUnRefRef(parse->intClass);
  parse->mathClass = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  jsvUnLock(jsvAddNamedChild(parse->root, parse->mathClass, "Math"));
  jsvUnRefRef(parse->mathClass);
  parse->jsonClass = jsvUnLock(jsvRef(jsvNewWithFlags(JSV_OBJECT)));
  jsvUnLock(jsvAddNamedChild(parse->root, parse->jsonClass, "JSON"));
  jsvUnRefRef(parse->jsonClass);
}

void jspKill(JsParse *parse) {
  jsvUnRefRef(parse->root);
}

JsVar *jspEvaluateVar(JsParse *parse, JsVar *str) {
  JsExecFlags execute = EXEC_YES;
  JsExecInfo oldExecInfo = execInfo;
  JsLex lex;
  JsVar *v = 0;

  jslInit(&lex, str, 0, -1);

  jspeiInit(parse, &lex);
  while (execInfo.lex->tk != LEX_EOF) {
    jsvUnLock(v);
    v = jspeStatement(&execute);
  }
  // clean up
  jspeiKill();
  jslKill(&lex);
  // restore state
  execInfo = oldExecInfo;

  // It may have returned a reference, but we just want the value...
  if (v) {
    return jsvSkipNameAndUnlock(v);
  }
  // nothing returned
  return 0;
}

JsVar *jspEvaluate(JsParse *parse, const char *str) {
  JsVar *v;

  JsVar *evCode = jsvNewFromString(str);
  v = jspEvaluateVar(parse, evCode);
  jsvUnLock(evCode);

  return v;
}
