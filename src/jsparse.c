#include "jsparse.h"

typedef struct {
  JsParse *parse;
  JsLex *lex;

  JsVarRef scopes[JSPARSE_MAX_SCOPES];
  int scopeCount;
} JsExecInfo;

typedef enum  {
  EXEC_NO = 0,
  EXEC_YES = 1,
  EXEC_RUN_MASK = 1,
  // EXEC_ERROR = 2 // maybe?
} JsExecFlags;




// ----------------------------------------------- Forward decls
JsVar *jspeBase(JsExecInfo *execInfo, JsExecFlags *execute);
JsVar *jspeBlock(JsExecInfo *execInfo, JsExecFlags *execute);
JsVar *jspeStatement(JsExecInfo *execInfo, JsExecFlags *execute);
// ----------------------------------------------- Utils
#define JSP_MATCH(TOKEN) {if (!jslMatch(execInfo->lex,(TOKEN))) return 0;}
#define JSP_SHOULD_EXECUTE(execute) (((*execute)&EXEC_RUN_MASK)==EXEC_YES)

JsVar *jspReplaceWith(JsExecInfo *execInfo, JsVar *dst, JsVar *src) {
  assert(dst && src);
  // if desination isn't there, isn't a 'name', or is used, just return source
  if (!jsvIsName(dst)) {
    jsErrorAt("Unable to assign value to non-reference", execInfo->lex, execInfo->lex->tokenLastEnd);
    return dst;
  }
  // all is fine, so replace the existing child...
  /* Existing child may be null in the case of Z = 0 where
   * we create 'Z' and pass it down to '=' to have the value
   * filled in.
   */
  if (dst->firstChild) jsvUnRefRef(dst->firstChild); // free existing
  dst->firstChild = jsvRef(src)->this;
  return dst;
}

void jspClean(JsVar *var) {
  if (var) jsvUnLockPtr(var);
}

void jspeiInit(JsExecInfo *execInfo, JsParse *parse, JsLex *lex) {
  execInfo->parse = parse;
  execInfo->lex = lex;
  execInfo->scopeCount = 0;
}

void jspeiKill(JsExecInfo *execInfo) {
  assert(execInfo->scopeCount==0);
}

void jspeiAddScope(JsExecInfo *execInfo, JsVarRef scope) {
  if (execInfo->scopeCount >= JSPARSE_MAX_SCOPES) {
    jsError("Maximum number of scopes exceeded");
    return;
  }
  execInfo->scopes[execInfo->scopeCount++] = jsvRefRef(scope);
}

void jspeiRemoveScope(JsExecInfo *execInfo) {
  if (execInfo->scopeCount <= 0) {
    jsError("INTERNAL: Too many scopes removed");
    return;
  }
  jsvUnRefRef(execInfo->scopes[--execInfo->scopeCount]);
}

JsVar *jspeiFindInScopes(JsExecInfo *execInfo, const char *name) {
  for (int i=execInfo->scopeCount-1;i>=0;i--) {
    JsVar *ref = jsvFindChild(execInfo->scopes[i], name, false);
    if (ref) return ref;
  }
  return jsvFindChild(execInfo->parse->root, name, false);
}

JsVar *jspeiFindOnTop(JsExecInfo *execInfo, const char *name, bool createIfNotFound) {
  if (execInfo->scopeCount>0)
    return jsvFindChild(execInfo->scopes[execInfo->scopeCount-1], name, createIfNotFound);
  return jsvFindChild(execInfo->parse->root, name, createIfNotFound);
}
// -----------------------------------------------

// we return a value so that JSP_MATCH can return 0 if it fails
bool jspeFunctionArguments(JsExecInfo *execInfo, JsVar *funcVar) {
  JSP_MATCH('(');
  while (execInfo->lex->tk!=')') {
      if (funcVar) {
        JsVar *param = jsvAddNamedChild(funcVar->this, 0, jslGetTokenValueAsString(execInfo->lex));
        param->flags = SCRIPTVAR_FUNCTION_PARAMETER;
        jspClean(param);
      }
      JSP_MATCH(LEX_ID);
      if (execInfo->lex->tk!=')') JSP_MATCH(',');
  }
  JSP_MATCH(')');
  return true;
}

#ifdef TODO
void jspAddNativeFunction(JsParse *parse, const char *funcDesc, JSCallback ptr, void *userdata) {
    CScriptLex *oldLex = l;
    l = new CScriptLex(funcDesc);

    CScriptVar *base = root;

    JSP_MATCH(LEX_R_FUNCTION);
    string funcName = execInfo->lex->tkStr;
    JSP_MATCH(LEX_ID);
    /* Check for dots, we might want to do something like function String.substring ... */
    while (execInfo->lex->tk == '.') {
      JSP_MATCH('.');
      CScriptVarLink *link = base->findChild(funcName);
      // if it doesn't exist, make an object class
      if (!link) link = base->addChild(funcName, new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT));
      base = link->var;
      funcName = execInfo->lex->tkStr;
      JSP_MATCH(LEX_ID);
    }

    CScriptVar *funcVar = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
    funcVar->setCallback(ptr, userdata);
    parseFunctionArguments(funcVar);
    delete l;
    l = oldLex;

    base->addChild(funcName, funcVar);
}
#endif

JsVar *jspeFunctionDefinition(JsExecInfo *execInfo, JsExecFlags *execute) {
  // actually parse a function... We assume that the LEX_FUNCTION and name
  // have already been parsed
  JsVar *funcVar = 0;
  if (JSP_SHOULD_EXECUTE(execute))
    funcVar = jsvNewWithFlags(SCRIPTVAR_FUNCTION);
  // Get arguments save them to the structure
  if (!jspeFunctionArguments(execInfo, funcVar)) {
    // parse failed
    return 0;
  }
  // Get the code - first parse it so we know where it stops
  int funcBegin = execInfo->lex->tokenStart;
  JsExecFlags noexecute = EXEC_NO;
  jspClean(jspeBlock(execInfo, &noexecute));
  // Then create var and set
  if (JSP_SHOULD_EXECUTE(execute)) {
    JsVar *funcCodeVar = jsvNewFromLexer(execInfo->lex, funcBegin, execInfo->lex->tokenLastEnd+1);
    jspClean(jsvAddNamedChild(funcVar->this, funcCodeVar->this, TINYJS_FUNCTION_CODE_NAME));
  }
  return funcVar;
}

/** Handle a function call (assumes we've parsed the function name and we're
 * on the start bracket). 'parent' is the object that contains this method,
 * if there was one (otherwise it's just a normnal function).
 */
JsVar *jspeFunctionCall(JsExecInfo *execInfo, JsExecFlags *execute, JsVar *function, JsVar *parent) {
  if (JSP_SHOULD_EXECUTE(execute)) {
    if (!jsvIsFunction(function)) {
        jsErrorAt("Expecting a function to call", execInfo->lex, execInfo->lex->tokenLastEnd );
        return 0;
    }
    JSP_MATCH('(');
    // create a new symbol table entry for execution of this function
    // OPT: can we cache this function execution environment + param variables?
    JsVar *functionRoot = jsvNewWithFlags(SCRIPTVAR_FUNCTION);
    if (parent)
      jspClean(jsvAddNamedChild(functionRoot->this, parent->this, TINYJS_THIS_VAR));
    // grab in all parameters
    JsVarRef v = function->firstChild;
    while (v) {
        JsVar *param = jsvLock(v);
        if (jsvIsFunctionParameter(param)) {
          JsVar *valueName = jspeBase(execInfo, execute);
          if (JSP_SHOULD_EXECUTE(execute)) {
            char paramName[JSLEX_MAX_TOKEN_LENGTH];
            jsvGetString(param, paramName, JSLEX_MAX_TOKEN_LENGTH);

            JsVar *value = jsvSkipName(valueName);
            // TODO: deep copy required?
            /*if (jsvIsBasic(value)) {
              // pass by value
              jsvAddNamedChild(functionRoot->this, v->name, value->var->deepCopy());
            } else {
              // pass by reference
              jsvAddNamedChild(functionRoot->this, v->name, value);
            }*/
            jspClean(jsvAddNamedChild(functionRoot->this, value->this, paramName));
            jspClean(value);
          }
          jspClean(valueName);
          if (execInfo->lex->tk!=')') JSP_MATCH(',');
        }
        v = param->nextSibling;
        jsvUnLockPtr(param);
    }
    JSP_MATCH(')');
    // setup a return variable
    JsVar *returnVarName = jsvAddNamedChild(functionRoot->this, 0, TINYJS_RETURN_VAR);
    // add the function's execute space to the symbol table so we can recurse
    jspeiAddScope(execInfo, functionRoot->this);
    //jsvTrace(functionRoot->this, 5); // debugging
#ifdef TINYJS_CALL_STACK
    call_stack.push_back(function->name + " from " + l->getPosition());
#endif

    if (jsvIsNative(function)) {
        assert(function->callback);
#ifdef TODO
        function->var->jsCallback(functionRoot, function->var->jsCallbackUserData);
#endif
    } else {
        /* we just want to execute the block, but something could
         * have messed up and left us with the wrong ScriptLex, so
         * we want to be careful here... */

        JsVar *functionCode = jsvFindChild(function->this, TINYJS_FUNCTION_CODE_NAME, false);
        if (functionCode) {
          JsVar* functionCodeVar = jsvSkipName(functionCode);
          JsLex newLex;
          jslInit(&newLex, functionCodeVar, 0, -1);
          jspClean(functionCodeVar);
          jspClean(functionCode);

          JsLex *oldLex = execInfo->lex;
          execInfo->lex = &newLex;
          JsExecFlags oldExecute = *execute;
          jspeBlock(execInfo, execute);
          // TODO: what about an error flag in execute?
          *execute = oldExecute; // because return will probably have called this, and set execute to false
          jslKill(&newLex);
          execInfo->lex = oldLex;
        }
    }
#ifdef TINYJS_CALL_STACK
    if (!call_stack.empty()) call_stack.pop_back();
#endif
    jspeiRemoveScope(execInfo);
    /* get the real return var before we remove it from our function */
    JsVar *returnVar = jsvSkipName(returnVarName);
    jsvUnLockPtr(functionRoot);
    if (returnVar)
      return returnVar;
    else
      return jsvNewWithFlags(SCRIPTVAR_UNDEFINED);
  } else {
    // function, but not executing - just parse args and be done
    JSP_MATCH('(');
    while (execInfo->lex->tk != ')') {
      JsVar *value = jspeBase(execInfo, execute);
      jspClean(value);
      if (execInfo->lex->tk!=')') JSP_MATCH(',');
    }
    JSP_MATCH(')');
    /* function will be a blank scriptvarlink if we're not executing,
     * so just return it rather than an alloc/free */
    return function;
  }
}

JsVar *jspeFactor(JsExecInfo *execInfo, JsExecFlags *execute) {
    if (execInfo->lex->tk=='(') {
        JSP_MATCH('(');
        JsVar *a = jspeBase(execInfo, execute);
        JSP_MATCH(')');
        return a;
    }
    if (execInfo->lex->tk==LEX_R_TRUE) {
        JSP_MATCH(LEX_R_TRUE);
        return jsvNewFromBool(true);
    }
    if (execInfo->lex->tk==LEX_R_FALSE) {
        JSP_MATCH(LEX_R_FALSE);
        return jsvNewFromBool(false);
    }
    if (execInfo->lex->tk==LEX_R_NULL) {
        JSP_MATCH(LEX_R_NULL);
        return jsvNewWithFlags(SCRIPTVAR_NULL);
    }
    if (execInfo->lex->tk==LEX_R_UNDEFINED) {
        JSP_MATCH(LEX_R_UNDEFINED);
        return jsvNewWithFlags(SCRIPTVAR_UNDEFINED);
    }
    if (execInfo->lex->tk==LEX_ID) {
        JsVar *a = JSP_SHOULD_EXECUTE(execute) ? jspeiFindInScopes(execInfo, jslGetTokenValueAsString(execInfo->lex)) : 0;
        /* The parent if we're executing a method call */
        JsVar *parent = 0;

        if (JSP_SHOULD_EXECUTE(execute) && !a) {
          /* Variable doesn't exist! JavaScript says we should create it
           * (we won't add it here. This is done in the assignment operator)*/
          a = jsvNewVariableNameFromLexerToken(0, execInfo->lex);
        }
        JSP_MATCH(LEX_ID);
        while (execInfo->lex->tk=='(' || execInfo->lex->tk=='.' || execInfo->lex->tk=='[') {
          if (execInfo->lex->tk=='(') { // ------------------------------------- Function Call
            JsVar *func = 0;
            if (JSP_SHOULD_EXECUTE(execute)) {
              func = jsvSkipName(a);
              jspClean(a);
            }
            a = jspeFunctionCall(execInfo, execute, func, parent);
            jspClean(func);
            } else if (execInfo->lex->tk == '.') { // ------------------------------------- Record Access
                JSP_MATCH('.');
                if (JSP_SHOULD_EXECUTE(execute)) {
                  // Note: name will go away when we oarse something else!
                  const char *name = jslGetTokenValueAsString(execInfo->lex);

                  JsVar *aVar = jsvSkipName(a);
                  JsVar *child = jsvFindChild(aVar->this, name, false);
#ifdef TODO
                  if (!child) child = findInParentClasses(aVar, name);
#endif
                  if (!child) {
                    /* if we haven't found this defined yet, use the built-in
                       'length' properly */
                    if (jsvIsArray(aVar) && strcmp(name,"length")==0) {
                      int l = jsvGetArrayLength(aVar);
                      child = jsvNewFromInteger(l);
                    } else if (jsvIsString(aVar) && strcmp(name,"length")==0) {
                      int l = jsvGetStringLength(aVar);
                      child = jsvNewFromInteger(l);
                    } else {
                      // TODO: ensure aVar is an object
                      child = jsvAddNamedChild(aVar->this, 0, name);
                    }
                  }
                  jspClean(parent);
                  parent = aVar;
                  a = child;
                }
                JSP_MATCH(LEX_ID);
            } else if (execInfo->lex->tk == '[') { // ------------------------------------- Array Access
                JSP_MATCH('[');
                JsVar *index = jspeBase(execInfo, execute);
                JSP_MATCH(']');
                if (JSP_SHOULD_EXECUTE(execute)) {
                  JsVar *aVar = jsvSkipName(a);
                  // OPT: pass variable value directly
                  char buf[JSLEX_MAX_TOKEN_LENGTH];
                  JsVar *indexValue = jsvSkipName(index);
                  jsvGetString(indexValue, buf, JSLEX_MAX_TOKEN_LENGTH);
                  jsvUnLockPtr(indexValue);

                  JsVar *child = jsvFindChild(aVar->this, buf, true);
                  jspClean(parent);
                  parent = aVar;
                  jspClean(a);
                  a = child;
                }
                jspClean(index);
            } else assert(0);
        }
        jspClean(parent);
        return a;
    }
    if (execInfo->lex->tk==LEX_INT) {
        long v = atol(jslGetTokenValueAsString(execInfo->lex));
        JSP_MATCH(LEX_INT);
        return jsvNewFromInteger(v);
    }
    if (execInfo->lex->tk==LEX_FLOAT) {
        double v = atof(jslGetTokenValueAsString(execInfo->lex));
        JSP_MATCH(LEX_FLOAT);
        return jsvNewFromFloat(v);
    }
    if (execInfo->lex->tk==LEX_STR) {
        JsVar *a = jsvNewFromString(jslGetTokenValueAsString(execInfo->lex));
        JSP_MATCH(LEX_STR);
        return a;
    }
    if (execInfo->lex->tk=='{') {
        JsVar *contents = jsvNewWithFlags(SCRIPTVAR_OBJECT);
        /* JSON-style object definition */
        JSP_MATCH('{');
        while (execInfo->lex->tk != '}') {
          char id[JSLEX_MAX_TOKEN_LENGTH];
          jslGetTokenString(execInfo->lex, id, JSLEX_MAX_TOKEN_LENGTH);
          // we only allow strings or IDs on the left hand side of an initialisation
          if (execInfo->lex->tk==LEX_STR) {
            JSP_MATCH(LEX_STR);
          } else {
            JSP_MATCH(LEX_ID);
          }
          JSP_MATCH(':');
          if (JSP_SHOULD_EXECUTE(execute)) {
            JsVar *a = jspeBase(execInfo, execute);
            assert(a);
            JsVar *aVar = jsvSkipName(a);
            jspClean(jsvAddNamedChild(contents->this, aVar->this, id));
            jspClean(aVar);
            jspClean(a);
          }
          // no need to clean here, as it will definitely be used
          if (execInfo->lex->tk != '}') JSP_MATCH(',');
        }

        JSP_MATCH('}');
        return contents;
    }
    if (execInfo->lex->tk=='[') {
        JsVar *contents = jsvNewWithFlags(SCRIPTVAR_ARRAY);
        /* JSON-style array */
        JSP_MATCH('[');
        int idx = 0;
        while (execInfo->lex->tk != ']') {
          if (JSP_SHOULD_EXECUTE(execute)) {
            // OPT: Store array indices as actual ints
            char idx_str[16]; // big enough for 2^32
            snprintf(idx_str, sizeof(idx_str), "%d",idx);

            JsVar *a = jspeBase(execInfo, execute);
            assert(a);
            JsVar *aVar = jsvSkipName(a);
            jspClean(jsvAddNamedChild(contents->this, aVar->this, idx_str));
            jspClean(aVar);
            jspClean(a);
          }
          // no need to clean here, as it will definitely be used
          if (execInfo->lex->tk != ']') JSP_MATCH(',');
          idx++;
        }
        JSP_MATCH(']');
        return contents;
    }
    if (execInfo->lex->tk==LEX_R_FUNCTION) {
      JSP_MATCH(LEX_R_FUNCTION);
      return jspeFunctionDefinition(execInfo, execute);
    }
#if TODO
    if (execInfo->lex->tk==LEX_R_NEW) {
      // new -> create a new object
      JSP_MATCH(LEX_R_NEW);
      const string &className = execInfo->lex->tkStr;
      if (JSP_SHOULD_EXECUTE(execute)) {
        JsVar *objClassOrFunc = findInScopes(className);
        if (!objClassOrFunc) {
          TRACE("%s is not a valid class name", className.c_str());
          return new JsVar(new CScriptVar());
        }
        JSP_MATCH(LEX_ID);
        CScriptVar *obj = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
        JsVar *objLink = new JsVar(obj);
        if (objClassOrFunc->var->isFunction()) {
          jspClean(functionCall(execute, objClassOrFunc, obj));
        } else {
          obj->addChild(TINYJS_PROTOTYPE_CLASS, objClassOrFunc->var);
          if (execInfo->lex->tk == '(') {
            JSP_MATCH('(');
            JSP_MATCH(')');
          }
        }
        return objLink;
      } else {
        JSP_MATCH(LEX_ID);
        if (execInfo->lex->tk == '(') {
          JSP_MATCH('(');
          JSP_MATCH(')');
        }
      }
    }
#endif
    // Nothing we can do here... just hope it's the end...
    JSP_MATCH(LEX_EOF);
    return 0;
}

JsVar *jspeUnary(JsExecInfo *execInfo, JsExecFlags *execute) {
    JsVar *a;
    if (execInfo->lex->tk=='!') {
        JSP_MATCH('!'); // binary not
        a = jspeFactor(execInfo, execute);
        if (JSP_SHOULD_EXECUTE(execute)) {
            JsVar *zero = jsvLock(execInfo->parse->zeroInt);
            JsVar *res = jsvMathsOpPtrSkipNames(a, zero, LEX_EQUAL);
            jsvUnLockPtr(zero);
            jspClean(a); a = res;
        }
    } else
        a = jspeFactor(execInfo, execute);
    return a;
}

JsVar *jspeTerm(JsExecInfo *execInfo, JsExecFlags *execute) {
    JsVar *a = jspeUnary(execInfo, execute);
    while (execInfo->lex->tk=='*' || execInfo->lex->tk=='/' || execInfo->lex->tk=='%') {
        int op = execInfo->lex->tk;
        JSP_MATCH(execInfo->lex->tk);
        JsVar *b = jspeUnary(execInfo, execute);
        if (JSP_SHOULD_EXECUTE(execute)) {
          JsVar *res = jsvMathsOpPtrSkipNames(a, b, op);
          jspClean(a); a = res;
        }
        jspClean(b);
    }
    return a;
}

JsVar *jspeExpression(JsExecInfo *execInfo, JsExecFlags *execute) {
    bool negate = false;
    if (execInfo->lex->tk=='-') {
        JSP_MATCH('-');
        negate = true;
    }
    JsVar *a = jspeTerm(execInfo, execute);
    if (negate) {
      JsVar *zero = jsvLock(execInfo->parse->zeroInt);
      JsVar *res = jsvMathsOpPtrSkipNames(zero, a, '-');
      jsvUnLockPtr(zero);
      jspClean(a); a = res;
    }

    while (execInfo->lex->tk=='+' || execInfo->lex->tk=='-' ||
        execInfo->lex->tk==LEX_PLUSPLUS || execInfo->lex->tk==LEX_MINUSMINUS) {
        int op = execInfo->lex->tk;
        JSP_MATCH(execInfo->lex->tk);
        if (op==LEX_PLUSPLUS || op==LEX_MINUSMINUS) {
            if (JSP_SHOULD_EXECUTE(execute)) {
                JsVar *one = jsvLock(execInfo->parse->oneInt);
                JsVar *res = jsvMathsOpPtrSkipNames(a, one, op==LEX_PLUSPLUS ? '+' : '-');
                jsvUnLockPtr(one);
                JsVar *oldValue = jsvLockPtr(a); // keep old value
                // in-place add/subtract
                jspReplaceWith(execInfo, a, res);
                jspClean(a); jspClean(res);
                // but then use the old value
                a = oldValue;
            }
        } else {
            JsVar *b = jspeTerm(execInfo, execute);
            if (JSP_SHOULD_EXECUTE(execute)) {
                // not in-place, so just replace
              JsVar *res = jsvMathsOpPtrSkipNames(a, b, op);
              jspClean(a); a = res;
            }
            jspClean(b);
        }
    }
    return a;
}

JsVar *jspeShift(JsExecInfo *execInfo, JsExecFlags *execute) {
  JsVar *a = jspeExpression(execInfo, execute);
  if (execInfo->lex->tk==LEX_LSHIFT || execInfo->lex->tk==LEX_RSHIFT || execInfo->lex->tk==LEX_RSHIFTUNSIGNED) {
    int op = execInfo->lex->tk;
    JSP_MATCH(op);
    JsVar *b = jspeBase(execInfo, execute);
    jspClean(b);
    if (JSP_SHOULD_EXECUTE(execute)) {
      JsVar *res = jsvMathsOpPtrSkipNames(a, b, op);
      jspClean(a); a = res;
    }
  }
  return a;
}

JsVar *jspeCondition(JsExecInfo *execInfo, JsExecFlags *execute) {
    JsVar *a = jspeShift(execInfo, execute);
    JsVar *b;
    while (execInfo->lex->tk==LEX_EQUAL || execInfo->lex->tk==LEX_NEQUAL ||
           execInfo->lex->tk==LEX_TYPEEQUAL || execInfo->lex->tk==LEX_NTYPEEQUAL ||
           execInfo->lex->tk==LEX_LEQUAL || execInfo->lex->tk==LEX_GEQUAL ||
           execInfo->lex->tk=='<' || execInfo->lex->tk=='>') {
        int op = execInfo->lex->tk;
        JSP_MATCH(execInfo->lex->tk);
        b = jspeShift(execInfo, execute);
        if (JSP_SHOULD_EXECUTE(execute)) {
            JsVar *res = jsvMathsOpPtrSkipNames(a, b, op);
            jspClean(a); a = res;
        }
        jspClean(b);
    }
    return a;
}

JsVar *jspeLogic(JsExecInfo *execInfo, JsExecFlags *execute) {
    JsVar *a = jspeCondition(execInfo, execute);
    JsVar *b;
    while (execInfo->lex->tk=='&' || execInfo->lex->tk=='|' || execInfo->lex->tk=='^' || execInfo->lex->tk==LEX_ANDAND || execInfo->lex->tk==LEX_OROR) {
        JsExecFlags noexecute = EXEC_NO;
        int op = execInfo->lex->tk;
        JSP_MATCH(execInfo->lex->tk);
        bool shortCircuit = false;
        bool boolean = false;
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
        b = jspeCondition(execInfo, shortCircuit ? &noexecute : execute);
        if (JSP_SHOULD_EXECUTE(execute) && !shortCircuit) {
            if (boolean) {
              JsVar *newa = jsvNewFromBool(jsvGetBoolSkipName(a));
              JsVar *newb = jsvNewFromBool(jsvGetBoolSkipName(b));
              jspClean(a); a = newa;
              jspClean(b); b = newb;
            }
            JsVar *res = jsvMathsOpPtrSkipNames(a, b, op);
            jspClean(a); a = res;
        }
        jspClean(b);
    }
    return a;
}

JsVar *jspeTernary(JsExecInfo *execInfo, JsExecFlags *execute) {
  JsVar *lhs = jspeLogic(execInfo, execute);
  JsExecFlags noexecute = EXEC_NO;
  if (execInfo->lex->tk=='?') {
    JSP_MATCH('?');
    if (!JSP_SHOULD_EXECUTE(execute)) {
      jspClean(lhs);
      jspClean(jspeBase(execInfo, &noexecute));
      JSP_MATCH(':');
      jspClean(jspeBase(execInfo, &noexecute));
    } else {
      bool first = jsvGetBoolSkipName(lhs);
      jspClean(lhs);
      if (first) {
        lhs = jspeBase(execInfo, execute);
        JSP_MATCH(':');
        jspClean(jspeBase(execInfo, &noexecute));
      } else {
        jspClean(jspeBase(execInfo, &noexecute));
        JSP_MATCH(':');
        lhs = jspeBase(execInfo, execute);
      }
    }
  }

  return lhs;
}

JsVar *jspeBase(JsExecInfo *execInfo, JsExecFlags *execute) {
    JsVar *lhs = jspeTernary(execInfo, execute);
    if (execInfo->lex->tk=='=' || execInfo->lex->tk==LEX_PLUSEQUAL || execInfo->lex->tk==LEX_MINUSEQUAL) {
        /* If we're assigning to this and we don't have a parent,
         * add it to the symbol table root as per JavaScript. */
        if (JSP_SHOULD_EXECUTE(execute) && !lhs->refs) {
          if (jsvIsName(lhs)/* && jsvGetStringLength(lhs)>0*/) {
            jsvAddName(execInfo->parse->root, lhs->this);
          } else // TODO: Why was this here? can it happen?
            jsWarnAt("Trying to assign to an un-named type\n", execInfo->lex, execInfo->lex->tokenLastEnd);
        }

        int op = execInfo->lex->tk;
        JSP_MATCH(execInfo->lex->tk);
        JsVar *rhs = jspeBase(execInfo, execute);
        if (JSP_SHOULD_EXECUTE(execute)) {
            if (op=='=') {
                jspReplaceWith(execInfo, lhs, rhs);
            } else if (op==LEX_PLUSEQUAL) {
                JsVar *res = jsvMathsOpPtrSkipNames(lhs,rhs, '+');
                jspReplaceWith(execInfo, lhs, res);
                jspClean(res);
            } else if (op==LEX_MINUSEQUAL) {
                JsVar *res = jsvMathsOpPtrSkipNames(lhs,rhs, '-');
                jspReplaceWith(execInfo, lhs, res);
                jspClean(res);
            } else assert(0);
        }
        jspClean(rhs);
    }
    return lhs;
}

JsVar *jspeBlock(JsExecInfo *execInfo, JsExecFlags *execute) {
    JSP_MATCH('{');
    if (JSP_SHOULD_EXECUTE(execute)) {
      while (execInfo->lex->tk && execInfo->lex->tk!='}')
        jspClean(jspeStatement(execInfo, execute));
      JSP_MATCH('}');
    } else {
      // fast skip of blocks
      int brackets = 1;
      while (execInfo->lex->tk && brackets) {
        if (execInfo->lex->tk == '{') brackets++;
        if (execInfo->lex->tk == '}') brackets--;
        JSP_MATCH(execInfo->lex->tk);
      }
    }
    return 0;
}

JsVar *jspeStatement(JsExecInfo *execInfo, JsExecFlags *execute) {
    if (execInfo->lex->tk==LEX_ID ||
        execInfo->lex->tk==LEX_INT ||
        execInfo->lex->tk==LEX_FLOAT ||
        execInfo->lex->tk==LEX_STR ||
        execInfo->lex->tk=='-') {
        /* Execute a simple statement that only contains basic arithmetic... */
        JsVar *res = jspeBase(execInfo, execute);
        if (execInfo->lex->tk==';') JSP_MATCH(';');
        return res;
    } else if (execInfo->lex->tk=='{') {
        /* A block of code */
        jspeBlock(execInfo, execute);
    } else if (execInfo->lex->tk==';') {
        /* Empty statement - to allow things like ;;; */
        JSP_MATCH(';');
    } else if (execInfo->lex->tk==LEX_R_VAR) {
        /* variable creation. TODO - we need a better way of parsing the left
         * hand side. Maybe just have a flag called can_create_var that we
         * set and then we parse as if we're doing a normal equals.*/
        JSP_MATCH(LEX_R_VAR);
        while (execInfo->lex->tk != ';') {
          JsVar *a = 0;
          if (JSP_SHOULD_EXECUTE(execute))
            a = jspeiFindOnTop(execInfo, jslGetTokenValueAsString(execInfo->lex), true);
          JSP_MATCH(LEX_ID);
          // now do stuff defined with dots
          while (execInfo->lex->tk == '.') {
              JSP_MATCH('.');
              if (JSP_SHOULD_EXECUTE(execute)) {
                  JsVar *lastA = a;
                  a = jsvFindChild(lastA->this, jslGetTokenValueAsString(execInfo->lex), true);
                  jspClean(lastA);
              }
              JSP_MATCH(LEX_ID);
          }
          // sort out initialiser
          if (execInfo->lex->tk == '=') {
              JSP_MATCH('=');
              JsVar *var = jspeBase(execInfo, execute);
              if (JSP_SHOULD_EXECUTE(execute))
                  jspReplaceWith(execInfo, a, var);
              jspClean(var);
          }
          jspClean(a);
          if (execInfo->lex->tk != ';')
            JSP_MATCH(',');
        }
        JSP_MATCH(';');
    } else if (execInfo->lex->tk==LEX_R_IF) {
        JSP_MATCH(LEX_R_IF);
        JSP_MATCH('(');
        JsVar *var = jspeBase(execInfo, execute);
        JSP_MATCH(')');
        bool cond = JSP_SHOULD_EXECUTE(execute) && jsvGetBoolSkipName(var);
        jspClean(var);
        JsExecFlags noexecute = EXEC_NO; // because we need to be abl;e to write to it
        jspeStatement(execInfo, cond ? execute : &noexecute);
        if (execInfo->lex->tk==LEX_R_ELSE) {
            JSP_MATCH(LEX_R_ELSE);
            jspeStatement(execInfo, cond ? &noexecute : execute);
        }
    } else if (execInfo->lex->tk==LEX_R_WHILE) {
        // We do repetition by pulling out the string representing our statement
        // there's definitely some opportunity for optimisation here
        JSP_MATCH(LEX_R_WHILE);
        JSP_MATCH('(');
        int whileCondStart = execInfo->lex->tokenStart;
        JsExecFlags noexecute = EXEC_NO;
        JsVar *cond = jspeBase(execInfo, execute);
        bool loopCond = JSP_SHOULD_EXECUTE(execute) && jsvGetBoolSkipName(cond);
        jspClean(cond);
        JsLex whileCond;
        jslInitFromLex(&whileCond, execInfo->lex, whileCondStart);
        JSP_MATCH(')');
        int whileBodyStart = execInfo->lex->tokenStart;
        jspeStatement(execInfo, loopCond ? execute : &noexecute);
        JsLex whileBody;
        jslInitFromLex(&whileBody, execInfo->lex, whileBodyStart);
        JsLex *oldLex = execInfo->lex;

        int loopCount = TINYJS_LOOP_MAX_ITERATIONS;
        while (loopCond && loopCount-->0) {
            jslReset(&whileCond);
            execInfo->lex = &whileCond;
            cond = jspeBase(execInfo, execute);
            loopCond = JSP_SHOULD_EXECUTE(execute) && jsvGetBoolSkipName(cond);
            jspClean(cond);
            if (loopCond) {
                jslReset(&whileBody);
                execInfo->lex = &whileBody;
                jspeStatement(execInfo, execute);
            }
        }
        execInfo->lex = oldLex;
        jslKill(&whileCond);
        jslKill(&whileBody);

        if (loopCount<=0) {
          jsErrorAt("WHILE Loop exceeded the maximum number of iterations", execInfo->lex, execInfo->lex->tokenLastEnd);
        }
    } else if (execInfo->lex->tk==LEX_R_FOR) {
        JSP_MATCH(LEX_R_FOR);
        JSP_MATCH('(');
        jspeStatement(execInfo, execute); // initialisation
        //JSP_MATCH(';');
        int forCondStart = execInfo->lex->tokenStart;
        JsExecFlags noexecute = EXEC_NO;
        JsVar *cond = jspeBase(execInfo, execute); // condition
        bool loopCond = JSP_SHOULD_EXECUTE(execute) && jsvGetBoolSkipName(cond);
        jspClean(cond);
        JsLex forCond;
        jslInitFromLex(&forCond, execInfo->lex, forCondStart);
        JSP_MATCH(';');
        int forIterStart = execInfo->lex->tokenStart;
        jspClean(jspeBase(execInfo, &noexecute)); // iterator
        JsLex forIter;
        jslInitFromLex(&forIter, execInfo->lex, forIterStart);
        JSP_MATCH(')');
        int forBodyStart = execInfo->lex->tokenStart;
        jspeStatement(execInfo, loopCond ? execute : &noexecute);
        JsLex forBody;
        jslInitFromLex(&forBody, execInfo->lex, forBodyStart);
        JsLex *oldLex = execInfo->lex;
        if (loopCond) {
            jslReset(&forIter);
            execInfo->lex = &forIter;
            jspClean(jspeBase(execInfo, execute));
        }
        int loopCount = TINYJS_LOOP_MAX_ITERATIONS;
        while (JSP_SHOULD_EXECUTE(execute) && loopCond && loopCount-->0) {
          jslReset(&forCond);
            execInfo->lex = &forCond;
            cond = jspeBase(execInfo, execute);
            loopCond = jsvGetBoolSkipName(cond);
            jspClean(cond);
            if (JSP_SHOULD_EXECUTE(execute) && loopCond) {
                jslReset(&forBody);
                execInfo->lex = &forBody;
                jspeStatement(execInfo, execute);
            }
            if (JSP_SHOULD_EXECUTE(execute) && loopCond) {
                jslReset(&forIter);
                execInfo->lex = &forIter;
                jspClean(jspeBase(execInfo, execute));
            }
        }
        execInfo->lex = oldLex;
        jslKill(&forCond);
        jslKill(&forIter);
        jslKill(&forBody);
        if (loopCount<=0) {
            jsErrorAt("FOR Loop exceeded the maximum number of iterations", execInfo->lex, execInfo->lex->tokenLastEnd);
        }
    } else if (execInfo->lex->tk==LEX_R_RETURN) {
        JSP_MATCH(LEX_R_RETURN);
        JsVar *result = 0;
        if (execInfo->lex->tk != ';')
          result = jspeBase(execInfo, execute);
        if (JSP_SHOULD_EXECUTE(execute)) {
          JsVar *resultVar = jspeiFindOnTop(execInfo, TINYJS_RETURN_VAR, false);
          if (resultVar) {
            jspReplaceWith(execInfo, resultVar, result);
            jspClean(resultVar);
          } else
            jsErrorAt("RETURN statement, but not in a function.\n", execInfo->lex, execInfo->lex->tokenLastEnd);
          *execute = (*execute & ~EXEC_RUN_MASK) | EXEC_NO;
        }
        jspClean(result);
        JSP_MATCH(';');
    } else if (execInfo->lex->tk==LEX_R_FUNCTION) {
        JSP_MATCH(LEX_R_FUNCTION);
        JsVar *func = 0;
        if (JSP_SHOULD_EXECUTE(execute)) func = jsvNewVariableNameFromLexerToken(0, execInfo->lex);
        JSP_MATCH(LEX_ID);
        JsVar *funcVar = jspeFunctionDefinition(execInfo, execute);
        if (JSP_SHOULD_EXECUTE(execute)) {
          char funcName[JSLEX_MAX_TOKEN_LENGTH];
          jsvGetString(func, funcName, JSLEX_MAX_TOKEN_LENGTH);
          // find a function with the same name (or make one)
          // OPT: can Find* use just a JsVar that is a 'name'?
          JsVar *existingFunc = jspeiFindOnTop(execInfo, funcName, true);
          // replace it
          jspReplaceWith(execInfo, existingFunc, funcVar);
          jspClean(func);
          func = existingFunc;
        }
        jspClean(funcVar);
        return func;
    } else JSP_MATCH(LEX_EOF);
    return 0;
}

// -----------------------------------------------------------------------------

void jspInit(JsParse *parse) {
  parse->root = jsvUnLockPtr(jsvRef(jsvNewWithFlags(SCRIPTVAR_OBJECT)));

  parse->zeroInt = jsvUnLockPtr(jsvRef(jsvNewFromInteger(0)));
  jspClean(jsvAddNamedChild(parse->root, parse->zeroInt, "__ZERO"));
  jsvUnRefRef(parse->zeroInt);
  parse->oneInt = jsvUnLockPtr(jsvRef(jsvNewFromInteger(1)));
  jspClean(jsvAddNamedChild(parse->root, parse->oneInt, "__ONE"));
  jsvUnRefRef(parse->oneInt);
  parse->stringClass = jsvUnLockPtr(jsvRef(jsvNewWithFlags(SCRIPTVAR_OBJECT)));
  jspClean(jsvAddNamedChild(parse->root, parse->stringClass, "String"));
  jsvUnRefRef(parse->stringClass);
  parse->objectClass = jsvUnLockPtr(jsvRef(jsvNewWithFlags(SCRIPTVAR_OBJECT)));
  jspClean(jsvAddNamedChild(parse->root, parse->objectClass, "Object"));
  jsvUnRefRef(parse->objectClass);
  parse->arrayClass = jsvUnLockPtr(jsvRef(jsvNewWithFlags(SCRIPTVAR_OBJECT)));
  jspClean(jsvAddNamedChild(parse->root, parse->arrayClass, "Array"));
  jsvUnRefRef(parse->arrayClass);
}

void jspKill(JsParse *parse) {
  jsvUnRefRef(parse->root);
}

JsVar *jspEvaluate(JsParse *parse, const char *str) {
  JsVar *code = jsvNewFromString(str);
  JsLex lex;
  jslInit(&lex, code, 0, -1);
  jsvUnLockPtr(code);

  JsExecInfo execInfo;
  jspeiInit(&execInfo, parse, &lex);
  JsExecFlags execute = EXEC_YES;

  JsVar *v = 0;
  while (execInfo.lex->tk != LEX_EOF) {
    jspClean(v);
    v = jspeStatement(&execInfo, &execute);
  }
  jslKill(&lex);

  // It may have returned a reference, but we just want the value...
  if (v) {
    JsVar *nv = jsvSkipName(v);
    jspClean(v);
    return nv;
  }
  // nothing returned
  return 0;
}
