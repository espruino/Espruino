/*
 * jsvar.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */
//#define INLINE_FUNC inline
#include "jsvar.h"
#include "jslex.h"

#define JSVAR_CACHE_UNUSED_REF 0xFFFF
#ifdef SDCC
#define JSVAR_CACHE_SIZE 10
#else
#define JSVAR_CACHE_SIZE 512
#endif
JsVar jsVars[JSVAR_CACHE_SIZE]; 
JsVarRef jsVarFirstEmpty; ///< reference of first unused variable

void jsvInit() {
  int i;
  for (i=0;i<JSVAR_CACHE_SIZE;i++) {
    jsVars[i].this = (JsVarRef)(i+1);
    jsVars[i].refs = JSVAR_CACHE_UNUSED_REF;
    jsVars[i].nextSibling = (JsVarRef)(i+2);
  }
  jsVars[JSVAR_CACHE_SIZE-1].nextSibling = (JsVarRef)(i+2);
  jsVarFirstEmpty = 1;
}

void jsvKill() {
}

// Get number of memory records (JsVars) used
int jsvGetMemoryUsage() {
  int usage = 0;
  int i;
  for (i=1;i<JSVAR_CACHE_SIZE;i++)
    if (jsVars[i].refs != JSVAR_CACHE_UNUSED_REF)
      usage++;
  return usage;
}

// Show what is still allocated, for debugging memory problems
void jsvShowAllocated() {
  int i;
  for (i=1;i<JSVAR_CACHE_SIZE;i++)
    if (jsVars[i].refs != JSVAR_CACHE_UNUSED_REF) {
      jsPrint("USED VAR #");
      jsPrintInt(jsVars[i].this);
      jsPrint(":");
      jsvTrace(jsVars[i].this, 2);
    }
}

JsVar *jsvNew() {
  if (jsVarFirstEmpty!=0) {
      JsVar *v = jsvLock(jsVarFirstEmpty);
      jsVarFirstEmpty = v->nextSibling; // move our reference to the next in the free list
      assert(v->refs == JSVAR_CACHE_UNUSED_REF);
      // reset it
      v->refs = 0;
      v->locks = 1;
      v->flags = 0;
      v->varData.callback = 0;
      v->firstChild = 0;
      v->lastChild = 0;
      v->prevSibling = 0;
      v->nextSibling = 0;
      // return pointer
      return v;
  }
  jsError("Out of Memory!");
  return 0;
}

void jsvFreePtr(JsVar *var) {
    // we shouldn't be linked from anywhere!
    assert(!var->nextSibling && !var->prevSibling);
    // free!
    var->refs = JSVAR_CACHE_UNUSED_REF;

    // add this to our free list
    var->nextSibling = jsVarFirstEmpty; 
    jsVarFirstEmpty = var->this;

    /* Now, unref children - see jsvar.h comments for how! */
    if (jsvIsString(var) || jsvIsStringExt(var) || jsvIsName(var)) {
      JsVarRef stringDataRef = var->lastChild;
      var->lastChild = 0;
      if (stringDataRef) {
        JsVar *child = jsvLock(stringDataRef);
        assert(jsvIsStringExt(child));
        child->prevSibling = 0; // these pointers may contain rubbish as StringEXT uses them for extra characters
        child->nextSibling = 0;
        jsvUnRef(child);
        jsvUnLock(child);
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
          jsvUnLock(child);
        }
      } else {
        // StringExts use firstChild for character data, so ignore them
        assert(jsvIsStringExt(var) || !var->firstChild);
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
        jsvUnLock(child);
      }
    } else {
      assert(!var->firstChild);
      assert(!var->lastChild);
    }
  }

//int c = 0;

JsVar *jsvLock(JsVarRef ref) {
  JsVar *var;
  assert(ref);
  var = &jsVars[ref-1];
  var->locks++;
  return var;
}

JsVarRef jsvUnLock(JsVar *var) {
  JsVarRef ref;
  if (!var) return 0;
  ref = var->this;
  assert(var->locks>0);
  var->locks--;
  if (var->locks == 0 && var->refs==0)
    jsvFreePtr(var);
  return ref;
}

JsVar *jsvRef(JsVar *v) {
  assert(v);
  v->refs++;
  return v;
}

void jsvUnRef(JsVar *var) {
  assert(var);
  assert(var->refs>0);
  var->refs--;
  if (var->locks == 0 && var->refs==0)
      jsvFreePtr(var);
}

JsVarRef jsvRefRef(JsVarRef ref) {
  JsVar *v;
  assert(ref);
  v = jsvLock(ref);
  jsvRef(v);
  jsvUnLock(v);
  return ref;
}
JsVarRef jsvUnRefRef(JsVarRef ref) {
  JsVar *v;
  assert(ref);
  v = jsvLock(ref);
  jsvUnRef(v);
  jsvUnLock(v);
  return 0;
}

JsVarRef jsvGetRef(JsVar *var) {
    if (!var) return 0;
    return var->this;
}

JsVar *jsvNewFromString(const char *str) {
  JsVar *var;
  // Create a var
  JsVar *first = jsvNew();
  if (!first) {
    jsWarn("Truncating string as not enough memory");
    return 0;
  }
  // Now we copy the string, but keep creating new jsVars if we go
  // over the end
  var = jsvLock(jsvGetRef(first));
  var->flags = JSV_STRING;
  var->varData.str[0] = 0; // in case str is empty!

  while (*str) {
    int i;
    // copy data in
    for (i=0;i<jsvGetMaxCharactersInVar(var);i++) {
      var->varData.str[i] = *str;
      if (*str) str++;
    }
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (*str) {
      JsVar *next = jsvNew();
      if (!next) {
        jsWarn("Truncating string as not enough memory");
        jsvUnLock(var);
        return first;
      }
      next = jsvRef(next);
      next->flags = JSV_STRING_EXT;
      var->lastChild = next->this;
      jsvUnLock(var);
      var = next;
    }
  }
  jsvUnLock(var);
  // return
  return first;
}

JsVar *jsvNewFromLexer(struct JsLex *lex, int charFrom, int charTo) {
  JsVar *first;
  JsVar *var;
  // Create a new STRING from part of the lexer
  // Create a new lexer to span the whole area
  // OPT: probably not the fastest, but safe. Only done when a new function is created anyway
  JsLex newLex;
  JsVar *sourceVar = jsvLock(lex->sourceVarRef);
  jslInit(&newLex, sourceVar, charFrom, charTo);
  jsvUnLock(sourceVar);
  // Reset (we must do this because normally it tries to get a new token)
  jslSeek(&newLex, newLex.sourceStartPos);
  jslGetNextCh(&newLex);
  jslGetNextCh(&newLex);

  // Create a var
  first = jsvNew();
  // Now we copy the string, but keep creating new jsVars if we go
  // over the end
  var = first;
  var->flags = JSV_STRING;
  var->varData.str[0] = 0; // in case str is empty!


  while (newLex.currCh) {
    int i;
    // copy data in
    for (i=0;i<jsvGetMaxCharactersInVar(var);i++) {
      var->varData.str[i] = newLex.currCh;
      if (newLex.currCh) jslGetNextCh(&newLex);
    }
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (newLex.currCh) {
      JsVar *next = jsvRef(jsvNew());
      next->flags = JSV_STRING_EXT;
      var->lastChild = next->this;
      if (var!=first) jsvUnLock(var);
      var = next;
    }
  }
  // free
  if (var!=first) jsvUnLock(var);
  jslKill(&newLex);
  // return
  return first;
}

JsVar *jsvNewWithFlags(JsVarFlags flags) {
  JsVar *var = jsvNew();
  if (!var) return 0; // no memory
  var->flags = flags;
  return var;
}
JsVar *jsvNewFromInteger(JsVarInt value) {
  JsVar *var = jsvNew();
  if (!var) return 0; // no memory
  var->flags = JSV_INTEGER;
  var->varData.integer = value;
  return var;
}
JsVar *jsvNewFromBool(bool value) {
  JsVar *var = jsvNew();
  if (!var) return 0; // no memory
  var->flags = JSV_INTEGER;
  var->varData.integer = value ? 1 : 0;
  return var;
}
JsVar *jsvNewFromFloat(JsVarFloat value) {
  JsVar *var = jsvNew();
  if (!var) return 0; // no memory
  var->flags = JSV_FLOAT;
  var->varData.floating = value;
  return var;
}
JsVar *jsvMakeIntoVariableName(JsVar *var, JsVarRef valueOrZero) {
  if (!var) return 0;
  assert(var->refs==0); // make sure it's unused
  var->flags |= JSV_NAME;
  if (valueOrZero)
    var->firstChild = jsvRefRef(valueOrZero);
  return var;
}

INLINE_FUNC bool jsvIsInt(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_INTEGER; }
INLINE_FUNC bool jsvIsFloat(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_FLOAT; }
INLINE_FUNC bool jsvIsString(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_STRING; }
INLINE_FUNC bool jsvIsStringExt(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_STRING_EXT; }
INLINE_FUNC bool jsvIsNumeric(const JsVar *v) { return v && (v->flags&JSV_NUMERICMASK)!=0; }
INLINE_FUNC bool jsvIsFunction(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_FUNCTION; }
INLINE_FUNC bool jsvIsFunctionParameter(const JsVar *v) { return v && (v->flags&JSV_FUNCTION_PARAMETER) == JSV_FUNCTION_PARAMETER; }
INLINE_FUNC bool jsvIsObject(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_OBJECT; }
INLINE_FUNC bool jsvIsArray(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_ARRAY; }
INLINE_FUNC bool jsvIsNative(const JsVar *v) { return v && (v->flags&JSV_NATIVE)!=0; }
INLINE_FUNC bool jsvIsUndefined(const JsVar *v) { return !v; }
INLINE_FUNC bool jsvIsNull(const JsVar *v) { return v && (v->flags&JSV_VARTYPEMASK)==JSV_NULL; }
INLINE_FUNC bool jsvIsBasic(const JsVar *v) { return jsvIsNumeric(v) || jsvIsString(v);} ///< Is this *not* an array/object/etc
INLINE_FUNC bool jsvIsName(const JsVar *v) { return v && (v->flags & JSV_NAME)!=0; }
INLINE_FUNC bool jsvHasCharacterData(const JsVar *v) { return jsvIsString(v) || jsvIsStringExt(v) || jsvIsFunctionParameter(v); } // does the v->data union contain character data?
/// This is the number of characters a JsVar can contain, NOT string length
INLINE_FUNC bool jsvGetMaxCharactersInVar(const JsVar *v) {
    if (jsvIsStringExt(v)) return JSVAR_DATA_STRING_LEN + sizeof(JsVarRef)*3;
    assert(jsvHasCharacterData(v));
    return JSVAR_DATA_STRING_LEN;
}

bool jsvIsBasicVarEqual(JsVar *a, JsVar *b) {
  // OPT: would this be useful as compare instead?
  assert(jsvIsBasic(a) && jsvIsBasic(b));
  if (jsvIsNumeric(a) && jsvIsNumeric(b)) {
    if (jsvIsInt(a)) {
      if (jsvIsInt(b)) {
        return a->varData.integer == b->varData.integer;
      } else {
        assert(jsvIsFloat(b));
        return a->varData.integer == b->varData.floating;
      }
    } else {
      assert(jsvIsFloat(a));
      if (jsvIsInt(b)) {
        return a->varData.floating == b->varData.integer;
      } else {
        assert(jsvIsFloat(b));
        return a->varData.floating == b->varData.floating;
      }
    }
  } else if (jsvIsString(a) && jsvIsString(b)) {
    int i;
    JsVar *va = a;
    JsVar *vb = b;
    while (true) {
      JsVarRef var, vbr;
      for (i=0;i<jsvGetMaxCharactersInVar(va);i++) {
        if (va->varData.str[i] != vb->varData.str[i]) return false;
        if (!va->varData.str[i]) return true; // equal, but end of string
      }
      // we're at the end of this, but are still ok. Move on
      // to STRING_EXTs
      var = a->lastChild;
      vbr = b->lastChild;
      if ((var==0) && (vbr==0)) return true; // both ended
      if ((var==0) != (vbr==0)) return false; // one longer than the other
      if (va!=a) jsvUnLock(va);
      if (vb!=b) jsvUnLock(vb);
      va = jsvLock(var);
      vb = jsvLock(vbr);
      // all done - keep going!
    }
    // we never get here
  } else {
    //TODO: are there any other combinations we should check here?? String v int?
    return false;
  }
}

/// Save this var as a string to the given buffer
void jsvGetString(JsVar *v, char *str, size_t len) {
    if (jsvIsUndefined(v)) {
          strncpy(str, "undefined", len);
    } else if (jsvIsInt(v)) {
      itoa(v->varData.integer, str, 10); 
    } else if (jsvIsFloat(v)) {
      ftoa(v->varData.floating, str);
    } else if (jsvIsNull(v)) {
      strncpy(str, "null", len);
    } else if (jsvIsString(v) || jsvIsStringExt(v) || jsvIsName(v) || jsvIsFunctionParameter(v)) {
      JsVar *var = v;
      JsVarRef ref = 0;
      if (jsvIsStringExt(v))
        jsWarn("INTERNAL: Calling jsvGetString on a JSV_STRING_EXT");
      // print the string - we have to do it a block
      // at a time!
      while (var) {
        JsVarRef refNext;
        int i;
        for (i=0;i<jsvGetMaxCharactersInVar(var);i++) {
          if (len--<=0) {
            *str = 0;
            jsWarn("jsvGetString overflowed\n");
            if (ref) jsvUnLock(var); // Note use of if (ref), not var
            return;
          }
          *(str++) = var->varData.str[i];
        }
        // Go to next
        refNext = var->lastChild;
        if (ref) jsvUnLock(var); // Note use of if (ref), not var
        ref = refNext;
        var = ref ? jsvLock(ref) : 0;
      }
      if (ref) jsvUnLock(var); // Note use of if (ref), not var
      // if it has not had a 0 appended, do it now...
      if (str[-1]) *str = 0;
    } else if (jsvIsFunction(v)) {
      strncpy(str, "function", len);
    } else assert(0);
}

int jsvGetStringLength(JsVar *v) {
  int strLength = 0;
  JsVar *var = v;
  JsVarRef ref = 0;

  if (!jsvIsString(v)) return 0;

  while (var) {
    JsVarRef refNext = var->lastChild;

    if (refNext!=0) {
      // we have more, so this section MUST be full
      strLength += jsvGetMaxCharactersInVar(var);
    } else {
      int i;
      // count
      for (i=0;i<jsvGetMaxCharactersInVar(var);i++) {
        if (var->varData.str[i])
          strLength++;
        else
          break;
      }
    }

    // Go to next
    if (ref) jsvUnLock(var); // note use of if (ref), not var
    ref = refNext;
    var = ref ? jsvLock(ref) : 0;
  }
  if (ref) jsvUnLock(var); // note use of if (ref), not var
  return strLength;
}

void jsvAppendString(JsVar *var, const char *str) {
  JsVar *block = jsvLock(jsvGetRef(var));
  int blockChars;
  assert(jsvIsString(var));
  // Find the block at end of the string...
  while (block->lastChild) {
    JsVarRef next = block->lastChild;
    jsvUnLock(block);
    block = jsvLock(next);
  }
  // find how full the block is
  blockChars=0;
  while (blockChars<jsvGetMaxCharactersInVar(block) && block->varData.str[blockChars])
        blockChars++;
  // now start appending
  while (*str) {
    int i;
    // copy data in
    for (i=blockChars;i<jsvGetMaxCharactersInVar(block);i++) {
      block->varData.str[i] = *str;
      if (*str) str++;
    }
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (*str) {
      JsVar *next = jsvRef(jsvNew());
      next->flags = JSV_STRING_EXT;
      block->lastChild = next->this;
      jsvUnLock(block);
      block = next;
      blockChars=0; // it's new, so empty
    }
  }
  jsvUnLock(block);
}

INLINE_FUNC JsVarInt jsvGetInteger(const JsVar *v) {
    if (!v) return 0;
    /* strtol understands about hex and octal */
    if (jsvIsInt(v)) return v->varData.integer;
    if (jsvIsNull(v)) return 0;
    if (jsvIsUndefined(v)) return 0;
    if (jsvIsFloat(v)) return (JsVarInt)v->varData.floating;
    return 0;
}

INLINE_FUNC bool jsvGetBool(const JsVar *v) {
  return jsvGetInteger(v)!=0;
}

INLINE_FUNC JsVarFloat jsvGetDouble(const JsVar *v) {
    if (!v) return 0;
    if (jsvIsFloat(v)) return v->varData.floating;
    if (jsvIsInt(v)) return (JsVarFloat)v->varData.integer;
    if (jsvIsNull(v)) return 0;
    if (jsvIsUndefined(v)) return 0;
    return 0; /* or NaN? */
}

INLINE_FUNC JsVarInt jsvGetIntegerSkipName(JsVar *v) {
    JsVar *a = jsvSkipName(v);
    JsVarInt l = jsvGetInteger(a);
    jsvUnLock(a);
    return l;
}

INLINE_FUNC bool jsvGetBoolSkipName(JsVar *v) {
  return jsvGetIntegerSkipName(v)!=0;
}

// Also see jsvIsBasicVarEqual
bool jsvIsStringEqual(JsVar *var, const char *str) {
  int i;
  JsVar *v = jsvLock(jsvGetRef(var));
  assert(jsvIsBasic(v) || jsvHasCharacterData(v));
  if (!jsvHasCharacterData(v)) return 0; // not a string so not equal!

  while (true) {
    JsVarRef next;
    for (i=0;i<jsvGetMaxCharactersInVar(v);i++) {
       if (v->varData.str[i] != *str) { jsvUnLock(v); return false; }
       if  (*str==0) { jsvUnLock(v); return true; } // end of string, all great!
       str++;
    }
    // End of what is built in, but keep going!
    next = v->lastChild;
    if  (*str==0) { // end of input string
      jsvUnLock(v);
      return next==0;
    }
    // if we have more data then they are not equal!
    jsvUnLock(v);
    if  (!next) return false; // end of this string, but not the input!
    v = jsvLock(next);
  }
  // never get here, but the compiler warns...
  return true;
}

/** Copy only a name, not what it points to */
JsVar *jsvCopyNameOnly(JsVar *src) {
  JsVar *dst = jsvNewWithFlags(src->flags);
  assert(jsvIsName(src));
  memcpy(&dst->varData, &src->varData, sizeof(JsVarData));

  dst->lastChild = 0;
  dst->firstChild = 0;
  dst->prevSibling = 0;
  dst->nextSibling = 0;
  // Copy LINK of what it points to
  if (src->firstChild)
    dst->firstChild = jsvRefRef(src->firstChild);
  // Copy extra string data if there was any
  if (jsvIsString(src)) {
      // copy extra bits of string if there were any
      if (src->lastChild) {
        JsVar *child = jsvLock(src->lastChild);
        dst->lastChild = jsvUnLock(jsvRef(jsvCopy(child)));
        jsvUnLock(child);
      }
  } else assert(jsvIsBasic(src)); // in case we missed something!
  return dst;
}

JsVar *jsvCopy(JsVar *src) {
  JsVar *dst = jsvNewWithFlags(src->flags);
  memcpy(&dst->varData, &src->varData, sizeof(JsVarData));

  dst->lastChild = 0;
  dst->firstChild = 0;
  dst->prevSibling = 0;
  dst->nextSibling = 0;

  // Copy what names point to
  if (jsvIsName(src)) {
    if (src->firstChild) {
      JsVar *child = jsvLock(src->firstChild);
      dst->firstChild = jsvUnLock(jsvRef(jsvCopy(child)));
      jsvUnLock(child);
    }
  }

  if (jsvIsString(src) || jsvIsStringExt(src)) {
    // copy extra bits of string if there were any
    if (src->lastChild) {
      JsVar *child = jsvLock(src->lastChild);
      dst->lastChild = jsvUnLock(jsvRef(jsvCopy(child)));
      jsvUnLock(child);
    }
  } else if (jsvIsObject(src) || jsvIsFunction(src)) {
    // Copy children..
    JsVarRef vr;
    vr = src->firstChild;
    while (vr) {
      JsVar *name = jsvLock(vr);
      JsVar *child = jsvCopyNameOnly(name); // NO DEEP COPY!
      jsvAddName(jsvGetRef(dst), jsvGetRef(child));
      jsvUnLock(child);
      vr = name->nextSibling;
      jsvUnLock(name);
    }
  } else assert(jsvIsBasic(src)); // in case we missed something!

  return dst;
}

void jsvAddName(JsVarRef parent, JsVarRef namedChildRef) {
  JsVar *v;
  JsVar *namedChild = jsvRef(jsvLock(namedChildRef));
  assert(jsvIsName(namedChild));
  v = jsvLock(parent);
  if (v->lastChild) {
    // Link 2 children together
    JsVar *lastChild = jsvLock(v->lastChild);
    lastChild->nextSibling = namedChildRef;
    jsvUnLock(lastChild);

    namedChild->prevSibling = v->lastChild;
    // finally set the new child as the last one
    v->lastChild = namedChildRef;
  } else {
    v->firstChild = namedChildRef;
    v->lastChild = namedChildRef;
  }
  jsvUnLock(namedChild);
  jsvUnLock(v);
}

JsVar *jsvAddNamedChild(JsVarRef parent, JsVarRef child, const char *name) {
  JsVar *namedChild = jsvMakeIntoVariableName(jsvNewFromString(name), child);
  jsvAddName(parent, jsvGetRef(namedChild));
  return namedChild;
}

JsVar *jsvSetValueOfName(JsVar *name, JsVar *src) {
  assert(name && jsvIsName(name));
  assert(name!=src); // no infinite loops!
  // all is fine, so replace the existing child...
  /* Existing child may be null in the case of Z = 0 where
   * we create 'Z' and pass it down to '=' to have the value
   * filled in (or it may be undefined). */
  if (name->firstChild) jsvUnRefRef(name->firstChild); // free existing
  if (src) {
      assert(!jsvIsName(src)); // ensure not linking to a name!
      name->firstChild = jsvGetRef(jsvRef(src));
  } else
      name->firstChild = 0;
  return name;
}

JsVar *jsvFindChildFromString(JsVarRef parentref, const char *name, bool createIfNotFound) {
  JsVar *parent = jsvLock(parentref);
  JsVar *child;
  JsVarRef childref = parent->firstChild;
  while (childref) {

    child = jsvLock(childref);
    if (jsvIsStringEqual(child, name)) {
       // found it! unlock parent but leave child locked
       jsvUnLock(parent);
       return child;
    }
    childref = child->nextSibling;
    jsvUnLock(child);
  }

  child = 0;
  if (createIfNotFound) {
    child = jsvMakeIntoVariableName(jsvNewFromString(name), 0);
    jsvAddName(parentref, child->this);
  }
  jsvUnLock(parent);
  return child;
}

/** Non-recursive finding */
JsVar *jsvFindChildFromVar(JsVarRef parentref, JsVar *childName, bool addIfNotFound) {
  JsVar *parent = jsvLock(parentref);
  JsVar *child;
  JsVarRef childref = parent->firstChild;
  while (childref) {
    child = jsvLock(childref);
    if (jsvIsBasicVarEqual(child, childName)) {
      // found it! unlock parent but leave child locked
      jsvUnLock(parent);
      return child;
    }
    childref = child->nextSibling;
    jsvUnLock(child);
  }

  child = 0;
  if (addIfNotFound) {
    if (childName->refs == 0) {
      // Not reffed - great! let's just use it
      if (!jsvIsName(childName))
        childName = jsvMakeIntoVariableName(childName, 0);
      jsvAddName(parentref, childName->this);
      child = jsvLock(childName->this);
    } else { // it was reffed, we must add a new one
      child = jsvMakeIntoVariableName(jsvCopy(childName), 0);
      jsvAddName(parentref, child->this);
    }
  }
  jsvUnLock(parent);
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
    jsvUnLock(child);
  }
  return children;
}


JsVarInt jsvGetArrayLength(JsVar *v) {
  JsVarInt lastIdx = 0;
  JsVarRef childref = v->firstChild;
  while (childref) {
    JsVarInt childIndex;
    JsVar *child = jsvLock(childref);
    
    assert(jsvIsInt(child));
    childIndex = jsvGetInteger(child);
    if (childIndex>lastIdx) 
        lastIdx = childIndex;
    childref = child->nextSibling;
    jsvUnLock(child);
  }
  return lastIdx+1;
}

JsVar *jsvGetArrayItem(JsVar *arr, int index) {
  JsVarRef childref = arr->firstChild;
  while (childref) {
    JsVarInt childIndex;
    JsVar *child = jsvLock(childref);
    
    assert(jsvIsInt(child));
    childIndex = jsvGetInteger(child);
    if (childIndex == index) {
      JsVar *item = jsvLock(child->firstChild);
      jsvUnLock(child);
      return item;
    }
    childref = child->nextSibling;
    jsvUnLock(child);
  }
  return 0; // undefined
}

/// Get the index of the value in the array
JsVar *jsvGetArrayIndexOf(JsVar *arr, JsVar *value) {
  JsVarRef indexref;
  assert(jsvIsArray(arr) || jsvIsObject(arr));
  indexref = arr->firstChild;
  while (indexref) {
    JsVar *childIndex = jsvLock(indexref);
    assert(jsvIsName(childIndex))
    JsVar *childValue = jsvLock(childIndex->firstChild);
    if (jsvIsBasicVarEqual(childValue, value)) {
      jsvUnLock(childValue);
      return childIndex;
    }
    jsvUnLock(childValue);
    indexref = childIndex->nextSibling;
    jsvUnLock(childIndex);
  }
  return 0; // undefined
}


/** If a is a name skip it and go to what it points to.
 * ALWAYS locks - so must unlock what it returns. It MAY
 * return 0.  */
INLINE_FUNC JsVar *jsvSkipName(JsVar *a) {
  JsVar *pa = a;
  if (!a) return a;
  while (jsvIsName(pa)) {
    JsVarRef n = pa->firstChild;
    if (pa!=a) jsvUnLock(pa);
    if (!n) return 0;
    pa = jsvLock(n);
  }
  if (pa==a) jsvLock(pa->this);
  return pa;
}

/** Same as jsvSkipName, but ensures that 'a' is unlocked if it was
 * a name, so it can be used INLINE_FUNC */
INLINE_FUNC JsVar *jsvSkipNameAndUnlock(JsVar *a) {
  JsVar *b = jsvSkipName(a);
  jsvUnLock(a);
  return b;
}

/** Same as jsvMathsOpPtr, but if a or b are a name, skip them
 * and go to what they point to. */
JsVar *jsvMathsOpPtrSkipNames(JsVar *a, JsVar *b, int op) {
  JsVar *pa = jsvSkipName(a);
  JsVar *pb = jsvSkipName(b);
  JsVar *res = jsvMathsOpPtr(pa,pb,op);
  jsvUnLock(pa);
  jsvUnLock(pb);
  return res;
}


JsVar *jsvMathsOpError(int op, const char *datatype) {
    char buf[JS_ERROR_BUF_SIZE];
    size_t bufpos = 0;
    strncpy(&buf[bufpos], "Operation ", JS_ERROR_BUF_SIZE-bufpos);
    bufpos=strlen(buf);
    jslTokenAsString(op, &buf[bufpos], JS_ERROR_TOKEN_BUF_SIZE-bufpos);
    bufpos=strlen(buf);
    strncat(&buf[bufpos], " not supported on the  ", JS_ERROR_BUF_SIZE-bufpos);
    bufpos=strlen(buf);
    strncat(&buf[bufpos], datatype, JS_ERROR_BUF_SIZE-bufpos);
    bufpos=strlen(buf);
    strncat(&buf[bufpos], " datatype", JS_ERROR_BUF_SIZE-bufpos);
    jsError(buf);
    return 0;
}

JsVar *jsvMathsOpPtr(JsVar *a, JsVar *b, int op) {
    // Type equality check
    if (op == LEX_TYPEEQUAL || op == LEX_NTYPEEQUAL) {
      // check type first, then call again to check data
      bool eql = (a==0) == (b==0);
      if (a && b) eql = ((a->flags & JSV_VARTYPEMASK) ==
                         (b->flags & JSV_VARTYPEMASK));
      if (eql) {
        JsVar *contents = jsvMathsOpPtr(a,b, LEX_EQUAL);
        if (!jsvGetBool(contents)) eql = false;
        jsvUnLock(contents);
      }
      if (op == LEX_TYPEEQUAL)
        return jsvNewFromBool(eql);
      else
        return jsvNewFromBool(!eql);
    }

    // do maths...
    if ((jsvIsUndefined(a) || jsvIsNull(a)) && (jsvIsUndefined(b) || jsvIsNull(b))) {
      if (op == LEX_EQUAL)
        return jsvNewFromBool(true);
      else if (op == LEX_NEQUAL)
        return jsvNewFromBool(false);
      else
        return 0; // undefined
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
                case LEX_RSHIFTUNSIGNED: return jsvNewFromInteger((JsVarInt)(((JsVarIntUnsigned)da) >> db));
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
    } else if (jsvIsArray(a) || jsvIsObject(a) ||
               jsvIsArray(b) || jsvIsObject(b)) {
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
             // FIXME we need to do string add properly
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
}

JsVar *jsvMathsOp(JsVarRef ar, JsVarRef br, int op) {
    JsVar *a;
    JsVar *b;
    JsVar *res;
    if (!ar || !br) return 0;
    a = jsvLock(ar);
    b = jsvLock(br);
    res = jsvMathsOpPtr(a,b,op);
    jsvUnLock(a);
    jsvUnLock(b);
    return res;
}

void jsvTraceLockInfo(JsVar *v) {
    jsPrint("#");
    jsPrintInt(jsvGetRef(v));
    jsPrint("[r");
    jsPrintInt(v->refs);
    jsPrint(",l");
    jsPrintInt(v->locks-1);
    jsPrint("] ");
}

/** Write debug info for this Var out to the console */
void jsvTrace(JsVarRef ref, int indent) {
#ifndef SDCC
    int i;
    char buf[JS_ERROR_BUF_SIZE];
    JsVar *var;

    for (i=0;i<indent;i++) jsPrint(" ");

    if (!ref) {
        jsPrint("undefined\n");
        return;
    }
    var = jsvLock(ref);
    jsvTraceLockInfo(var);


    if (jsvIsName(var)) {
      jsvGetString(var, buf, JS_ERROR_BUF_SIZE);
      if (jsvIsInt(var)) {
          jsPrint("Name: int ");
          jsPrint(buf);
          jsPrint("  ");
      } else if (jsvIsFloat(var)) {
          jsPrint("Name: flt ");
          jsPrint(buf);
          jsPrint("  ");
      } else if (jsvIsString(var) || jsvIsFunctionParameter(var)) {
          jsPrint("Name: '");
          jsPrint(buf);
          jsPrint("'  ");
      } else assert(0);
      // go to what the name points to
      ref = var->firstChild;
      jsvUnLock(var);
      if (ref) {
        var = jsvLock(ref);
        jsvTraceLockInfo(var);
      } else {
          jsPrint("undefined\n");
        return;
      }
    }
    if (jsvIsName(var)) {
        jsPrint("\n");
      jsvTrace(var->this, indent+1);
      jsvUnLock(var);
      return;
    }
    if (jsvIsObject(var)) jsPrint("Object {\n");
    else if (jsvIsArray(var)) jsPrint("Array [\n");
    else if (jsvIsInt(var)) jsPrint("Integer ");
    else if (jsvIsFloat(var)) jsPrint("Double ");
    else if (jsvIsString(var)) jsPrint("String ");
    else if (jsvIsFunction(var)) jsPrint("Function {\n");
    else {
        jsPrint("Flags ");
        jsPrintInt(var->flags);
        jsPrint("\n");
    }

    if (!jsvIsObject(var) && !jsvIsArray(var) && !jsvIsFunction(var)) {
      jsvGetString(var, buf, JS_ERROR_BUF_SIZE);
      jsPrint(buf);
    }

    if (jsvIsString(var) || jsvIsName(var)) {
      JsVarRef child = var->firstChild;
      if (child) {
          jsPrint("( Multi-block string ");
        while (child) {
          JsVar *childVar = jsvLock(child);
          jsvTraceLockInfo(childVar);
          child = childVar->firstChild;
          jsvUnLock(childVar);
        }
        jsPrint(")\n");
      } else
          jsPrint("\n");
    } else {
      JsVarRef child = var->firstChild;
      jsPrint("\n");
      // dump children
      while (child) {
        JsVar *childVar;
        jsvTrace(child, indent+1);
        childVar = jsvLock(child);
        child = childVar->nextSibling;
        jsvUnLock(childVar);
      }
    }


    if (jsvIsObject(var) || jsvIsFunction(var)) {
      int i;
      for (i=0;i<indent;i++) jsPrint(" ");
      jsPrint("}\n");
    } else if (jsvIsArray(var)) {
      int i;
      for (i=0;i<indent;i++) jsPrint(" ");
      jsPrint("]\n");
    }

    jsvUnLock(var);
#endif
}
