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
 * Recursive descent JIT
 * ----------------------------------------------------------------------------
 */

//#ifdef ESPR_JIT

#include "jsjit.h"
#include "jsjitc.h"

#define JSP_ASSERT_MATCH(TOKEN) { assert(lex->tk==(TOKEN));jslGetNextToken(); } // Match where if we have the wrong token, it's an internal error
#define JSP_MATCH(TOKEN) if (!jslMatch((TOKEN))) return; // Match where the user could have given us the wrong token
#define JSJ_PARSING true

// ----------------------------------------------------------------------------
void jsjUnaryExpression();
void jsjStatement();
// ----------------------------------------------------------------------------

void jsjPopAsVar(int reg) {
  JsjValueType varType = jsjcPop(reg);
  if (varType==JSJVT_JSVAR) return;
  if (varType==JSJVT_INT) {
    if (reg) jsjcMov(0, reg);
    jsjcCall(jsvNewFromInteger); // FIXME: what about clobbering r1-r3?
    if (reg) jsjcMov(reg, 0);
    return;
  }
  assert(0);
}

void jsjPopAndUnLock() {
  jsjPopAsVar(0); // a -> r0
  // optimisation: if item on stack is NOT a variable, no need to covert+unlock!
  jsjcCall(jsvUnLock); // we're throwing this away now - unlock
}

void jsjFactor() {
  if (lex->tk==LEX_ID) {
    JsVar *a = jslGetTokenValueAsVar();
    jsjcLiteralString(0, a, true); // null terminated
    jsvUnLock(a);
    JSP_ASSERT_MATCH(LEX_ID);
    jsjcCall(jspGetNamedVariable);
    jsjcPush(0, JSJVT_JSVAR); // We're pushing a NAME here
  } else if (lex->tk==LEX_INT) {
    int64_t v = stringToInt(jslGetTokenValueAsString());
    JSP_ASSERT_MATCH(LEX_INT);
    if (v>>32) {
      jsjcLiteral64(0, (uint64_t)v);
      jsjcCall(jsvNewFromLongInteger);
    } else {
      jsjcLiteral32(0, (uint32_t)v);
      jsjcCall(jsvNewFromInteger);
    }
    jsjcPush(0, JSJVT_JSVAR); // FIXME - push an int and convert later
  } else if (lex->tk==LEX_FLOAT) {
    double v = stringToFloat(jslGetTokenValueAsString());
    JSP_ASSERT_MATCH(LEX_FLOAT);
    jsjcLiteral64(0, *((uint64_t*)&v));
    jsjcCall(jsvNewFromFloat);
    jsjcPush(0, JSJVT_JSVAR);
  } else if (lex->tk=='(') {
    JSP_ASSERT_MATCH('(');
    // Just parse a normal expression (which can include commas)
    //JsVar *a = jsjExpression(); // FIXME
    JSP_MATCH(')');
  } else if (lex->tk==LEX_R_TRUE || lex->tk==LEX_R_FALSE) {
    jsjcLiteral32(0, lex->tk==LEX_R_TRUE);
    JSP_ASSERT_MATCH(lex->tk);
    jsjcCall(jsvNewFromBool); 
    jsjcPush(0, JSJVT_JSVAR);
  } else if (lex->tk==LEX_R_NULL) {
    JSP_ASSERT_MATCH(LEX_R_NULL);
    jsjcLiteral32(0, JSV_NULL);
    jsjcCall(jsvNewWithFlags);
    jsjcPush(0, JSJVT_JSVAR);
  } else if (lex->tk==LEX_R_UNDEFINED) {
    JSP_ASSERT_MATCH(LEX_R_UNDEFINED);
    jsjcLiteral32(0, 0);
    jsjcPush(0, JSJVT_JSVAR);
  } else if (lex->tk==LEX_STR) {
    JsVar *a = jslGetTokenValueAsVar();
    JSP_ASSERT_MATCH(LEX_STR);
    int len = jsjcLiteralString(1, a, false);
    jsvUnLock(a);
    jsjcLiteral32(0, len);
    jsjcCall(jsvNewStringOfLength);
    jsjcPush(0, JSJVT_JSVAR);
  }/* else if (lex->tk=='{') {
    if (!jspCheckStackPosition()) return 0;
    return jsjFactorObject();
  } else if (lex->tk=='[') {
    if (!jspCheckStackPosition()) return 0;
    return jsjFactorArray();
  } else if (lex->tk==LEX_R_FUNCTION) {
    if (!jspCheckStackPosition()) return 0;
    JSP_ASSERT_MATCH(LEX_R_FUNCTION);
    return jsjFunctionDefinition(true);
  } else if (lex->tk==LEX_R_THIS) {
    JSP_ASSERT_MATCH(LEX_R_THIS);
    return jsvLockAgain( execInfo.thisVar ? execInfo.thisVar : execInfo.root );
  } else if (lex->tk==LEX_R_DELETE) {
    if (!jspCheckStackPosition()) return 0;
    return jsjFactorDelete();
  } else if (lex->tk==LEX_R_TYPEOF) {
    if (!jspCheckStackPosition()) return 0;
    return jsjFactorTypeOf();
  } */else if (lex->tk==LEX_R_VOID) {
    JSP_ASSERT_MATCH(LEX_R_VOID);
    jsjUnaryExpression();
    jsjcCall(jsvUnLock);
    jsjcLiteral32(0, 0);
    jsjcPush(0, JSJVT_JSVAR);
  } else JSP_MATCH(LEX_EOF);
}

void jsjFactorFunctionCall() {
  jsjFactor();
  // jsjFactorMember(); // FIXME
}

void __jsjPostfixExpression() {
  while (lex->tk==LEX_PLUSPLUS || lex->tk==LEX_MINUSMINUS) {
    int op = lex->tk;
    JSP_ASSERT_MATCH(op);

      /*JsVar *one = jsvNewFromInteger(1);
      JsVar *oldValue = jsvAsNumberAndUnLock(jsvSkipName(a)); // keep the old value (but convert to number)
      JsVar *res = jsvMathsOpSkipNames(oldValue, one, op==LEX_PLUSPLUS ? '+' : '-');
      jsvUnLock(one);

      // in-place add/subtract
      jsvReplaceWith(a, res);
      jsvUnLock(res);
      // but then use the old value
      jsvUnLock(a);
      a = oldValue;*/
  }
}

void jsjPostfixExpression() {
  if (lex->tk==LEX_PLUSPLUS || lex->tk==LEX_MINUSMINUS) {
    int op = lex->tk;
    JSP_ASSERT_MATCH(op);
    /*
    a = jspePostfixExpression();
    if (JSP_SHOULD_EXECUTE) {
      JsVar *one = jsvNewFromInteger(1);
      JsVar *res = jsvMathsOpSkipNames(a, one, op==LEX_PLUSPLUS ? '+' : '-');
      jsvUnLock(one);
      // in-place add/subtract
      jsvReplaceWith(a, res);
      jsvUnLock(res);
    }
     *//*
    jsjPostfixExpression();
    jsjcLiteral32(0, 1);
    jsjcCall(jsvNewFromInteger);
    jsjcMov(1, 0);
    jsjcMov(4, 0); // preserve 'one' for call
    jsjcPop(1);
    jsjcMov(5, 0); // preserve 'a' for call
    jsjcLiteral32(3, op==LEX_PLUSPLUS ? '+' : '-');
    jsjcCall(jsvMathsOpSkipNames);
    jsjcMov(0, 4); // one -> r0
    jsjcCall(jsvUnLock); // jsvUnLock(one)
    jsjcMov(1, 0); // res -> r1
    jsjcMov(4, 0); // res -> r0
    jsjcMov(0, 5); // a -> r0
    jsjcCall(jsvReplaceWith);
    jsjcMov(0, 4); // res -> r0
    jsjcCall(jsvUnLock); // jsvUnLock(res)
    */
  } else
    jsjFactorFunctionCall();
   __jsjPostfixExpression();
}

void jsjUnaryExpression() {
  if (lex->tk=='!' || lex->tk=='~' || lex->tk=='-' || lex->tk=='+') {
    short tk = lex->tk;
    JSP_ASSERT_MATCH(tk);
/*    if (tk=='!') { // logical not
      return jsvNewFromBool(!jsvGetBoolAndUnLock(jsvSkipNameAndUnLock(jsjUnaryExpression())));
    } else if (tk=='~') { // bitwise not
      return jsvNewFromInteger(~jsvGetIntegerAndUnLock(jsvSkipNameAndUnLock(jsjUnaryExpression())));
    } else if (tk=='-') { // unary minus
      return jsvNegateAndUnLock(jsjUnaryExpression()); // names already skipped
    }  else if (tk=='+') { // unary plus (convert to number)
      JsVar *v = jsvSkipNameAndUnLock(jsjUnaryExpression());
      JsVar *r = jsvAsNumber(v); // names already skipped
      jsvUnLock(v);
      return r;
    }*/
    assert(0);
  } else
    jsjPostfixExpression();
}

// Get the precedence of a BinaryExpression - or return 0 if not one
unsigned int jsjGetBinaryExpressionPrecedence(int op) {
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

void __jsjBinaryExpression(unsigned int lastPrecedence) {
  /* This one's a bit strange. Basically all the ops have their own precedence, it's not
   * like & and | share the same precedence. We don't want to recurse for each one,
   * so instead we do this.
   *
   * We deal with an expression in recursion ONLY if it's of higher precedence
   * than the current one, otherwise we stick in the while loop.
   */
  unsigned int precedence = jsjGetBinaryExpressionPrecedence(lex->tk);
  while (precedence && precedence>lastPrecedence) {
    int op = lex->tk;
    JSP_ASSERT_MATCH(op);

    // if we have short-circuit ops, then if we know the outcome
    // we don't bother to execute the other op. Even if not
    // we need to tell mathsOp it's an & or |
 /*   if (op==LEX_ANDAND || op==LEX_OROR) {
      bool aValue = jsvGetBoolAndUnLock(jsvSkipName(a));
      if ((!aValue && op==LEX_ANDAND) ||
          (aValue && op==LEX_OROR)) {
        // use first argument (A)
        JSP_SAVE_EXECUTE();
        jspSetNoExecute();
        jsvUnLock(__jsjBinaryExpression(jsjUnaryExpression(),precedence));
        JSP_RESTORE_EXECUTE();
      } else {
        // use second argument (B)
        jsvUnLock(a);
        a = __jsjBinaryExpression(jsjUnaryExpression(),precedence);
      }
    } else*/ { // else it's a more 'normal' logical expression - just use Maths
      jsjUnaryExpression();
      __jsjBinaryExpression(precedence);
     /*
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
      } else */{  // --------------------------------------------- NORMAL
        jsjPopAsVar(1); // b -> r1
        jsjcMov(5, 0); // b -> r5 (for unlock later)
        jsjPopAsVar(0); // a -> r0
        jsjcMov(4, 0); // a -> r4 (for unlock later)
        jsjcLiteral32(2, op);
        jsjcCall(jsvMathsOpSkipNames);
        jsjcPush(0, JSJVT_JSVAR); // push result
        jsjcMov(1, 5); // b -> r1
        jsjcMov(0, 4); // a -> r0
        jsjcCall(jsvUnLock2);
      }
    }
    precedence = jsjGetBinaryExpressionPrecedence(lex->tk);
  }
}

void jsjBinaryExpression() {
  jsjUnaryExpression();
  __jsjBinaryExpression(0);
}

// ',' is allowed to add multiple expressions, this is not allowed in jspeAssignmentExpression
void jsjExpression() {
  while (JSJ_PARSING) {
    jsjBinaryExpression(); // FIXME should be jsjAssignmentExpression();
    if (lex->tk!=',') return;
    // if we get a comma, we just unlock this data and parse the next bit...
    jsjPopAndUnLock();
    JSP_ASSERT_MATCH(',');
  }
}

void jsjBlockNoBrackets() {
  while (lex->tk && lex->tk!='}' && JSJ_PARSING) {
    jsjStatement();
  }
  return;
}

void jsjBlock() {
  JSP_MATCH('{');
  jsjBlockNoBrackets();
  JSP_MATCH('}');
}

void jsjStatement() {
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
    jsjExpression();
    jsjPopAndUnLock();
  } else if (lex->tk=='{') {
    /* A block of code */
    jsjBlock();
  } else if (lex->tk==';') {
    JSP_ASSERT_MATCH(';');/* Empty statement - to allow things like ;;; */
/*} else if (lex->tk==LEX_R_VAR ||
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
    return jspeStatementTry();*/
  } else if (lex->tk==LEX_R_RETURN) {
    JSP_ASSERT_MATCH(LEX_R_RETURN);
    if (lex->tk != ';' && lex->tk != '}') {
      jsjExpression();
      jsjPopAsVar(0); // a -> r0
      jsjcCall(jsvSkipNameAndUnLock); // we only want the value, so skip the name if there was one
    } else {
      jsjcLiteral32(0, 0);
    }
    jsjcPopAllAndReturn();
/*} else if (lex->tk==LEX_R_THROW) {
  } else if (lex->tk==LEX_R_FUNCTION) {
  } else if (lex->tk==LEX_R_CONTINUE) {
    JSP_ASSERT_MATCH(LEX_R_CONTINUE);
  } else if (lex->tk==LEX_R_BREAK) {
    JSP_ASSERT_MATCH(LEX_R_BREAK);
    if (JSP_SHOULD_EXECUTE) {
  } else if (lex->tk==LEX_R_SWITCH) {
    return jspeStatementSwitch();*/
  } else JSP_MATCH(LEX_EOF);
}

void jsjBlockOrStatement() {
  if (lex->tk=='{') {
    jsjBlock();
  } else {
    jsjStatement();
    if (lex->tk==';') JSP_ASSERT_MATCH(';');
    // FIXME pop?
  }
}

JsVar *jsjParseFunction() {
  jsjcStart();
  // FIXME: I guess we need to create a function execution scope and unpack parameters?
  // Maybe we could use jspeFunctionCall to do all this for us (not creating a native function but a 'normal' one
  // with native function code...
  jsjcPushAll(); // Function start
  jsjBlockNoBrackets();
  // optimisation: if the last statement was a return, no need for this
  // Return 'undefined' from function if no other return statement
  jsjcLiteral32(0, 0);
  jsjcPopAllAndReturn();
  return jsjcStop();
}

JsVar *jsjEvaluateVar(JsVar *str) {
  JsLex lex;
  assert(jsvIsString(str));
  JsLex *oldLex = jslSetLex(&lex);
  jslInit(str);
  jsjcStart();
  jsjcPushAll();
  jsjExpression();
  jsjPopAsVar(0); // a -> r0
  jsjcCall(jsvSkipNameAndUnLock); // we only want the value, so skip the name if there was one
  jsjcPopAllAndReturn();
  JsVar *v = jsjcStop();
  jslKill();
  jslSetLex(oldLex);
  return v;
}

JsVar *jsjEvaluate(const char *str) {
  JsVar *evCode;
  evCode = jsvNewNativeString((char*)str, strlen(str));
  if (!evCode) return 0;
  JsVar *v = 0;
  if (!jsvIsMemoryFull())
    v = jsjEvaluateVar(evCode);
  jsvUnLock(evCode);

  return v;
}

//#endif /*#ifdef ESPR_JIT*/
