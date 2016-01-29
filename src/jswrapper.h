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

/// Do a binary search of the symbol table list
JsVar *jswBinarySearch(const JswSymList *symbolsPtr, JsVar *parent, const char *name);

// For instances of builtins like Pin, String, etc, search in X.prototype
JsVar *jswFindInObjectProto(JsVar *parent, const char *name);

/** If 'name' is something that belongs to an internal function, execute it.
 * parentInstance is the actual object ('this'), but parent may be a prototype of another object */
JsVar *jswFindBuiltIn(JsVar *parentInstance, JsVar *parent, const char *name);

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

const JswSymList *jswSymbolTable_Object_prototype;

#endif // JSWRAPPER_H
