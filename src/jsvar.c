/*
 * jsvar.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */
//#define INLINE_FUNC inline
#include "jsvar.h"
#include "jslex.h"
#include "jsparse.h"

#define JSVAR_CACHE_UNUSED_REF 0xFFFF
JsVar jsVars[JSVAR_CACHE_SIZE];
int jsVarsSize = JSVAR_CACHE_SIZE;
JsVarRef jsVarFirstEmpty; ///< reference of first unused variable

void *jsvGetVarDataPointer() { return &jsVars[0]; }
int jsvGetVarDataSize() { return sizeof(jsVars); }

// For debugging/testing ONLY - maximum # of vars we are allowed to use
void jsvSetMaxVarsUsed(int size) {
  assert(size < JSVAR_CACHE_SIZE);
  jsVarsSize = size;
}


// maps the empty variables in...
void jsvSoftInit() {
  int i;
  jsVarFirstEmpty = 0;
  JsVar *lastEmpty = 0;
  for (i=0;i<jsVarsSize;i++) {
    if (jsVars[i].refs == JSVAR_CACHE_UNUSED_REF) {
      jsVars[i].nextSibling = 0;
      if (lastEmpty)
        lastEmpty->nextSibling = jsvGetRef(&jsVars[i]);
      else
        jsVarFirstEmpty = jsvGetRef(&jsVars[i]);
      lastEmpty = &jsVars[i];
    }
  }
}

void jsvSoftKill() {
}

void jsvInit() {
  int i;
  for (i=0;i<jsVarsSize;i++) {
    jsVars[i].flags = JSV_UNUSED;
#ifdef LARGE_MEM
    jsVars[i].this = (JsVarRef)(i+1);
#endif
    jsVars[i].refs = JSVAR_CACHE_UNUSED_REF;
    jsVars[i].locks = 0;
    jsVars[i].nextSibling = (JsVarRef)(i+2);
  }
  jsVars[jsVarsSize-1].nextSibling = 0;
  jsVarFirstEmpty = 1;
  jsvSoftInit();
}

void jsvKill() {
}

/** Find or create the ROOT variable item - used mainly
 * if recovering from a saved state. */
JsVar *jsvFindOrCreateRoot() {
  int i;

  for (i=0;i<jsVarsSize;i++)
    if (jsVars[i].flags==JSV_ROOT)
      return jsvLockAgain(&jsVars[i]);

  return jsvRef(jsvNewWithFlags(JSV_ROOT));
}

// Get number of memory records (JsVars) used
int jsvGetMemoryUsage() {
  int usage = 0;
  int i;
  for (i=1;i<jsVarsSize;i++)
    if (jsVars[i].refs != JSVAR_CACHE_UNUSED_REF)
      usage++;
  return usage;
}

/// Get whether memory is full or not
bool jsvIsMemoryFull() {
  return !jsVarFirstEmpty;
}

// Show what is still allocated, for debugging memory problems
void jsvShowAllocated() {
  int i;
  for (i=1;i<jsVarsSize;i++)
    if (jsVars[i].refs != JSVAR_CACHE_UNUSED_REF) {
      jsiConsolePrint("USED VAR #");
      jsiConsolePrintInt(jsvGetRef(&jsVars[i]));
      jsiConsolePrint(":");
      jsvTrace(jsvGetRef(&jsVars[i]), 2);
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
  jspSetInterrupted(true);
  return 0;
}

void jsvFreeLoopedRefPtr(JsVar *var); // forward decl

void jsvFreePtr(JsVar *var) {
    // we shouldn't be linked from anywhere!
    assert(!var->nextSibling && !var->prevSibling);

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
          if (child->refs > 0 && child->locks==1 && jsvGetRefCount(child, child)==child->refs)
              jsvFreeLoopedRefPtr(child);
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
        if (child->refs > 0 && child->locks==1 && jsvGetRefCount(child, child)==child->refs)
            jsvFreeLoopedRefPtr(child);
        jsvUnLock(child);
      }
    } else {
      assert(!var->firstChild);
      assert(!var->lastChild);
    }

    // free!
    var->refs = JSVAR_CACHE_UNUSED_REF;
    // add this to our free list
    var->nextSibling = jsVarFirstEmpty;
    jsVarFirstEmpty = jsvGetRef(var);
}

// Just for debugging so we can see when something has been freed in a non-normal way.
void jsvFreeLoopedRefPtr(JsVar *var) {
    //printf("jsvFreeLoopedRefPtr refs %d\n", var->refs);
    //jsvTrace(jsvGetRef(var), 2);
    jsvFreePtr(var);
}

JsVar *jsvNewFromString(const char *str) {
  JsVar *var;
  // Create a var
  JsVar *first = jsvNew();
  if (!first) {
    jsWarn("Unable to create string as not enough memory");
    return 0;
  }
  // Now we copy the string, but keep creating new jsVars if we go
  // over the end
  var = jsvLockAgain(first);
  var->flags = JSV_STRING;
  var->varData.str[0] = 0; // in case str is empty!

  while (*str) {
    int i;
    // copy data in
    for (i=0;i<(int)jsvGetMaxCharactersInVar(var);i++) {
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
      var->lastChild = jsvGetRef(next);
      jsvUnLock(var);
      var = next;
    }
  }
  jsvUnLock(var);
  // return
  return first;
}

JsVar *jsvNewFromLexer(struct JsLex *lex, int charFrom, int charTo) {
  // Create a var
  JsVar *first = jsvNew();
  if (!first) { // out of memory
    return 0;
  }

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

  // Now we copy the string, but keep creating new jsVars if we go
  // over the end
  var = jsvLockAgain(first);
  var->flags = JSV_STRING;
  var->varData.str[0] = 0; // in case str is empty!


  while (newLex.currCh) {
    int i;
    // copy data in
    for (i=0;i<(int)jsvGetMaxCharactersInVar(var);i++) {
      var->varData.str[i] = newLex.currCh;
      if (newLex.currCh) jslGetNextCh(&newLex);
    }
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (newLex.currCh) {
      JsVar *next = jsvNew();
      if (!next) break; // out of memory
      next = jsvRef(next);
      next->flags = JSV_STRING_EXT;
      var->lastChild = jsvGetRef(next);
      jsvUnLock(var);
      var = next;
    }
  }
  // free
  jsvUnLock(var);
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
JsVar *jsvMakeIntoVariableName(JsVar *var, JsVar *valueOrZero) {
  if (!var) return 0;
  assert(var->refs==0); // make sure it's unused
  var->flags |= JSV_NAME;
  if (valueOrZero)
    var->firstChild = jsvGetRef(jsvRef(valueOrZero));
  return var;
}

/// Lock this reference and return a pointer - UNSAFE for null refs
JsVar *jsvLock(JsVarRef ref) {
  assert(ref);
  JsVar *var = &jsVars[ref-1];
  var->locks++;
  if (var->locks==0) {
    jsError("Too many locks to Variable!");
    //jsPrint("Var #");jsPrintInt(ref);jsPrint("\n");
  }
  return var;
}

/// Lock this pointer and return a pointer - UNSAFE for null pointer
JsVar *jsvLockAgain(JsVar *var) {
  var->locks++;
  if (var->locks==0) {
    jsError("Too many locks to Variable!");
    //jsPrint("Var #");jsPrintInt(ref);jsPrint("\n");
  }
  return var;
}

/// Unlock this variable - this is SAFE for null variables
JsVarRef jsvUnLock(JsVar *var) {
  JsVarRef ref;
  if (!var) return 0;
  ref = jsvGetRef(var);
  assert(var->locks>0);
  var->locks--;
  if (var->locks == 0 && var->refs==0)
    jsvFreePtr(var);
  return ref;
}


bool jsvIsBasicVarEqual(JsVar *a, JsVar *b) {
  // quick checks
  if (a==b) return true;
  if (!a || !b) return false; // one of them is undefined
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
      for (i=0;i<(int)jsvGetMaxCharactersInVar(va);i++) {
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

bool jsvIsEqual(JsVar *a, JsVar *b) {
  if (jsvIsBasic(a) && jsvIsBasic(b))
    return jsvIsBasicVarEqual(a,b);
  return jsvGetRef(a)==jsvGetRef(b);
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
      JsVar *var = jsvLockAgain(v);
      JsVarRef ref = 0;
      if (jsvIsStringExt(v))
        jsWarn("INTERNAL: Calling jsvGetString on a JSV_STRING_EXT");
      // print the string - we have to do it a block
      // at a time!
      while (var) {
        JsVarRef refNext;
        int i;
        for (i=0;i<(int)jsvGetMaxCharactersInVar(var);i++) {
          if (len--<=0) {
            *str = 0;
            jsWarn("jsvGetString overflowed\n");
            jsvUnLock(var); // Note use of if (ref), not var
            return;
          }
          *(str++) = var->varData.str[i];
        }
        // Go to next
        refNext = var->lastChild;
        jsvUnLock(var);
        ref = refNext;
        var = ref ? jsvLock(ref) : 0;
      }
      jsvUnLock(var);
      // if it has not had a 0 appended, do it now...
      if (str[-1]) *str = 0;
    } else if (jsvIsFunction(v)) {
      strncpy(str, "function", len);
    } else {
      assert(0);
    }
}

size_t jsvGetStringLength(JsVar *v) {
  size_t strLength = 0;
  JsVar *var = v;
  JsVarRef ref = 0;

  if (!jsvIsString(v)) return 0;

  while (var) {
    JsVarRef refNext = var->lastChild;

    if (refNext!=0) {
      // we have more, so this section MUST be full
      strLength += jsvGetMaxCharactersInVar(var);
    } else {
      size_t i; 
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
  JsVar *block = jsvLockAgain(var);
  unsigned int blockChars;
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
    size_t i;
    // copy data in
    for (i=blockChars;i<jsvGetMaxCharactersInVar(block);i++) {
      block->varData.str[i] = *str;
      if (*str) str++;
    }
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (*str) {
      JsVar *next = jsvNew();
      if (!next) break;
      next = jsvRef(next);
      next->flags = JSV_STRING_EXT;
      block->lastChild = jsvGetRef(next);
      jsvUnLock(block);
      block = next;
      blockChars=0; // it's new, so empty
    }
  }
  jsvUnLock(block);
}

void jsvAppendCharacter(JsVar *var, char ch) {
  // Append the character to our input line
  char buf[2];
  buf[0] = ch;
  buf[1] = 0;
  jsvAppendString(var, buf);
}

/** If var is a string, lock and return it, else
 * create a new string. unlockVar means this will auto-unlock 'var'  */
JsVar *jsvAsString(JsVar *var, bool unlockVar) {
  if (jsvIsString(var) || jsvIsName(var)) {
    if (unlockVar) return var;
    return jsvLockAgain(var);
  }
  /* TODO: If this is an array return a string with elements concatenated by '.'
   * TODO: If this is an object, search for 'toString'
   * TODO: Try and do without the string buffer
   */

  char buf[JSVAR_STRING_OP_BUFFER_SIZE];
  jsvGetString(var, buf, JSVAR_STRING_OP_BUFFER_SIZE);
  if (unlockVar) jsvUnLock(var);
  return jsvNewFromString(buf);
}

/** Append str to var. Both must be strings. stridx = start char or str, maxLength = max number of characters.
 *  stridx can be negative to go from end of string */
void jsvAppendStringVar(JsVar *var, JsVar *str, int stridx, int maxLength) {
  assert(jsvIsString(str) || jsvIsName(str));
  str = jsvLockAgain(str);

  JsVar *block = jsvLockAgain(var);
  unsigned int blockChars;
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
  // Now make sure we're in the correct block of str
  if (stridx < 0) stridx += (int)jsvGetStringLength(str);
  while (stridx >= (int)jsvGetMaxCharactersInVar(str)) {
    JsVarRef n = str->lastChild;
    stridx -= (int)jsvGetMaxCharactersInVar(str);
    jsvUnLock(str);
    str = n ? jsvLock(n) : 0;
  }

  // now start appending
  while (str) {
    size_t i;
    // copy data in
    for (i=blockChars;i<jsvGetMaxCharactersInVar(block);i++) {
      char ch = 0;
      if (str) {
        ch = str->varData.str[stridx];
        if (ch && --maxLength>0) {
          stridx++;
          if (stridx >= (int)jsvGetMaxCharactersInVar(str)) {
            JsVarRef n = str->lastChild;
            stridx = 0;
            jsvUnLock(str);
            str = n ? jsvLock(n) : 0;
          }
        } else {
          // we're done with str, deallocate it
          jsvUnLock(str);
          str = 0;
        }
      }
      block->varData.str[i] = ch;
    }
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (str) {
      JsVar *next = jsvNew();
      if (!next) break; // out of memory
      next = jsvRef(next);
      next->flags = JSV_STRING_EXT;
      block->lastChild = jsvGetRef(next);
      jsvUnLock(block);
      block = next;
      blockChars=0; // it's new, so empty
    }
  }
  jsvUnLock(block);
}

/** Append all of str to var. Both must be strings.  */
void jsvAppendStringVarComplete(JsVar *var, JsVar *str) {
  jsvAppendStringVar(var, str, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
}

char jsvGetCharInString(JsVar *v, int idx) {
  if (!jsvIsString(v)) return 0;
  if (idx<0) idx += (int)jsvGetStringLength(v); // <0 goes from end of string
  if (idx<0) return 0;

  v = jsvLockAgain(v);
  while (v && idx >= (int)jsvGetMaxCharactersInVar(v)) {
    JsVarRef next;
    idx -= (int)jsvGetMaxCharactersInVar(v);
    next = v->lastChild;
    jsvUnLock(v);
    v = jsvLock(next);
  }

  char c = 0;
  if (v) {
    c = v->varData.str[idx];
    jsvUnLock(v);
  }
  return c;
}

/// Print the contents of a string var - directly
void jsvPrintStringVar(JsVar *v) {
  assert(jsvIsString(v) || jsvIsName(v));
  JsVarRef r = jsvGetRef(v);
  while (r) {
    v = jsvLock(r);
    size_t l = jsvGetMaxCharactersInVar(v);
    char buf[JSVAR_DATA_STRING_MAX_LEN+1];
    memcpy(buf, v->varData.str, l);
    buf[l] = 0;
    jsiConsolePrint(buf);
    r = v->lastChild;
    jsvUnLock(v);
  }
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

INLINE_FUNC void jsvSetInteger(JsVar *v, JsVarInt value) {
  assert(jsvIsInt(v));
  v->varData.integer  = value;
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
  JsVar *v;
  assert(jsvIsBasic(var) || jsvHasCharacterData(var));
  if (!jsvHasCharacterData(var)) {
    return 0; // not a string so not equal!
  }
  v = jsvLockAgain(var);

  while (true) {
    size_t i;
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


/** Compare 2 strings, starting from the given character positions. equalAtEndOfString means that
 * if one of the strings ends, we treat them as equal.
 * For a basic strcmp, do: jsvCompareString(a,b,0,0,false)
 *  */
int jsvCompareString(JsVar *va, JsVar *vb, int starta, int startb, bool equalAtEndOfString) {
  int idxa = starta;
  int idxb = startb;
  assert(jsvIsString(va) || jsvIsName(va)); // we hope! Might just want to return 0?
  assert(jsvIsString(vb) || jsvIsName(vb)); // we hope! Might just want to return 0?
  va = jsvLockAgain(va);
  vb = jsvLockAgain(vb);

  // step to first positions
  while (true) {
    while (va && idxa >= (int)jsvGetMaxCharactersInVar(va)) {
      JsVarRef n = va->lastChild;
      idxa -= (int)jsvGetMaxCharactersInVar(va);
      jsvUnLock(va);
      va = n ? jsvLock(n) : 0;
    }
    while (vb && idxb >= (int)jsvGetMaxCharactersInVar(vb)) {
      JsVarRef n = vb->lastChild;
      idxb -= (int)jsvGetMaxCharactersInVar(vb);
      jsvUnLock(vb);
      vb = n ? jsvLock(n) : 0;
    }

    char ca = (char) (va ? va->varData.str[idxa] : 0);
    char cb = (char) (vb ? vb->varData.str[idxb] : 0);
    if (ca != cb) {
      jsvUnLock(va);
      jsvUnLock(vb);
      if ((ca==0 || cb==0) && equalAtEndOfString) return 0;
      return ca - cb;
    }
    if (ca == 0) { // end of string - equal!
      jsvUnLock(va);
      jsvUnLock(vb);
      return 0;
    }

    idxa++;
    idxb++;
  }
  // never get here, but the compiler warns...
  return true;
}

/** Copy only a name, not what it points to. ALTHOUGH the link to what it points to is maintained unless linkChildren=false */
JsVar *jsvCopyNameOnly(JsVar *src, bool linkChildren) {
  JsVar *dst = jsvNewWithFlags(src->flags);
  if (!dst) return 0; // out of memory
  assert(jsvIsName(src));
  memcpy(&dst->varData, &src->varData, sizeof(JsVarData));

  dst->lastChild = 0;
  dst->firstChild = 0;
  dst->prevSibling = 0;
  dst->nextSibling = 0;
  // Copy LINK of what it points to
  if (linkChildren && src->firstChild)
    dst->firstChild = jsvRefRef(src->firstChild);
  // Copy extra string data if there was any
  if (jsvIsString(src)) {
      // copy extra bits of string if there were any
      if (src->lastChild) {
        JsVar *child = jsvLock(src->lastChild);
        JsVar *childCopy = jsvCopy(child);
        if (childCopy) // could be out of memory
          dst->lastChild = jsvUnLock(jsvRef(childCopy));
        jsvUnLock(child);
      }
  } else {
    assert(jsvIsBasic(src)); // in case we missed something!
  }
  return dst;
}

JsVar *jsvCopy(JsVar *src) {
  JsVar *dst = jsvNewWithFlags(src->flags);
  if (!dst) return 0; // out of memory
  if (!jsvIsStringExt(src)) {
    memcpy(&dst->varData, &src->varData, sizeof(JsVarData));
    dst->lastChild = 0;
    dst->firstChild = 0;
    dst->prevSibling = 0;
    dst->nextSibling = 0;
  } else {
    // stringexts use the extra pointers after varData to store characters
    // see jsvGetMaxCharactersInVar
    memcpy(&dst->varData, &src->varData, sizeof(JsVarData)+sizeof(JsVarRef)*3);
    dst->lastChild = 0;
  }

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
      JsVar *childCopy = jsvCopy(child);
      if (childCopy) // could be out of memory
        dst->lastChild = jsvUnLock(jsvRef(childCopy));
      jsvUnLock(child);
    }
  } else if (jsvIsObject(src) || jsvIsFunction(src)) {
    // Copy children..
    JsVarRef vr;
    vr = src->firstChild;
    while (vr) {
      JsVar *name = jsvLock(vr);
      JsVar *child = jsvCopyNameOnly(name, true); // NO DEEP COPY!
      if (child) { // could have been out of memory
        jsvAddName(dst, child);
        jsvUnLock(child);
      }
      vr = name->nextSibling;
      jsvUnLock(name);
    }
  } else {
    assert(jsvIsBasic(src)); // in case we missed something!
  }

  return dst;
}

void jsvAddName(JsVar *parent, JsVar *namedChild) {
  namedChild = jsvRef(namedChild);
  assert(jsvIsName(namedChild));
  // TODO: if array, insert in correct order
  if (parent->lastChild) {
    // Link 2 children together
    JsVar *lastChild = jsvLock(parent->lastChild);
    lastChild->nextSibling = jsvGetRef(namedChild);
    jsvUnLock(lastChild);

    namedChild->prevSibling = parent->lastChild;
    // finally set the new child as the last one
    parent->lastChild = jsvGetRef(namedChild);
  } else {
    parent->firstChild = parent->lastChild = jsvGetRef(namedChild);

  }
}

JsVar *jsvAddNamedChild(JsVar *parent, JsVar *child, const char *name) {
  JsVar *namedChild = jsvMakeIntoVariableName(jsvNewFromString(name), child);
  if (!namedChild) return 0; // Out of memory
  jsvAddName(parent, namedChild);
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
    if (child) // could be out of memory
      jsvAddName(parent, child);
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
  if (addIfNotFound && childName) {
    if (childName->refs == 0) {
      // Not reffed - great! let's just use it
      if (!jsvIsName(childName))
        childName = jsvMakeIntoVariableName(childName, 0);
      jsvAddName(parent, childName);
      child = jsvLockAgain(childName);
    } else { // it was reffed, we must add a new one
      child = jsvMakeIntoVariableName(jsvCopy(childName), 0);
      jsvAddName(parent, child);
    }
  }
  jsvUnLock(parent);
  return child;
}

void jsvRemoveChild(JsVar *parent, JsVar *child) {
    assert(jsvIsArray(parent) || jsvIsObject(parent) || jsvIsFunction(parent));
    JsVarRef childref = jsvGetRef(child);
    // unlink from parent
    if (parent->firstChild == childref)
        parent->firstChild = child->nextSibling;
    if (parent->lastChild == childref)
        parent->lastChild = child->prevSibling;
    // unlink from child list
    if (child->prevSibling) {
        JsVar *v = jsvLock(child->prevSibling);
        v->nextSibling = child->nextSibling;
        jsvUnLock(v);
    }
    if (child->nextSibling) {
        JsVar *v = jsvLock(child->nextSibling);
        v->prevSibling = child->prevSibling;
        jsvUnLock(v);
    }
    child->prevSibling = 0;
    child->nextSibling = 0;

    jsvUnRef(child);
}

void jsvRemoveAllChildren(JsVar *parent) {
    assert(jsvIsArray(parent) || jsvIsObject(parent) || jsvIsFunction(parent));
    while (parent->firstChild) {
      JsVar *v = jsvLock(parent->firstChild);
      jsvRemoveChild(parent, v);
      jsvUnLock(v);
    }
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


JsVarInt jsvGetArrayLength(JsVar *arr) {
  JsVarInt lastIdx = 0;
  JsVarRef childref = arr->firstChild;
  // TODO: when everying is in order in an array, we can just look at the last element
  while (childref) {
    JsVarInt childIndex;
    JsVar *child = jsvLock(childref);
    
    assert(jsvIsInt(child));
    childIndex = jsvGetInteger(child)+1;
    if (childIndex>lastIdx) 
        lastIdx = childIndex;
    childref = child->nextSibling;
    jsvUnLock(child);
  }
  return lastIdx;
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

/// Adds new elements to the end of an array, and returns the new length
JsVarInt jsvArrayPush(JsVar *arr, JsVar *value) {
  assert(jsvIsArray(arr));
  JsVarInt index = jsvGetArrayLength(arr);
  JsVar *idx = jsvMakeIntoVariableName(jsvNewFromInteger(index), value);
  if (!idx) {
    jsWarn("Out of memory while appending to array");
    return 0;
  }
  jsvAddName(arr, idx);
  jsvUnLock(idx);
  return index+1; // new size
}

/// Removes the last element of an array, and returns that element (or 0 if empty)
JsVar *jsvArrayPop(JsVar *arr) {
  assert(jsvIsArray(arr));
  if (arr->lastChild) {
    JsVar *child = jsvLock(arr->lastChild);
    if (arr->firstChild == arr->lastChild)
      arr->firstChild = 0; // if 1 item in array
    arr->lastChild = child->prevSibling; // unlink from end of array
    jsvUnRef(child); // as no longer in array
    return child; // and return it
  } else {
    // no children!
    return 0;
  }
}

/** If a is a name skip it and go to what it points to.
 * ALWAYS locks - so must unlock what it returns. It MAY
 * return 0.  */
INLINE_FUNC JsVar *jsvSkipName(JsVar *a) {
  JsVar *pa = a;
  if (!a) return 0;
  while (jsvIsName(pa)) {
    JsVarRef n = pa->firstChild;
    if (pa!=a) jsvUnLock(pa);
    if (!n) return 0;
    pa = jsvLock(n);
  }
  if (pa==a) jsvLockAgain(pa);
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
JsVar *jsvMathsOpSkipNames(JsVar *a, JsVar *b, int op) {
  JsVar *pa = jsvSkipName(a);
  JsVar *pb = jsvSkipName(b);
  JsVar *res = jsvMathsOp(pa,pb,op);
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

JsVar *jsvMathsOp(JsVar *a, JsVar *b, int op) {
    // Type equality check
    if (op == LEX_TYPEEQUAL || op == LEX_NTYPEEQUAL) {
      // check type first, then call again to check data
      bool eql = (a==0) == (b==0);
      if (a && b) eql = ((a->flags & JSV_VARTYPEMASK) ==
                         (b->flags & JSV_VARTYPEMASK));
      if (eql) {
        JsVar *contents = jsvMathsOp(a,b, LEX_EQUAL);
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
       JsVar *da = jsvAsString(a, false);
       JsVar *db = jsvAsString(b, false);
       if (!da || !db) { // out of memory
         jsvUnLock(da);
         jsvUnLock(db);
         return 0;
       }
       if (op=='+') {
         JsVar *v = jsvCopy(da);
         // TODO: can we be fancy and not copy da if we know it isn't reffed? what about locks?
         if (v) // could be out of memory
           jsvAppendStringVar(v, db, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
         jsvUnLock(da);
         jsvUnLock(db);
         return v;
       }

       int cmp = jsvCompareString(da,db,0,0,false);
       jsvUnLock(da);
       jsvUnLock(db);
       // use strings
       switch (op) {
           case LEX_EQUAL:     return jsvNewFromBool(cmp==0);
           case LEX_NEQUAL:    return jsvNewFromBool(cmp!=0);
           case '<':           return jsvNewFromBool(cmp<0);
           case LEX_LEQUAL:    return jsvNewFromBool(cmp<=0);
           case '>':           return jsvNewFromBool(cmp>0);
           case LEX_GEQUAL:    return jsvNewFromBool(cmp>=0);
           default: return jsvMathsOpError(op, "String");
       }
    }
}

void jsvTraceLockInfo(JsVar *v) {
    jsiConsolePrint("#");
    jsiConsolePrintInt(jsvGetRef(v));
    jsiConsolePrint("[r");
    jsiConsolePrintInt(v->refs);
    jsiConsolePrint(",l");
    jsiConsolePrintInt(v->locks-1);
    jsiConsolePrint("] ");
}

/** Write debug info for this Var out to the console */
void jsvTrace(JsVarRef ref, int indent) {
#ifndef SDCC
    int i;
    char buf[JS_ERROR_BUF_SIZE];
    JsVar *var;

    for (i=0;i<indent;i++) jsiConsolePrint(" ");

    if (!ref) {
        jsiConsolePrint("undefined\n");
        return;
    }
    var = jsvLock(ref);
    jsvTraceLockInfo(var);


    if (jsvIsName(var)) {
      if (jsvIsFunctionParameter(var))
        jsiConsolePrint("Param ");
      jsvGetString(var, buf, JS_ERROR_BUF_SIZE);
      if (jsvIsInt(var)) {
        jsiConsolePrint("Name: int ");
        jsiConsolePrint(buf);
        jsiConsolePrint("  ");
      } else if (jsvIsFloat(var)) {
        jsiConsolePrint("Name: flt ");
        jsiConsolePrint(buf);
        jsiConsolePrint("  ");
      } else if (jsvIsString(var) || jsvIsFunctionParameter(var)) {
        jsiConsolePrint("Name: '");
        jsiConsolePrint(buf);
        jsiConsolePrint("'  ");
      } else {
        assert(0);
      }
      // go to what the name points to
      ref = var->firstChild;
      jsvUnLock(var);
      if (ref) {
        var = jsvLock(ref);
        jsvTraceLockInfo(var);
      } else {
          jsiConsolePrint("undefined\n");
        return;
      }
    }
    if (jsvIsName(var)) {
        jsiConsolePrint("\n");
      jsvTrace(jsvGetRef(var), indent+1);
      jsvUnLock(var);
      return;
    }
    if (jsvIsObject(var)) jsiConsolePrint("Object {\n");
    else if (jsvIsArray(var)) jsiConsolePrint("Array [\n");
    else if (jsvIsInt(var)) jsiConsolePrint("Integer ");
    else if (jsvIsFloat(var)) jsiConsolePrint("Double ");
    else if (jsvIsString(var)) jsiConsolePrint("String ");
    else if (jsvIsFunction(var)) jsiConsolePrint("Function {\n");
    else {
        jsiConsolePrint("Flags ");
        jsiConsolePrintInt(var->flags);
        jsiConsolePrint("\n");
    }

    if (!jsvIsObject(var) && !jsvIsArray(var) && !jsvIsFunction(var)) {
      if (jsvIsString(var) || jsvIsName(var))
        jsvPrintStringVar(var);
      else {
        jsvGetString(var, buf, JS_ERROR_BUF_SIZE);
        jsiConsolePrint(buf);
      }
    }

    if (jsvIsString(var) || jsvIsStringExt(var) || jsvIsName(var)) {
      if (!jsvIsStringExt(var) && var->firstChild) { // stringext don't have children (the use them for chars)
        jsiConsolePrint("( Multi-block string ");
        JsVarRef child = var->firstChild;
        while (child) {
          JsVar *childVar = jsvLock(child);
          jsvTraceLockInfo(childVar);
          child = childVar->firstChild;
          jsvUnLock(childVar);
        }
        jsiConsolePrint(")\n");
      } else
          jsiConsolePrint("\n");
    } else if (!(var->flags & JSV_IS_RECURSING)) {
      /* IS_RECURSING check stops infinite loops */
      var->flags |= JSV_IS_RECURSING;
      JsVarRef child = var->firstChild;
      jsiConsolePrint("\n");
      // dump children
      while (child) {
        JsVar *childVar;
        jsvTrace(child, indent+1);
        childVar = jsvLock(child);
        child = childVar->nextSibling;
        jsvUnLock(childVar);
      }
      var->flags &= ~JSV_IS_RECURSING;
    } else {
        jsiConsolePrint(" ... ");
    }


    if (jsvIsObject(var) || jsvIsFunction(var)) {
      int i;
      for (i=0;i<indent;i++) jsiConsolePrint(" ");
      jsiConsolePrint("}\n");
    } else if (jsvIsArray(var)) {
      int i;
      for (i=0;i<indent;i++) jsiConsolePrint(" ");
      jsiConsolePrint("]\n");
    }

    jsvUnLock(var);
#endif
}

/** Count references of 'toCount' starting from 'var' - for garbage collection on free */
int jsvGetRefCount(JsVar *toCount, JsVar *var) {
    int refCount = 0;

    if (jsvIsName(var)) {
      JsVarRef child = var->firstChild;
      if (child) {
        JsVar *childVar = jsvLock(child);
        if (childVar == toCount)
          refCount+=1;
        else
          refCount += jsvGetRefCount(toCount, childVar);
        child = childVar->firstChild;
        jsvUnLock(childVar);
      }
    } else if (jsvHasChildren(var)) {
      JsVarRef child = var->firstChild;
      while (child) {
        JsVar *childVar;
        childVar = jsvLock(child);
        if (childVar == toCount)
            refCount+=1;
        else
            refCount += jsvGetRefCount(toCount, childVar);
        child = childVar->nextSibling;
        jsvUnLock(childVar);
      }
    }
    return refCount;
}
