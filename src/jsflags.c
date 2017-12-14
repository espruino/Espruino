/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * JS Interpreter specific flags
 * ----------------------------------------------------------------------------
 */
#include "jsflags.h"

volatile JsFlags jsFlags;
const char *jsFlagNames = JSFLAG_NAMES;


/// Get the state of a flag
bool jsfGetFlag(JsFlags flag) {
  return (jsFlags & flag)!=0;
}

/// Set the state of a flag
void jsfSetFlag(JsFlags flag, bool isOn) {
  if (isOn)
    jsFlags |= flag;
  else
    jsFlags &= ~flag;
}

/// Get a list of all flags and their status
JsVar *jsfGetFlags() {
 JsVar *o = jsvNewWithFlags(JSV_OBJECT);
 if (!o) return 0;
 const char *p = jsFlagNames;
 JsFlags flag = 1;
 while (*p) {
   jsvObjectSetChildAndUnLock(o, p, jsvNewFromInteger(jsfGetFlag(flag)?1:0));
   p += strlen(p)+1;
   flag<<=1;
 }
 return o;
}

/// Set any of the specified flags
void jsfSetFlags(JsVar *flags) {
  if (!jsvIsObject(flags)) return;
  const char *p = jsFlagNames;
  JsFlags flag = 1;
  while (*p) {
    JsVar *v = jsvObjectGetChild(flags, p, 0);
    if (v) jsfSetFlag(flag, jsvGetBoolAndUnLock(v));
    p += strlen(p)+1;
    flag<<=1;
  }
}
