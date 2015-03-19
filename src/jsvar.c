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
 * Variables
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"
#include "jslex.h"
#include "jsparse.h"
#include "jswrap_json.h"
#include "jsinteractive.h"
#include "jswrapper.h"
#include "jswrap_math.h" // for jswrap_math_mod
#include "jswrap_object.h" // for jswrap_object_toString
#include "jswrap_arraybuffer.h" // for jsvNewTypedArray

/** Basically, JsVars are stored in one big array, so save the need for
 * lots of memory allocation. On Linux, the arrays are in blocks, so that
 * more blocks can be allocated. We can't use realloc on one big block as
 * this may change the address of vars that are already locked!
 *
 */

#ifdef RESIZABLE_JSVARS
JsVar **jsVarBlocks = 0;
unsigned int jsVarsSize = 0;
#define JSVAR_BLOCK_SIZE 1024
#define JSVAR_BLOCK_SHIFT 10
#else
JsVar jsVars[JSVAR_CACHE_SIZE];
unsigned int jsVarsSize = JSVAR_CACHE_SIZE;
#endif

JsVarRef jsVarFirstEmpty; ///< reference of first unused variable (variables are in a linked list)

/** Return a pointer - UNSAFE for null refs.
 * This is effectively a Lock without locking! */
static ALWAYS_INLINE JsVar *jsvGetAddressOf(JsVarRef ref) {
  assert(ref);
#ifdef RESIZABLE_JSVARS
  JsVarRef t = ref-1;
  return &jsVarBlocks[t>>JSVAR_BLOCK_SHIFT][t&(JSVAR_BLOCK_SIZE-1)];
#else
  return &jsVars[ref-1];
#endif
}

JsVar *_jsvGetAddressOf(JsVarRef ref) {
  return jsvGetAddressOf(ref);
}

#ifdef JSVARREF_PACKED_BITS
#define JSVARREF_PACKED_BIT_MASK ((1U<<JSVARREF_PACKED_BITS)-1)
JsVarRef jsvGetFirstChild(const JsVar *v) { return (JsVarRef)(v->varData.ref.firstChild | (((v->varData.ref.pack)&JSVARREF_PACKED_BIT_MASK))<<8); }
JsVarRefSigned jsvGetFirstChildSigned(const JsVar *v) {
  JsVarRefSigned r = (JsVarRefSigned)jsvGetFirstChild(v);
  if (r & (1<<(JSVARREF_PACKED_BITS+7)))
    r -= 1<<(JSVARREF_PACKED_BITS+8);
  return r;
}
JsVarRef jsvGetLastChild(const JsVar *v) { return (JsVarRef)(v->varData.ref.lastChild | (((v->varData.ref.pack >> (JSVARREF_PACKED_BITS*1))&JSVARREF_PACKED_BIT_MASK))<<8); }
JsVarRef jsvGetNextSibling(const JsVar *v) { return (JsVarRef)(v->varData.ref.nextSibling | (((v->varData.ref.pack >> (JSVARREF_PACKED_BITS*2))&JSVARREF_PACKED_BIT_MASK))<<8); }
JsVarRef jsvGetPrevSibling(const JsVar *v) { return (JsVarRef)(v->varData.ref.prevSibling | (((v->varData.ref.pack >> (JSVARREF_PACKED_BITS*3))&JSVARREF_PACKED_BIT_MASK))<<8); }
void jsvSetFirstChild(JsVar *v, JsVarRef r) {
  v->varData.ref.firstChild = (unsigned char)(r & 0xFF);
  v->varData.ref.pack = (unsigned char)((v->varData.ref.pack & ~JSVARREF_PACKED_BIT_MASK) | ((r >> 8) & JSVARREF_PACKED_BIT_MASK));
}
void jsvSetLastChild(JsVar *v, JsVarRef r) {
  v->varData.ref.lastChild = (unsigned char)(r & 0xFF);
  v->varData.ref.pack = (unsigned char)((v->varData.ref.pack & ~(JSVARREF_PACKED_BIT_MASK<<(JSVARREF_PACKED_BITS*1))) | (((r >> 8) & JSVARREF_PACKED_BIT_MASK) << (JSVARREF_PACKED_BITS*1)));
}
void jsvSetNextSibling(JsVar *v, JsVarRef r) {
  v->varData.ref.nextSibling = (unsigned char)(r & 0xFF);
  v->varData.ref.pack = (unsigned char)((v->varData.ref.pack & ~(JSVARREF_PACKED_BIT_MASK<<(JSVARREF_PACKED_BITS*2))) | (((r >> 8) & JSVARREF_PACKED_BIT_MASK) << (JSVARREF_PACKED_BITS*2)));
}
void jsvSetPrevSibling(JsVar *v, JsVarRef r) {
  v->varData.ref.prevSibling = (unsigned char)(r & 0xFF);
  v->varData.ref.pack = (unsigned char)((v->varData.ref.pack & ~(JSVARREF_PACKED_BIT_MASK<<(JSVARREF_PACKED_BITS*3))) | (((r >> 8) & JSVARREF_PACKED_BIT_MASK) << (JSVARREF_PACKED_BITS*3)));
}
#endif


// For debugging/testing ONLY - maximum # of vars we are allowed to use
void jsvSetMaxVarsUsed(unsigned int size) {
#ifdef RESIZABLE_JSVARS
  assert(size < JSVAR_BLOCK_SIZE); // remember - this is only for DEBUGGING - as such it doesn't use multiple blocks
#else
  assert(size < JSVAR_CACHE_SIZE);
#endif
  jsVarsSize = size;
}

// maps the empty variables in...
void jsvCreateEmptyVarList() {
  jsVarFirstEmpty = 0;
  JsVar *lastEmpty = 0;
  JsVarRef i;
  for (i=1;i<=jsVarsSize;i++) {
    JsVar *var = jsvGetAddressOf(i);
    if ((var->flags&JSV_VARTYPEMASK) == JSV_UNUSED) {
      jsvSetNextSibling(var, 0);
      if (lastEmpty)
        jsvSetNextSibling(lastEmpty, i);
      else
        jsVarFirstEmpty = i;
      lastEmpty = var;
    } else if (jsvIsFlatString(var)) {
      // skip over used blocks for flat strings
      i = (JsVarRef)(i+jsvGetFlatStringBlocks(var));
    }
  }
}


void jsvSoftInit() {
  jsvCreateEmptyVarList();
}

void jsvSoftKill() {
}

/** This links all JsVars together, so we can have our nice
 * linked list of free JsVars. It returns the ref of the first
 * item - that we should set jsVarFirstEmpty to (if it is 0) */
static JsVarRef jsvInitJsVars(JsVarRef start, unsigned int count) {
  JsVarRef i;
  for (i=start;i<start+count;i++) {
    JsVar *v = jsvGetAddressOf(i);
    v->flags = JSV_UNUSED;
    // v->locks = 0; // locks is 0 anyway because it is stored in flags
    jsvSetNextSibling(v, (JsVarRef)(i+1)); // link to next
  }
  jsvSetNextSibling(jsvGetAddressOf((JsVarRef)(start+count-1)), (JsVarRef)0); // set the final one to 0
  return start;
}

void jsvInit() {
#ifdef RESIZABLE_JSVARS
  jsVarsSize = JSVAR_BLOCK_SIZE;
  jsVarBlocks = malloc(sizeof(JsVar*)); // just 1
  jsVarBlocks[0] = malloc(sizeof(JsVar) * JSVAR_BLOCK_SIZE);
#endif

  jsVarFirstEmpty = jsvInitJsVars(1/*first*/, jsVarsSize);
  jsvSoftInit();
}

void jsvKill() {
#ifdef RESIZABLE_JSVARS
  unsigned int i;
  for (i=0;i<jsVarsSize>>JSVAR_BLOCK_SHIFT;i++)
    free(jsVarBlocks[i]);
  free(jsVarBlocks);
  jsVarBlocks = 0;
  jsVarsSize = 0;
#endif
}

/** Find or create the ROOT variable item - used mainly
 * if recovering from a saved state. */
JsVar *jsvFindOrCreateRoot() {
  JsVarRef i;
  for (i=1;i<=jsVarsSize;i++)
    if (jsvIsRoot(jsvGetAddressOf(i)))
      return jsvLock(i);

  return jsvRef(jsvNewWithFlags(JSV_ROOT));
}

/// Get number of memory records (JsVars) used
unsigned int jsvGetMemoryUsage() {
  unsigned int usage = 0;
  unsigned int i;
  for (i=1;i<=jsVarsSize;i++) {
    JsVar *v = jsvGetAddressOf((JsVarRef)i);
    if ((v->flags&JSV_VARTYPEMASK) != JSV_UNUSED) {
      usage++;
      if (jsvIsFlatString(v)) {
        unsigned int b = (unsigned int)jsvGetFlatStringBlocks(v);
        i+=b;
        usage+=b;
      }
    }
  }
  return usage;
}

/// Get total amount of memory records
unsigned int jsvGetMemoryTotal() {
  return jsVarsSize;
}

/// Try and allocate more memory - only works if RESIZABLE_JSVARS is defined
void jsvSetMemoryTotal(unsigned int jsNewVarCount) {
#ifdef RESIZABLE_JSVARS
  if (jsNewVarCount <= jsVarsSize) return; // never allow us to have less!
  // When resizing, we just allocate a bunch more
  unsigned int oldSize = jsVarsSize;
  unsigned int oldBlockCount = jsVarsSize >> JSVAR_BLOCK_SHIFT;
  unsigned int newBlockCount = (jsNewVarCount+JSVAR_BLOCK_SIZE-1) >> JSVAR_BLOCK_SHIFT;
  jsVarsSize = newBlockCount << JSVAR_BLOCK_SHIFT;
  // resize block table
  jsVarBlocks = realloc(jsVarBlocks, sizeof(JsVar*)*newBlockCount);
  // allocate more blocks
  unsigned int i;
  for (i=oldBlockCount;i<newBlockCount;i++)
    jsVarBlocks[i] = malloc(sizeof(JsVar) * JSVAR_BLOCK_SIZE);
  /** and now reset all the newly allocated vars. We know jsVarFirstEmpty
   * is 0 (because jsiFreeMoreMemory returned 0) so we can just assign it.  */
  assert(!jsVarFirstEmpty);
  jsVarFirstEmpty = jsvInitJsVars(oldSize+1, jsVarsSize-oldSize);
  // jsiConsolePrintf("Resized memory from %d blocks to %d\n", oldBlockCount, newBlockCount);
#else
  NOT_USED(jsNewVarCount);
  assert(0);
#endif
}

/// Get whether memory is full or not
bool jsvIsMemoryFull() {
  return !jsVarFirstEmpty;
}

// Show what is still allocated, for debugging memory problems
void jsvShowAllocated() {
  JsVarRef i;
  for (i=1;i<=jsVarsSize;i++) {
    if ((jsvGetAddressOf(i)->flags&JSV_VARTYPEMASK) != JSV_UNUSED) {
      jsiConsolePrintf("USED VAR #%d:",i);
      jsvTrace(jsvGetAddressOf(i), 2);
    }
  }
}

bool jsvHasCharacterData(const JsVar *v) {
  return jsvIsString(v) || jsvIsStringExt(v);
}

bool jsvHasStringExt(const JsVar *v) {
  return jsvIsString(v) || jsvIsStringExt(v);
}

bool jsvHasChildren(const JsVar *v) {
  return jsvIsFunction(v) || jsvIsObject(v) || jsvIsArray(v) || jsvIsRoot(v);
}

/// Is this variable a type that uses firstChild to point to a single Variable (ie. it doesn't have multiple children)
bool jsvHasSingleChild(const JsVar *v) {
  return jsvIsArrayBuffer(v) ||
         (jsvIsName(v) && !jsvIsNameWithValue(v));
}

void jsvResetVariable(JsVar *v, JsVarFlags flags) {
  assert((v->flags&JSV_VARTYPEMASK) == JSV_UNUSED);
  // make sure we clear all data...
  ((unsigned int*)&v->varData.integer)[0] = 0;
  ((unsigned int*)&v->varData.integer)[1] = 0;
  // and the rest...
  jsvSetNextSibling(v, 0);
  jsvSetPrevSibling(v, 0);
  jsvSetRefs(v, 0);
  jsvSetFirstChild(v, 0);
  jsvSetLastChild(v, 0);
  // set flags
  assert(!(flags & JSV_LOCK_MASK));
  v->flags = flags | JSV_LOCK_ONE;
}

JsVar *jsvNewWithFlags(JsVarFlags flags) {
  if (jsVarFirstEmpty!=0) {
    assert(jsvGetAddressOf(jsVarFirstEmpty)->flags == JSV_UNUSED);
    jshInterruptOff(); // to allow this to be used from an IRQ
    JsVar *v = jsvLock(jsVarFirstEmpty);
    jsVarFirstEmpty = jsvGetNextSibling(v); // move our reference to the next in the free list
    jshInterruptOn();
    jsvResetVariable(v, flags); // setup variable, and add one lock
    // return pointer
    return v;
  }
  jsErrorFlags |= JSERR_LOW_MEMORY;
  /* we don't have memory - second last hope - run garbage collector */
  if (jsvGarbageCollect())
    return jsvNewWithFlags(flags); // if it freed something, continue
  /* we don't have memory - last hope - ask jsInteractive to try and free some it
   may have kicking around */
  if (jsiFreeMoreMemory())
    return jsvNewWithFlags(flags);
  /* We couldn't claim any more memory by Garbage collecting... */
#ifdef RESIZABLE_JSVARS
  jsvSetMemoryTotal(jsVarsSize*2);
  return jsvNewWithFlags(flags);
#else
  // On a micro, we're screwed.
  if (!(jsErrorFlags&JSERR_MEMORY))
    jsError("Out of Memory!");
  jsErrorFlags |= JSERR_MEMORY;
  jspSetInterrupted(true);
  return 0;
#endif
}

ALWAYS_INLINE void jsvFreePtrInternal(JsVar *var) {
  assert(jsvGetLocks(var)==0);
  var->flags = JSV_UNUSED;
  // add this to our free list
  jshInterruptOff(); // to allow this to be used from an IRQ
  jsvSetNextSibling(var, jsVarFirstEmpty);
  jsVarFirstEmpty = jsvGetRef(var);
  jshInterruptOn();
}

ALWAYS_INLINE void jsvFreePtr(JsVar *var) {
    /* To be here, we're not supposed to be part of anything else. If
     * we were, we'd have been freed by jsvGarbageCollect */
    assert((!jsvGetNextSibling(var) && !jsvGetPrevSibling(var)) || // check that next/prevSibling are not set
           jsvIsRefUsedForData(var) ||  // UNLESS we're part of a string and nextSibling/prevSibling are used for string data
           (jsvIsName(var) && (jsvGetNextSibling(var)==jsvGetPrevSibling(var)))); // UNLESS we're signalling that we're jsvIsNewChild

    // Names that Link to other things
    if (jsvIsNameWithValue(var)) {
      jsvSetFirstChild(var, 0); // it just contained random data - zero it
    } else if (jsvHasSingleChild(var)) {
      if (jsvGetFirstChild(var)) {
        JsVar *child = jsvLock(jsvGetFirstChild(var));
        jsvUnRef(child); jsvSetFirstChild(var, 0); // unlink the child
        jsvUnLock(child); // unlock should trigger a free
      }
    }
    /* No else, because a String Name may have a single child, but
     * also StringExts  */

    /* Now, free children - see jsvar.h comments for how! */
    if (jsvHasStringExt(var)) {
      // Free the string without recursing
      JsVarRef stringDataRef = jsvGetLastChild(var);
      jsvSetLastChild(var, 0);
      while (stringDataRef) {
        JsVar *child = jsvGetAddressOf(stringDataRef);
        assert(jsvIsStringExt(child));
        stringDataRef = jsvGetLastChild(child);
        jsvFreePtrInternal(child);
      }
      // We might be a flat string
      if (jsvIsFlatString(var)) {
        // in which case we need to free all the blocks.
        size_t count = jsvGetFlatStringBlocks(var);
        JsVarRef i = (JsVarRef)(jsvGetRef(var)+count);
        // do it in reverse, so the free list ends up in kind of the right order
        while (count--) {
          JsVar *p = jsvGetAddressOf(i--);
          p->flags = JSV_UNUSED; // just so the assert in jsvFreePtrInternal doesn't get fed up
          jsvFreePtrInternal(p);
        }
      }
    }
    /* NO ELSE HERE - because jsvIsNewChild stuff can be for Names, which
    can be ints or strings */

    if (jsvHasChildren(var)) {
      JsVarRef childref = jsvGetFirstChild(var);
      jsvSetFirstChild(var, 0);
      jsvSetLastChild(var, 0);
      while (childref) {
        JsVar *child = jsvLock(childref);
        assert(jsvIsName(child));
        childref = jsvGetNextSibling(child);
        jsvSetPrevSibling(child, 0);
        jsvSetNextSibling(child, 0);
        jsvUnRef(child);
        jsvUnLock(child);
      }
    } else {
#if JSVARREF_SIZE==1
      assert(jsvIsFloat(var) || !jsvGetFirstChild(var));
      assert(jsvIsFloat(var) || !jsvGetLastChild(var));
#else
      assert(!jsvGetFirstChild(var));
      assert(!jsvGetLastChild(var));
#endif
      if (jsvIsName(var)) {
        assert(jsvGetNextSibling(var)==jsvGetPrevSibling(var)); // the case for jsvIsNewChild
        if (jsvGetNextSibling(var)) {
          jsvUnRefRef(jsvGetNextSibling(var));
          jsvUnRefRef(jsvGetPrevSibling(var));
        }
      }
    }

    // free!
    jsvFreePtrInternal(var);
}

/// Get a reference from a var - SAFE for null vars
ALWAYS_INLINE JsVarRef jsvGetRef(JsVar *var) {
    if (!var) return 0;
 #ifdef RESIZABLE_JSVARS
    unsigned int i, c = jsVarsSize>>JSVAR_BLOCK_SHIFT;
    for (i=0;i<c;i++) {
      if (var>=jsVarBlocks[i] && var<&jsVarBlocks[i][JSVAR_BLOCK_SIZE]) {
        JsVarRef r = (JsVarRef)(1 + (i<<JSVAR_BLOCK_SHIFT) + (var - jsVarBlocks[i]));
        return r;
      }
    }
    return 0;
 #else
    return (JsVarRef)(1 + (var - jsVars));
 #endif
}

/// Lock this reference and return a pointer - UNSAFE for null refs
ALWAYS_INLINE JsVar *jsvLock(JsVarRef ref) {
  JsVar *var = jsvGetAddressOf(ref);
  //var->locks++;
  assert(jsvGetLocks(var) < JSV_LOCK_MAX);
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
ALWAYS_INLINE JsVar *jsvLockAgain(JsVar *var) {
  assert(var);
  assert(jsvGetLocks(var) < JSV_LOCK_MAX);
  var->flags += JSV_LOCK_ONE;
  return var;
}

/// Lock this pointer and return a pointer - UNSAFE for null pointer
ALWAYS_INLINE JsVar *jsvLockAgainSafe(JsVar *var) {
  return var ? jsvLockAgain(var) : 0;
}

// CALL ONLY FROM jsvUnlock
// jsvGetLocks(var) must == 0
static NO_INLINE void jsvUnLockFreeIfNeeded(JsVar *var) {
  assert(jsvGetLocks(var) == 0);
  /* if we know we're free, then we can just free this variable right now.
   * Loops of variables are handled by the Garbage Collector.
   * Note: we checked locks already in jsvUnLock as it is fastest to check */
  if (jsvGetRefs(var) == 0 && jsvHasRef(var) && (var->flags&JSV_VARTYPEMASK)!=JSV_UNUSED) {
    jsvFreePtr(var);
  }
}


/// Unlock this variable - this is SAFE for null variables
ALWAYS_INLINE void jsvUnLock(JsVar *var) {
  if (!var) return;
  assert(jsvGetLocks(var)>0);
  var->flags -= JSV_LOCK_ONE;
  // Now see if we can properly free the data
  // Note: we check locks first as they are already in a register
  if ((var->flags & JSV_LOCK_MASK) == 0) jsvUnLockFreeIfNeeded(var);
}


/// Reference - set this variable as used by something
JsVar *jsvRef(JsVar *var) {
  assert(var && jsvHasRef(var));
  jsvSetRefs(var, (JsVarRefCounter)(jsvGetRefs(var)+1));
  return var;
}

/// Unreference - set this variable as not used by anything
void jsvUnRef(JsVar *var) {
  assert(var && jsvGetRefs(var)>0 && jsvHasRef(var));
  jsvSetRefs(var, (JsVarRefCounter)(jsvGetRefs(var)-1));
  // locks are never 0 here, so why bother checking!
  assert(jsvGetLocks(var)>0);
}

/// Helper fn, Reference - set this variable as used by something
JsVarRef jsvRefRef(JsVarRef ref) {
  JsVar *v;
  assert(ref);
  v = jsvLock(ref);
  assert(!jsvIsStringExt(v));
  jsvRef(v);
  jsvUnLock(v);
  return ref;
}

/// Helper fn, Unreference - set this variable as not used by anything
JsVarRef jsvUnRefRef(JsVarRef ref) {
  JsVar *v;
  assert(ref);
  v = jsvLock(ref);
  assert(!jsvIsStringExt(v));
  jsvUnRef(v);
  jsvUnLock(v);
  return 0;
}

JsVar *jsvNewFlatStringOfLength(unsigned int byteLength) {
  // Work out how many blocks we need. One for the header, plus some for the characters
  size_t blocks = 1 + ((byteLength+sizeof(JsVar)-1) / sizeof(JsVar));
  // Now try and find them
  unsigned int blockCount = 0;
  JsVarRef i;
  for (i=1;i<=jsVarsSize;i++)  {
    JsVar *var = jsvGetAddressOf(i);
    if ((var->flags&JSV_VARTYPEMASK) == JSV_UNUSED) {
      blockCount++;
      if (blockCount>=blocks) { // Wohoo! We found enough blocks
        var = jsvGetAddressOf((JsVarRef)(unsigned int)((unsigned)i+1-blocks)); // the first block
        // Set up the header block (including one lock)
        jsvResetVariable(var, JSV_FLAT_STRING);
        var->varData.integer = (JsVarInt)byteLength;
        // clear data
        memset(sizeof(JsVar)+(char*)var, 0, sizeof(JsVar)*(blocks-1));
        // Now re-link all the free variables
        jsvCreateEmptyVarList();
        return var;
      }
    } else {
      blockCount = 0; // non-continuous
      if (jsvIsFlatString(var))
        i = (JsVarRef)(i+jsvGetFlatStringBlocks(var));
    }
  }
  // can't make it - return undefined
  return 0;
}

JsVar *jsvNewFromString(const char *str) {
  // Create a var
  JsVar *first = jsvNewWithFlags(JSV_STRING_0);
  if (!first) {
    jsWarn("Unable to create string as not enough memory");
    return 0;
  }
  // Now we copy the string, but keep creating new jsVars if we go
  // over the end
  JsVar *var = jsvLockAgain(first);
  while (*str) {
    // quickly set contents to 0
    var->varData.integer = 0;
    // copy data in
    size_t i, l = jsvGetMaxCharactersInVar(var);
    for (i=0;i<l && *str;i++)
      var->varData.str[i] = *(str++);
    // we already set the variable data to 0, so no need for adding one

    // we've stopped if the string was empty
    jsvSetCharactersInVar(var, i);

    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (*str) {
      JsVar *next = jsvNewWithFlags(JSV_STRING_EXT_0);
      if (!next) {
        jsWarn("Truncating string as not enough memory");
        jsvUnLock(var);
        return first;
      }
      // we don't ref, because  StringExts are never reffed as they only have one owner (and ALWAYS have an owner)
      jsvSetLastChild(var, jsvGetRef(next));
      jsvUnLock(var);
      var = next;
    }
  }
  jsvUnLock(var);
  // return
  return first;
}

JsVar *jsvNewStringOfLength(unsigned int byteLength) {
  // Create a var
    JsVar *first = jsvNewWithFlags(JSV_STRING_0);
    if (!first) {
      jsWarn("Unable to create string as not enough memory");
      return 0;
    }
    // Now zero the string, but keep creating new jsVars if we go
    // over the end
    JsVar *var = jsvLockAgain(first);
    while (byteLength>0) {
      // copy data in
      size_t i, l = jsvGetMaxCharactersInVar(var);
      for (i=0;i<l && byteLength>0;i++,byteLength--)
        var->varData.str[i] = 0;
      // might as well shove a zero terminator on it if we can
      if (i<l) var->varData.str[i]=0;
      // we've stopped if the string was empty
      jsvSetCharactersInVar(var, i);

      // if there is still some left, it's because we filled up our var...
      // make a new one, link it in, and unlock the old one.
      if (byteLength>0) {
        JsVar *next = jsvNewWithFlags(JSV_STRING_EXT_0);
        if (!next) {
          jsWarn("Truncating string as not enough memory");
          jsvUnLock(var);
          return first;
        }
        // we don't ref, because  StringExts are never reffed as they only have one owner (and ALWAYS have an owner)
        jsvSetLastChild(var, jsvGetRef(next));
        jsvUnLock(var);
        var = next;
      }
    }
    jsvUnLock(var);
    // return
    return first;
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
JsVar *jsvNewFromLongInteger(long long value) {
  if (value>=-2147483648LL && value<=2147483647LL)
    return jsvNewFromInteger((JsVarInt)value);
  else
    return jsvNewFromFloat((JsVarFloat)value);
}


JsVar *jsvMakeIntoVariableName(JsVar *var, JsVar *valueOrZero) {
  if (!var) return 0;
  assert(jsvGetRefs(var)==0); // make sure it's unused
  assert(jsvIsSimpleInt(var) || jsvIsString(var));
  if ((var->flags & JSV_VARTYPEMASK)==JSV_INTEGER) {
    int t = JSV_NAME_INT;
    if ((jsvIsInt(valueOrZero) || jsvIsBoolean(valueOrZero)) && !jsvIsPin(valueOrZero)) {
      JsVarInt v = valueOrZero->varData.integer;
      if (v>=JSVARREF_MIN && v<=JSVARREF_MAX) {
        t = jsvIsInt(valueOrZero) ? JSV_NAME_INT_INT : JSV_NAME_INT_BOOL;
        jsvSetFirstChild(var, (JsVarRef)v);
        valueOrZero = 0;
      }
    }
    var->flags = (JsVarFlags)(var->flags & ~JSV_VARTYPEMASK) | t;
  } else if ((var->flags & JSV_VARTYPEMASK)>=JSV_STRING_0 && (var->flags & JSV_VARTYPEMASK)<=JSV_STRING_MAX) {
    size_t t = JSV_NAME_STRING_0;
    if (jsvIsInt(valueOrZero) && !jsvIsPin(valueOrZero)) {
      JsVarInt v = valueOrZero->varData.integer;
      if (v>=JSVARREF_MIN && v<=JSVARREF_MAX) {
        t = JSV_NAME_STRING_INT_0;
        jsvSetFirstChild(var, (JsVarRef)v);
        valueOrZero = 0;
      }
    }
    var->flags = (var->flags & (JsVarFlags)~JSV_VARTYPEMASK) | (t+jsvGetCharactersInVar(var));
  } else assert(0);

  if (valueOrZero)
    jsvSetFirstChild(var, jsvGetRef(jsvRef(valueOrZero)));
  return var;
}

void jsvMakeFunctionParameter(JsVar *v) {
  assert(jsvIsString(v));
  if (!jsvIsName(v)) jsvMakeIntoVariableName(v,0);
  v->flags = (JsVarFlags)(v->flags | JSV_NATIVE);
}

JsVar *jsvNewFromPin(int pin) {
  JsVar *v = jsvNewFromInteger((JsVarInt)pin);
  if (v) {
    v->flags = (JsVarFlags)((v->flags & ~JSV_VARTYPEMASK) | JSV_PIN);
  }
  return v;
}

/// Create an array containing the given elements
JsVar *jsvNewArray(JsVar **elements, int elementCount) {
  JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
  if (!arr) return 0;
  int i;
  for (i=0;i<elementCount;i++)
    jsvArrayPush(arr, elements[i]);
  return arr;
}

JsVar *jsvNewNativeFunction(void (*ptr)(void), unsigned short argTypes) {
  JsVar *func = jsvNewWithFlags(JSV_FUNCTION | JSV_NATIVE);
  if (!func) return 0;
  func->varData.native.ptr = ptr;
  func->varData.native.argTypes = argTypes;
  return func;
}

void *jsvGetNativeFunctionPtr(const JsVar *function) {
  /* see descriptions in jsvar.h. If we have a child called JSPARSE_FUNCTION_CODE_NAME
   * then we execute code straight from that */
  JsVar *flatString = jsvFindChildFromString((JsVar*)function, JSPARSE_FUNCTION_CODE_NAME, 0);
  if (flatString) {
    flatString = jsvSkipNameAndUnLock(flatString);
    void *v = (void*)((size_t)function->varData.native.ptr + (char*)jsvGetFlatStringPointer(flatString));
    jsvUnLock(flatString);
    return v;
  } else
    return (void *)function->varData.native.ptr;
}

/// Create a new ArrayBuffer backed by the given string. If length is not specified, it will be worked out
JsVar *jsvNewArrayBufferFromString(JsVar *str, unsigned int lengthOrZero) {
  JsVar *arr = jsvNewWithFlags(JSV_ARRAYBUFFER);
  if (!arr) return 0;
  jsvSetFirstChild(arr, jsvGetRef(jsvRef(str)));
  arr->varData.arraybuffer.type = ARRAYBUFFERVIEW_ARRAYBUFFER;
  arr->varData.arraybuffer.byteOffset = 0;
  if (lengthOrZero==0) lengthOrZero = (unsigned int)jsvGetStringLength(str);
  arr->varData.arraybuffer.length = (unsigned short)lengthOrZero;
  return arr;
}

bool jsvIsBasicVarEqual(JsVar *a, JsVar *b) {
  // quick checks
  if (a==b) return true;
  if (!a || !b) return false; // one of them is undefined
  // OPT: would this be useful as compare instead?
  assert(jsvIsBasic(a) && jsvIsBasic(b));
  if (jsvIsNumeric(a) && jsvIsNumeric(b)) {
    if (jsvIsIntegerish(a)) {
      if (jsvIsIntegerish(b)) {
        return a->varData.integer == b->varData.integer;
      } else {
        assert(jsvIsFloat(b));
        return a->varData.integer == b->varData.floating;
      }
    } else {
      assert(jsvIsFloat(a));
      if (jsvIsIntegerish(b)) {
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
    return false; // make compiler happy
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
const char *jsvGetConstString(const JsVar *v) {
    if (jsvIsUndefined(v)) {
      return "undefined";
    } else if (jsvIsNull(v)) {
      return "null";
    } else if (jsvIsBoolean(v)) {
      return jsvGetBool(v) ? "true" : "false";
    }
    return 0;
}

/// Return the 'type' of the JS variable (eg. JS's typeof operator)
const char *jsvGetTypeOf(const JsVar *v) {
  if (jsvIsUndefined(v)) return "undefined";
  if (jsvIsNull(v) || jsvIsObject(v) || jsvIsArray(v)) return "object";
  if (jsvIsFunction(v)) return "function";
  if (jsvIsString(v)) return "string";
  if (jsvIsBoolean(v)) return "boolean";
  if (jsvIsNumeric(v)) return "number";
  return "?";
}

/// Return the JsVar, or if it's an object and has a valueOf function, call that
JsVar *jsvGetValueOf(JsVar *v) {
  if (!jsvIsObject(v)) return jsvLockAgainSafe(v);
  JsVar *valueOf = jspGetNamedField(v, "valueOf", false);
  if (!jsvIsFunction(valueOf)) {
    jsvUnLock(valueOf);
    return jsvLockAgain(v);
  }
  v = jspeFunctionCall(valueOf, 0, v, false, 0, 0);
  jsvUnLock(valueOf);
  return v;
}

/// Save this var as a string to the given buffer, and return how long it was (return val doesn't include terminating 0)
size_t jsvGetString(const JsVar *v, char *str, size_t len) {
   const char *s = jsvGetConstString(v);
   if (s) {
     strncpy(str, s, len);
     return strlen(s);
   } else if (jsvIsInt(v)) {
     itostr(v->varData.integer, str, 10);
     return strlen(str);
   } else if (jsvIsFloat(v)) {
     ftoa_bounded(v->varData.floating, str, len);
     return strlen(str);
   } else if (jsvHasCharacterData(v)) {
      assert(!jsvIsStringExt(v));
      size_t l = len;
      JsvStringIterator it;
      jsvStringIteratorNewConst(&it, v, 0);
      while (jsvStringIteratorHasChar(&it)) {
        if (l--<=1) {
          *str = 0;
          jsWarn("jsvGetString overflowed\n");
          jsvStringIteratorFree(&it);
          return len;
        }
        *(str++) = jsvStringIteratorGetChar(&it);
        jsvStringIteratorNext(&it);
      }
      jsvStringIteratorFree(&it);
      *str = 0;
      return len-l;
    } else {
      // Try and get as a JsVar string, and try again
      JsVar *stringVar = jsvAsString((JsVar*)v, false); // we know we're casting to non-const here
      if (stringVar) {
        size_t l = jsvGetString(stringVar, str, len); // call again - but this time with converted var
        jsvUnLock(stringVar);
        return l;
      } else {
        strncpy(str, "", len);
        jsExceptionHere(JSET_INTERNALERROR, "Variable type cannot be converted to string");
        return 0;
      }
    }
}

/// Get len bytes of string data from this string. Does not error if string len is not equal to len
size_t jsvGetStringChars(const JsVar *v, size_t startChar, char *str, size_t len) {
  assert(jsvHasCharacterData(v));
  size_t l = len;
  JsvStringIterator it;
  jsvStringIteratorNewConst(&it, v, startChar);
  while (jsvStringIteratorHasChar(&it)) {
    if (l--<=0) {
      jsvStringIteratorFree(&it);
      return len;
    }
    *(str++) = jsvStringIteratorGetChar(&it);
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  *str = 0;
  return len-l;
}

/// Set the Data in this string. This must JUST overwrite - not extend or shrink
void jsvSetString(JsVar *v, char *str, size_t len) {
  assert(jsvHasCharacterData(v));
  assert(len == jsvGetStringLength(v));

  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, 0);
  size_t i;
  for (i=0;i<len;i++) {
    jsvStringIteratorSetChar(&it, str[i]);
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
}

/** If var is a string, lock and return it, else
 * create a new string. unlockVar means this will auto-unlock 'var'  */
JsVar *jsvAsString(JsVar *v, bool unlockVar) {
  JsVar *str = 0;
  // If it is string-ish, but not quite a string, copy it
  if (jsvHasCharacterData(v) && jsvIsName(v)) {
    str = jsvNewFromStringVar(v,0,JSVAPPENDSTRINGVAR_MAXLENGTH);
  } else if (jsvIsString(v)) { // If it is a string - just return a reference
    str = jsvLockAgain(v);
  } else if (jsvIsObject(v)) { // If it is an object and we can call toString on it
    JsVar *toStringFn = jspGetNamedField(v, "toString", false);
    if (toStringFn && toStringFn->varData.native.ptr != (void (*)(void))jswrap_object_toString) {
      // Function found and it's not the default one - execute it
      JsVar *result = jspExecuteFunction(toStringFn,v,0,0);
      jsvUnLock(toStringFn);
      return jsvAsString(result, true);
    } else {
      jsvUnLock(toStringFn);
      return jsvNewFromString("[object Object]");
    }
  } else {
    const char *constChar = jsvGetConstString(v);
    char buf[JS_NUMBER_BUFFER_SIZE];
    if (constChar) {
      // if we could get this as a simple const char, do that..
      str = jsvNewFromString(constChar);
    } else if (jsvIsPin(v)) {
      jshGetPinString(buf, (Pin)v->varData.integer);
      str = jsvNewFromString(buf);
    } else if (jsvIsInt(v)) {
      itostr(v->varData.integer, buf, 10);
      str = jsvNewFromString(buf);
    } else if (jsvIsFloat(v)) {
      ftoa_bounded(v->varData.floating, buf, sizeof(buf));
      str = jsvNewFromString(buf);
    } else if (jsvIsArray(v) || jsvIsArrayBuffer(v)) {
      JsVar *filler = jsvNewFromString(",");
      str = jsvArrayJoin(v, filler);
      jsvUnLock(filler);
    } else if (jsvIsFunction(v)) {
      str = jsvNewFromEmptyString();
      if (str) jsfGetJSON(v, str, JSON_NONE);
    } else {
      jsExceptionHere(JSET_INTERNALERROR, "Variable type cannot be converted to string");
      str = 0;
    }
  }

  if (unlockVar) jsvUnLock(v);
  return str;
}

JsVar *jsvAsFlatString(JsVar *var) {
  if (jsvIsFlatString(var)) return jsvLockAgain(var);
  JsVar *str = jsvAsString(var, false);
  size_t len = jsvGetStringLength(str);
  JsVar *flat = jsvNewFlatStringOfLength((unsigned int)len);
  if (flat) {
    JsvStringIterator src;
    JsvStringIterator dst;
    jsvStringIteratorNew(&src, str, 0);
    jsvStringIteratorNew(&dst, flat, 0);
    while (len--) {
      jsvStringIteratorSetChar(&dst, jsvStringIteratorGetChar(&src));    
      if (len>0) {
        jsvStringIteratorNext(&src);
        jsvStringIteratorNext(&dst);  
      }
    }
    jsvStringIteratorFree(&src);
    jsvStringIteratorFree(&dst);
  }
  jsvUnLock(str);
  return flat;
}

/** Given a JsVar meant to be an index to an array, convert it to
 * the actual variable type we'll use to access the array. For example
 * a["0"] is actually translated to a[0]
 */
JsVar *jsvAsArrayIndex(JsVar *index) {
  if (jsvIsSimpleInt(index)) {
    return jsvLockAgain(index); // we're ok!
  } else if (jsvIsString(index)) {
    /* Index filtering (bug #19) - if we have an array index A that is:
     is_string(A) && int_to_string(string_to_int(A)) == A
     then convert it to an integer. Shouldn't be too nasty for performance
     as we only do this when accessing an array with a string */
    if (jsvIsStringNumericStrict(index))
      return jsvNewFromInteger(jsvGetInteger(index));
  } else if (jsvIsFloat(index)) {
    // if it's a float that is actually integral, return an integer...
    JsVarFloat v = jsvGetFloat(index);
    JsVarInt vi = jsvGetInteger(index);
    if (v == vi) return jsvNewFromInteger(vi);
  }

  // else if it's not a simple numeric type, convert it to a string
  return jsvAsString(index, false);
}

/** Same as jsvAsArrayIndex, but ensures that 'index' is unlocked */
JsVar *jsvAsArrayIndexAndUnLock(JsVar *a) {
  JsVar *b = jsvAsArrayIndex(a);
  jsvUnLock(a);
  return b;
}

/// Returns true if the string is empty - faster than jsvGetStringLength(v)==0
bool jsvIsEmptyString(JsVar *v) {
  if (!jsvHasCharacterData(v)) return true;
  return jsvGetCharactersInVar(v)==0;
}

size_t jsvGetStringLength(JsVar *v) {
  size_t strLength = 0;
  JsVar *var = v;
  JsVarRef ref = 0;

  if (!jsvHasCharacterData(v)) return 0;

  while (var) {
    JsVarRef refNext = jsvGetLastChild(var);
    strLength += jsvGetCharactersInVar(var);

    // Go to next
    if (ref) jsvUnLock(var); // note use of if (ref), not var
    ref = refNext;
    var = ref ? jsvLock(ref) : 0;
  }
  if (ref) jsvUnLock(var); // note use of if (ref), not var
  return strLength;
}

size_t jsvGetFlatStringBlocks(JsVar *v) {
  assert(jsvIsFlatString(v));
  return ((size_t)v->varData.integer+sizeof(JsVar)-1) / sizeof(JsVar);
}

char *jsvGetFlatStringPointer(JsVar *v) {
  assert(jsvIsFlatString(v));
  if (!jsvIsFlatString(v)) return 0;
  return (char*)v+1; // pointer to the next JsVar
}

//  IN A STRING  get the number of lines in the string (min=1)
size_t jsvGetLinesInString(JsVar *v) {
  size_t lines = 1;
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
size_t jsvGetCharsOnLine(JsVar *v, size_t line) {
  size_t currentLine = 1;
  size_t chars = 0;
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
void jsvGetLineAndCol(JsVar *v, size_t charIdx, size_t* line, size_t *col) {
  size_t x = 1;
  size_t y = 1;
  size_t n = 0;
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
size_t jsvGetIndexFromLineAndCol(JsVar *v, size_t line, size_t col) {
  size_t x = 1;
  size_t y = 1;
  size_t n = 0;
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
  assert(jsvIsString(var));
  JsVar *block = jsvLockAgain(var);
  // Find the block at end of the string...
  while (jsvGetLastChild(block)) {
    JsVarRef next = jsvGetLastChild(block);
    jsvUnLock(block);
    block = jsvLock(next);
  }
  // find how full the block is
  size_t blockChars = jsvGetCharactersInVar(block);
  // now start appending
  while (*str) {
    // copy data in
    size_t i, l=jsvGetMaxCharactersInVar(block);
    for (i=blockChars;i<l && *str;i++) {
      block->varData.str[i] = *(str++);
    }
    jsvSetCharactersInVar(block, i);
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (*str) {
      JsVar *next = jsvNewWithFlags(JSV_STRING_EXT_0);
      if (!next) break;
      // we don't ref, because  StringExts are never reffed as they only have one owner (and ALWAYS have an owner)
      jsvSetLastChild(block, jsvGetRef(next));
      jsvUnLock(block);
      block = next;
      blockChars=0; // it's new, so empty
    }
  }
  jsvUnLock(block);
}

// Append the given string to this one - but does not use null-terminated strings. returns false on failure (from out of memory)
bool jsvAppendStringBuf(JsVar *var, const char *str, size_t length) {
  assert(jsvIsString(var));
  JsVar *block = jsvLockAgain(var);
  // Find the block at end of the string...
  while (jsvGetLastChild(block)) {
    JsVarRef next = jsvGetLastChild(block);
    jsvUnLock(block);
    block = jsvLock(next);
  }
  // find how full the block is
  size_t blockChars = jsvGetCharactersInVar(block);
  // now start appending
  while (length) {
    // copy data in
    size_t i, l=jsvGetMaxCharactersInVar(block);
    for (i=blockChars;i<l && length;i++) {
      block->varData.str[i] = *(str++);
      length--;
    }
    jsvSetCharactersInVar(block, i);
    // if there is still some left, it's because we filled up our var...
    // make a new one, link it in, and unlock the old one.
    if (length) {
      JsVar *next = jsvNewWithFlags(JSV_STRING_EXT_0);
      if (!next) {
        jsvSetLastChild(block, 0);
        jsvUnLock(block);
        return false;
      }
      // we don't ref, because  StringExts are never reffed as they only have one owner (and ALWAYS have an owner)
      jsvSetLastChild(block, jsvGetRef(next));
      jsvUnLock(block);
      block = next;
      blockChars=0; // it's new, so empty
    }
  }
  jsvUnLock(block);
  return true;
}

/// Special version of append designed for use with vcbprintf_callback (See jsvAppendPrintf)
void jsvStringIteratorPrintfCallback(const char *str, void *user_data) {
  while (*str)
    jsvStringIteratorAppend((JsvStringIterator *)user_data, *(str++));
}

void jsvAppendPrintf(JsVar *var, const char *fmt, ...) {
  JsvStringIterator it;
  jsvStringIteratorNew(&it, var, 0);
  jsvStringIteratorGotoEnd(&it);

  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsvStringIteratorPrintfCallback,&it, fmt, argp);
  va_end(argp);

  jsvStringIteratorFree(&it);
}

JsVar *jsvVarPrintf( const char *fmt, ...) {
  JsVar *str = jsvNewFromEmptyString();
  if (!str) return 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  jsvStringIteratorGotoEnd(&it);

  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsvStringIteratorPrintfCallback,&it, fmt, argp);
  va_end(argp);

  jsvStringIteratorFree(&it);
  return str;
}

/** Append str to var. Both must be strings. stridx = start char or str, maxLength = max number of characters (can be JSVAPPENDSTRINGVAR_MAXLENGTH) */
void jsvAppendStringVar(JsVar *var, const JsVar *str, size_t stridx, size_t maxLength) {
  JsVar *block = jsvLockAgain(var);
  assert(jsvIsString(var));
  // Find the block at end of the string...
  while (jsvGetLastChild(block)) {
    JsVarRef next = jsvGetLastChild(block);
    jsvUnLock(block);
    block = jsvLock(next);
  }
  // find how full the block is
  size_t blockChars = jsvGetCharactersInVar(block);
  // now start appending
  JsvStringIterator it;
  jsvStringIteratorNewConst(&it, str, stridx);
  while (jsvStringIteratorHasChar(&it) && (maxLength-->0)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (blockChars >= jsvGetMaxCharactersInVar(block)) {
      jsvSetCharactersInVar(block, blockChars);
      JsVar *next = jsvNewWithFlags(JSV_STRING_EXT_0);
      if (!next) break; // out of memory
      // we don't ref, because  StringExts are never reffed as they only have one owner (and ALWAYS have an owner)
      jsvSetLastChild(block, jsvGetRef(next));
      jsvUnLock(block);
      block = next;
      blockChars=0; // it's new, so empty
    }
    block->varData.str[blockChars++] = ch;
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  jsvSetCharactersInVar(block, blockChars);
  jsvUnLock(block);
}

/** Create a new variable from a substring. argument must be a string. stridx = start char or str, maxLength = max number of characters (can be JSVAPPENDSTRINGVAR_MAXLENGTH) */
JsVar *jsvNewFromStringVar(const JsVar *str, size_t stridx, size_t maxLength) {
  JsVar *var = jsvNewFromEmptyString();
  if (var) jsvAppendStringVar(var, str, stridx, maxLength);
  return var;
}

/** Append all of str to var. Both must be strings.  */
void jsvAppendStringVarComplete(JsVar *var, const JsVar *str) {
  jsvAppendStringVar(var, str, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
}

char jsvGetCharInString(JsVar *v, size_t idx) {
  if (!jsvIsString(v)) return 0;

  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, idx);
  char ch = jsvStringIteratorGetChar(&it);
  jsvStringIteratorFree(&it);
  return ch;
}

/// Get the index of a character in a string, or -1
int jsvGetStringIndexOf(JsVar *str, char ch) {
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  while (jsvStringIteratorHasChar(&it)) {
    if (jsvStringIteratorGetChar(&it) == ch) {
      int idx = (int)jsvStringIteratorGetIndex(&it);
      jsvStringIteratorFree(&it);
      return idx;
    };
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  return -1;
}

/** Does this string contain only Numeric characters (with optional '-'/'+' at the front)? NOT '.'/'e' and similar (allowDecimalPoint is for '.' only) */
bool jsvIsStringNumericInt(const JsVar *var, bool allowDecimalPoint) {
  assert(jsvIsString(var));
  JsvStringIterator it;
  jsvStringIteratorNewConst(&it, var, 0); // we know it's non const

  // skip whitespace
  while (jsvStringIteratorHasChar(&it) && isWhitespace(jsvStringIteratorGetChar(&it)))
    jsvStringIteratorNext(&it);

  // skip a minus. if there was one
  if (jsvStringIteratorHasChar(&it) && (jsvStringIteratorGetChar(&it)=='-' || jsvStringIteratorGetChar(&it)=='+'))
    jsvStringIteratorNext(&it);
  // now check...
  int chars=0;
  while (jsvStringIteratorHasChar(&it)) {
    chars++;
    char ch = jsvStringIteratorGetChar(&it);
    if (ch=='.' && allowDecimalPoint) {
      allowDecimalPoint = false; // there can be only one
    } else if (!isNumeric(ch)) { // FIXME: should check for non-integer values (floating point?)
      jsvStringIteratorFree(&it);
      return false;
    }
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  return chars>0;
}

/** Does this string contain only Numeric characters? This is for arrays
 * and makes the assertion that int_to_string(string_to_int(var))==var */
bool jsvIsStringNumericStrict(const JsVar *var) {
  assert(jsvIsString(var));
  JsvStringIterator it;
  jsvStringIteratorNewConst(&it, var, 0);  // we know it's non const
  bool hadNonZero = false;
  bool hasLeadingZero = false;
  int chars = 0;
  while (jsvStringIteratorHasChar(&it)) {
    chars++;
    char ch = jsvStringIteratorGetChar(&it);
    if (!isNumeric(ch)) {
      // test for leading zero ensures int_to_string(string_to_int(var))==var
      jsvStringIteratorFree(&it);
      return false;
    }
    if (!hadNonZero && ch=='0') hasLeadingZero=true;
    if (ch!='0') hadNonZero=true;
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  return chars>0 && (!hasLeadingZero || chars==1);
}


JsVarInt jsvGetInteger(const JsVar *v) {
    if (!v) return 0; // undefined
    /* strtol understands about hex and octal */
    if (jsvIsNull(v)) return 0;
    if (jsvIsUndefined(v)) return 0;
    if (jsvIsIntegerish(v) || jsvIsArrayBufferName(v)) return v->varData.integer;
    if (jsvIsArray(v) && jsvGetArrayLength(v)==1)
      return jsvGetIntegerAndUnLock(jsvSkipNameAndUnLock(jsvGetArrayItem(v,0)));
    if (jsvIsFloat(v)) {
      if (isfinite(v->varData.floating))
        return (JsVarInt)(long long)v->varData.floating;
      return 0;
    }
    if (jsvIsString(v) && jsvIsStringNumericInt(v, true/* allow decimal point*/)) {
      char buf[32];
      jsvGetString(v, buf, sizeof(buf));
      return (JsVarInt)stringToInt(buf);
    }
    return 0;
}

long long jsvGetLongInteger(const JsVar *v) {
  if (jsvIsInt(v)) return jsvGetInteger(v);
  return (long long)jsvGetFloat(v);
}

long long jsvGetLongIntegerAndUnLock(JsVar *v) {
  long long i = jsvGetLongInteger(v);
  jsvUnLock(v);
  return i;
}


void jsvSetInteger(JsVar *v, JsVarInt value) {
  assert(jsvIsInt(v));
  v->varData.integer  = value;
}

bool jsvGetBool(const JsVar *v) {
  if (jsvIsString(v))
    return jsvGetStringLength((JsVar*)v)!=0;
  if (jsvIsFunction(v) || jsvIsArray(v) || jsvIsObject(v))
    return true;
  if (jsvIsFloat(v)) {
    JsVarFloat f = jsvGetFloat(v);
    return !isnan(f) && f!=0.0;
  }
  return jsvGetInteger(v)!=0;
}

JsVarFloat jsvGetFloat(const JsVar *v) {
    if (!v) return NAN; // undefined
    if (jsvIsFloat(v)) return v->varData.floating;
    if (jsvIsIntegerish(v)) return (JsVarFloat)v->varData.integer;
    if (jsvIsArray(v)) {
      JsVarInt l = jsvGetArrayLength(v);
      if (l==0) return 0; // zero element array==0 (not undefined)
      if (l==1) return jsvGetFloatAndUnLock(jsvSkipNameAndUnLock(jsvGetArrayItem(v,0)));
    }
    if (jsvIsString(v)) {
      char buf[32];
      jsvGetString(v, buf, sizeof(buf));
      if (buf[0]==0) return 0; // empty string -> 0
      if (!strcmp(buf,"Infinity")) return INFINITY;
      if (!strcmp(buf,"-Infinity")) return -INFINITY;
      return stringToFloat(buf);
    }
    return NAN;
}

/// Convert the given variable to a number
JsVar *jsvAsNumber(JsVar *var) {
  // stuff that we can just keep
  if (jsvIsInt(var) || jsvIsFloat(var)) return jsvLockAgain(var);
  // stuff that can be converted to an int
  if (jsvIsBoolean(var) ||
      jsvIsPin(var) ||
      jsvIsNull(var) ||
      jsvIsBoolean(var) ||
      jsvIsArrayBufferName(var))
    return jsvNewFromInteger(jsvGetInteger(var));
  if (jsvIsString(var) && (jsvIsEmptyString(var) || jsvIsStringNumericInt(var, false/* no decimal pt - handle that with GetFloat */))) {
    // handle strings like this, in case they're too big for an int
    char buf[32];
    jsvGetString(var, buf, sizeof(buf));
    return jsvNewFromLongInteger(stringToInt(buf));
  }
  // Else just try and get a float
  return jsvNewFromFloat(jsvGetFloat(var));
}

#ifdef SAVE_ON_FLASH
JsVarInt jsvGetIntegerAndUnLock(JsVar *v) { return _jsvGetIntegerAndUnLock(v); }
JsVarFloat jsvGetFloatAndUnLock(JsVar *v) { return _jsvGetFloatAndUnLock(v); }
bool jsvGetBoolAndUnLock(JsVar *v) { return _jsvGetBoolAndUnLock(v); }
#endif

/** Get the item at the given location in the array buffer and return the result */
size_t jsvGetArrayBufferLength(JsVar *arrayBuffer) {
  assert(jsvIsArrayBuffer(arrayBuffer));
  return arrayBuffer->varData.arraybuffer.length;
}

/** Get the String the contains the data for this arrayBuffer */
JsVar *jsvGetArrayBufferBackingString(JsVar *arrayBuffer) {
  jsvLockAgain(arrayBuffer);
  while (jsvIsArrayBuffer(arrayBuffer)) {
    JsVar *s = jsvLock(jsvGetFirstChild(arrayBuffer));
    jsvUnLock(arrayBuffer);
    arrayBuffer = s;
  }
  assert(jsvIsString(arrayBuffer));
  return arrayBuffer;
}

/** Get the item at the given location in the array buffer and return the result */
JsVar *jsvArrayBufferGet(JsVar *arrayBuffer, size_t idx) {
  JsvArrayBufferIterator it;
  jsvArrayBufferIteratorNew(&it, arrayBuffer, idx);
  JsVar *v = jsvArrayBufferIteratorGetValue(&it);
  jsvArrayBufferIteratorFree(&it);
  return v;
}

/** Set the item at the given location in the array buffer */
void jsvArrayBufferSet(JsVar *arrayBuffer, size_t idx, JsVar *value) {
  JsvArrayBufferIterator it;
  jsvArrayBufferIteratorNew(&it, arrayBuffer, idx);
  jsvArrayBufferIteratorSetValue(&it, value);
  jsvArrayBufferIteratorFree(&it);
}


/** Given an integer name that points to an arraybuffer or an arraybufferview, evaluate it and return the result */
JsVar *jsvArrayBufferGetFromName(JsVar *name) {
  assert(jsvIsArrayBufferName(name));
  size_t idx = (size_t)jsvGetInteger(name);
  JsVar *arrayBuffer = jsvLock(jsvGetFirstChild(name));
  JsVar *value = jsvArrayBufferGet(arrayBuffer, idx);
  jsvUnLock(arrayBuffer);
  return value;
}


JsVar *jsvGetFunctionArgumentLength(JsVar *functionScope) {
  JsVar *args = jsvNewWithFlags(JSV_ARRAY);
  if (!args) return 0; // out of memory

  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, functionScope);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *idx = jsvObjectIteratorGetKey(&it);
    if (jsvIsFunctionParameter(idx)) {
      JsVar *val = jsvSkipOneName(idx);
      jsvArrayPushAndUnLock(args, val);
    }
    jsvUnLock(idx);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);

  return args;
}

/** If a is a name skip it and go to what it points to - and so on.
 * ALWAYS locks - so must unlock what it returns. It MAY
 * return 0. */
JsVar *jsvSkipName(JsVar *a) {
  JsVar *pa = a;
  if (!a) return 0;
  if (jsvIsArrayBufferName(pa)) return jsvArrayBufferGetFromName(pa);
  if (jsvIsNameInt(pa)) return jsvNewFromInteger((JsVarInt)jsvGetFirstChildSigned(pa));
  if (jsvIsNameIntBool(pa)) return jsvNewFromBool(jsvGetFirstChild(pa)!=0);
  while (jsvIsName(pa)) {
    JsVarRef n = jsvGetFirstChild(pa);
    if (pa!=a) jsvUnLock(pa);
    if (!n) return 0;
    pa = jsvLock(n);
  }
  if (pa==a) jsvLockAgain(pa);
  return pa;
}

/** If a is a name skip it and go to what it points to.
 * ALWAYS locks - so must unlock what it returns. It MAY
 * return 0.  */
JsVar *jsvSkipOneName(JsVar *a) {
  JsVar *pa = a;
  if (!a) return 0;
  if (jsvIsArrayBufferName(pa)) return jsvArrayBufferGetFromName(pa);
  if (jsvIsNameInt(pa)) return jsvNewFromInteger((JsVarInt)jsvGetFirstChildSigned(pa));
  if (jsvIsNameIntBool(pa)) return jsvNewFromBool(jsvGetFirstChild(pa)!=0);
  if (jsvIsName(pa)) {
    JsVarRef n = jsvGetFirstChild(pa);
    if (pa!=a) jsvUnLock(pa);
    if (!n) return 0;
    pa = jsvLock(n);
  }
  if (pa==a) jsvLockAgain(pa);
  return pa;
}

/** If a is a's child is a name skip it and go to what it points to.
 * ALWAYS locks - so must unlock what it returns.  */
JsVar *jsvSkipToLastName(JsVar *a) {
  assert(jsvIsName(a));
  a = jsvLockAgain(a);
  while (true) {
    if (!jsvGetFirstChild(a)) return a;
    JsVar *child = jsvLock(jsvGetFirstChild(a));
    if (jsvIsName(child)) {
      jsvUnLock(a);
      a = child;
    } else {
      jsvUnLock(child);
      return a;
    }
  }
  return 0; // not called
}

/** Same as jsvSkipName, but ensures that 'a' is unlocked */
JsVar *jsvSkipNameAndUnLock(JsVar *a) {
  JsVar *b = jsvSkipName(a);
  jsvUnLock(a);
  return b;
}

/** Same as jsvSkipOneName, but ensures that 'a' is unlocked */
JsVar *jsvSkipOneNameAndUnLock(JsVar *a) {
  JsVar *b = jsvSkipOneName(a);
  jsvUnLock(a);
  return b;
}


/*
jsvIsStringEqualOrStartsWith(A, B, false) is a proper A==B
jsvIsStringEqualOrStartsWith(A, B, true) is A.startsWith(B)
*/
bool jsvIsStringEqualOrStartsWith(JsVar *var, const char *str, bool isStartsWith) {
  if (!jsvHasCharacterData(var)) {
    return 0; // not a string so not equal!
  }

  JsvStringIterator it;
  jsvStringIteratorNew(&it, var, 0);
  while (jsvStringIteratorHasChar(&it) && *str) {
    if (jsvStringIteratorGetChar(&it) != *str) {
      jsvStringIteratorFree(&it);
      if (!isStartsWith) return false;
      return *str==0;
    }
    str++;
    jsvStringIteratorNext(&it);
  }
  bool eq = jsvStringIteratorGetChar(&it)==*str; // should both be 0 if equal
  jsvStringIteratorFree(&it);
  return eq;
}

// Also see jsvIsBasicVarEqual
bool jsvIsStringEqual(JsVar *var, const char *str) {
  return jsvIsStringEqualOrStartsWith(var, str, false);
}


/** Compare 2 strings, starting from the given character positions. equalAtEndOfString means that
 * if one of the strings ends (even if the other hasn't), we treat them as equal.
 * For a basic strcmp, do: jsvCompareString(a,b,0,0,false)
 *  */
int jsvCompareString(JsVar *va, JsVar *vb, size_t starta, size_t startb, bool equalAtEndOfString) {
  JsvStringIterator ita, itb;
  jsvStringIteratorNew(&ita, va, starta);
  jsvStringIteratorNew(&itb, vb, startb);
   // step to first positions
  while (true) {
    int ca = jsvStringIteratorGetCharOrMinusOne(&ita);
    int cb = jsvStringIteratorGetCharOrMinusOne(&itb);

    if (ca != cb) {
      jsvStringIteratorFree(&ita);
      jsvStringIteratorFree(&itb);
      if ((ca<0 || cb<0) && equalAtEndOfString) return 0;
      return ca - cb;
    }
    if (ca < 0) { // both equal, but end of string
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
  if (!keepAsName) {
    JsVarFlags t = src->flags & JSV_VARTYPEMASK;
    if (t>=_JSV_NAME_INT_START && t<=_JSV_NAME_INT_END) {
      flags = (flags & ~JSV_VARTYPEMASK) | JSV_INTEGER;
    } else {
      assert((JSV_NAME_STRING_INT_0 < JSV_NAME_STRING_0) &&
             (JSV_NAME_STRING_0 < JSV_STRING_0) &&
             (JSV_STRING_0 < JSV_STRING_EXT_0)); // this relies on ordering
      assert(t>=JSV_NAME_STRING_INT_0 && t<=JSV_NAME_STRING_MAX);
      flags = (flags & (JsVarFlags)~JSV_VARTYPEMASK) | (JSV_STRING_0 + jsvGetCharactersInVar(src));
    }
  }
  JsVar *dst = jsvNewWithFlags(flags & JSV_VARIABLEINFOMASK);
  if (!dst) return 0; // out of memory

  memcpy(&dst->varData, &src->varData, JSVAR_DATA_STRING_LEN);

  jsvSetLastChild(dst, 0);
  jsvSetFirstChild(dst, 0);
  jsvSetPrevSibling(dst, 0);
  jsvSetNextSibling(dst, 0);
  // Copy LINK of what it points to
  if (linkChildren && jsvGetFirstChild(src)) {
    jsvSetFirstChild(dst, jsvRefRef(jsvGetFirstChild(src)));
  }
  // Copy extra string data if there was any
  if (jsvHasStringExt(src)) {
      // copy extra bits of string if there were any
      if (jsvGetLastChild(src)) {
        JsVar *child = jsvLock(jsvGetLastChild(src));
        JsVar *childCopy = jsvCopy(child);
        if (childCopy) { // could be out of memory
          jsvSetLastChild(dst, jsvGetRef(childCopy)); // no ref for stringext
          jsvUnLock(childCopy);
        }
        jsvUnLock(child);
      }
  } else {
    assert(jsvIsBasic(src)); // in case we missed something!
  }
  return dst;
}

JsVar *jsvCopy(JsVar *src) {
  JsVar *dst = jsvNewWithFlags(src->flags & JSV_VARIABLEINFOMASK);
  if (!dst) return 0; // out of memory
  if (!jsvIsStringExt(src)) {
    memcpy(&dst->varData, &src->varData, JSVAR_DATA_STRING_LEN);
    jsvSetLastChild(dst, 0);
    jsvSetFirstChild(dst, 0);
    jsvSetPrevSibling(dst, 0);
    jsvSetNextSibling(dst, 0);
  } else {
    // stringexts use the extra pointers after varData to store characters
    // see jsvGetMaxCharactersInVar
    memcpy(&dst->varData, &src->varData, JSVAR_DATA_STRING_MAX_LEN);
    jsvSetLastChild(dst, 0);
  }

  // Copy what names point to
  if (jsvIsName(src)) {
    if (jsvGetFirstChild(src)) {
      JsVar *child = jsvLock(jsvGetFirstChild(src));
      JsVar *childCopy = jsvRef(jsvCopy(child));
      jsvUnLock(child);
      if (childCopy) { // could have been out of memory
        jsvSetFirstChild(dst, jsvGetRef(childCopy));
        jsvUnLock(childCopy);
      }
    }
  }

  if (jsvHasStringExt(src)) {
    // copy extra bits of string if there were any
    if (jsvGetLastChild(src)) {
      JsVar *child = jsvLock(jsvGetLastChild(src));
      JsVar *childCopy = jsvCopy(child);
      if (childCopy) {// could be out of memory
        jsvSetLastChild(dst, jsvGetRef(childCopy)); // no ref for stringext
        jsvUnLock(childCopy);
      }
      jsvUnLock(child);
    }
  } else if (jsvHasChildren(src)) {
    // Copy children..
    JsVarRef vr;
    vr = jsvGetFirstChild(src);
    while (vr) {
      JsVar *name = jsvLock(vr);
      JsVar *child = jsvCopyNameOnly(name, true/*link children*/, true/*keep as name*/); // NO DEEP COPY!
      if (child) { // could have been out of memory
        jsvAddName(dst, child);
        jsvUnLock(child);
      }
      vr = jsvGetNextSibling(name);
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

  // update array length
  if (jsvIsArray(parent) && jsvIsInt(namedChild)) {
    JsVarInt index = namedChild->varData.integer;
    if (index >= jsvGetArrayLength(parent)) {
      jsvSetArrayLength(parent, index + 1, false);
    }
  }

  if (jsvGetLastChild(parent)) { // we have children already
    JsVar *insertAfter = jsvLock(jsvGetLastChild(parent));
    if (jsvIsArray(parent)) {
      // we must insert in order - so step back until we get the right place
      while (insertAfter && jsvCompareInteger(namedChild, insertAfter)<0) {
        JsVarRef prev = jsvGetPrevSibling(insertAfter);
        jsvUnLock(insertAfter);
        insertAfter = prev ? jsvLock(prev) : 0;
      }
    }

    if (insertAfter) {
      if (jsvGetNextSibling(insertAfter)) {
        // great, we're in the middle...
        JsVar *insertBefore = jsvLock(jsvGetNextSibling(insertAfter));
        jsvSetPrevSibling(insertBefore, jsvGetRef(namedChild));
        jsvSetNextSibling(namedChild, jsvGetRef(insertBefore));
        jsvUnLock(insertBefore);
      } else {
        // We're at the end - just set up the parent
        jsvSetLastChild(parent, jsvGetRef(namedChild));
      }
      jsvSetNextSibling(insertAfter, jsvGetRef(namedChild));
      jsvSetPrevSibling(namedChild, jsvGetRef(insertAfter));
      jsvUnLock(insertAfter);
    } else { // Insert right at the beginning of the array
      // Link 2 children together
      JsVar *firstChild = jsvLock(jsvGetFirstChild(parent));
      jsvSetPrevSibling(firstChild, jsvGetRef(namedChild));
      jsvUnLock(firstChild);

      jsvSetNextSibling(namedChild, jsvGetFirstChild(parent));
      // finally set the new child as the first one
      jsvSetFirstChild(parent, jsvGetRef(namedChild));
    }
  } else { // we have no children - just add it
    JsVarRef r = jsvGetRef(namedChild);
    jsvSetFirstChild(parent, r);
    jsvSetLastChild(parent, r);
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
  if (jsvIsNameWithValue(name)) {
    if (jsvIsString(name))
      name->flags = (name->flags & (JsVarFlags)~JSV_VARTYPEMASK) | (JSV_NAME_STRING_0 + jsvGetCharactersInVar(name));
    else
      name->flags = (name->flags & (JsVarFlags)~JSV_VARTYPEMASK) | JSV_NAME_INT;
    jsvSetFirstChild(name, 0);
  } else if (jsvGetFirstChild(name))
    jsvUnRefRef(jsvGetFirstChild(name)); // free existing
  if (src) {
      if (jsvIsInt(name)) {
        if ((jsvIsInt(src) || jsvIsBoolean(src)) && !jsvIsPin(src)) {
          JsVarInt v = src->varData.integer;
          if (v>=JSVARREF_MIN && v<=JSVARREF_MAX) {
            name->flags = (name->flags & (JsVarFlags)~JSV_VARTYPEMASK) | (jsvIsInt(src) ? JSV_NAME_INT_INT : JSV_NAME_INT_BOOL);
            jsvSetFirstChild(name, (JsVarRef)v);
            return name;
          }
        }
      } else if (jsvIsString(name)) {
        if (jsvIsInt(src) && !jsvIsPin(src)) {
          JsVarInt v = src->varData.integer;
          if (v>=JSVARREF_MIN && v<=JSVARREF_MAX) {
            name->flags = (name->flags & (JsVarFlags)~JSV_VARTYPEMASK) | (JSV_NAME_STRING_INT_0 + jsvGetCharactersInVar(name));
            jsvSetFirstChild(name, (JsVarRef)v);
            return name;
          }
        }
      }
      // we can link to a name if we want (so can remove the assert!)
      jsvSetFirstChild(name, jsvGetRef(jsvRef(src)));
  } else
    jsvSetFirstChild(name, 0);
  return name;
}

JsVar *jsvFindChildFromString(JsVar *parent, const char *name, bool addIfNotFound) {
  /* Pull out first 4 bytes, and ensure that everything
   * is 0 padded so that we can do a nice speedy check. */
  char fastCheck[4];
  fastCheck[0] = name[0];
  if (name[0]) {
    fastCheck[1] = name[1];
    if (name[1]) {
      fastCheck[2] = name[2];
      if (name[2]) {
        fastCheck[3] = name[3];
      } else {
        fastCheck[3] = 0;
      }
    } else {
      fastCheck[2] = 0;
      fastCheck[3] = 0;
    }
  } else {
    fastCheck[1] = 0;
    fastCheck[2] = 0;
    fastCheck[3] = 0;
  }

  assert(jsvHasChildren(parent));
  JsVarRef childref = jsvGetFirstChild(parent);
  while (childref) {
    // Don't Lock here, just use GetAddressOf - to try and speed up the finding
    // TODO: We can do this now, but when/if we move to cacheing vars, it'll break
    JsVar *child = jsvGetAddressOf(childref);
    if (*(int*)fastCheck==*(int*)child->varData.str && // speedy check of first 4 bytes
        jsvIsStringEqual(child, name)) {
       // found it! unlock parent but leave child locked
       return jsvLockAgain(child);
    }
    childref = jsvGetNextSibling(child);
  }

  JsVar *child = 0;
  if (addIfNotFound) {
    child = jsvMakeIntoVariableName(jsvNewFromString(name), 0);
    if (child) // could be out of memory
      jsvAddName(parent, child);
  }
  return child;
}

/// See jsvIsNewChild - for fields that don't exist yet
JsVar *jsvCreateNewChild(JsVar *parent, JsVar *index, JsVar *child) {
  JsVar *newChild = jsvAsName(index);
  assert(!jsvGetFirstChild(newChild));
  if (child) jsvSetValueOfName(newChild, child);
  assert(!jsvGetNextSibling(newChild) && !jsvGetPrevSibling(newChild));
  // by setting the siblings as the same, we signal that if set,
  // we should be made a member of the given object
  JsVarRef r = jsvGetRef(jsvRef(jsvRef(parent)));
  jsvSetNextSibling(newChild, r);
  jsvSetPrevSibling(newChild, r);

  return newChild;
}

/** Try and turn the supplied variable into a name. If not, make a new one. This locks again. */
JsVar *jsvAsName(JsVar *var) {
  if (jsvGetRefs(var) == 0) {
    // Not reffed - great! let's just use it
    if (!jsvIsName(var))
      var = jsvMakeIntoVariableName(var, 0);
    return jsvLockAgain(var);
  } else { // it was reffed, we must add a new one
    return jsvMakeIntoVariableName(jsvCopy(var), 0);
  }
}

/** Non-recursive finding */
JsVar *jsvFindChildFromVar(JsVar *parent, JsVar *childName, bool addIfNotFound) {
  JsVar *child;
  JsVarRef childref = jsvGetFirstChild(parent);

  while (childref) {
    child = jsvLock(childref);
    if (jsvIsBasicVarEqual(child, childName)) {
      // found it! unlock parent but leave child locked
      return child;
    }
    childref = jsvGetNextSibling(child);
    jsvUnLock(child);
  }

  child = 0;
  if (addIfNotFound && childName) {
    child = jsvAsName(childName);
    jsvAddName(parent, child);
  }
  return child;
}

void jsvRemoveChild(JsVar *parent, JsVar *child) {
    assert(jsvHasChildren(parent));
    JsVarRef childref = jsvGetRef(child);
    bool wasChild = false;
    // unlink from parent
    if (jsvGetFirstChild(parent) == childref) {
      jsvSetFirstChild(parent, jsvGetNextSibling(child));
      wasChild = true;
    }
    if (jsvGetLastChild(parent) == childref) {
      jsvSetLastChild(parent, jsvGetPrevSibling(child));
      wasChild = true;
    }
    // unlink from child list
    if (jsvGetPrevSibling(child)) {
        JsVar *v = jsvLock(jsvGetPrevSibling(child));
        assert(jsvGetNextSibling(v) == jsvGetRef(child));
        jsvSetNextSibling(v, jsvGetNextSibling(child));
        jsvUnLock(v);
        wasChild = true;
    }
    if (jsvGetNextSibling(child)) {
        JsVar *v = jsvLock(jsvGetNextSibling(child));
        assert(jsvGetPrevSibling(v) == jsvGetRef(child));
        jsvSetPrevSibling(v, jsvGetPrevSibling(child));
        jsvUnLock(v);
        wasChild = true;
    }

    jsvSetPrevSibling(child, 0);
    jsvSetNextSibling(child, 0);
    if (wasChild)
      jsvUnRef(child);
}

void jsvRemoveAllChildren(JsVar *parent) {
    assert(jsvHasChildren(parent));
    while (jsvGetFirstChild(parent)) {
      JsVar *v = jsvLock(jsvGetFirstChild(parent));
      jsvRemoveChild(parent, v);
      jsvUnLock(v);
    }
}

void jsvRemoveNamedChild(JsVar *parent, const char *name) {
  JsVar *child = jsvFindChildFromString(parent, name, false);
  if (child) {
    jsvRemoveChild(parent, child);
    jsvUnLock(child);
  }
}

/// Check if the given name is a child of the parent
bool jsvIsChild(JsVar *parent, JsVar *child) {
  assert(jsvIsArray(parent) || jsvIsObject(parent));
  assert(jsvIsName(child));
  JsVarRef childref = jsvGetRef(child);
  JsVarRef indexref;
  indexref = jsvGetFirstChild(parent);
  while (indexref) {
    if (indexref == childref) return true;
    // get next
    JsVar *indexVar = jsvLock(indexref);
    indexref = jsvGetNextSibling(indexVar);
    jsvUnLock(indexVar);
  }
  return false; // not found undefined
}

/// Get the named child of an object. If createChild!=0 then create the child
JsVar *jsvObjectGetChild(JsVar *obj, const char *name, JsVarFlags createChild) {
  if (!obj) return 0;
  assert(jsvHasChildren(obj));
  JsVar *childName = jsvFindChildFromString(obj, name, createChild!=0);
  JsVar *child = jsvSkipName(childName);
  if (!child && createChild && childName!=0/*out of memory?*/) {
    child = jsvNewWithFlags(createChild);
    jsvSetValueOfName(childName, child);
    jsvUnLock(childName);
    return child;
  }
  jsvUnLock(childName);
  return child;
}

/// Set the named child of an object, and return the child (so you can choose to unlock it if you want)
JsVar *jsvObjectSetChild(JsVar *obj, const char *name, JsVar *child) {
  assert(jsvHasChildren(obj));
  // child can actually be a name (for instance if it is a named function)
  JsVar *childName = jsvFindChildFromString(obj, name, true);
  if (!childName) return 0; // out of memory
  jsvSetValueOfName(childName, child);
  jsvUnLock(childName);
  return child;
}

int jsvGetChildren(JsVar *v) {
  //OPT: could length be stored as the value of the array?
  int children = 0;
  JsVarRef childref = jsvGetFirstChild(v);
  while (childref) {
    JsVar *child = jsvLock(childref);
    children++;
    childref = jsvGetNextSibling(child);
    jsvUnLock(child);
  }
  return children;
}

/// Get the first child's name from an object,array or function
JsVar *jsvGetFirstName(JsVar *v) {
  assert(jsvHasChildren(v));
  if (!jsvGetFirstChild(v)) return 0;
  return jsvLock(jsvGetFirstChild(v));
}

JsVarInt jsvGetArrayLength(const JsVar *arr) {
  if (!arr) return 0;
  return arr->varData.integer;
}

JsVarInt jsvSetArrayLength(JsVar *arr, JsVarInt length, bool truncate) {
  if (truncate && length < arr->varData.integer) {
    // @TODO implement truncation here
  }
  arr->varData.integer = length;
  return length;
}

JsVarInt jsvGetLength(JsVar *src) {
  if (jsvIsArray(src)) {
    return jsvGetArrayLength(src);
  } else if (jsvIsArrayBuffer(src)) {
    return (JsVarInt)jsvGetArrayBufferLength(src);
  } else if (jsvIsString(src)) {
    return (JsVarInt)jsvGetStringLength(src);
  } else if (jsvIsObject(src) || jsvIsFunction(src)) {
      return jsvGetChildren(src);
  } else {
    return 1;
  }
}

/** Count the amount of JsVars used. Mostly useful for debugging */
static size_t _jsvCountJsVarsUsedRecursive(JsVar *v, bool resetRecursionFlag) {
  // Use IS_RECURSING  flag to stop recursion
  if (resetRecursionFlag) {
    if (!(v->flags & JSV_IS_RECURSING))
      return 0;
    v->flags &= ~JSV_IS_RECURSING;
  } else {
    if (v->flags & JSV_IS_RECURSING)
      return 0;
    v->flags |= JSV_IS_RECURSING;
  }

  size_t count = 1;
  if (jsvHasSingleChild(v)) {
    JsVar *child = jsvLock(jsvGetFirstChild(v));
    count += _jsvCountJsVarsUsedRecursive(child, resetRecursionFlag);
    jsvUnLock(child);
  } else if (jsvHasChildren(v)) {
    JsVarRef childref = jsvGetFirstChild(v);
    while (childref) {
      JsVar *child = jsvLock(childref);
      count += _jsvCountJsVarsUsedRecursive(child, resetRecursionFlag);
      childref = jsvGetNextSibling(child);
      jsvUnLock(child);
    }
  } else if (jsvIsFlatString(v))
    count += jsvGetFlatStringBlocks(v);
  if (jsvHasCharacterData(v)) {
    JsVarRef childref = jsvGetLastChild(v);
    while (childref) {
      JsVar *child = jsvLock(childref);
      count++;
      childref = jsvGetLastChild(child);
      jsvUnLock(child);
    }
  }
  if (jsvIsName(v) && !jsvIsNameWithValue(v) && jsvGetFirstChild(v)) {
    JsVar *child = jsvLock(jsvGetFirstChild(v));
    count += _jsvCountJsVarsUsedRecursive(child, resetRecursionFlag);
    jsvUnLock(child);
  }
  return count;
}

/** Count the amount of JsVars used. Mostly useful for debugging */
size_t jsvCountJsVarsUsed(JsVar *v) {
  // we do this so we don't count the same item twice, but don't use too much memory
  size_t c = _jsvCountJsVarsUsedRecursive(v, false);
  _jsvCountJsVarsUsedRecursive(v, true);
  return c;
}


JsVar *jsvGetArrayItem(const JsVar *arr, JsVarInt index) {
  JsVarRef childref = jsvGetLastChild(arr);
  JsVarInt lastArrayIndex = 0;
  // Look at last non-string element!
  while (childref) {
    JsVar *child = jsvLock(childref);
    if (jsvIsInt(child)) {
      lastArrayIndex = child->varData.integer;
      // it was the last element... sorted!
      if (lastArrayIndex == index) {
        return jsvSkipNameAndUnLock(child);
      }
      jsvUnLock(child);
      break;
    }
    // if not an int, keep going
    childref = jsvGetPrevSibling(child);
    jsvUnLock(child);
  }
  // it's not in this array - don't search the whole lot...
  if (index > lastArrayIndex)
    return 0;
  // otherwise is it more than halfway through?
  if (index > lastArrayIndex/2) {
    // it's in the final half of the array (probably) - search backwards
    while (childref) {
      JsVar *child = jsvLock(childref);

      assert(jsvIsInt(child));
      if (child->varData.integer == index) {
        return jsvSkipNameAndUnLock(child);
      }
      childref = jsvGetPrevSibling(child);
      jsvUnLock(child);
    }
  } else {
    // it's in the first half of the array (probably) - search forwards
    childref = jsvGetFirstChild(arr);
    while (childref) {
      JsVar *child = jsvLock(childref);

      assert(jsvIsInt(child));
      if (child->varData.integer == index) {
        return jsvSkipNameAndUnLock(child);
      }
      childref = jsvGetNextSibling(child);
      jsvUnLock(child);
    }
  }
  return 0; // undefined
}

/// Get the index of the value in the array (matchExact==use pointer, not equality check)
JsVar *jsvGetArrayIndexOf(JsVar *arr, JsVar *value, bool matchExact) {
  JsVarRef indexref;
  assert(jsvIsArray(arr) || jsvIsObject(arr));
  indexref = jsvGetFirstChild(arr);
  while (indexref) {
    JsVar *childIndex = jsvLock(indexref);
    assert(jsvIsName(childIndex))
    JsVar *childValue = jsvSkipName(childIndex);
    if (childValue==value ||
        (!matchExact && jsvMathsOpTypeEqual(childValue, value))) {
      jsvUnLock(childValue);
      return childIndex;
    }
    jsvUnLock(childValue);
    indexref = jsvGetNextSibling(childIndex);
    jsvUnLock(childIndex);
  }
  return 0; // undefined
}

/// Adds new elements to the end of an array, and returns the new length. initialValue is the item index when no items are currently in the array.
JsVarInt jsvArrayAddToEnd(JsVar *arr, JsVar *value, JsVarInt initialValue) {
  assert(jsvIsArray(arr));
  JsVarInt index = initialValue;
  if (jsvGetLastChild(arr)) {
    JsVar *last = jsvLock(jsvGetLastChild(arr));
    index = jsvGetInteger(last)+1;
    jsvUnLock(last);
  }

  JsVar *idx = jsvMakeIntoVariableName(jsvNewFromInteger(index), value);
  if (!idx) {
    jsWarn("Out of memory while appending to array");
    return 0;
  }
  jsvAddName(arr, idx);
  jsvUnLock(idx);
  return index+1;
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
  return jsvGetArrayLength(arr);
}

/// Adds a new element to the end of an array, unlocks it, and returns the new length
JsVarInt jsvArrayPushAndUnLock(JsVar *arr, JsVar *value) {
  JsVarInt l = jsvArrayPush(arr, value);
  jsvUnLock(value);
  return l;
}

/// Removes the last element of an array, and returns that element (or 0 if empty). includes the NAME
JsVar *jsvArrayPop(JsVar *arr) {
  assert(jsvIsArray(arr));
  JsVar *child = 0;
  JsVarInt length = jsvGetArrayLength(arr);
  if (length > 0) {
    length--;

    if (jsvGetLastChild(arr)) {
      // find last child with an integer key
      JsVarRef ref = jsvGetLastChild(arr);
      child = jsvLock(ref);
      while (child && !jsvIsInt(child)) {
        ref = jsvGetPrevSibling(child);
        jsvUnLock(child);
        if (ref) {
          child = jsvLock(ref);
        } else {
          child = 0;
        }
      }
      // check if the last integer key really is the last element
      if (child) {
        if (jsvGetInteger(child) == length) {
          // child is the last element - remove it
          jsvRemoveChild(arr, child);
        } else {
          // child is not the last element
          jsvUnLock(child);
          child = 0;
        }
      }
    }
    // and finally shrink the array
    jsvSetArrayLength(arr, length, false);
  }

  return child;
}

/// Removes the first element of an array, and returns that element (or 0 if empty). DOES NOT RENUMBER.
JsVar *jsvArrayPopFirst(JsVar *arr) {
  assert(jsvIsArray(arr));
  if (jsvGetFirstChild(arr)) {
    JsVar *child = jsvLock(jsvGetFirstChild(arr));
    if (jsvGetFirstChild(arr) == jsvGetLastChild(arr))
      jsvSetLastChild(arr, 0); // if 1 item in array
    jsvSetFirstChild(arr, jsvGetNextSibling(child)); // unlink from end of array
    jsvUnRef(child); // as no longer in array
    if (jsvGetNextSibling(child)) {
      JsVar *v = jsvLock(jsvGetNextSibling(child));
      jsvSetPrevSibling(v, 0);
      jsvUnLock(v);
    }
    jsvSetNextSibling(child, 0);
    return child; // and return it
  } else {
    // no children!
    return 0;
  }
}

/// Adds a new String element to the end of an array (IF it was not already there)
void jsvArrayAddString(JsVar *arr, const char *text) {
  JsVar *v = jsvNewFromString(text);
  JsVar *idx = jsvGetArrayIndexOf(arr, v, false); // did it already exist?
  if (!idx) jsvArrayPush(arr, v);
  else jsvUnLock(idx);
  jsvUnLock(v);
}

/// Join all elements of an array together into a string
JsVar *jsvArrayJoin(JsVar *arr, JsVar *filler) {
  JsVar *str = jsvNewFromEmptyString();
  if (!str) return 0; // out of memory

  JsVarInt index = 0;
  JsvIterator it;
  jsvIteratorNew(&it, arr);
  bool hasMemory = true;
  while (hasMemory && jsvIteratorHasElement(&it)) {
    JsVar *key = jsvIteratorGetKey(&it);
    if (jsvIsInt(key)) {
      JsVarInt thisIndex = jsvGetInteger(key);
      // add the filler
      if (filler) {
        while (index<thisIndex) {
          index++;
          jsvAppendStringVarComplete(str, filler);
        }
      }
      // add the value
      JsVar *value = jsvIteratorGetValue(&it);
      if (value && !jsvIsNull(value)) {
        JsVar *valueStr = jsvAsString(value, false);
        if (valueStr) { // could be out of memory
          jsvAppendStringVarComplete(str, valueStr);
          jsvUnLock(valueStr);
        } else {
          hasMemory = false;
        }
      }
      jsvUnLock(value);
    }
    jsvUnLock(key);
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);

  // pad missing elements from sparse arrays
  if (hasMemory && filler && jsvIsArray(arr)) {
    JsVarInt length = jsvGetArrayLength(arr);
    while (++index < length) {
      jsvAppendStringVarComplete(str, filler);
    }
  }

  return str;
}

/// Insert a new element before beforeIndex, DOES NOT UPDATE INDICES
void jsvArrayInsertBefore(JsVar *arr, JsVar *beforeIndex, JsVar *element) {
  if (beforeIndex) {
    JsVar *idxVar = jsvMakeIntoVariableName(jsvNewFromInteger(0), element);
    if (!idxVar) return; // out of memory

    JsVarRef idxRef = jsvGetRef(jsvRef(idxVar));
    JsVarRef prev = jsvGetPrevSibling(beforeIndex);
    if (prev) {
      JsVar *prevVar = jsvRef(jsvLock(prev));
      jsvSetInteger(idxVar, jsvGetInteger(prevVar)+1); // update index number
      jsvSetNextSibling(prevVar, idxRef);
      jsvUnLock(prevVar);
      jsvSetPrevSibling(idxVar, prev);
    } else {
      jsvSetPrevSibling(idxVar, 0);
      jsvSetFirstChild(arr, idxRef);
    }
    jsvSetPrevSibling(beforeIndex, idxRef);
    jsvSetNextSibling(idxVar, jsvGetRef(jsvRef(beforeIndex)));
    jsvUnLock(idxVar);
  } else
    jsvArrayPush(arr, element);
}

/** Same as jsvMathsOpPtr, but if a or b are a name, skip them
 * and go to what they point to. Also handle the case where
 * they may be objects with valueOf functions. */
JsVar *jsvMathsOpSkipNames(JsVar *a, JsVar *b, int op) {
  JsVar *pa = jsvSkipName(a);
  JsVar *pb = jsvSkipName(b);
  JsVar *oa = jsvGetValueOf(pa);
  JsVar *ob = jsvGetValueOf(pb);
  jsvUnLock(pa);
  jsvUnLock(pb);
  JsVar *res = jsvMathsOp(oa,ob,op);
  jsvUnLock(oa);
  jsvUnLock(ob);
  return res;
}


JsVar *jsvMathsOpError(int op, const char *datatype) {
  char opName[32];
  jslTokenAsString(op, opName, sizeof(opName));
  jsError("Operation %s not supported on the %s datatype", opName, datatype);
  return 0;
}

bool jsvMathsOpTypeEqual(JsVar *a, JsVar *b) {
  // check type first, then call again to check data
  bool eql = (a==0) == (b==0);
  if (a && b) {
    // Check whether both are numbers, otherwise check the variable
    // type flags themselves
    eql = ((jsvIsInt(a)||jsvIsFloat(a)) && (jsvIsInt(b)||jsvIsFloat(b))) ||
          ((a->flags & JSV_VARTYPEMASK) == (b->flags & JSV_VARTYPEMASK));
  }
  if (eql) {
    JsVar *contents = jsvMathsOp(a,b, LEX_EQUAL);
    if (!jsvGetBool(contents)) eql = false;
    jsvUnLock(contents);
  }
  return eql;
}

JsVar *jsvMathsOp(JsVar *a, JsVar *b, int op) {
  // Type equality check
  if (op == LEX_TYPEEQUAL || op == LEX_NTYPEEQUAL) {
    bool eql = jsvMathsOpTypeEqual(a,b);
    if (op == LEX_TYPEEQUAL)
      return jsvNewFromBool(eql);
    else
      return jsvNewFromBool(!eql);
  }

  bool needsInt = op=='&' || op=='|' || op=='^' || op==LEX_LSHIFT || op==LEX_RSHIFT || op==LEX_RSHIFTUNSIGNED;
  bool needsNumeric = needsInt || op=='*' || op=='/' || op=='%' || op=='-';
  bool isCompare = op==LEX_EQUAL || op==LEX_NEQUAL || op=='<' || op==LEX_LEQUAL || op=='>'|| op==LEX_GEQUAL;
  if (isCompare) {
    if (jsvIsNumeric(a) && jsvIsString(b)) {
      needsNumeric = true;
      needsInt = jsvIsIntegerish(a) && jsvIsStringNumericInt(b, false);
    } else if (jsvIsNumeric(b) && jsvIsString(a)) {
      needsNumeric = true;
      needsInt = jsvIsIntegerish(b) && jsvIsStringNumericInt(a, false);
    }
  }

  // do maths...
  if (jsvIsUndefined(a) && jsvIsUndefined(b)) {
    if (op == LEX_EQUAL)
      return jsvNewFromBool(true);
    else if (op == LEX_NEQUAL)
      return jsvNewFromBool(false);
    else
      return 0; // undefined
  } else if (needsNumeric ||
             ((jsvIsNumeric(a) || jsvIsUndefined(a) || jsvIsNull(a)) &&
              (jsvIsNumeric(b) || jsvIsUndefined(b) || jsvIsNull(b)))) {
    if (needsInt || (jsvIsIntegerish(a) && jsvIsIntegerish(b))) {
          // note that int+undefined should be handled as a double
          // use ints
          JsVarInt da = jsvGetInteger(a);
          JsVarInt db = jsvGetInteger(b);
          switch (op) {
              case '+': return jsvNewFromLongInteger((long long)da + (long long)db);
              case '-': return jsvNewFromLongInteger((long long)da - (long long)db);
              case '*': return jsvNewFromLongInteger((long long)da * (long long)db);
              case '/': return jsvNewFromFloat((JsVarFloat)da/(JsVarFloat)db);
              case '&': return jsvNewFromInteger(da&db);
              case '|': return jsvNewFromInteger(da|db);
              case '^': return jsvNewFromInteger(da^db);
              case '%': return db ? jsvNewFromInteger(da%db) : jsvNewFromFloat(NAN);
              case LEX_LSHIFT: return jsvNewFromInteger(da << db);
              case LEX_RSHIFT: return jsvNewFromInteger(da >> db);
              case LEX_RSHIFTUNSIGNED: return jsvNewFromInteger((JsVarInt)(((JsVarIntUnsigned)da) >> db));
              case LEX_EQUAL:     return jsvNewFromBool(da==db && jsvIsNull(a)==jsvIsNull(b));
              case LEX_NEQUAL:    return jsvNewFromBool(da!=db || jsvIsNull(a)!=jsvIsNull(b));
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
              case '%': return jsvNewFromFloat(jswrap_math_mod(da, db));
              case LEX_EQUAL:
              case LEX_NEQUAL:  { bool equal = da==db;
                                  if ((jsvIsNull(a) && jsvIsUndefined(b)) ||
                                      (jsvIsNull(b) && jsvIsUndefined(a))) equal = true; // JS quirk :)
                                  return jsvNewFromBool((op==LEX_EQUAL) ? equal : ((bool)!equal));
                                }
              case '<':           return jsvNewFromBool(da<db);
              case LEX_LEQUAL:    return jsvNewFromBool(da<=db);
              case '>':           return jsvNewFromBool(da>db);
              case LEX_GEQUAL:    return jsvNewFromBool(da>=db);
              default: return jsvMathsOpError(op, "Double");
          }
      }
  } else if ((jsvIsArray(a) || jsvIsObject(a) || jsvIsFunction(a) ||
              jsvIsArray(b) || jsvIsObject(b) || jsvIsFunction(b)) &&
              jsvIsArray(a)==jsvIsArray(b) && // Fix #283 - convert to string and test if only one is an array
              (op == LEX_EQUAL || op==LEX_NEQUAL)) {
    bool equal = a==b;

    if (jsvIsNativeFunction(a) || jsvIsNativeFunction(b)) {
      // even if one is not native, the contents will be different
      equal = a->varData.native.ptr == b->varData.native.ptr &&
              a->varData.native.argTypes == b->varData.native.argTypes &&
              jsvGetFirstChild(a) == jsvGetFirstChild(b);
    }

    /* Just check pointers */
    switch (op) {
         case LEX_EQUAL:  return jsvNewFromBool(equal);
         case LEX_NEQUAL: return jsvNewFromBool(!equal);
         default: return jsvMathsOpError(op, jsvIsArray(a)?"Array":"Object");
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
         jsvAppendStringVarComplete(v, db);
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
  JsVar *res = jsvMathsOpSkipNames(zero, v, '-');
  jsvUnLock(zero);
  jsvUnLock(v);
  return res;
}

/** If the given element is found, return the path to it as a string of
 * the form 'foo.bar', else return 0. If we would have returned a.b and
 * ignoreParent is a, don't! */
JsVar *jsvGetPathTo(JsVar *root, JsVar *element, int maxDepth, JsVar *ignoreParent) {
  if (maxDepth<=0) return 0;
  JsvIterator it;
  jsvIteratorNew(&it, root);
  while (jsvIteratorHasElement(&it)) {
    JsVar *el = jsvIteratorGetValue(&it);
    if (el == element && root != ignoreParent) {
      // if we found it - send the key name back!
      JsVar *name = jsvAsString(jsvIteratorGetKey(&it), true);
      jsvIteratorFree(&it);
      return name;
    } else if (jsvIsObject(el) || jsvIsArray(el) || jsvIsFunction(el)) {
      // recursively search
      JsVar *n = jsvGetPathTo(el, element, maxDepth-1, ignoreParent);
      if (n) {
        // we found it! Append our name onto it as well
        JsVar *keyName = jsvIteratorGetKey(&it);
        JsVar *name = jsvVarPrintf(jsvIsObject(el) ? "%v.%v" : "%v[%q]",keyName,n);
        jsvUnLock(keyName);
        jsvUnLock(n);
        jsvIteratorFree(&it);
        return name;
      }
    }
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);
  return 0;
}

void jsvTraceLockInfo(JsVar *v) {
    jsiConsolePrintf("#%d[r%d,l%d] ",jsvGetRef(v),jsvGetRefs(v),jsvGetLocks(v));
}

/** Get the lowest level at which searchRef appears */
int _jsvTraceGetLowestLevel(JsVar *var, JsVar *searchVar) {
  if (var == searchVar) return 0;
  int found = -1;

  // Use IS_RECURSING  flag to stop recursion
  if (var->flags & JSV_IS_RECURSING)
    return -1;
  var->flags |= JSV_IS_RECURSING;

  if (jsvHasSingleChild(var) && jsvGetFirstChild(var)) {
    JsVar *child = jsvLock(jsvGetFirstChild(var));
    int f = _jsvTraceGetLowestLevel(child, searchVar);
    jsvUnLock(child);
    if (f>=0 && (found<0 || f<found)) found=f+1;
  }
  if (jsvHasChildren(var)) {
    JsVarRef childRef = jsvGetFirstChild(var);
    while (childRef) {
      JsVar *child = jsvLock(childRef);
      int f = _jsvTraceGetLowestLevel(child, searchVar);
      if (f>=0 && (found<0 || f<found)) found=f+1;
      childRef = jsvGetNextSibling(child);
      jsvUnLock(child);
    }
  }

  var->flags &= ~JSV_IS_RECURSING;

  return found; // searchRef not found
}

void _jsvTrace(JsVar *var, int indent, JsVar *baseVar, int level) {
#ifdef SAVE_ON_FLASH
  jsiConsolePrint("Trace unimplemented in this version.\n");
#else
  int i;
  for (i=0;i<indent;i++) jsiConsolePrint(" ");

  if (!var) {
    jsiConsolePrint("undefined");
    return;
  }

  jsvTraceLockInfo(var);

  int lowestLevel = _jsvTraceGetLowestLevel(baseVar, var);
  if (lowestLevel < level) {
    // If this data is available elsewhere in the tree (but nearer the root)
    // then don't print it. This makes the dump significantly more readable!
    // It also stops us getting in recursive loops ...
    jsiConsolePrint("...\n");
    return;
  }

  if (jsvIsName(var)) jsiConsolePrint("Name ");

  char endBracket = ' ';
  if (jsvIsObject(var)) { jsiConsolePrint("Object { "); endBracket = '}'; }
  else if (jsvIsArray(var)) { jsiConsolePrintf("Array(%d) [ ", var->varData.integer); endBracket = ']'; }
  else if (jsvIsFunction(var)) { jsiConsolePrint("Function { "); endBracket = '}'; }
  else if (jsvIsPin(var)) jsiConsolePrintf("Pin %d", jsvGetInteger(var));
  else if (jsvIsInt(var)) jsiConsolePrintf("Integer %d", jsvGetInteger(var));
  else if (jsvIsBoolean(var)) jsiConsolePrintf("Bool %s", jsvGetBool(var)?"true":"false");
  else if (jsvIsFloat(var)) jsiConsolePrintf("Double %f", jsvGetFloat(var));
  else if (jsvIsFunctionParameter(var)) jsiConsolePrint("Param ");
  else if (jsvIsArrayBufferName(var)) jsiConsolePrintf("ArrayBufferName[%d] ", jsvGetInteger(var));
  else if (jsvIsArrayBuffer(var)) jsiConsolePrintf("%s ", jswGetBasicObjectName(var)); // way to get nice name
  else if (jsvIsString(var)) {
    size_t blocks = 1;
    if (jsvGetLastChild(var)) {
      JsVar *v = jsvLock(jsvGetLastChild(var));
      blocks += jsvCountJsVarsUsed(v);
      jsvUnLock(v);
    }
    if (jsvIsFlatString(var)) {
      blocks += jsvGetFlatStringBlocks(var);
    }
    jsiConsolePrintf("%sString [%d blocks] %q", jsvIsFlatString(var)?"Flat":"", blocks, var);
  } else {
    jsiConsolePrintf("Unknown %d", var->flags & (JsVarFlags)~(JSV_LOCK_MASK));
  }

  // print a value if it was stored in here as well...
  if (jsvIsNameInt(var)) {
    jsiConsolePrintf("= int %d\n", (int)jsvGetFirstChildSigned(var));
    return;
  } else if (jsvIsNameIntBool(var)) {
    jsiConsolePrintf("= bool %s\n", jsvGetFirstChild(var)?"true":"false");
    return;
  }

  if (jsvHasSingleChild(var)) {
    JsVar *child = jsvGetFirstChild(var) ? jsvLock(jsvGetFirstChild(var)) : 0;
    _jsvTrace(child, indent+2, baseVar, level+1);
    jsvUnLock(child);
  } else if (jsvHasChildren(var)) {
    JsvIterator it;
    jsvIteratorNew(&it, var);
    bool first = true;
    while (jsvIteratorHasElement(&it)) {
      if (first) jsiConsolePrintf("\n");
      first = false;
      JsVar *child = jsvIteratorGetKey(&it);
      _jsvTrace(child, indent+2, baseVar, level+1);
      jsvUnLock(child);
      jsiConsolePrintf("\n");
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
    if (!first)
      for (i=0;i<indent;i++) jsiConsolePrint(" ");
  }
  jsiConsolePrintf("%c", endBracket);
#endif
}

/** Write debug info for this Var out to the console */
void jsvTrace(JsVar *var, int indent) {
  _jsvTrace(var,indent,var,0);
  jsiConsolePrintf("\n");
}


/** Recursively mark the variable */
static void jsvGarbageCollectMarkUsed(JsVar *var) {
  var->flags &= (JsVarFlags)~JSV_GARBAGE_COLLECT;

  if (jsvHasCharacterData(var)) {
    // non-recursively scan strings
    JsVarRef child = jsvGetLastChild(var);
    while (child) {
      JsVar *childVar;
      childVar = jsvGetAddressOf(child);
      childVar->flags &= (JsVarFlags)~JSV_GARBAGE_COLLECT;
      child = jsvGetLastChild(childVar);
    }
  }
  // intentionally no else
  if (jsvHasSingleChild(var)) {
    if (jsvGetFirstChild(var)) {
      JsVar *childVar = jsvGetAddressOf(jsvGetFirstChild(var));
      if (childVar->flags & JSV_GARBAGE_COLLECT)
        jsvGarbageCollectMarkUsed(childVar);
    }
  } else if (jsvHasChildren(var)) {
    JsVarRef child = jsvGetFirstChild(var);
    while (child) {
      JsVar *childVar;
      childVar = jsvGetAddressOf(child);
      if (childVar->flags & JSV_GARBAGE_COLLECT)
        jsvGarbageCollectMarkUsed(childVar);
      child = jsvGetNextSibling(childVar);
    }
  }
}

/** Run a garbage collection sweep - return true if things have been freed */
bool jsvGarbageCollect() {
  JsVarRef i;
  // clear garbage collect flags
  for (i=1;i<=jsVarsSize;i++)  {
    JsVar *var = jsvGetAddressOf(i);
    if ((var->flags&JSV_VARTYPEMASK) != JSV_UNUSED) { // if it is not unused
      var->flags |= (JsVarFlags)JSV_GARBAGE_COLLECT;
      // if we have a flat string, skip that many blocks
      if (jsvIsFlatString(var))
        i = (JsVarRef)(i+jsvGetFlatStringBlocks(var));
    }
  }
  // recursively add 'native' vars
  for (i=1;i<=jsVarsSize;i++)  {
    JsVar *var = jsvGetAddressOf(i);
    if ((var->flags & JSV_GARBAGE_COLLECT) && // not already GC'd
        jsvGetLocks(var)>0) // or it is locked
      jsvGarbageCollectMarkUsed(var);
    // if we have a flat string, skip that many blocks
    if (jsvIsFlatString(var))
      i = (JsVarRef)(i+jsvGetFlatStringBlocks(var));
  }
  // now sweep for things that we can GC!
  bool freedSomething = false;
  for (i=1;i<=jsVarsSize;i++)  {
    JsVar *var = jsvGetAddressOf(i);
    if (var->flags & JSV_GARBAGE_COLLECT) {
      freedSomething = true;
      // free!
      var->flags = JSV_UNUSED;
      // add this to our free list
      jsvSetNextSibling(var, jsVarFirstEmpty);
      jsVarFirstEmpty = jsvGetRef(var);
    }
    // if we have a flat string, skip that many blocks
    if (jsvIsFlatString(var))
      i = (JsVarRef)(i+jsvGetFlatStringBlocks(var));
  }
  return freedSomething;
}

/** Remove whitespace to the right of a string - on MULTIPLE LINES */
JsVar *jsvStringTrimRight(JsVar *srcString) {
  JsvStringIterator src, dst;
  JsVar *dstString = jsvNewFromEmptyString();
  jsvStringIteratorNew(&src, srcString, 0);
  jsvStringIteratorNew(&dst, dstString, 0);
  int spaces = 0;
  while (jsvStringIteratorHasChar(&src)) {
    char ch = jsvStringIteratorGetChar(&src);
    jsvStringIteratorNext(&src);

    if (ch==' ') spaces++;
    else if (ch=='\n') {
      spaces = 0;
      jsvStringIteratorAppend(&dst, ch);
    } else {
      for (;spaces>0;spaces--)
        jsvStringIteratorAppend(&dst, ' ');
      jsvStringIteratorAppend(&dst, ch);
    }
  }
  jsvStringIteratorFree(&src);
  jsvStringIteratorFree(&dst);
  return dstString;
}

/// If v is the key of a function, return true if it is internal and shouldn't be visible to the user
bool jsvIsInternalFunctionKey(JsVar *v) {
  return (jsvIsString(v) && (
              v->varData.str[0]==JS_HIDDEN_CHAR)
                            ) ||
         jsvIsFunctionParameter(v);
}

/// If v is the key of an object, return true if it is internal and shouldn't be visible to the user
bool jsvIsInternalObjectKey(JsVar *v) {
  return (jsvIsString(v) && (
              v->varData.str[0]==JS_HIDDEN_CHAR ||
              jsvIsStringEqual(v, JSPARSE_INHERITS_VAR) ||
              jsvIsStringEqual(v, JSPARSE_CONSTRUCTOR_VAR)
                            ));
}

/// Get the correct checker function for the given variable. see jsvIsInternalFunctionKey/jsvIsInternalObjectKey
JsvIsInternalChecker jsvGetInternalFunctionCheckerFor(JsVar *v) {
  if (jsvIsFunction(v)) return jsvIsInternalFunctionKey;
  if (jsvIsObject(v)) return jsvIsInternalObjectKey;
  return 0;
}

/** Using 'configs', this reads 'object' into the given pointers, returns true on success.
 *  If object is not undefined and not an object, an error is raised.
 *  If there are fields that are not  in the list of configs, an error is raised
 */
bool jsvReadConfigObject(JsVar *object, jsvConfigObject *configs, int nConfigs) {
  if (jsvIsUndefined(object)) return true;
  if (!jsvIsObject(object)) {
    jsExceptionHere(JSET_ERROR, "Expecting an Object, or undefined");
    return false;
  }
  // Ok, it's an object
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, object);
  bool ok = true;
  while (ok && jsvObjectIteratorHasValue(&it)) {
    JsVar *key = jsvObjectIteratorGetKey(&it);
    bool found = false;
    int i;
    for (i=0;i<nConfigs;i++) {
      if (jsvIsStringEqual(key, configs[i].name)) {
        found = true;
        if (configs[i].ptr) {
          JsVar *val = jsvObjectIteratorGetValue(&it);
          switch (configs[i].type) {
            case 0: break;
            case JSV_OBJECT:
            case JSV_STRING_0:
            case JSV_ARRAY:
            case JSV_FUNCTION:
              *((JsVar**)configs[i].ptr) = jsvLockAgain(val); break;
            case JSV_PIN: *((Pin*)configs[i].ptr) = jshGetPinFromVar(val); break;
            case JSV_BOOLEAN: *((bool*)configs[i].ptr) = jsvGetBool(val); break;
            case JSV_INTEGER: *((JsVarInt*)configs[i].ptr) = jsvGetInteger(val); break;
            case JSV_FLOAT: *((JsVarFloat*)configs[i].ptr) = jsvGetFloat(val); break;
            default: assert(0); break;
          }
          jsvUnLock(val);
        }
      }
    }
    if (!found)
      jsWarn("Unknown option %q", key);
    jsvUnLock(key);

    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  return ok;
}

JsVar *jsvNewTypedArray(JsVarDataArrayBufferViewType type, JsVarInt length) {
  JsVar *lenVar = jsvNewFromInteger(length);
  if (!lenVar) return 0;
  JsVar *array = jswrap_typedarray_constructor(type, lenVar,0,0);
  jsvUnLock(lenVar);
  return array;
}
