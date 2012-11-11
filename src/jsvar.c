/*
 * jsvar.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#include "jsvar.h"
#include "jslex.h"
#include "jsparse.h"
#include "jsfunctions.h"
#include "jsinteractive.h"

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
    // jsVars[i].locks = 0; // locks is 0 anyway because it is stored in flags
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
    if (jsvIsRoot(&jsVars[i]))
      return jsvLockAgain(&jsVars[i]);

  return jsvRef(jsvNewWithFlags(JSV_ROOT));
}

/// Get number of memory records (JsVars) used
int jsvGetMemoryUsage() {
  int usage = 0;
  int i;
  for (i=1;i<jsVarsSize;i++)
    if (jsVars[i].refs != JSVAR_CACHE_UNUSED_REF)
      usage++;
  return usage;
}

/// Get total amount of memory records
int jsvGetMemoryTotal() {
  return jsVarsSize;
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
      //v->locks = 1;
      v->flags = JSV_LOCK_ONE;
      v->varData.callback = 0;
      v->firstChild = 0;
      v->lastChild = 0;
      v->prevSibling = 0;
      v->nextSibling = 0;
      // return pointer
      return v;
  }
  /* we don't have memort - second last hope - run garbage collector */
  if (jsvGarbageCollect())
    return jsvNew(); // if it freed something, continue
  /* we don't have memory - last hope - ask jsInteractive to try and free some it
   may have kicking around */
  if (jsiFreeMoreMemory())
    return jsvNew();
  jsError("Out of Memory!");
  jspSetInterrupted(true);
  return 0;
}

void jsvFreeLoopedRefPtr(JsVar *var); // forward decl

void jsvFreePtr(JsVar *var) {
    /* To be here, we're not supposed to be part of anything else. If
     * we were, we'd have been freed by jsvGarbageCollect */
    assert(!var->nextSibling && !var->prevSibling);

    // Names Link to other things
    if (jsvIsName(var)) {
      if (var->firstChild) {
        JsVar *child = jsvLock(var->firstChild);
        jsvUnRef(child); var->firstChild = 0; // unlink the child
        jsvUnLock(child); // unlock should trigger a free
      }
    }

    /* Now, unref children - see jsvar.h comments for how! */
    if (jsvIsString(var) || jsvIsStringExt(var)) {
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
    } else if (jsvIsObject(var) || jsvIsFunction(var) || jsvIsArray(var) || jsvIsRoot(var)) {
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

    // free!
    var->refs = JSVAR_CACHE_UNUSED_REF;
    // add this to our free list
    var->nextSibling = jsVarFirstEmpty;
    jsVarFirstEmpty = jsvGetRef(var);
}

/// Get a reference from a var - SAFE for null vars
inline JsVarRef jsvGetRef(JsVar *var) {
    if (!var) return 0;
#ifdef LARGE_MEM
    return var->this;
#else
    return (JsVarRef)(1 + (var - jsVars));
#endif
}

/// Lock this reference and return a pointer - UNSAFE for null refs
static inline JsVar *jsvGetAddressOf(JsVarRef ref) {
  assert(ref);
  return &jsVars[ref-1];
}

/// Lock this reference and return a pointer - UNSAFE for null refs
inline JsVar *jsvLock(JsVarRef ref) {
  JsVar *var = jsvGetAddressOf(ref);
  //var->locks++;
  var->flags += JSV_LOCK_ONE;
#ifdef DEBUG
  if (jsvGetLocks(var)==0) {
    jsError("Too many locks to Variable!");
    //jsPrint("Var #");jsPrintInt(ref);jsPrint("\n");
  }
#endif
  return var;
}

/// Lock this pointer and return a pointer - UNSAFE for null pointer
inline JsVar *jsvLockAgain(JsVar *var) {
  var->flags += JSV_LOCK_ONE;
#ifdef DEBUG
  if (var->locks==0) {
    jsError("Too many locks to Variable!");
    //jsPrint("Var #");jsPrintInt(ref);jsPrint("\n");
  }
#endif
  return var;
}

/// Unlock this variable - this is SAFE for null variables
inline JsVarRef jsvUnLock(JsVar *var) {
  JsVarRef ref;
  if (!var) return 0;
  ref = jsvGetRef(var);
  assert(jsvGetLocks(var)>0);
  var->flags -= JSV_LOCK_ONE;
  /* if we know we're free, then we can just free
   * this variable right now. Loops of variables
   * are handled by the Garbage Collector */
  if (jsvGetLocks(var) == 0 && var->refs == 0) {
    jsvFreePtr(var);
    return 0;
  } else
    return ref;
}

JsVar *jsvNewFromString(const char *str) {
  JsVar *var;
  // Create a var
  JsVar *first = jsvNewWithFlags(JSV_STRING);
  if (!first) {
    jsWarn("Unable to create string as not enough memory");
    return 0;
  }
  // Now we copy the string, but keep creating new jsVars if we go
  // over the end
  var = jsvLockAgain(first);
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
      JsVar *next = jsvNewWithFlags(JSV_STRING_EXT);
      if (!next) {
        jsWarn("Truncating string as not enough memory");
        jsvUnLock(var);
        return first;
      }
      next = jsvRef(next);
      var->lastChild = jsvGetRef(next);
      jsvUnLock(var);
      var = next;
    }
  }
  jsvUnLock(var);
  // return
  return first;
}

JsVar *jsvNewFromLexer(struct JsLex *lex, JslCharPos charFrom, JslCharPos charTo) {
  // Create a var
  JsVar *var = jsvNewFromString("");
  if (!var) { // out of memory
    return 0;
  }

  jsvAppendStringVar(var, lex->sourceVar, charFrom, (JslCharPos)charTo-charFrom);
  return var;
}

JsVar *jsvNewWithFlags(JsVarFlags flags) {
  JsVar *var = jsvNew();
  if (!var) return 0; // no memory
  var->flags = (var->flags&(JsVarFlags)(~JSV_VARTYPEMASK)) | (flags&(JsVarFlags)(~JSV_LOCK_MASK));
  return var;
}
JsVar *jsvNewFromInteger(JsVarInt value) {
  JsVar *var = jsvNewWithFlags(JSV_INTEGER);
  if (!var) return 0; // no memory
  var->varData.integer = value;
  return var;
}
JsVar *jsvNewFromBool(bool value) {
  JsVar *var = jsvNewWithFlags(JSV_BOOLEAN);
  if (!var) return 0; // no memory
  var->varData.integer = value ? 1 : 0;
  return var;
}
JsVar *jsvNewFromFloat(JsVarFloat value) {
  JsVar *var = jsvNewWithFlags(JSV_FLOAT);
  if (!var) return 0; // no memory
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

/** Given a variable, return the basic object name of it */
const char *jsvGetBasicObjectName(JsVar *v) {
  if (jsvIsInt(v))
      return "Integer";
  if (jsvIsFloat(v))
      return "Double";
  if (jsvIsString(v))
      return "String";
  if (jsvIsArray(v))
      return "Array";
  if (jsvIsObject(v))
      return "Object";
  if (jsvIsFunction(v))
      return "Function";
  return 0;
}

bool jsvIsBuiltInObject(const char *name) {
  return
      strcmp(name, "String")==0 ||
      strcmp(name, "Object")==0 ||
      strcmp(name, "Array")==0 ||
      strcmp(name, "Integer")==0 ||
      strcmp(name, "Double")==0 ||
      strcmp(name, "Function")==0 ||
      strcmp(name, "Math")==0 ||
      strcmp(name, "JSON")==0;
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
    JsvStringIterator ita, itb;
    jsvStringIteratorNew(&ita, a, 0);
    jsvStringIteratorNew(&itb, b, 0);
    while (true) {
      char a = jsvStringIteratorGetChar(&ita);
      char b = jsvStringIteratorGetChar(&itb);
      if (a != b) {
        jsvStringIteratorFree(&ita);
        jsvStringIteratorFree(&itb);
        return false;
      }
      if (!a) { // equal, but end of string
        jsvStringIteratorFree(&ita);
        jsvStringIteratorFree(&itb);
        return true;
      }
      jsvStringIteratorNext(&ita);
      jsvStringIteratorNext(&itb);
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

/// Get a const string representing this variable - if we can. Otherwise return 0
const char *jsvGetConstString(JsVar *v) {
    if (jsvIsUndefined(v)) {
      return "undefined";
    } else if (jsvIsNull(v)) {
      return "null";
    } else if (jsvIsBoolean(v)) {
      return jsvGetBool(v) ? "true" : "false";
    } else if (jsvIsObject(v)) {
      return "[object Object]";
    } else if (jsvIsRoot(v)) {
      return "[ROOT]";
    }
    return 0;
}

/// Save this var as a string to the given buffer
void jsvGetString(JsVar *v, char *str, size_t len) {
   const char *s = jsvGetConstString(v);
   if (s) {
     strncpy(str, s, len);
   } else if (jsvIsInt(v)) {
     itoa(v->varData.integer, str, 10);
   } else if (jsvIsFloat(v)) {
     ftoa(v->varData.floating, str);
   } else if (jsvIsString(v) || jsvIsStringExt(v) || jsvIsFunctionParameter(v)) {
       if (jsvIsStringExt(v))
        jsWarn("INTERNAL: Calling jsvGetString on a JSV_STRING_EXT");
      JsvStringIterator it;
      jsvStringIteratorNew(&it, v, 0);
      while (jsvStringIteratorHasChar(&it)) {
        if (len--<=1) {
          *str = 0;
          jsWarn("jsvGetString overflowed\n");
          jsvStringIteratorFree(&it);
          return;
        }
        *(str++) = jsvStringIteratorGetChar(&it);
        jsvStringIteratorNext(&it);
      }
      jsvStringIteratorFree(&it);
      *str = 0;
    } else {
      // Try and get as a JsVar string, and try again
      JsVar *stringVar = jsvAsString(v, false);
      if (stringVar) {
        jsvGetString(stringVar, str, len); // call again - but this tm
        jsvUnLock(stringVar);
      } else {
        strncpy(str, "", len);
        jsWarn("INTERNAL: variable type cannot be converted to string");
      }
    }
}

/** If var is a string, lock and return it, else
 * create a new string. unlockVar means this will auto-unlock 'var'  */
JsVar *jsvAsString(JsVar *v, bool unlockVar) {
  JsVar *str = 0;
  // If it is string-ish, but not quite a string, copy it
  if (jsvHasCharacterData(v) && jsvIsName(v)) {
    str = jsvNewFromString("");
    if (str) jsvAppendStringVar(str,v,0,JSVAPPENDSTRINGVAR_MAXLENGTH);
  } else if (jsvIsString(v)) { // If it is a string - just return a reference
    str = jsvLockAgain(v);
  } else {
    const char *constChar = jsvGetConstString(v);
    if (constChar) {
      // if we could get this as a simple const char, do that..
      str = jsvNewFromString(constChar);
    } else if (jsvIsInt(v)) {
      char buf[JS_NUMBER_BUFFER_SIZE];
      itoa(v->varData.integer, buf, 10);
      str = jsvNewFromString(buf);
    } else if (jsvIsFloat(v)) {
      char buf[JS_NUMBER_BUFFER_SIZE];
      ftoa(v->varData.floating, buf);
      str = jsvNewFromString(buf);
    } else if (jsvIsArray(v)) {
      JsVar *filler = jsvNewFromString(",");
      str = jsvArrayJoin(v, filler);
      jsvUnLock(filler);
    } else if (jsvIsFunction(v)) {
      str = jsvNewFromString("");
      if (str) jsfGetJSON(v, str);
    } else {
      jsWarn("INTERNAL: variable type cannot be converted to string");
      str = 0;
    }
  }

  if (unlockVar) jsvUnLock(v);
  return str;
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

//  IN A STRING  get the number of lines in the string (min=1)
int jsvGetLinesInString(JsVar *v) {
  int lines = 1;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, 0);
  while (jsvStringIteratorHasChar(&it)) {
    if (jsvStringIteratorGetChar(&it)=='\n') lines++;
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  return lines;
}

// IN A STRING Get the number of characters on a line - lines start at 1
int jsvGetCharsOnLine(JsVar *v, int line) {
  int currentLine = 1;
  int chars = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, 0);
  while (jsvStringIteratorHasChar(&it)) {
    if (jsvStringIteratorGetChar(&it)=='\n') {
      currentLine++;
      if (currentLine > line) break;
    } else if (currentLine==line) chars++;
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  return chars;
}

//  IN A STRING, get the line and column of the given character. Both values must be non-null
void jsvGetLineAndCol(JsVar *v, int charIdx, int* line, int *col) {
  int x = 1;
  int y = 1;
  int n = 0;
  assert(line && col);

  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (n==charIdx) {
      jsvStringIteratorFree(&it);
      *line = y;
      *col = x;
      return;
    }
    x++;
    if (ch=='\n') {
      x=1; y++;
    }
    n++;
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  // uh-oh - not found
  *line = y;
  *col = x;
}

//  IN A STRING, get a character index from a line and column
int jsvGetIndexFromLineAndCol(JsVar *v, int line, int col) {
  int x = 1;
  int y = 1;
  int n = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if ((y==line && x>=col) || y>line) {
      jsvStringIteratorFree(&it);
      return (y>line) ? (n-1) : n;
    }
    x++;
    if (ch=='\n') {
      x=1; y++;
    }
    n++;
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  return n;
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
      JsVar *next = jsvNewWithFlags(JSV_STRING_EXT);
      if (!next) break;
      next = jsvRef(next);
      block->lastChild = jsvGetRef(next);
      jsvUnLock(block);
      block = next;
      blockChars=0; // it's new, so empty
    }
  }
  jsvUnLock(block);
}

void jsvAppendInteger(JsVar *var, JsVarInt i) {
  char buf[32];
  itoa(i,buf,10);
  jsvAppendString(var, buf);
}

void jsvAppendCharacter(JsVar *var, char ch) {
  // Append the character to our input line
  char buf[2];
  buf[0] = ch;
  buf[1] = 0;
  jsvAppendString(var, buf);
}

/** Append str to var. Both must be strings. stridx = start char or str, maxLength = max number of characters (can be JSVAPPENDSTRINGVAR_MAXLENGTH).
 *  stridx can be negative to go from end of string */
void jsvAppendStringVar(JsVar *var, JsVar *str, int stridx, int maxLength) {
  JsVar *block = jsvLockAgain(var);
  assert(jsvIsString(var));
  // Find the block at end of the string...
  while (block->lastChild) {
    JsVarRef next = block->lastChild;
    jsvUnLock(block);
    block = jsvLock(next);
  }
  // find how full the block is
  unsigned int blockChars=0;
  while (blockChars<jsvGetMaxCharactersInVar(block) && block->varData.str[blockChars])
        blockChars++; // TODO: fix for zeros in strings
  // now start appending
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, stridx);
  while (jsvStringIteratorHasChar(&it) && (maxLength-->0)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (blockChars >= jsvGetMaxCharactersInVar(block)) {
      JsVar *next = jsvNewWithFlags(JSV_STRING_EXT);
      if (!next) break; // out of memory
      next = jsvRef(next);
      block->lastChild = jsvGetRef(next);
      jsvUnLock(block);
      block = next;
      blockChars=0; // it's new, so empty
    }
    block->varData.str[blockChars++] = ch;
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
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

  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, idx);
  char ch = jsvStringIteratorGetChar(&it);
  jsvStringIteratorFree(&it);
  return ch;
}

JsVarInt jsvGetInteger(const JsVar *v) {
    if (!v) return 0;
    /* strtol understands about hex and octal */
    if (jsvIsInt(v) || jsvIsBoolean(v)) return v->varData.integer;
    if (jsvIsNull(v)) return 0;
    if (jsvIsUndefined(v)) return 0;
    if (jsvIsFloat(v)) return (JsVarInt)v->varData.floating;
    return 0;
}

void jsvSetInteger(JsVar *v, JsVarInt value) {
  assert(jsvIsInt(v));
  v->varData.integer  = value;
}

bool jsvGetBool(const JsVar *v) {
  return jsvGetInteger(v)!=0;
}

JsVarFloat jsvGetFloat(const JsVar *v) {
    if (!v) return 0;
    if (jsvIsFloat(v)) return v->varData.floating;
    if (jsvIsInt(v)) return (JsVarFloat)v->varData.integer;
    if (jsvIsNull(v)) return 0;
    if (jsvIsUndefined(v)) return 0;
    return 0; /* or NaN? */
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
    size_t i, l = jsvGetMaxCharactersInVar(v);
    JsVarRef next;
    for (i=0;i<l;i++) {
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
  JsvStringIterator ita, itb;
  jsvStringIteratorNew(&ita, va, starta);
  jsvStringIteratorNew(&itb, vb, startb);
   // step to first positions
  while (true) {
    char ca = jsvStringIteratorGetChar(&ita);
    char cb = jsvStringIteratorGetChar(&itb);
    if (ca != cb) {
      jsvStringIteratorFree(&ita);
      jsvStringIteratorFree(&itb);
      if ((ca==0 || cb==0) && equalAtEndOfString) return 0;
      return ca - cb;
    }
    if (ca == 0) { // both equal, but end of string
      jsvStringIteratorFree(&ita);
      jsvStringIteratorFree(&itb);
      return 0;
    }
    jsvStringIteratorNext(&ita);
    jsvStringIteratorNext(&itb);
  }
  // never get here, but the compiler warns...
  return true;
}

/** Compare 2 integers, >0 if va>vb,  <0 if va<vb. If compared with a non-integer, that gets put later */
int jsvCompareInteger(JsVar *va, JsVar *vb) {
  if (jsvIsInt(va) && jsvIsInt(vb))
    return (int)(jsvGetInteger(va) - jsvGetInteger(vb));
  else if (jsvIsInt(va))
    return -1;
  else if (jsvIsInt(vb))
    return 1;
  else
    return 0;
}

/** Copy only a name, not what it points to. ALTHOUGH the link to what it points to is maintained unless linkChildren=false
    If keepAsName==false, this will be converted into a normal variable */
JsVar *jsvCopyNameOnly(JsVar *src, bool linkChildren, bool keepAsName) {
  assert(jsvIsName(src));
  JsVarFlags flags = src->flags;
  if (!keepAsName) flags &= (JsVarFlags)~JSV_NAME; // make sure this is NOT a name
  JsVar *dst = jsvNewWithFlags(flags);
  if (!dst) return 0; // out of memory
  
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
      JsVar *child = jsvCopyNameOnly(name, true/*link children*/, true/*keep as name*/); // NO DEEP COPY!
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
  namedChild = jsvRef(namedChild); // ref here VERY important as adding to structure!
  assert(jsvIsName(namedChild));

  if (parent->lastChild) { // we have children already
    JsVar *insertAfter = jsvLock(parent->lastChild);
    if (jsvIsArray(parent)) {
      // we must insert in order - so step back until we get the right place
      while (insertAfter && jsvCompareInteger(namedChild, insertAfter)<0) {
        JsVarRef prev = insertAfter->prevSibling;
        jsvUnLock(insertAfter);
        insertAfter = prev ? jsvLock(prev) : 0;
      }
    }

    if (insertAfter) {
      if (insertAfter->nextSibling) {
        // great, we're in the middle...
        JsVar *insertBefore = jsvLock(insertAfter->nextSibling);
        insertBefore->prevSibling = jsvGetRef(namedChild);
        namedChild->nextSibling = jsvGetRef(insertBefore);
        jsvUnLock(insertBefore);
      } else {
        // We're at the end - just set up the parent
        parent->lastChild = jsvGetRef(namedChild);
      }
      insertAfter->nextSibling = jsvGetRef(namedChild);
      namedChild->prevSibling = jsvGetRef(insertAfter);
      jsvUnLock(insertAfter);
    } else { // Insert right at the beginning of the array
      // Link 2 children together
      JsVar *firstChild = jsvLock(parent->firstChild);
      firstChild->prevSibling = jsvGetRef(namedChild);
      jsvUnLock(firstChild);

      namedChild->nextSibling = parent->firstChild;
      // finally set the new child as the first one
      parent->firstChild = jsvGetRef(namedChild);
    }
  } else { // we have no children - just add it
    parent->firstChild = parent->lastChild = jsvGetRef(namedChild);

  }
}

JsVar *jsvAddNamedChild(JsVar *parent, JsVar *child, const char *name) {
  JsVar *namedChild = jsvMakeIntoVariableName(jsvNewFromString(name), child);
  if (!namedChild) return 0; // Out of memory
  jsvAddName(parent, namedChild);
  return namedChild;
}

JsVar *jsvSetNamedChild(JsVar *parent, JsVar *child, const char *name) {
  JsVar *namedChild = jsvFindChildFromString(parent, name, true);
  if (namedChild) // could be out of memory
    return jsvSetValueOfName(namedChild, child);
  return 0;
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
      // we can link to a name if we want (so can remove the assert!)
      name->firstChild = jsvGetRef(jsvRef(src));
  } else
      name->firstChild = 0;
  return name;
}

JsVar *jsvFindChildFromString(JsVar *parent, const char *name, bool addIfNotFound) {
  JsVarRef childref = parent->firstChild;
  while (childref) {
    JsVar *child = jsvLock(childref);
    if (jsvIsStringEqual(child, name)) {
       // found it! unlock parent but leave child locked
       return child;
    }
    childref = child->nextSibling;
    jsvUnLock(child);
  }

  JsVar *child = 0;
  if (addIfNotFound) {
    child = jsvMakeIntoVariableName(jsvNewFromString(name), 0);
    if (child) // could be out of memory
      jsvAddName(parent, child);
  }
  return child;
}

/** Non-recursive finding */
JsVar *jsvFindChildFromVar(JsVar *parent, JsVar *childName, bool addIfNotFound) {
  JsVar *child;
  JsVarRef childref = parent->firstChild;

  while (childref) {
    child = jsvLock(childref);
    if (jsvIsBasicVarEqual(child, childName)) {
      // found it! unlock parent but leave child locked
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
      child = jsvLockAgain(childName);
    } else { // it was reffed, we must add a new one
      child = jsvMakeIntoVariableName(jsvCopy(childName), 0);
    }
    jsvAddName(parent, child);
  }
  return child;
}

void jsvRemoveChild(JsVar *parent, JsVar *child) {
    assert(jsvIsArray(parent) || jsvIsObject(parent) || jsvIsFunction(parent) || jsvIsRoot(parent));
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
  JsVarRef childref = arr->lastChild;
  // Just look at last non-string element!
  while (childref) {
    JsVar *child = jsvLock(childref);
    if (jsvIsInt(child)) {
      JsVarInt lastIdx = jsvGetInteger(child);
      jsvUnLock(child);
      return lastIdx+1;
    }
    // if not an int, keep going
    childref = child->prevSibling;
    jsvUnLock(child);
  }
  return 0;
}

JsVar *jsvGetArrayItem(JsVar *arr, int index) {
  JsVarRef childref = arr->firstChild;
  while (childref) {
    JsVarInt childIndex;
    JsVar *child = jsvLock(childref);
    
    assert(jsvIsInt(child));
    childIndex = jsvGetInteger(child);
    if (childIndex == index) {
      JsVar *item = child->firstChild ? jsvLock(child->firstChild) : 0;
      jsvUnLock(child);
      return item;
    }
    childref = child->nextSibling;
    jsvUnLock(child);
  }
  return 0; // undefined
}

/// Get the index of the value in the array (matchExact==use pointer, not equality check)
JsVar *jsvGetArrayIndexOf(JsVar *arr, JsVar *value, bool matchExact) {
  JsVarRef indexref;
  assert(jsvIsArray(arr) || jsvIsObject(arr));
  indexref = arr->firstChild;
  while (indexref) {
    JsVar *childIndex = jsvLock(indexref);
    assert(jsvIsName(childIndex))
    if (childIndex->firstChild) {
      JsVar *childValue = jsvLock(childIndex->firstChild);
      if ((matchExact && childValue==value) ||
          (!matchExact && jsvIsBasicVarEqual(childValue, value))) {
        jsvUnLock(childValue);
        return childIndex;
      }
      jsvUnLock(childValue);
    } else if (jsvIsUndefined(value)) 
      return childIndex; // both are undefined, so we return the index
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
    if (child->prevSibling) {
      JsVar *v = jsvLock(child->prevSibling);
      v->nextSibling = 0;
      jsvUnLock(v);
    }
    child->prevSibling = 0;
    return child; // and return it
  } else {
    // no children!
    return 0;
  }
}

/// Removes the first element of an array, and returns that element (or 0 if empty). 
JsVar *jsvArrayPopFirst(JsVar *arr) {
  assert(jsvIsArray(arr));
  if (arr->firstChild) {
    JsVar *child = jsvLock(arr->firstChild);
    if (arr->firstChild == arr->lastChild)
      arr->lastChild = 0; // if 1 item in array
    arr->firstChild = child->nextSibling; // unlink from end of array
    jsvUnRef(child); // as no longer in array
    if (child->nextSibling) {
      JsVar *v = jsvLock(child->nextSibling);
      v->prevSibling = 0;
      jsvUnLock(v);
    }
    child->nextSibling = 0;
    return child; // and return it
  } else {
    // no children!
    return 0;
  }
}

///  Get the last element of an array (does not remove, unlike jsvArrayPop), and returns that element (or 0 if empty) includes the NAME
JsVar *jsvArrayGetLast(JsVar *arr) {
  assert(jsvIsArray(arr));
  if (arr->lastChild) {
    return jsvLock(arr->lastChild);
  } else { // no children!    
    return 0;
  }
}

/// Join all elements of an array together into a string
JsVar *jsvArrayJoin(JsVar *arr, JsVar *filler) {
  JsVar *str = jsvNewFromString("");
  if (!str) return 0; // out of memory
  JsVarInt index = 0;
  JsVarRef childRef = arr->firstChild;
  while (childRef) {
   JsVar *child = jsvLock(childRef);
   if (jsvIsInt(child)) {
     JsVarInt thisIndex = jsvGetInteger(child);
     if (filler) {
       while (index<thisIndex) {
         index++;
         jsvAppendStringVar(str, filler, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
       }
     }

     if (child->firstChild) {
       JsVar *data = jsvAsString(jsvLock(child->firstChild), true);
       if (data) { // could be out of memory
         jsvAppendStringVar(str, data, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
         jsvUnLock(data);
       }
     }
   }
   childRef = child->nextSibling;
   jsvUnLock(child);
  }
  return str;
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
        bool needsInt = op=='&' || op=='|' || op=='^' || op=='%' || op==LEX_LSHIFT || op==LEX_RSHIFT || op==LEX_RSHIFTUNSIGNED;

        if (needsInt || (!jsvIsFloat(a) && !jsvIsFloat(b))) {
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
            JsVarFloat da = jsvGetFloat(a);
            JsVarFloat db = jsvGetFloat(b);
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
    } else if ((jsvIsArray(a) || jsvIsObject(a) ||
                jsvIsArray(b) || jsvIsObject(b)) &&
                (op == LEX_EQUAL || op==LEX_NEQUAL)) {
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

JsVar *jsvNegateAndUnLock(JsVar *v) {
  JsVar *zero = jsvNewFromInteger(0);
  JsVar *res = jsvMathsOpSkipNames(v, zero, LEX_EQUAL);
  jsvUnLock(zero);
  jsvUnLock(v);
  return res;
}

void jsvTraceLockInfo(JsVar *v) {
    jsiConsolePrint("#");
    jsiConsolePrintInt(jsvGetRef(v));
    jsiConsolePrint("[r");
    jsiConsolePrintInt(v->refs);
    jsiConsolePrint(",l");
    jsiConsolePrintInt(jsvGetLocks(v)-1);
    jsiConsolePrint("] ");
}

/** Write debug info for this Var out to the console */
void jsvTrace(JsVarRef ref, int indent) {
#ifndef SDCC
    int i;
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
      JsVar *str = jsvAsString(var, false);
      if (jsvIsInt(var)) {
        jsiConsolePrint("Name: int ");
        jsiConsolePrintStringVar(str);
        jsiConsolePrint("  ");
      } else if (jsvIsFloat(var)) {
        jsiConsolePrint("Name: flt ");
        jsiConsolePrintStringVar(str);
        jsiConsolePrint("  ");
      } else if (jsvIsString(var) || jsvIsFunctionParameter(var)) {
        jsiConsolePrint("Name: '");
        jsiConsolePrintStringVar(str);
        jsiConsolePrint("'  ");
      } else {
        assert(0);
      }
      jsvUnLock(str);
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
      jsvTrace(jsvGetRef(var), indent+2);
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
        jsiConsolePrintInt(var->flags & (JsVarFlags)~(JSV_LOCK_MASK|JSV_STRING_LEN_MASK));
        jsiConsolePrint("\n");
    }

    if (!jsvIsObject(var) && !jsvIsArray(var) && !jsvIsFunction(var)) {
      JsVar *str = jsvAsString(var, false);
      if (str) {
        jsiConsolePrintStringVar(str);
        jsvUnLock(str);
      }
    }

    if (jsvIsString(var) || jsvIsStringExt(var)) {
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
        jsvTrace(child, indent+2);
        childVar = jsvLock(child);
        child = childVar->nextSibling;
        jsvUnLock(childVar);
      }
      var->flags &= (JsVarFlags)~JSV_IS_RECURSING;
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


/** Recursively mark the variable */
static void jsvGarbageCollectMarkUsed(JsVar *var) {
  var->flags &= (JsVarFlags)~JSV_GARBAGE_COLLECT;

  if (jsvHasCharacterData(var)) {
      if (var->lastChild) {
        JsVar *childVar = jsvGetAddressOf(var->lastChild);
        if (childVar->flags & JSV_GARBAGE_COLLECT)
          jsvGarbageCollectMarkUsed(childVar);
      }
  } // intentionally no else
  if (jsvIsName(var)) {
    if (var->firstChild) {
      JsVar *childVar = jsvGetAddressOf(var->firstChild);
      if (childVar->flags & JSV_GARBAGE_COLLECT)
        jsvGarbageCollectMarkUsed(childVar);
    }
  } else if (jsvHasChildren(var)) {
    JsVarRef child = var->firstChild;
    while (child) {
      JsVar *childVar;
      childVar = jsvGetAddressOf(child);
      if (childVar->flags & JSV_GARBAGE_COLLECT)
        jsvGarbageCollectMarkUsed(childVar);
      child = childVar->nextSibling;
    }
  }
}

/** Run a garbage collection sweep - return true if things have been freed */
bool jsvGarbageCollect() {
  return false;
  int i;
  // clear garbage collect flags
  for (i=0;i<jsVarsSize;i++)  {
    JsVar *var = &jsVars[i];
    if (var->refs == JSVAR_CACHE_UNUSED_REF) // if it is not unused
      var->flags = 0; // no garbage collect!
    else
      var->flags |= (JsVarFlags)JSV_GARBAGE_COLLECT;
  }
  // recursively add 'native' vars
  for (i=0;i<jsVarsSize;i++)  {
    JsVar *var = &jsVars[i];
    if ((var->flags & JSV_GARBAGE_COLLECT) && // not already GC'd
        jsvGetLocks(var)>0) // or it is locked
      jsvGarbageCollectMarkUsed(var);
  }
  // now sweep for things that we can GC!
  bool freedSomething = false;
  for (i=0;i<jsVarsSize;i++)  {
    JsVar *var = &jsVars[i];
    if (var->flags & JSV_GARBAGE_COLLECT) {
      freedSomething = true;
      // free!
      var->refs = JSVAR_CACHE_UNUSED_REF;
      // add this to our free list
      var->nextSibling = jsVarFirstEmpty;
      jsVarFirstEmpty = jsvGetRef(var);
    }
  }
  return freedSomething;
}

/** Dotty output for the graphviz package - helps
 *  visualize the data structure  */
void jsvDottyOutput() {
  int i;
  bool ignoreStringExt = true;
  char buf[256];
  jsiConsolePrint("digraph G {\n");
  //jsiConsolePrint("  rankdir=LR;\n");
  for (i=0;i<jsVarsSize;i++) {
    if (jsVars[i].refs != JSVAR_CACHE_UNUSED_REF) {
      JsVar *var = jsvLock((JsVarRef)(i+1));
      if (ignoreStringExt && jsvIsStringExt(var)) {
        jsvUnLock(var);
        continue;
      }
      jsiConsolePrint("V");
      jsiConsolePrintInt(i+1);
      jsiConsolePrint(" [shape=box,label=\"");
      jsvTraceLockInfo(var);
      jsiConsolePrint(":");
      if (jsvIsName(var)) jsiConsolePrint("Name");
      else if (jsvIsObject(var)) jsiConsolePrint("Object");
      else if (jsvIsArray(var)) jsiConsolePrint("Array");
      else if (jsvIsInt(var)) jsiConsolePrint("Integer");
      else if (jsvIsFloat(var)) jsiConsolePrint("Double");
      else if (jsvIsString(var)) jsiConsolePrint("String");
      else if (jsvIsStringExt(var)) jsiConsolePrint("StringExt");
      else if (jsvIsFunction(var)) jsiConsolePrint("Function");
      else {
          jsiConsolePrint("Flags ");
          jsiConsolePrintInt(var->flags & (JsVarFlags)~(JSV_LOCK_MASK|JSV_STRING_LEN_MASK));
      }
      if (!jsvIsStringExt(var) && !jsvIsObject(var) && !jsvIsArray(var)) {
        jsiConsolePrint(":");
        jsvGetString(var,buf,256);
        jsiConsolePrintEscaped(buf);
      }
      jsiConsolePrint("\"];\n");

      if (jsvHasChildren(var)) {
        if (var->firstChild) {
          jsiConsolePrint("V");
          jsiConsolePrintInt(i+1);
          jsiConsolePrint(":n -> V");
          jsiConsolePrintInt(var->firstChild);
          jsiConsolePrint(":n [label=\"first\"]\n");
        }
        if (var->lastChild) {
          jsiConsolePrint("V");
          jsiConsolePrintInt(var->lastChild);
          jsiConsolePrint(":s -> V");
          jsiConsolePrintInt(i+1);
          jsiConsolePrint(":s [label=\"last\"]\n");
        }
      }
      if (jsvIsName(var)) {
        if (var->nextSibling) {
          jsiConsolePrint("V");
          jsiConsolePrintInt(i+1);
          jsiConsolePrint(":s -> V");
          jsiConsolePrintInt(var->nextSibling);
          jsiConsolePrint(":n [weight=5,label=\"next\"]\n");
        }
        if (var->prevSibling) {
          jsiConsolePrint("V");
          jsiConsolePrintInt(i+1);
          jsiConsolePrint(":n -> V");
          jsiConsolePrintInt(var->prevSibling);
          jsiConsolePrint(":s [weight=5,style=dotted,label=\"prev\"]\n");
        }
        if (var->firstChild) {
          jsiConsolePrint("V");
          jsiConsolePrintInt(i+1);
          jsiConsolePrint(":e -> V");
          jsiConsolePrintInt(var->firstChild);
          jsiConsolePrint(":w [style=bold]\n");
        }
      }
      if (!ignoreStringExt && jsvHasCharacterData(var)) {
        if (var->lastChild) {
          jsiConsolePrint("V");
          jsiConsolePrintInt(i+1);
          jsiConsolePrint(":e -> V");
          jsiConsolePrintInt(var->lastChild);
          jsiConsolePrint(":w\n");
        }
      }
      jsvUnLock(var);
    }
  }
  jsiConsolePrint("}\n");
}
