/*
 * jsvar.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#include "jsvar.h"

#define JSVAR_CACHE_SIZE 256
JsVar jsVars[JSVAR_CACHE_SIZE];

void jsvInit() {
  for (int i=0;i<JSVAR_CACHE_SIZE;i++)
    jsVars[i].refs = -1;
}

void jsvKill() {
}

JsVarRef jsvNew() {
  for (int i=1;i<JSVAR_CACHE_SIZE;i++) {
    if (jsVars[i].refs == -1) {
      // this variable is empty
      // reset it
      jsVars[i].refs = 0;
      jsVars[i].locks = 0;
      jsVars[i].flags = 0;
      jsVars[i].callback = 0;
      jsVars[i].firstChild = 0;
      jsVars[i].lastChild = 0;
      jsVars[i].prevSibling = 0;
      jsVars[i].nextSibling = 0;
      // return ref
      return i+1;
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

void jsvRef(JsVar *v) {
  v->refs++;
}

void jsvUnRef(JsVar *v) {
  assert(v->refs>0);
  v->refs--;
  if (v->refs==0) {
    // free!
    // TODO: if string, unref substrings
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

/// Save this var as a string to the given buffer
void jsvGetString(JsVar *v, char *str, int len) {
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
