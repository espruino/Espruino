/*
 * jsvar.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#include "jsvar.h"
#include "jslex.h"

#define JSVAR_CACHE_UNUSED_REF 0xFFFF
#define JSVAR_CACHE_SIZE 128
JsVar jsVars[JSVAR_CACHE_SIZE];

void jsvInit() {
  for (int i=0;i<JSVAR_CACHE_SIZE;i++) {
    jsVars[i].this = (JsVarRef)(i+1);
    jsVars[i].refs = JSVAR_CACHE_UNUSED_REF;
  }
}

void jsvKill() {
}

// Get number of memory records (JsVars) used
int jsvGetMemoryUsage() {
  int usage = 0;
  for (int i=1;i<JSVAR_CACHE_SIZE;i++)
    if (jsVars[i].refs != JSVAR_CACHE_UNUSED_REF)
      usage++;
  return usage;
}

// Show what is still allocated, for debugging memory problems
void jsvShowAllocated() {
  for (int i=1;i<JSVAR_CACHE_SIZE;i++)
    if (jsVars[i].refs != JSVAR_CACHE_UNUSED_REF) {
      printf("USED VAR #%d:", jsVars[i].this);
      jsvTrace(jsVars[i].this, 2);
    }
}

JsVar *jsvNew() {
  for (int i=1;i<JSVAR_CACHE_SIZE;i++) {
    if (jsVars[i].refs == JSVAR_CACHE_UNUSED_REF) {
      JsVar *v = &jsVars[i];
      // this variable is empty
      // reset it
      v->refs = 0;
      v->locks = 1;
      v->flags = 0;
      v->callback = 0;
      v->firstChild = 0;
      v->lastChild = 0;
      v->prevSibling = 0;
      v->nextSibling = 0;
      // return ref
      return v;
    }
  }
  jsError("Out of Memory!");
  return 0;
}

void jsvFreePtr(JsVar *var) {
    /*var->refs = 1000; // just ensure we don't get freed while tracing!
    printf("FREEING #%d:\n", var->this);
    jsvTrace(var->this, 4);*/

    // we shouldn't be linked from anywhere!
    assert(!var->nextSibling && !var->prevSibling);
    // free!
    var->refs = JSVAR_CACHE_UNUSED_REF;

    /* Now, unref children - see jsvar.h comments for how! */
    if (jsvIsString(var) || jsvIsStringExt(var) || jsvIsName(var)) {
      JsVarRef stringDataRef = var->lastChild;
      var->lastChild = 0;
      if (stringDataRef) {
        JsVar *child = jsvLock(stringDataRef);
        assert(jsvIsStringExt(child) && !child->prevSibling && !child->nextSibling);
        child->prevSibling = 0;
        child->nextSibling = 0;
        jsvUnRef(child);
        jsvUnLockPtr(child);
      }
      // Names Link to other things
      if (jsvIsName(var)) {
        JsVarRef childref = var->firstChild;
        var->firstChild = 0;
        while (childref) {
          JsVar *child = jsvLock(childref);
          childref = child->nextSibling;
          child->prevSibling = 0;
          child->nextSibling = 0;
          jsvUnRef(child);
          jsvUnLockPtr(child);
        }
      } else {
        assert(!var->firstChild);
      }
    } else if (jsvIsObject(var) || jsvIsFunction(var) || jsvIsArray(var)) {
      JsVarRef childref = var->firstChild;
      var->firstChild = 0;
      var->lastChild = 0;
      while (childref) {
        JsVar *child = jsvLock(childref);
        assert(jsvIsName(child));
        childref = child->nextSibling;
        child->prevSibling = 0;
        child->nextSibling = 0;
        jsvUnRef(child);
        jsvUnLockPtr(child);
      }
    } else {
      assert(!var->firstChild);
      assert(!var->lastChild);
    }
  }

//int c = 0;

JsVar *jsvLock(JsVarRef ref) {
  assert(ref);
  JsVar *var = &jsVars[ref-1];
//  if (ref==29) printf("+ %d : %d\n", ++c, var->locks);
  var->locks++;
  return var;
}

void jsvUnLock(JsVarRef ref) {
  assert(ref);
  JsVar *var = &jsVars[ref-1];
  assert(var->locks>0);
//  if (ref==29) printf("- %d : %d\n", c, var->locks);
  var->locks--;
  if (var->locks == 0 && var->refs==0)
    jsvFreePtr(var);
}


JsVar *jsvLockPtr(JsVar *var) {
  assert(var);
  jsvLock(var->this);
  return var;
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

void jsvUnRef(JsVar *var) {
  assert(var->refs>0);
  var->refs--;
  if (var->locks == 0 && var->refs==0)
      jsvFreePtr(var);
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

JsVar *jsvNewFromString(const char *str) {
  // Create a var
  JsVar *first = jsvNew();
  // Now we copy the string, but keep creating new jsVars if we go
  // over the end
  JsVar *var = first;
  var->flags = JSV_STRING;
  var->strData[0] = 0; // in case str is empty!

  while (*str) {
    // copy data in
    for (int i=0;i<JSVAR_STRING_LEN;i++) {
      var->strData[i] = *str;
      if (*str) str++;
    }
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (*str) {
      JsVar *next = jsvRef(jsvNew());
      next->flags = JSV_STRING_EXT;
      var->lastChild = next->this;
      if (var!=first) jsvUnLockPtr(var);
      var = next;
    }
  }
  if (var!=first) jsvUnLockPtr(var);
  // return
  return first;
}

JsVar *jsvNewFromLexer(struct JsLex *lex, int charFrom, int charTo) {
  // Create a new STRING from part of the lexer
  // Create a new lexer to span the whole area
  // OPT: probably not the fastest, but safe. Only done when a new function is created anyway
  JsLex newLex;
  jslInit(&newLex, jsvLock(lex->sourceVarRef), charFrom, charTo);
  jsvUnLock(lex->sourceVarRef);
  // Reset (we must do this because normally it tries to get a new token)
  jslSeek(&newLex, newLex.sourceStartPos);
  jslGetNextCh(&newLex);
  jslGetNextCh(&newLex);

  // Create a var
  JsVar *first = jsvNew();
  // Now we copy the string, but keep creating new jsVars if we go
  // over the end
  JsVar *var = first;
  var->flags = JSV_STRING;
  var->strData[0] = 0; // in case str is empty!


  while (newLex.currCh) {
    // copy data in
    for (int i=0;i<JSVAR_STRING_LEN;i++) {
      var->strData[i] = newLex.currCh;
      if (newLex.currCh) jslGetNextCh(&newLex);
    }
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (newLex.currCh) {
      JsVar *next = jsvRef(jsvNew());
      next->flags = JSV_STRING_EXT;
      var->lastChild = next->this;
      if (var!=first) jsvUnLockPtr(var);
      var = next;
    }
  }
  // free
  if (var!=first) jsvUnLockPtr(var);
  jslKill(&newLex);
  // return
  return first;
}

JsVar *jsvNewWithFlags(JsVarFlags flags) {
  JsVar *var = jsvNew();
  var->flags = flags;
  return var;
}
JsVar *jsvNewFromInteger(JsVarInt value) {
  JsVar *var = jsvNew();
  var->flags = JSV_INTEGER;
  var->intData = value;
  return var;
}
JsVar *jsvNewFromBool(bool value) {
  JsVar *var = jsvNew();
  var->flags = JSV_INTEGER;
  var->intData = value ? 1 : 0;
  return var;
}
JsVar *jsvNewFromFloat(JsVarFloat value) {
  JsVar *var = jsvNew();
  var->flags = JSV_FLOAT;
  var->doubleData = value;
  return var;
}
JsVar *jsvNewVariableName(JsVarRef variable, const char *name) {
  JsVar *var = jsvNewFromString(name);
  var->flags |= JSV_NAME;
  if (variable)
    var->firstChild = jsvRefRef(variable);
  return var;
}
JsVar *jsvNewVariableNameFromLexerToken(JsVarRef variable, struct JsLex *lex) {
  JsVar *var = jsvNewFromString(jslGetTokenValueAsString(lex));
  var->flags |= JSV_NAME;
  if (variable)
    var->firstChild = jsvRefRef(variable);
  return var;
}

bool jsvIsInt(JsVar *v) { return (v->flags&JSV_VARTYPEMASK)==JSV_INTEGER; }
bool jsvIsFloat(JsVar *v) { return (v->flags&JSV_VARTYPEMASK)==JSV_FLOAT; }
bool jsvIsString(JsVar *v) { return (v->flags&JSV_VARTYPEMASK)==JSV_STRING; }
bool jsvIsStringExt(JsVar *v) { return (v->flags&JSV_VARTYPEMASK)==JSV_STRING_EXT; }
bool jsvIsNumeric(JsVar *v) { return (v->flags&JSV_NUMERICMASK)!=0; }
bool jsvIsFunction(JsVar *v) { return (v->flags&JSV_VARTYPEMASK)==JSV_FUNCTION; }
bool jsvIsFunctionParameter(JsVar *v) { return (v->flags&JSV_FUNCTION_PARAMETER) == JSV_FUNCTION_PARAMETER; }
bool jsvIsObject(JsVar *v) { return (v->flags&JSV_VARTYPEMASK)==JSV_OBJECT; }
bool jsvIsArray(JsVar *v) { return (v->flags&JSV_VARTYPEMASK)==JSV_ARRAY; }
bool jsvIsNative(JsVar *v) { return (v->flags&JSV_NATIVE)!=0; }
bool jsvIsUndefined(JsVar *v) { return (v->flags & JSV_VARTYPEMASK) == JSV_UNDEFINED; }
bool jsvIsNull(JsVar *v) { return (v->flags&JSV_VARTYPEMASK)==JSV_NULL; }
bool jsvIsBasic(JsVar *v) { return v->firstChild==0; /* Fixme */ } ///< Is this *not* an array/object/etc
bool jsvIsName(JsVar *v) { return (v->flags & JSV_NAME)!=0; }

/// Save this var as a string to the given buffer
void jsvGetString(JsVar *v, char *str, size_t len) {
    if (!v) {
      assert(len>0);
      str[0] = 0;
      return;
    }
    if (jsvIsInt(v)) {
      //OPT could use itoa
      snprintf(str, len, "%ld", v->intData);
    } else if (jsvIsFloat(v)) {
      //OPT could use ftoa
      snprintf(str, len, "%f", v->doubleData);
    } else if (jsvIsNull(v)) {
      snprintf(str, len, "null");
    } else if (jsvIsUndefined(v)) {
      snprintf(str, len, "undefined");
    } else if (jsvIsString(v) || jsvIsFunctionParameter(v)) {
      // print the string - we have to do it a block
      // at a time!
      JsVar *var = v;
      JsVarRef ref = 0;
      while (var) {
        for (int i=0;i<JSVAR_STRING_LEN;i++) {
          if (len--<=0) {
            *str = 0;
            jsWarn("jsvGetString overflowed\n");
            if (ref) jsvUnLock(ref);
            return;
          }
          *(str++) = var->strData[i];
        }
        // Go to next
        JsVarRef refNext = var->firstChild;
        if (ref) jsvUnLock(ref);
        ref = refNext;
        var = 0;
        if (ref) var = jsvLock(ref);
      }
      if (ref) jsvUnLock(ref);
    } else if (jsvIsFunction(v)) {
      snprintf(str, len, "function");
    } else assert(0);
}

int jsvGetStringLength(JsVar *v) {
  if (!jsvIsString(v)) return 0;
  int strLength = 0;

  JsVar *var = v;
  JsVarRef ref = 0;
  while (var) {
    JsVarRef refNext = var->lastChild;

    if (refNext!=0) {
      // we have more, so this section MUST be full
      strLength += JSVAR_STRING_LEN;
    } else {
      // count
      for (int i=0;i<JSVAR_STRING_LEN;i++) {
        if (var->strData[i])
          strLength++;
        else
          break;
      }
    }

    // Go to next
    if (ref) jsvUnLock(ref);
    ref = refNext;
    var = 0;
    if (ref) var = jsvLock(ref);
  }
  if (ref) jsvUnLock(ref);
  return strLength;
}

JsVarInt jsvGetInteger(JsVar *v) {
    if (!v) return 0;
    /* strtol understands about hex and octal */
    if (jsvIsInt(v)) return v->intData;
    if (jsvIsNull(v)) return 0;
    if (jsvIsUndefined(v)) return 0;
    if (jsvIsFloat(v)) return (JsVarInt)v->doubleData;
    return 0;
}

bool jsvGetBool(JsVar *v) {
  return jsvGetInteger(v)!=0;
}

double jsvGetDouble(JsVar *v) {
    if (!v) return 0;
    if (jsvIsFloat(v)) return v->doubleData;
    if (jsvIsInt(v)) return (double)v->intData;
    if (jsvIsNull(v)) return 0;
    if (jsvIsUndefined(v)) return 0;
    return 0; /* or NaN? */
}

JsVarInt jsvGetIntegerSkipName(JsVar *v) {
    JsVar *a = jsvSkipName(v);
    JsVarInt l = jsvGetInteger(a);
    jsvUnLockPtr(a);
    return l;
}

bool jsvGetBoolSkipName(JsVar *v) {
  return jsvGetIntegerSkipName(v)!=0;
}

bool jsvIsStringEqual(JsVar *var, const char *str) {
  assert(jsvIsString(var) || jsvIsName(var)); // we hope! Might just want to return 0?

  for (int i=0;i<JSVAR_STRING_LEN;i++) {
     if (var->strData[i] != str[i]) return false;
     if  (str[i]==0) return true; // end of string, all great!
  }

  jsError("INTERNAL: TODO: String equality check past boundary of string");
  return false; //
}

void jsvAddName(JsVarRef parent, JsVarRef namedChild) {
  namedChild = jsvRefRef(namedChild);
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
  jsvUnLockPtr(v);
}

JsVar *jsvAddNamedChild(JsVarRef parent, JsVarRef child, const char *name) {
  JsVar *namedChild = jsvNewVariableName(child, name);
  jsvAddName(parent, namedChild->this);
  return namedChild;
}

JsVar *jsvSetValueOfName(JsVar *name, JsVar *src) {
  assert(name && src);
  assert(jsvIsName(name));
  // all is fine, so replace the existing child...
  /* Existing child may be null in the case of Z = 0 where
   * we create 'Z' and pass it down to '=' to have the value
   * filled in (or it may be undefined). */
  if (name->firstChild) jsvUnRefRef(name->firstChild); // free existing
  name->firstChild = jsvRef(src)->this;
  return name;
}


/** Non-recursive finding */
JsVar *jsvFindChild(JsVarRef parentref, const char *name, bool createIfNotFound) {
  JsVar *parent = jsvLock(parentref);
  JsVarRef childref = parent->firstChild;
  while (childref) {
    JsVar *child = jsvLock(childref);
    if (jsvIsStringEqual(child, name)) {
      // found it! unlock parent but leave child locked
      jsvUnLockPtr(parent);
      return child;
    }
    childref = child->nextSibling;
    jsvUnLockPtr(child);
  }

  JsVar *child = 0;
  if (createIfNotFound) {
    child = jsvNewVariableName(0, name);
    jsvAddName(parentref, child->this);
  }
  jsvUnLockPtr(parent);
  return child;
}

int jsvGetChildren(JsVar *v) {
  //OPT: could length be stored as the value of the array?
  int children = 0;
  JsVarRef childref = v->firstChild;
  while (childref) {
    JsVar *child = jsvLock(childref);
    children++;
    childref = child->nextSibling;
    jsvUnLockPtr(child);
  }
  return children;
}


int jsvGetArrayLength(JsVar *v) {
  // TODO: ArrayLength != children, for instance a sparse array
  return jsvGetChildren(v);
}


/** If a is a name skip it and go to what it points to.
 * ALWAYS locks - so must unlock what it returns. It MAY
 * return 0.  */
JsVar *jsvSkipName(JsVar *a) {
  JsVar *pa = a;
  while (jsvIsName(pa)) {
    JsVarRef n = pa->firstChild;
    if (pa!=a) jsvUnLockPtr(pa);
    if (!n) return 0;
    pa = jsvLock(n);
  }
  if (pa==a) jsvLockPtr(pa);
  return pa;
}

/** Same as jsvSkipName, but ensures that 'a' is unlocked if it was
 * a name, so it can be used inline */
JsVar *jsvSkipNameAndUnlock(JsVar *a) {
  JsVar *b = jsvSkipName(a);
  jsvUnLockPtr(a);
  return b;
}

/** Same as jsvMathsOpPtr, but if a or b are a name, skip them
 * and go to what they point to. */
JsVar *jsvMathsOpPtrSkipNames(JsVar *a, JsVar *b, int op) {
  JsVar *pa = jsvSkipName(a);
  JsVar *pb = jsvSkipName(b);
  JsVar *res = jsvMathsOpPtr(pa,pb,op);
  jsvUnLockPtr(pa);
  jsvUnLockPtr(pb);
  return res;
}


JsVar *jsvMathsOpError(int op, const char *datatype) {
    char tbuf[JS_ERROR_TOKEN_BUF_SIZE];
    char buf[JS_ERROR_BUF_SIZE];
    jslTokenAsString(op, tbuf, JS_ERROR_TOKEN_BUF_SIZE);
    snprintf(buf,JS_ERROR_BUF_SIZE, "Operation %s expected not supported on the %s datatype", tbuf, datatype);
    jsError(buf);
    return 0;
}

JsVar *jsvMathsOpPtr(JsVar *a, JsVar *b, int op) {
    if (!a || !b) return 0;
    // Type equality check
    if (op == LEX_TYPEEQUAL || op == LEX_NTYPEEQUAL) {
      // check type first, then call again to check data
      bool eql = ((a->flags & JSV_VARTYPEMASK) ==
                  (b->flags & JSV_VARTYPEMASK));
      if (eql) {
        JsVar *contents = jsvMathsOpPtr(a,b, LEX_EQUAL);
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
        return jsvNewWithFlags(JSV_UNDEFINED); // undefined
    } else if ((jsvIsNumeric(a) || jsvIsUndefined(a)) &&
               (jsvIsNumeric(b) || jsvIsUndefined(b))) {
        if (!jsvIsFloat(a) && !jsvIsFloat(b)) {
            // use ints
            JsVarInt da = jsvGetInteger(a);
            JsVarInt db = jsvGetInteger(b);
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
                case LEX_RSHIFTUNSIGNED: return jsvNewFromInteger(((JsVarIntUnsigned)da) >> db);
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
            JsVarFloat da = jsvGetDouble(a);
            JsVarFloat db = jsvGetDouble(b);
            switch (op) {
                case '+': return jsvNewFromFloat(da+db);
                case '-': return jsvNewFromFloat(da-db);
                case '*': return jsvNewFromFloat(da*db);
                case '/': return jsvNewFromFloat(da/db);
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

JsVar *jsvMathsOp(JsVarRef ar, JsVarRef br, int op) {
    if (!ar || !br) return 0;
    JsVar *a = jsvLock(ar);
    JsVar *b = jsvLock(br);
    JsVar *res = jsvMathsOpPtr(a,b,op);
    jsvUnLockPtr(a);
    jsvUnLockPtr(b);
    return res;
}

/** Write debug info for this Var out to the console */
void jsvTrace(JsVarRef ref, int indent) {
    char buf[JS_ERROR_BUF_SIZE];

    for (int i=0;i<indent;i++) printf(" ");
    JsVar *var = jsvLock(ref);

    printf("#%d[r%d,l%d] ", ref, var->refs, var->locks-1);


    if (jsvIsName(var)) {

      jsvGetString(var, buf, JS_ERROR_BUF_SIZE);
      printf("Name: '%s'  ", buf);
      // go to what the name points to
      ref = var->firstChild;
      jsvUnLockPtr(var);
      if (ref) {
        var = jsvLock(ref);
      } else {
        printf("[UNSET]\n");
        return;
      }
    }
    if (jsvIsName(var)) {
      printf("\n");
      jsvTrace(var->this, indent+1);
      jsvUnLockPtr(var);
      return;
    }
    if (jsvIsObject(var)) printf("Object {\n");
    else if (jsvIsArray(var)) printf("Array [\n");
    else if (jsvIsInt(var)) printf("Integer ");
    else if (jsvIsFloat(var)) printf("Double ");
    else if (jsvIsString(var)) printf("String ");
    else if (jsvIsFunction(var)) printf("Function {\n");
    else printf("Flags %d\n", var->flags);

    if (!jsvIsObject(var) && !jsvIsArray(var) && !jsvIsFunction(var)) {
      jsvGetString(var, buf, JS_ERROR_BUF_SIZE);
      printf("%s", buf);
    }

    if (jsvIsString(var) || jsvIsName(var)) {
      JsVarRef child = var->firstChild;
      if (child) {
        printf("( Multi-block string ");
        while (child) {
          JsVar *childVar = jsvLock(child);
          printf("#%d[r%d,l%d] ", child, childVar->refs, childVar->locks-1);
          child = childVar->firstChild;
          jsvUnLockPtr(childVar);
        }
      }
      printf(")\n");
    } else {
      printf("\n");
      // dump children
      JsVarRef child = var->firstChild;
      while (child) {
        jsvTrace(child, indent+1);
        JsVar *childVar = jsvLock(child);
        child = childVar->nextSibling;
        jsvUnLockPtr(childVar);
      }
    }


    if (jsvIsObject(var) || jsvIsFunction(var)) {
      for (int i=0;i<indent;i++) printf(" ");
      printf("}\n");
    } else if (jsvIsArray(var)) {
      for (int i=0;i<indent;i++) printf(" ");
      printf("]\n");
    }

    jsvUnLockPtr(var);
}
