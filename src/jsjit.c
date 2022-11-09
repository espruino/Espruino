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

#ifdef ESPR_JIT

#include "jsjit.h"
#include "jsjitc.h"
#include "jsinteractive.h"

#define JSP_ASSERT_MATCH(TOKEN) { assert(lex->tk==(TOKEN));jslGetNextToken(); } // Match where if we have the wrong token, it's an internal error
#define JSP_MATCH_WITH_RETURN(TOKEN, RETURN_VAL) if (!jslMatch((TOKEN))) return RETURN_VAL;
#define JSP_MATCH(TOKEN) if (!jslMatch((TOKEN))) return; // Match where the user could have given us the wrong token
#define JSJ_PARSING (!(execInfo.execute&EXEC_EXCEPTION))

// ----------------------------------------------------------------------------
void jsjUnaryExpression();
void jsjAssignmentExpression();
void jsjExpression();
void jsjStatement();
void jsjBlockOrStatement();
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

void jsjPopAsBool(int reg) {
  // FIXME handle int/bool differently?
  jsjPopAsVar(0);
  jsjcCall(jsvGetBoolAndUnLock); // optimisation: we should know if we have a var or a name here, so can skip jsvSkipNameAndUnLock sometimes
  if (reg != 0) jsjcMov(reg, 0);
}

void jsjPopAndUnLock() {
  jsjPopAsVar(0); // a -> r0
  // optimisation: if item on stack is NOT a variable, no need to covert+unlock!
  jsjcCall(jsvUnLock); // we're throwing this away now - unlock
}

void jsjPopNoName(int reg) {
  jsjPopAsVar(0); // a -> r0
  jsjcCall(jsvSkipNameAndUnLock); // optimisation: we should know if we have a var or a name here, so can skip jsvSkipNameAndUnLock sometimes
  if (reg != 0) jsjcMov(reg, 0);
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
    jsjExpression();
    // FIXME: Arrow functions??
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

/// Look up 'parent.a[index]'. Utility function called from JIT code
uint64_t _jsjxObjectLookup(JsVar *index, JsVar *parent, JsVar *a) {
  JsVar *resultParent = jsvSkipNameWithParent(a,true,parent);
  jsvUnLock2(a, parent);
  JsVar *resultA = 0;
  if (resultParent)
    resultA = jspGetVarNamedField(resultParent, index, true);
  if (!resultA) {
    if (jsvHasChildren(resultParent)) {
      // if no child found, create a pointer to where it could be
      // as we don't want to allocate it until it's written
      resultA = jsvCreateNewChild(resultParent, index, 0);
    } else {
      jsExceptionHere(JSET_ERROR, "Field or method %q does not already exist, and can't create it on %t", index, resultParent);
    }
  }
  jsvUnLock(index);
  return ((uint64_t)(size_t)resultA) | (((uint64_t)(size_t)resultParent)<<32);
}

// Like jspeFunctionCall but we unlock ALL the vars supplied
NO_INLINE JsVar *_jsjxFunctionCallAndUnLock(JsVar *functionName, JsVar *thisArg, bool isParsing, int argCount, JsVar **argPtr) {
  JsVar *function = jsvSkipName(functionName);
  JsVar *r = jspeFunctionCall(function, functionName, thisArg, isParsing, argCount, argPtr);
  jsvUnLockMany(argCount, argPtr);
  jsvUnLock3(function, functionName, thisArg);
  return r;
}

// Call jsvReplaceWithOrAddToRoot but unlock the second argument
NO_INLINE void _jsxReplaceWithOrAddToRootUnlockSrc(JsVar *dst, JsVar *src) {
  jsvReplaceWithOrAddToRoot(dst, src);
  jsvUnLock(src);
}

// Handle postfix inc and dec nicely - without us having to add a bunch of extra code. var is unlocked automatically, the result is returned
NO_INLINE JsVar *_jsxPostfixIncDec(JsVar *var, char op) {
  JsVar *one = jsvNewFromInteger(1);
  JsVar *oldValue = jsvAsNumberAndUnLock(jsvSkipName(var)); // keep the old value (but convert to number)
  JsVar *res = jsvMathsOpSkipNames(var, one, op);
  jsvReplaceWith(var, res);
  jsvUnLock3(var,res,one);
  return oldValue; // return the number from before we incremented
}

// Handle prefix inc and dec nicely - without us having to add a bunch of extra code
NO_INLINE JsVar *_jsxPrefixIncDec(JsVar *var, char op) {
  JsVar *one = jsvNewFromInteger(1);
  JsVar *res = jsvMathsOpSkipNames(var, one, op);
  jsvReplaceWith(var, res);
  jsvUnLock2(res, one);
  return var; // return the number from before we incremented
}

NO_INLINE JsVar *_jsxMathsOpSkipNamesAndUnLock(JsVar *a, JsVar *b, int op) {
  JsVar *r = jsvMathsOpSkipNames(a,b,op);
  jsvUnLock2(a,b);
  return r;
}

// Add a variable to the current scope (eg VAR statement)
NO_INLINE void _jsxAddVar(const char *name, bool isConstant, JsVar *initialValue) {
  JsVar *scope = jspeiGetTopScope();
  JsVar *a = jsvFindChildFromString(scope, name, true);
  jsvUnLock(scope);
  if (!a) return; // no memory
  if (isConstant)
    a->flags |= JSV_CONSTANT;
  if (initialValue) {
    initialValue = jsvSkipNameAndUnLock(initialValue);
    jsvReplaceWith(a, initialValue);
  }
  jsvUnLock2(a,initialValue);
}

// Parse ./[] - return true if the parent of the current item is currently on the stack
bool jsjFactorMember() {
  bool parentOnStack = false;
  while ((lex->tk=='.' || lex->tk=='[') && JSJ_PARSING) {
    if (lex->tk == '.') { // ------------------------------------- Record Access
      JSP_ASSERT_MATCH('.');
      if (jslIsIDOrReservedWord()) {
        JsVar *a = jslGetTokenValueAsVar();
        jsjcLiteralString(0, a, true); // null terminated
        jsvUnLock(a);
        // r0 = string pointer
        jslGetNextToken(); // skip over current token (we checked above that it was an ID or reserved word)
        jsjcCall(jsvNewFromString);
        // r0 = index (as JsVar)
      } else {
        // incorrect token - force a match fail by asking for an ID
        JSP_MATCH_WITH_RETURN(LEX_ID, false); // if we fail we're stopping compilation anyway
      }
    } else if (lex->tk == '[') { // ------------------------------------- Array Access
      JSP_ASSERT_MATCH('[');
      jsjAssignmentExpression();
      jsjcPop(0);
      jsjcCall(jsvAsArrayIndexAndUnLock);
      JSP_MATCH_WITH_RETURN(']', false); // if we fail we're stopping compilation anyway
      // r0 = index
    } else {
      assert(0);
    }
    // r0 currently = index
    if (parentOnStack) jsjcPop(1); // r1 = parent
    else jsjcLiteral32(1, 0);
    jsjcPop(2); // r2 = the variable itself
    jsjcCall(_jsjxObjectLookup); // (a,parent) = _jsjxObjectLookup(index, parent, a)
    jsjcPush(0, JSJVT_JSVAR); // a
    jsjcPush(1, JSJVT_JSVAR); // parent
    parentOnStack = true;
  }
  return parentOnStack;
}

void jsjFactorFunctionCall() {
  jsjFactor();
  bool parentOnStack = jsjFactorMember(); // FIXME we need to call this and also somehow remember 'parent'
  // FIXME: what about 'new'?

  while (lex->tk=='(' /*|| (isConstructor && JSP_SHOULD_EXECUTE))*/ && JSJ_PARSING) {
    if (parentOnStack) {
      DEBUG_JIT("; FUNCTION CALL r6 = 'this'\n");
      jsjcPop(6); // r6 = this/parent
      parentOnStack = false;
    } else {
      jsjcLiteral32(6, 0); // no parent
    }
    DEBUG_JIT("; FUNCTION CALL r4 = funcName\n");
    jsjcPop(4); // r4 = funcName
    DEBUG_JIT("; FUNCTION CALL arguments\n");
    /* PARSE OUR ARGUMENTS
     * Push each new argument onto the stack (it grows down)
     * Args are in the wrong order, so we emit code to swap around the args in the array
     * At the end, SP = what we need as 'argPtr' for jspeFunctionCall

     optimisation: If we knew how many args we had ahead of time, we could subtract that
     from the stack pointer, save it, and then instead of pushing onto the stack we could
     just write direct to the correct address.
     */
    int argCount = 0;
    JSP_MATCH('(');
    while (JSJ_PARSING && lex->tk!=')' && lex->tk!=LEX_EOF) {
      argCount++;
      jsjAssignmentExpression();
      jsjPopNoName(0);
      jsjcPush(0, JSJVT_JSVAR); // push argument to stack
      if (lex->tk!=')') JSP_MATCH(',');
    }
    JSP_MATCH(')');
    // r4=funcName, args on the stack
    jsjcMov(7, JSJAR_SP); // r7 = argPtr
    jsjcPush(7, JSJVT_INT); // argPtr (5th arg - on stack)
    // Args are in the wrong order - we have to swap them around if we have >1!
    if (argCount>1) {
      DEBUG_JIT("; FUNCTION CALL reverse arguments\n");
      for (int i=0;i<argCount/2;i++) {
        int a1 = i*4;
        int a2 = (argCount-(i+1))*4;
        jsjcLoadImm(0, 7, a1); // r0 = memory[argPtr+a1]
        jsjcLoadImm(1, 7, a2); // ...
        jsjcStoreImm(0, 7, a2);
        jsjcStoreImm(1, 7, a1);
      }
    }
    DEBUG_JIT("; FUNCTION CALL jspeFunctionCall\n");
    // First arg
    jsjcMov(0, 4); // r0 = funcName
    // for constructors we'd have to do something special here
    jsjcMov(1, 6); // parent (from r6)
    jsjcLiteral32(2, 0); // isParsing = false
    jsjcLiteral32(3, argCount); // argCount 4th arg
    jsjcCall(_jsjxFunctionCallAndUnLock); // a = _jsjxFunctionCallAndUnLock(funcName, thisArg/parent, isParsing, argCount, argPtr[on stack]);
    DEBUG_JIT("; FUNCTION CALL cleanup stack\n");
    jsjcAddSP(4*(1+argCount)); // pop off argPtr + all the arguments
    jsjcPush(0, JSJVT_JSVAR); // push return value from jspeFunctionCall
    DEBUG_JIT("; FUNCTION CALL end\n");
    // 'parent', 'funcName' and all args are unlocked by _jsjxFunctionCallAndUnLock
  }
  if (parentOnStack) {
    jsjcPop(0); // remove parent from the stack and unlock it
    jsjcCall(jsvUnLock);
  }
}

void __jsjPostfixExpression() {
  while (lex->tk==LEX_PLUSPLUS || lex->tk==LEX_MINUSMINUS) {
    int op = lex->tk; // POSFIX expression =>  i++, i--
    JSP_ASSERT_MATCH(op);
    jsjPopAsVar(0); // old value -> r0
    jsjcLiteral32(1, op==LEX_PLUSPLUS ? '+' : '-'); // add the operation
    jsjcCall(_jsxPostfixIncDec); // JsVar *_jsxPostfixIncDec(JsVar *var, char op)
    jsjcPush(0, JSJVT_JSVAR); // push result (value BEFORE we inc/dec)
  }
}

void jsjPostfixExpression() {
  if (lex->tk==LEX_PLUSPLUS || lex->tk==LEX_MINUSMINUS) {
    // PREFIX expression =>  ++i, --i
    int op = lex->tk;
    JSP_ASSERT_MATCH(op);
    jsjPostfixExpression(); // recurse to get our var...
    jsjPopAsVar(0); // old value -> r0
    jsjcLiteral32(1, op==LEX_PLUSPLUS ? '+' : '-'); // add the operation
    jsjcCall(_jsxPrefixIncDec); // JsVar *_jsxPrefixIncDec(JsVar *var, char op)
    jsjcPush(0, JSJVT_JSVAR); // push result (value AFTER we inc/dec)
  } else
    jsjFactorFunctionCall();
  __jsjPostfixExpression();
}

void jsjUnaryExpression() {
  if (lex->tk=='!' || lex->tk=='~' || lex->tk=='-' || lex->tk=='+') {
    int op = lex->tk;
    JSP_ASSERT_MATCH(op);
    jsjUnaryExpression();
    jsjPopNoName(0); // value -> r0 (but ensure it's not a name)
    if (op=='!') { // logical not
      jsjcCall(jsvGetBoolAndUnLock);
      jsjcMVN(0,0); // ~
      jsjcLiteral32(1, 1);
      jsjcAND(0,1); // &1   -> convert it back to a boolean
      jsjcCall(jsvNewFromBool);
    } else if (op=='~') { // bitwise not
      jsjcCall(jsvGetIntegerAndUnLock);
      jsjcMVN(0,0); // ~
      jsjcCall(jsvNewFromInteger);
    } else if (op=='-') { // unary minus
      jsjcCall(jsvNegateAndUnLock);
    } else if (op=='+') { // unary plus (convert to number)
      jsjcCall(jsvAsNumberAndUnLock);
    } else assert(0);
    jsjcPush(0, JSJVT_JSVAR);
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
        jsjPopAsVar(0); // a -> r0
        jsjcLiteral32(2, op);
        jsjcCall(_jsxMathsOpSkipNamesAndUnLock); // unlocks arguments
        jsjcPush(0, JSJVT_JSVAR); // push result
      }
    }
    precedence = jsjGetBinaryExpressionPrecedence(lex->tk);
  }
}

void jsjBinaryExpression() {
  jsjUnaryExpression();
  __jsjBinaryExpression(0);
}

void jsjConditionalExpression() {
  // FIXME return __jsjConditionalExpression(jsjBinaryExpression());
  jsjBinaryExpression();
}

NO_INLINE void jsjAssignmentExpression() {
  // parse LHS
  jsjConditionalExpression();
  if (!JSJ_PARSING) return;
  if (lex->tk=='='/* || lex->tk==LEX_PLUSEQUAL || lex->tk==LEX_MINUSEQUAL ||
      lex->tk==LEX_MULEQUAL || lex->tk==LEX_DIVEQUAL || lex->tk==LEX_MODEQUAL ||
      lex->tk==LEX_ANDEQUAL || lex->tk==LEX_OREQUAL ||
      lex->tk==LEX_XOREQUAL || lex->tk==LEX_RSHIFTEQUAL ||
      lex->tk==LEX_LSHIFTEQUAL || lex->tk==LEX_RSHIFTUNSIGNEDEQUAL*/) {

    int op = lex->tk;
    JSP_ASSERT_MATCH(op);
    jsjAssignmentExpression();
    jsjPopNoName(1); // ensure we get rid of any references on the RHS
    jsjcPop(0); // pop LHS
    jsjcPush(0, JSJVT_JSVAR); // push LHS back on as this is our result value
    //jsjcPush(1, JSJVT_JSVAR); // push RHS back on, so we can pop it off and unlock after jsvReplaceWithOrAddToRoot


    if (op=='=') {
      // this is like jsvReplaceWithOrAddToRoot but it unlocks the RHS for us
      jsjcCall(_jsxReplaceWithOrAddToRootUnlockSrc); // void _jsxReplaceWithOrAddToRootUnlockSrc(JsVar *dst, JsVar *src)
    } else {
/*
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
          // A special case for string += where this is the only use of the string
          // and we're not appending to ourselves. In this case we can do a
          // simple append (rather than clone + append)
          JsVar *str = jsvAsString(rhs);
          jsvAppendStringVarComplete(currentValue, str);
          jsvUnLock(str);
          op = 0;
        }
        jsvUnLock(currentValue);
      }
      if (op) {
        // Fallback which does a proper add
        JsVar *res = jsvMathsOpSkipNames(lhs,rhs,op);
        jsvReplaceWith(lhs, res);
        jsvUnLock(res);
      }
*/
    }
  }
}

// ',' is allowed to add multiple expressions, this is not allowed in jsjAssignmentExpression
void jsjExpression() {
  while (JSJ_PARSING) {
    jsjAssignmentExpression();
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

void jsjStatementVar() {
  assert(lex->tk==LEX_R_VAR || lex->tk==LEX_R_LET || lex->tk==LEX_R_CONST);
  // FIXME: Ignore block scoping for now
  bool isConstant = lex->tk==LEX_R_CONST;
  jslGetNextToken();
  bool hasComma = true; // for first time in loop
  while (hasComma && lex->tk == LEX_ID && JSJ_PARSING) {
    JsVar *a = 0;
    // Get the name
    JsVar *name = jslGetTokenValueAsVar();
    JSP_ASSERT_MATCH(LEX_ID);
    bool hasInitialiser = lex->tk == '=';
    if (hasInitialiser) { // sort out initialiser
      JSP_ASSERT_MATCH('=');
      jsjAssignmentExpression();

    }
    // _jsxAddVar(r0:name, r1:isConstant, r2:initialValue)
    jsjcLiteralString(0, name, true); // null terminated
    jsvUnLock(name);
    jsjcLiteral8(1, isConstant?1:0); // r1 -> if we're a constant
    if (hasInitialiser) jsjPopAsVar(2); // r2 -> initial value
    else jsjcLiteral8(2, 0); // r2 -> no initial value
    jsjcCall(_jsxAddVar); // add the variable
    hasComma = lex->tk == ',';
    if (hasComma) JSP_ASSERT_MATCH(',');
  }
}

void jsjStatementIf() {
  JSP_ASSERT_MATCH(LEX_R_IF);
  DEBUG_JIT("; IF condition\n");
  JSP_MATCH('(');
  jsjExpression();
  jsjPopAsBool(0);
  jsjcCompareImm(0, 0);
  JSP_MATCH(')');

  DEBUG_JIT("; capture IF true block\n");
  JsVar *oldBlock = jsjcStartBlock();
  jsjBlockOrStatement();
  JsVar *trueBlock = jsjcStopBlock(oldBlock);
  JsVar *falseBlock = 0;

  if (lex->tk==LEX_R_ELSE) {
    JSP_ASSERT_MATCH(LEX_R_ELSE);
    DEBUG_JIT("; capture IF false block\n");
    oldBlock = jsjcStartBlock();
    jsjBlockOrStatement();
    falseBlock = jsjcStopBlock(oldBlock);
  }
  DEBUG_JIT("; IF jump after condition\n");
  // if false, jump after true block (if an 'else' we need to jump over the jsjcBranchRelative
  jsjcBranchConditionalRelative(JSJAC_EQ, jsvGetStringLength(trueBlock) + (falseBlock?2:0));
  DEBUG_JIT("; IF true block\n");
  jsjcEmitBlock(trueBlock);
  jsvUnLock(trueBlock);
  if (falseBlock) {
    jsjcBranchRelative(jsvGetStringLength(falseBlock)); // jump over false block
    DEBUG_JIT("; IF false block\n");
    jsjcEmitBlock(falseBlock);
    jsvUnLock(falseBlock);
  }
  DEBUG_JIT("; IF end\n");

}

void jsjStatementFor() {
  JSP_ASSERT_MATCH(LEX_R_FOR);
  JSP_MATCH('(');
  // we could have 'for (;;)' - so don't munch up our semicolon if that's all we have
  // Parse initialiser - we always run this so march right in and create code
  DEBUG_JIT("; FOR initialiser\n");
  if (lex->tk != ';')
    jsjStatement();
  JSP_MATCH(';');
  // Condition - we run this first time, so we go straight through here, but save the position so we can jump back here
  // after the main loop
  int codePosCondition = jsjcGetByteCount();
  DEBUG_JIT("; FOR condition\n");
  if (lex->tk != ';') {
    jsjExpression(); // condition
    jsjPopAsBool(0);
    jsjcCompareImm(0, 0);
    // We add a jump to the end after we've parsed everything and know the size
  }
  JSP_MATCH(';');
  DEBUG_JIT("; Parsing FOR Iterator block\n");
  JsVar *oldBlock = jsjcStartBlock();
  if (lex->tk != ')')  { // we could have 'for (;;)'
    jsjExpression(); // iterator
    jsjPopAndUnLock();
  }
  JsVar *iteratorBlock = jsjcStopBlock(oldBlock);
  JSP_MATCH(')'); // FIXME: clean up on exit
  // Now parse the actual code to execute
  DEBUG_JIT("; Parsing FOR Main block\n");
  oldBlock = jsjcStartBlock();
  jsjBlockOrStatement();
  JsVar *mainBlock = jsjcStopBlock(oldBlock);
  DEBUG_JIT("; Branch OVER main block to END\n");
  // Now figure out the jump length and jump (if condition is false)
  jsjcBranchConditionalRelative(JSJAC_EQ, jsvGetStringLength(iteratorBlock) + jsvGetStringLength(mainBlock) + 2);
  DEBUG_JIT("; FOR Main block\n");
  jsjcEmitBlock(mainBlock);
  jsvUnLock(mainBlock);
  DEBUG_JIT("; FOR Iterator block\n");
  jsjcEmitBlock(iteratorBlock);
  jsvUnLock(iteratorBlock);
  // after the iterator, jump back to condition
  DEBUG_JIT("; FOR jump back to condition\n");
  jsjcBranchRelative(codePosCondition - (jsjcGetByteCount()+2));
  DEBUG_JIT("; FOR end\n");
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
} else if (lex->tk==LEX_R_VAR ||
            lex->tk==LEX_R_LET ||
            lex->tk==LEX_R_CONST) {
    return jsjStatementVar();
  } else if (lex->tk==LEX_R_IF) {
    return jsjStatementIf();
  /*} else if (lex->tk==LEX_R_DO) {
    return jsjStatementDoOrWhile(false);
  } else if (lex->tk==LEX_R_WHILE) {
    return jsjStatementDoOrWhile(true);*/
  } else if (lex->tk==LEX_R_FOR) {
    return jsjStatementFor();
  /*} else if (lex->tk==LEX_R_TRY) {
    return jsjStatementTry();*/
  } else if (lex->tk==LEX_R_RETURN) {
    JSP_ASSERT_MATCH(LEX_R_RETURN);
    if (lex->tk != ';' && lex->tk != '}') {
      jsjExpression();
      jsjPopNoName(0); // a -> r0, we only want the value, so skip the name if there was one
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
    return jsjStatementSwitch();*/
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
  // optimisation: if the last statement was a return, no need for this. Could check if last instruction was 'POP {r4,r5,r6,r7,pc}'
  // Return 'undefined' from function if no other return statement
  jsjcLiteral32(0, 0);
  jsjcPopAllAndReturn();
  JsVar *v = jsjcStop();
  JsVar *exception = jspGetException();
  if (!exception) return v;
  // We had an error - don't return half-complete code
  jsiConsolePrintf("JIT %v\n", exception);
  if (jsvIsObject(exception)) {
    JsVar *stackTrace = jsvObjectGetChild(exception, "stack", 0);
    if (stackTrace) {
      jsiConsolePrintStringVar(stackTrace);
      jsvUnLock(stackTrace);
    }
  }
  jsvUnLock(exception);
  jsvUnLock(v);
  return 0;
}

JsVar *jsjEvaluateVar(JsVar *str) {
  JsLex lex;
  assert(jsvIsString(str));
  JsLex *oldLex = jslSetLex(&lex);
  jslInit(str);
  jsjcStart();
  jsjcPushAll();
  jsjExpression();
  jsjPopNoName(0); // a -> r0, we only want the value, so skip the name if there was one
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

#endif /*#ifdef ESPR_JIT*/
