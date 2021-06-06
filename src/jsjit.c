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

#include "jsjit.h"
#include "jsjitc.h"

JsVar *jsjStringPool;

NO_INLINE void jsjFactor() {
  if (lex->tk==LEX_ID) {
    const char *v = jslGetTokenValueAsString();
    int offset = jsvGetStringLength(jsjStringPool);
    jsvAppendStringBuf(jsjStringPool, v, strlen(v)+1); // include trailing 0
    JSP_ASSERT_MATCH(LEX_ID);
    jsjcLiteral32(1, offset); // TODO: store token values
    jsjcCall(jspGetNamedVariable);
    return a;
  } else if (lex->tk==LEX_INT) {
    int64_t v = stringToInt(jslGetTokenValueAsString()));
    JSP_ASSERT_MATCH(LEX_INT);
    jsjcLiteral64(1, (uint64_t)v);
    jsjcCall(jsvNewFromLongInteger); // TODO: could do 32 bit here
    return v;
  } else if (lex->tk==LEX_FLOAT) {
    double v = stringToFloat(jslGetTokenValueAsString());
    JSP_ASSERT_MATCH(LEX_FLOAT);
    jsjcLiteral64(1, *((uint64_t*)&v));
    jsjcCall(jsvNewFromFloat); 
  } else if (lex->tk=='(') {
    JSP_ASSERT_MATCH('(');
    if (!jspCheckStackPosition()) return 0;
    // Just parse a normal expression (which can include commas)
    JsVar *a = jspeExpression();
    if (!JSP_SHOULDNT_PARSE) JSP_MATCH_WITH_RETURN(')',a);
    return a;
  } else if (lex->tk==LEX_R_TRUE) {
    JSP_ASSERT_MATCH(LEX_R_TRUE);
    jsjcLiteral32(1, 1);
    jsjcCall(jsvNewFromBool); 
  } else if (lex->tk==LEX_R_FALSE) {
    JSP_ASSERT_MATCH(LEX_R_FALSE);
    jsjcLiteral32(1, 0);
    jsjcCall(jsvNewFromBool); 
  } else if (lex->tk==LEX_R_NULL) {
    JSP_ASSERT_MATCH(LEX_R_NULL);
    jsjcLiteral32(1, JSV_NULL);
    jsjcCall(jsvNewWithFlags);
  } else if (lex->tk==LEX_R_UNDEFINED) {
    JSP_ASSERT_MATCH(LEX_R_UNDEFINED);
    jsjcLiteral32(1, 0);
  } else if (lex->tk==LEX_STR) {
    JsVar *a = jslGetTokenValueAsVar();
    JSP_ASSERT_MATCH(LEX_STR);
    int offset = jsvGetStringLength(jsjStringPool);
    int len = jsvGetStringLength(a);
    jsvAppendStringVarComplete(jsjStringPool, a);
    jsvUnLock(a);
    jsjcLiteral32(1, len);
    jsjcLiteral32(2, offset);
    jsjcCall(jsvNewStringOfLength); 
  }/* else if (lex->tk=='{') {
    if (!jspCheckStackPosition()) return 0;
    return jspeFactorObject();
  } else if (lex->tk=='[') {
    if (!jspCheckStackPosition()) return 0;
    return jspeFactorArray();
  } else if (lex->tk==LEX_R_FUNCTION) {
    if (!jspCheckStackPosition()) return 0;
    JSP_ASSERT_MATCH(LEX_R_FUNCTION);
    return jspeFunctionDefinition(true);
  } else if (lex->tk==LEX_R_THIS) {
    JSP_ASSERT_MATCH(LEX_R_THIS);
    return jsvLockAgain( execInfo.thisVar ? execInfo.thisVar : execInfo.root );
  } else if (lex->tk==LEX_R_DELETE) {
    if (!jspCheckStackPosition()) return 0;
    return jspeFactorDelete();
  } else if (lex->tk==LEX_R_TYPEOF) {
    if (!jspCheckStackPosition()) return 0;
    return jspeFactorTypeOf();
  } */else if (lex->tk==LEX_R_VOID) {
    JSP_ASSERT_MATCH(LEX_R_VOID);
    // jsjUnaryExpression(); // FIXME
    jsjcCall(jsvUnLock);
    return 0;
  }
  JSP_MATCH(LEX_EOF);
  return 0;
}

JsVar *jsjParse() {
  jsjFactor();
  return 0;
}

JsVar *jsjEvaluateVar(JsVar *str) {
  JsLex lex;
  assert(jsvIsString(str));
  JsLex *oldLex = jslSetLex(&lex);
  jslInit(str);
  JsVar *v = jsjParse();
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
    v = jsjEvaluateVar(evCode,);
  jsvUnLock(evCode);

  return v;
}

