#include "jsparse.h"

typedef struct {
  JsParse *parse;
  JsLex *lex;
} JsExecInfo;

typedef enum  {
  EXEC_NO = 0,
  EXEC_YES = 1
} JsExecFlags;




// ----------------------------------------------- Forward decls
JsVar *jspeBase(JsExecInfo *execInfo, JsExecFlags execute);
// ----------------------------------------------- Utils
#define JSP_MATCH(TOKEN) if (!jslMatch(execInfo->lex,(TOKEN))) return 0;

JsVar *jspReplaceWith(JsExecInfo *execInfo, JsVar *dst, JsVar *src) {
  assert(dst && src);
  // if desination isn't there, isn't a 'name', or is used, just return source
  if (!jsvIsName(dst)) {
    jsErrorAt("Unable to assign value to non-reference", execInfo->lex, execInfo->lex->tokenLastEnd);
    return dst;
  }
  // all is fine, so replace the existing child...
  jsvUnRefRef(dst->firstChild); // free existing
  dst->firstChild = jsvRef(src)->this;
  return dst;
}

void jspClean(JsVar *var) {
  if (var) jsvUnLockPtr(var);
}
// -----------------------------------------------


JsVar *jspeFactor(JsExecInfo *execInfo, JsExecFlags execute) {
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
#if TODO
    if (execInfo->lex->tk==LEX_ID) {
        JsVar *a = execute ? findInScopes(execInfo->lex->tkStr) : new JsVar(new CScriptVar());
        //printf("0x%08X for %s at %s\n", (unsigned int)a, execInfo->lex->tkStr.c_str(), l->getPosition().c_str());
        /* The parent if we're executing a method call */
        CScriptVar *parent = 0;

        if (execute && !a) {
          /* Variable doesn't exist! JavaScript says we should create it
           * (we won't add it here. This is done in the assignment operator)*/
          a = new JsVar(new CScriptVar(), execInfo->lex->tkStr);
        }
        JSP_MATCH(LEX_ID);
        while (execInfo->lex->tk=='(' || execInfo->lex->tk=='.' || execInfo->lex->tk=='[') {
            if (execInfo->lex->tk=='(') { // ------------------------------------- Function Call
                a = functionCall(execute, a, parent);
            } else if (execInfo->lex->tk == '.') { // ------------------------------------- Record Access
                JSP_MATCH('.');
                if (execInfo, execute) {
                  const string &name = execInfo->lex->tkStr;
                  JsVar *child = a->var->findChild(name);
                  if (!child) child = findInParentClasses(a->var, name);
                  if (!child) {
                    /* if we haven't found this defined yet, use the built-in
                       'length' properly */
                    if (a->var->isArray() && name == "length") {
                      int l = a->var->getArrayLength();
                      child = new JsVar(new CScriptVar(l));
                    } else if (a->var->isString() && name == "length") {
                      int l = a->var->getString().size();
                      child = new JsVar(new CScriptVar(l));
                    } else {
                      child = a->var->addChild(name);
                    }
                  }
                  parent = a->var;
                  a = child;
                }
                JSP_MATCH(LEX_ID);
            } else if (execInfo->lex->tk == '[') { // ------------------------------------- Array Access
                JSP_MATCH('[');
                JsVar *index = base(execInfo, execute);
                JSP_MATCH(']');
                if (execInfo, execute) {
                  JsVar *child = a->var->findChildOrCreate(index->var->getString());
                  parent = a->var;
                  a = child;
                }
                jspClean(index);
            } else ASSERT(0);
        }
        return a;
    }
#endif
    if (execInfo->lex->tk==LEX_INT) {
        long v = atol(jslGetTokenValueAsString(execInfo->lex));
        JSP_MATCH(LEX_INT);
        return jsvNewFromInteger(v);
    }
    if (execInfo->lex->tk==LEX_FLOAT) {
        double v = atof(jslGetTokenValueAsString(execInfo->lex));
        JSP_MATCH(LEX_INT);
        return jsvNewFromDouble(v);
    }
    if (execInfo->lex->tk==LEX_STR) {
        JsVar *a = jsvNewFromString(jslGetTokenValueAsString(execInfo->lex));
        JSP_MATCH(LEX_STR);
        return a;
    }
#if TODO
    if (execInfo->lex->tk=='{') {
        CScriptVar *contents = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
        /* JSON-style object definition */
        JSP_MATCH('{');
        while (execInfo->lex->tk != '}') {
          string id = execInfo->lex->tkStr;
          // we only allow strings or IDs on the left hand side of an initialisation
          if (execInfo->lex->tk==LEX_STR) JSP_MATCH(LEX_STR);
          else JSP_MATCH(LEX_ID);
          JSP_MATCH(':');
          if (execInfo, execute) {
            JsVar *a = base(execInfo, execute);
            contents->addChild(id, a->var);
            jspClean(a);
          }
          // no need to clean here, as it will definitely be used
          if (execInfo->lex->tk != '}') JSP_MATCH(',');
        }

        JSP_MATCH('}');
        return new JsVar(contents);
    }
    if (execInfo->lex->tk=='[') {
        CScriptVar *contents = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_ARRAY);
        /* JSON-style array */
        JSP_MATCH('[');
        int idx = 0;
        while (execInfo->lex->tk != ']') {
          if (execInfo, execute) {
            char idx_str[16]; // big enough for 2^32
            sprintf_s(idx_str, sizeof(idx_str), "%d",idx);

            JsVar *a = base(execInfo, execute);
            contents->addChild(idx_str, a->var);
            jspClean(a);
          }
          // no need to clean here, as it will definitely be used
          if (execInfo->lex->tk != ']') JSP_MATCH(',');
          idx++;
        }
        JSP_MATCH(']');
        return new JsVar(contents);
    }
    if (execInfo->lex->tk==LEX_R_FUNCTION) {
      JsVar *funcVar = parseFunctionDefinition();
        if (funcVar->name != TINYJS_TEMP_NAME)
          TRACE("Functions not defined at statement-level are not meant to have a name");
        return funcVar;
    }
    if (execInfo->lex->tk==LEX_R_NEW) {
      // new -> create a new object
      JSP_MATCH(LEX_R_NEW);
      const string &className = execInfo->lex->tkStr;
      if (execInfo, execute) {
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

JsVar *jspeUnary(JsExecInfo *execInfo, JsExecFlags execute) {
    JsVar *a;
    if (execInfo->lex->tk=='!') {
        JSP_MATCH('!'); // binary not
        a = jspeFactor(execInfo, execute);
        if (execute) {
            JsVar *zero = jsvLock(execInfo->parse->zeroInt);
            JsVar *res = jsvMathsOpPtr(a, zero, LEX_EQUAL);
            jsvUnLockPtr(zero);
            jspClean(a); a = res;
        }
    } else
        a = jspeFactor(execInfo, execute);
    return a;
}

JsVar *jspeTerm(JsExecInfo *execInfo, JsExecFlags execute) {
    JsVar *a = jspeUnary(execInfo, execute);
    while (execInfo->lex->tk=='*' || execInfo->lex->tk=='/' || execInfo->lex->tk=='%') {
        int op = execInfo->lex->tk;
        JSP_MATCH(execInfo->lex->tk);
        JsVar *b = jspeUnary(execInfo, execute);
        if (execute) {
          JsVar *res = jsvMathsOpPtr(a, b, op);
          jspClean(a); a = res;
        }
        jspClean(b);
    }
    return a;
}

JsVar *jspeExpression(JsExecInfo *execInfo, JsExecFlags execute) {
    bool negate = false;
    if (execInfo->lex->tk=='-') {
        JSP_MATCH('-');
        negate = true;
    }
    JsVar *a = jspeTerm(execInfo, execute);
    if (negate) {
      JsVar *zero = jsvLock(execInfo->parse->zeroInt);
      JsVar *res = jsvMathsOpPtr(zero, a, '-');
      jsvUnLockPtr(zero);
      jspClean(a); a = res;
    }

    while (execInfo->lex->tk=='+' || execInfo->lex->tk=='-' ||
        execInfo->lex->tk==LEX_PLUSPLUS || execInfo->lex->tk==LEX_MINUSMINUS) {
        int op = execInfo->lex->tk;
        JSP_MATCH(execInfo->lex->tk);
        if (op==LEX_PLUSPLUS || op==LEX_MINUSMINUS) {
            if (execute) {
                JsVar *one = jsvLock(execInfo->parse->oneInt);
                JsVar *res = jsvMathsOpPtr(a, one, op==LEX_PLUSPLUS ? '+' : '-');
                jsvUnLockPtr(one);
                JsVar *oldValue = jsvRef(a); // keep old value
                // in-place add/subtract
                jspReplaceWith(execInfo, a, res);
                jspClean(a); jspClean(res);
                // but then use the old value
                a = oldValue;
            }
        } else {
            JsVar *b = jspeTerm(execInfo, execute);
            if (execute) {
                // not in-place, so just replace
              JsVar *res = jsvMathsOpPtr(a, b, op);
              jspClean(a); a = res;
            }
            jspClean(b);
        }
    }
    return a;
}

JsVar *jspeShift(JsExecInfo *execInfo, JsExecFlags execute) {
  JsVar *a = jspeExpression(execInfo, execute);
  if (execInfo->lex->tk==LEX_LSHIFT || execInfo->lex->tk==LEX_RSHIFT || execInfo->lex->tk==LEX_RSHIFTUNSIGNED) {
    int op = execInfo->lex->tk;
    JSP_MATCH(op);
    JsVar *b = jspeBase(execInfo, execute);
    jspClean(b);
    if (execute) {
      JsVar *res = jsvMathsOpPtr(a, b, op);
      jspClean(a); a = res;
    }
  }
  return a;
}

JsVar *jspeCondition(JsExecInfo *execInfo, JsExecFlags execute) {
    JsVar *a = jspeShift(execInfo, execute);
    JsVar *b;
    while (execInfo->lex->tk==LEX_EQUAL || execInfo->lex->tk==LEX_NEQUAL ||
           execInfo->lex->tk==LEX_TYPEEQUAL || execInfo->lex->tk==LEX_NTYPEEQUAL ||
           execInfo->lex->tk==LEX_LEQUAL || execInfo->lex->tk==LEX_GEQUAL ||
           execInfo->lex->tk=='<' || execInfo->lex->tk=='>') {
        int op = execInfo->lex->tk;
        JSP_MATCH(execInfo->lex->tk);
        b = jspeShift(execInfo, execute);
        if (execute) {
            JsVar *res = jsvMathsOpPtr(a, b, op);
            jspClean(a); a = res;
        }
        jspClean(b);
    }
    return a;
}

JsVar *jspeLogic(JsExecInfo *execInfo, JsExecFlags execute) {
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
            shortCircuit = !jsvGetBool(a);
            boolean = true;
        } else if (op==LEX_OROR) {
            op = '|';
            shortCircuit = jsvGetBool(a);
            boolean = true;
        }
        b = jspeCondition(execInfo, shortCircuit ? noexecute : execute);
        if (execute && !shortCircuit) {
            if (boolean) {
              JsVar *newa = jsvNewFromBool(jsvGetBool(a));
              JsVar *newb = jsvNewFromBool(jsvGetBool(b));
              jspClean(a); a = newa;
              jspClean(b); b = newb;
            }
            JsVar *res = jsvMathsOpPtr(a, b, op);
            jspClean(a); a = res;
        }
        jspClean(b);
    }
    return a;
}

JsVar *jspeTernary(JsExecInfo *execInfo, JsExecFlags execute) {
  JsVar *lhs = jspeLogic(execInfo, execute);
  JsExecFlags noexecute = EXEC_NO;
  if (execInfo->lex->tk=='?') {
    JSP_MATCH('?');
    if (!execute) {
      jspClean(lhs);
      jspClean(jspeBase(execInfo, noexecute));
      JSP_MATCH(':');
      jspClean(jspeBase(execInfo, noexecute));
    } else {
      bool first = jsvGetBool(lhs);
      jspClean(lhs);
      if (first) {
        lhs = jspeBase(execInfo, execute);
        JSP_MATCH(':');
        jspClean(jspeBase(execInfo, noexecute));
      } else {
        jspClean(jspeBase(execInfo, noexecute));
        JSP_MATCH(':');
        lhs = jspeBase(execInfo, execute);
      }
    }
  }

  return lhs;
}

JsVar *jspeBase(JsExecInfo *execInfo, JsExecFlags execute) {
    JsVar *lhs = jspeTernary(execInfo, execute);
    if (execInfo->lex->tk=='=' || execInfo->lex->tk==LEX_PLUSEQUAL || execInfo->lex->tk==LEX_MINUSEQUAL) {
        /* If we're assigning to this and we don't have a parent,
         * add it to the symbol table root as per JavaScript. */
        if (execute && !lhs->refs) {
          if (jsvIsName(lhs)/* && jsvGetStringLength(lhs)>0*/) {
            jsvAddName(execInfo->parse->root, lhs->this);
          } else // TODO: Why was this here? can it happen?
            jsWarnAt("Trying to assign to an un-named type\n", execInfo->lex, execInfo->lex->tokenLastEnd);
        }

        int op = execInfo->lex->tk;
        JSP_MATCH(execInfo->lex->tk);
        JsVar *rhs = jspeBase(execInfo, execute);
        if (execute) {
            if (op=='=') {
                jspReplaceWith(execInfo, lhs, rhs);
            } else if (op==LEX_PLUSEQUAL) {
                JsVar *res = jsvMathsOpPtr(lhs,rhs, '+');
                jspReplaceWith(execInfo, lhs, res);
                jspClean(res);
            } else if (op==LEX_MINUSEQUAL) {
                JsVar *res = jsvMathsOpPtr(lhs,rhs, '-');
                jspReplaceWith(execInfo, lhs, res);
                jspClean(res);
            } else assert(0);
        }
        jspClean(rhs);
    }
    return lhs;
}

// -----------------------------------------------------------------------------

void jspInit(JsParse *parse) {
  parse->root = jsvUnLockPtr(jsvRef(jsvNewWithFlags(SCRIPTVAR_OBJECT)));

  parse->zeroInt = jsvUnLockPtr(jsvRef(jsvNewFromInteger(0)));
  jsvAddNamedChild(parse->root, parse->zeroInt, "__ZERO");
  jsvUnRefRef(parse->zeroInt);
  parse->oneInt = jsvUnLockPtr(jsvRef(jsvNewFromInteger(1)));
  jsvAddNamedChild(parse->root, parse->oneInt, "__ONE");
  jsvUnRefRef(parse->oneInt);
  parse->stringClass = jsvUnLockPtr(jsvRef(jsvNewWithFlags(SCRIPTVAR_OBJECT)));
  jsvAddNamedChild(parse->root, parse->stringClass, "String");
  jsvUnRefRef(parse->stringClass);
  parse->objectClass = jsvUnLockPtr(jsvRef(jsvNewWithFlags(SCRIPTVAR_OBJECT)));
  jsvAddNamedChild(parse->root, parse->objectClass, "Object");
  jsvUnRefRef(parse->objectClass);
  parse->arrayClass = jsvUnLockPtr(jsvRef(jsvNewWithFlags(SCRIPTVAR_OBJECT)));
  jsvAddNamedChild(parse->root, parse->arrayClass, "Array");
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
  execInfo.parse = parse;
  execInfo.lex = &lex;
  JsExecFlags execute = EXEC_YES;

  JsVar *v = jspeBase(&execInfo, execute);

  jslKill(&lex);

  return v;
}
