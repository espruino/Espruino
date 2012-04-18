/*
 * jsvar.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#include "jsvar.h"
#include "jslex.h"

#define JSVAR_CACHE_SIZE 256
JsVar jsVars[JSVAR_CACHE_SIZE];

void jsvInit() {
  for (int i=0;i<JSVAR_CACHE_SIZE;i++) {
    jsVars[i].this = i+1;
    jsVars[i].refs = -1;
  }
}

void jsvKill() {
}

JsVarRef jsvNew() {
  for (int i=1;i<JSVAR_CACHE_SIZE;i++) {
    if (jsVars[i].refs == -1) {
      JsVar *v = &jsVars[i];
      // this variable is empty
      // reset it
      v->refs = 0;
      v->locks = 0;
      v->flags = 0;
      v->callback = 0;
      v->firstChild = 0;
      v->lastChild = 0;
      v->prevSibling = 0;
      v->nextSibling = 0;
      // return ref
      return v->this;
    }
  }
  return 0;
}

JsVar *jsvLock(JsVarRef ref) {
  assert(ref);
  jsVars[ref-1].locks++;
  return &jsVars[ref-1];
}

void jsvUnLock(JsVarRef ref) {
  assert(ref);
  assert(jsVars[ref-1].locks>0);
  jsVars[ref-1].locks--;
}

JsVarRef jsvUnLockPtr(JsVar *var) {
  assert(var);
  JsVarRef ref = var->this;
  jsvUnLock(ref);
  return ref;
}

JsVar *jsvRef(JsVar *v) {
  v->refs++;
  return v;
}

void jsvUnRef(JsVar *v) {
  assert(v->refs>0);
  v->refs--;
  if (v->refs==0) {
    // free!
    // TODO: if string, unref substrings
    // If a NAME, unref firstChild
    // If an object/array, unref children
    printf("TODO: Free var\n");
  }
}

JsVarRef jsvRefRef(JsVarRef ref) {
  JsVar *v = jsvLock(ref);
  jsvRef(v);
  jsvUnLock(ref);
  return ref;
}
JsVarRef jsvUnRefRef(JsVarRef ref) {
  JsVar *v = jsvLock(ref);
  jsvUnRef(v);
  jsvUnLock(ref);
  return 0;
}

JsVarRef jsvNewFromString(const char *str) {
  // Create a var
  JsVarRef firstRef = jsvNew();
  // Now we copy the string, but keep creating new jsVars if we go
  // over the end
  JsVarRef ref = firstRef;
  JsVar *var = jsvLock(ref);
  var->flags = SCRIPTVAR_STRING;
  var->strData[0] = 0; // in case str is empty!

  while (*str) {
    // copy data in
    for (int i=0;i<JSVAR_STRING_LEN;i++) {
      var->strData[i] = *str;
      if (*str) str++;
    }
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    JsVarRef refNext = jsvNew();
    var->firstChild = refNext;
    jsvUnLock(ref);
    ref = refNext;
    var = jsvLock(ref);
    jsvRef(var);
    var->flags = SCRIPTVAR_STRING_EXT;
  }

  jsvUnLock(ref);
  // return
  return firstRef;
}

JsVarRef jsvNewWithFlags(SCRIPTVAR_FLAGS flags) {
  JsVarRef ref = jsvNew();
  JsVar *var = jsvLock(ref);
  var->flags = flags;
  jsvUnLock(ref);
  return ref;
}
JsVarRef jsvNewFromInteger(int value) {
  JsVarRef ref = jsvNew();
  JsVar *var = jsvLock(ref);
  var->flags = SCRIPTVAR_INTEGER;
  var->intData = value;
  jsvUnLock(ref);
  return ref;
}
JsVarRef jsvNewFromBool(bool value) {
  JsVarRef ref = jsvNew();
  JsVar *var = jsvLock(ref);
  var->flags = SCRIPTVAR_INTEGER;
  var->intData = value ? 1 : 0;
  jsvUnLock(ref);
  return ref;
}
JsVarRef jsvNewFromDouble(double value) {
  JsVarRef ref = jsvNew();
  JsVar *var = jsvLock(ref);
  var->flags = SCRIPTVAR_DOUBLE;
  var->doubleData = value;
  jsvUnLock(ref);
  return ref;
}
JsVarRef jsvNewVariableName(JsVarRef variable, const char *name) {
  JsVarRef ref = jsvNewFromString(name);
  JsVar *var = jsvLock(ref);
  var->flags |= SCRIPTVAR_NAME;
  var->firstChild = jsvRefRef(variable);
  jsvUnLock(ref);
  return ref;
}

bool jsvIsInt(JsVar *v) { return (v->flags&SCRIPTVAR_INTEGER)!=0; }
bool jsvIsDouble(JsVar *v) { return (v->flags&SCRIPTVAR_DOUBLE)!=0; }
bool jsvIsString(JsVar *v) { return (v->flags&SCRIPTVAR_STRING)!=0; }
bool jsvIsNumeric(JsVar *v) { return (v->flags&SCRIPTVAR_NUMERICMASK)!=0; }
bool jsvIsFunction(JsVar *v) { return (v->flags&SCRIPTVAR_FUNCTION)!=0; }
bool jsvIsObject(JsVar *v) { return (v->flags&SCRIPTVAR_OBJECT)!=0; }
bool jsvIsArray(JsVar *v) { return (v->flags&SCRIPTVAR_ARRAY)!=0; }
bool jsvIsNative(JsVar *v) { return (v->flags&SCRIPTVAR_NATIVE)!=0; }
bool jsvIsUndefined(JsVar *v) { return (v->flags & SCRIPTVAR_VARTYPEMASK) == SCRIPTVAR_UNDEFINED; }
bool jsvIsNull(JsVar *v) { return (v->flags & SCRIPTVAR_NULL)!=0; }
bool jsvIsBasic(JsVar *v) { return v->firstChild==0; } ///< Is this *not* an array/object/etc
bool jsvIsName(JsVar *v) { return (v->flags & SCRIPTVAR_NAME)!=0; }

/// Save this var as a string to the given buffer
void jsvGetString(JsVar *v, char *str, int len) {
    if (!v) {
      assert(len>0);
      str[0] = 0;
      return;
    }
    if (jsvIsInt(v)) {
      //OPT could use itoa
      snprintf(str, len, "%ld", v->intData);
    } else if (jsvIsDouble(v)) {
      //OPT could use ftoa
      snprintf(str, len, "%f", v->doubleData);
    } else if (jsvIsNull(v)) {
      snprintf(str, len, "null");
    } else if (jsvIsUndefined(v)) {
      snprintf(str, len, "undefined");
    } else { assert(jsvIsString(v));
      // print the string - we have to do it a block
      // at a time!
      JsVar *var = v;
      JsVarRef ref = 0;
      while (var) {
        for (int i=0;i<JSVAR_STRING_LEN;i++) {
          if (len--<=0) {
            *str = 0;
            printf("jsvGetString overflowed\n");
            return;
          }
          *(str++) = var->strData[i];
        }

        JsVarRef refNext = var->firstChild;
        if (ref) jsvUnLock(ref);
        ref = refNext;
        var = 0;
        if (ref) var = jsvLock(ref);
      }
      if (ref) jsvUnLock(ref);
    }
}

int jsvGetInteger(JsVar *v) {
    if (!v) return 0;
    /* strtol understands about hex and octal */
    if (jsvIsInt(v)) return v->intData;
    if (jsvIsNull(v)) return 0;
    if (jsvIsUndefined(v)) return 0;
    if (jsvIsDouble(v)) return (int)v->doubleData;
    return 0;
}

bool jsvGetBool(JsVar *v) {
  return jsvGetInteger(v)!=0;
}

double jsvGetDouble(JsVar *v) {
    if (!v) return 0;
    if (jsvIsDouble(v)) return v->doubleData;
    if (jsvIsInt(v)) return v->intData;
    if (jsvIsNull(v)) return 0;
    if (jsvIsUndefined(v)) return 0;
    return 0; /* or NaN? */
}


void jsvAddNamedChild(JsVarRef parent, JsVarRef child, const char *name) {
  JsVarRef namedChild = jsvRefRef(jsvNewVariableName(child, name));
  JsVar *v = jsvLock(parent);
  if (v->lastChild) {
    // Link 2 children together
    JsVar *lastChild = jsvLock(v->lastChild);
    lastChild->nextSibling = namedChild;
    jsvUnLockPtr(lastChild);

    JsVar *thisChild = jsvLock(namedChild);
    thisChild->prevSibling = v->lastChild;
    jsvUnLockPtr(thisChild);
    // finally set the new child as the last one
    v->lastChild = namedChild;
  } else {
    v->firstChild = namedChild;
    v->lastChild = namedChild;
  }
  jsvUnLock(child);
}


JsVarRef jsvMathsOpError(int op, const char *datatype) {
    char tbuf[JS_ERROR_TOKEN_BUF_SIZE];
    char buf[JS_ERROR_BUF_SIZE];
    jslTokenAsString(op, tbuf, JS_ERROR_TOKEN_BUF_SIZE);
    snprintf(buf,JS_ERROR_BUF_SIZE, "Operation %s expected not supported on the %s datatype", tbuf, datatype);
    jsError(buf);
    return 0;
}

JsVarRef jsvMathsOpPtr(JsVar *a, JsVar *b, int op) {
    if (!a || !b) return 0;
    // Type equality check
    if (op == LEX_TYPEEQUAL || op == LEX_NTYPEEQUAL) {
      // check type first, then call again to check data
      bool eql = ((a->flags & SCRIPTVAR_VARTYPEMASK) ==
                  (b->flags & SCRIPTVAR_VARTYPEMASK));
      if (eql) {
        JsVar *contents = jsvLock(jsvMathsOpPtr(a,b, LEX_EQUAL));
        if (!jsvGetBool(contents)) eql = false;
        jsvUnLockPtr(contents);
      }
      if (op == LEX_TYPEEQUAL)
        return jsvNewFromBool(eql);
      else
        return jsvNewFromBool(!eql);
    }

    // do maths...
    if (jsvIsUndefined(a) && jsvIsUndefined(b)) {
      if (op == LEX_EQUAL)
        return jsvNewFromBool(true);
      else if (op == LEX_NEQUAL)
        return jsvNewFromBool(false);
      else
        return jsvNewWithFlags(SCRIPTVAR_UNDEFINED); // undefined
    } else if ((jsvIsNumeric(a) || jsvIsUndefined(a)) &&
               (jsvIsNumeric(b) || jsvIsUndefined(b))) {
        if (!jsvIsDouble(a) && !jsvIsDouble(b)) {
            // use ints
            int da = jsvGetInteger(a);
            int db = jsvGetInteger(b);
            switch (op) {
                case '+': return jsvNewFromInteger(da+db);
                case '-': return jsvNewFromInteger(da-db);
                case '*': return jsvNewFromInteger(da*db);
                case '/': return jsvNewFromInteger(da/db);
                case '&': return jsvNewFromInteger(da&db);
                case '|': return jsvNewFromInteger(da|db);
                case '^': return jsvNewFromInteger(da^db);
                case '%': return jsvNewFromInteger(da%db);
                case LEX_LSHIFT: return jsvNewFromInteger(da << db);
                case LEX_RSHIFT: return jsvNewFromInteger(da >> db);
                case LEX_RSHIFTUNSIGNED: return jsvNewFromInteger(((unsigned int)da) >> db);
                case LEX_EQUAL:     return jsvNewFromBool(da==db);
                case LEX_NEQUAL:    return jsvNewFromBool(da!=db);
                case '<':           return jsvNewFromBool(da<db);
                case LEX_LEQUAL:    return jsvNewFromBool(da<=db);
                case '>':           return jsvNewFromBool(da>db);
                case LEX_GEQUAL:    return jsvNewFromBool(da>=db);
                default: return jsvMathsOpError(op, "Integer");
            }
        } else {
            // use doubles
            double da = jsvGetDouble(a);
            double db = jsvGetDouble(b);
            switch (op) {
                case '+': return jsvNewFromDouble(da+db);
                case '-': return jsvNewFromDouble(da-db);
                case '*': return jsvNewFromDouble(da*db);
                case '/': return jsvNewFromDouble(da/db);
                case LEX_EQUAL:     return jsvNewFromBool(da==db);
                case LEX_NEQUAL:    return jsvNewFromBool(da!=db);
                case '<':           return jsvNewFromBool(da<db);
                case LEX_LEQUAL:    return jsvNewFromBool(da<=db);
                case '>':           return jsvNewFromBool(da>db);
                case LEX_GEQUAL:    return jsvNewFromBool(da>=db);
                default: return jsvMathsOpError(op, "Double");
            }
        }
    } else if (jsvIsArray(a) || jsvIsObject(a) || jsvIsArray(b) || jsvIsObject(b)) {
      bool isArray = jsvIsArray(a);
      /* Just check pointers */
      switch (op) {
           case LEX_EQUAL:  return jsvNewFromBool(a==b);
           case LEX_NEQUAL: return jsvNewFromBool(a!=b);
           default: return jsvMathsOpError(op, isArray?"Array":"Object");
      }
    } else {
       // FIXME - proper string compare and add using the var blocks
       char da[JSVAR_STRING_OP_BUFFER_SIZE];
       char db[JSVAR_STRING_OP_BUFFER_SIZE];
       jsvGetString(a, da, JSVAR_STRING_OP_BUFFER_SIZE);
       jsvGetString(b, db, JSVAR_STRING_OP_BUFFER_SIZE);
       // use strings
       switch (op) {
           case '+': {
             strncat(da, db, JSVAR_STRING_OP_BUFFER_SIZE);
             return jsvNewFromString(da);
           }
           case LEX_EQUAL:     return jsvNewFromBool(strcmp(da,db)==0);
           case LEX_NEQUAL:    return jsvNewFromBool(strcmp(da,db)!=0);
           case '<':           return jsvNewFromBool(strcmp(da,db)<0);
           case LEX_LEQUAL:    return jsvNewFromBool(strcmp(da,db)<=0);
           case '>':           return jsvNewFromBool(strcmp(da,db)>0);
           case LEX_GEQUAL:    return jsvNewFromBool(strcmp(da,db)>=0);
           default: return jsvMathsOpError(op, "String");
       }
    }
    assert(0);
    return 0;
}

JsVarRef jsvMathsOp(JsVarRef ar, JsVarRef br, int op) {
    if (!ar || !br) return 0;
    JsVar *a = jsvLock(ar);
    JsVar *b = jsvLock(br);
    JsVarRef res = jsvMathsOpPtr(a,b,op);
    jsvUnLockPtr(a);
    jsvUnLockPtr(b);
    return res;
}
