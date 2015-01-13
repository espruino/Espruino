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
 * Header for auto-generated Wrapper functions
 * ----------------------------------------------------------------------------
 */

#ifndef JSWRAPPER_H
#define JSWRAPPER_H

#include "jsutils.h"
#include "jsvar.h"

typedef enum {
  JSWAT_FINISH = 0, // no argument
  JSWAT_VOID = 0, // Only for return values
  JSWAT_JSVAR, // standard variable
  JSWAT_ARGUMENT_ARRAY, // a JsVar array containing all subsequent arguments
  JSWAT_BOOL, // boolean
  JSWAT_INT32, // 32 bit int
  JSWAT_PIN, // A pin
  JSWAT_JSVARFLOAT, // 64 bit float
  JSWAT__LAST = JSWAT_JSVARFLOAT,
  JSWAT_MASK = NEXT_POWER_2(JSWAT__LAST)-1,

  // should this just be executed right away and the value returned? Used to encode constants in the symbol table
  // We encode this by setting all bits in the last argument, but leaving the second-last argument as zero
  JSWAT_EXECUTE_IMMEDIATELY = 0x7000,  
  JSWAT_EXECUTE_IMMEDIATELY_MASK = 0x7E00, 

  JSWAT_THIS_ARG    = 0x8000, // whether a 'this' argument should be tacked onto the start
  JSWAT_ARGUMENTS_MASK = ~(JSWAT_MASK | JSWAT_THIS_ARG)
} JsnArgumentType;
// number of bits needed for each argument bit
#define JSWAT_BITS GET_BIT_NUMBER(JSWAT_MASK+1)

/// Structure for each symbol in the list of built-in symbols
typedef struct {
  unsigned short strOffset;
  void (*functionPtr)(void);
  unsigned short functionSpec; // JsnArgumentType
} PACKED_FLAGS JswSymPtr;

/// Information for each list of built-in symbols
typedef struct {
  const JswSymPtr *symbols;
  unsigned char symbolCount;
  const char *symbolChars;
} PACKED_FLAGS JswSymList;

/// Do a binary search of the symbol table list
JsVar *jswBinarySearch(const JswSymList *symbolsPtr, JsVar *parent, const char *name);

/** If 'name' is something that belongs to an internal function, execute it.  */
JsVar *jswFindBuiltInFunction(JsVar *parent, const char *name);

/// Given an object, return the list of symbols for it
const JswSymList *jswGetSymbolListForObject(JsVar *parent);

/// Given an object, return the list of symbols for its prototype
const JswSymList *jswGetSymbolListForObjectProto(JsVar *parent);

/// Given the name of an Object, see if we should set it up as a builtin or not
bool jswIsBuiltInObject(const char *name);

/** If we get this in 'require', do we have the object for this
  inside the interpreter already? If so, return the native function
  pointer of the object's constructor */
void *jswGetBuiltInLibrary(const char *name);

/** Given a variable, return the basic object name of it */
const char *jswGetBasicObjectName(JsVar *var);

/** Given the name of a Basic Object, eg, Uint8Array, String, etc. Return the prototype object's name - or 0.
 * For instance jswGetBasicObjectPrototypeName("Object")==0, jswGetBasicObjectPrototypeName("Integer")=="Object",
 * jswGetBasicObjectPrototypeName("Uint8Array")=="ArrayBufferView"
 *  */
const char *jswGetBasicObjectPrototypeName(const char *name);

/** Tasks to run on Idle. Returns true if either one of the tasks returned true (eg. they're doing something and want to avoid sleeping) */
bool jswIdle();

/** Tasks to run on Initialisation */
void jswInit();

/** Tasks to run on Deinitialisation */
void jswKill();

#endif // JSWRAPPER_H
