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
 *  This file is designed to be parsed during the build process
 *
 *  JavaScript methods for Regular Expressions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_regexp.h"
#include "jsparse.h"
#include "jshardware.h"
#include "jslex.h"

/* TODO:
 *
 * Groups
 * Escape characters in regex
 * Handling of []
 * Handling of 'global' flag especially
 * lastIndex support?
 */

// Based on
// http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
// - this is a super basic regex parser

typedef struct {
  JsVar *sourceStr;
  size_t startIndex;
  JsVar *group;
} matchInfo;

JsVar *matchhere(char *regexp, JsvStringIterator *txtIt, matchInfo *info);
JsVar *matchstar(int c, char *regexp, JsvStringIterator *txtIt, matchInfo *info);

JsVar *matchfound(JsvStringIterator *txtIt, matchInfo *info) {
  JsVar *rmatch = jsvNewEmptyArray();
  size_t endIndex = jsvStringIteratorGetIndex(txtIt);
  JsVar *matchStr = jsvNewFromStringVar(info->sourceStr, info->startIndex, endIndex-info->startIndex);
  jsvSetArrayItem(rmatch, 0, matchStr);
  jsvUnLock(matchStr);
  jsvObjectSetChildAndUnLock(rmatch, "index", jsvNewFromInteger(info->startIndex));
  jsvObjectSetChild(rmatch, "input", info->sourceStr);

  return rmatch;
}

/* match: search for regexp anywhere in text */
JsVar *match(char *regexp, JsVar *str, size_t startIndex) {
  matchInfo info;
  info.sourceStr = str;
  info.startIndex = startIndex;
  info.group = 0;

  JsvStringIterator txtIt;
  jsvStringIteratorNew(&txtIt, str, startIndex);
  if (regexp[0] == '^')
    return matchhere(regexp+1, &txtIt, &info);
  /* must look even if string is empty */
  JsVar *rmatch = matchhere(regexp, &txtIt, &info);
  while (!rmatch && jsvStringIteratorHasChar(&txtIt)) {
    jsvStringIteratorNext(&txtIt);
    info.startIndex++;
    JsvStringIterator txtIt2 = jsvStringIteratorClone(&txtIt);
    rmatch = matchhere(regexp, &txtIt2, &info);
    jsvStringIteratorFree(&txtIt2);
  }
  jsvStringIteratorFree(&txtIt);
  return rmatch;
}

/* matchhere: search for regexp at beginning of text */
JsVar *matchhere(char *regexp, JsvStringIterator *txtIt, matchInfo *info) {
  if (regexp[0] == '\0')
    return matchfound(txtIt, info);
  if (regexp[1] == '*')
    return matchstar(regexp[0], regexp+2, txtIt, info);
  if (regexp[0] == '$' && regexp[1] == '\0') {
    if (!jsvStringIteratorHasChar(txtIt))
      return matchfound(txtIt, info);
    else
      return 0;
  }
  if (jsvStringIteratorHasChar(txtIt) &&
      (regexp[0]=='.' || regexp[0]==jsvStringIteratorGetChar(txtIt))) {
    jsvStringIteratorNext(txtIt);
    return matchhere(regexp+1, txtIt, info);
  }
  return 0;
}

/* matchstar: search for c*regexp at beginning of text */
JsVar *matchstar(int c, char *regexp, JsvStringIterator *txtIt, matchInfo *info) {
  /* a * matches zero or more instances */
  JsvStringIterator txtIt2 = jsvStringIteratorClone(txtIt);
  JsVar *rmatch = matchhere(regexp, &txtIt2, info);
  jsvStringIteratorFree(&txtIt2);
  if (rmatch) return rmatch;
  while (jsvStringIteratorHasChar(txtIt) &&
         (jsvStringIteratorGetChar(txtIt) == c || c == '.')) {
    jsvStringIteratorNext(txtIt);
    txtIt2 = jsvStringIteratorClone(txtIt);
    rmatch = matchhere(regexp, &txtIt2, info);
    jsvStringIteratorFree(&txtIt2);
    if (rmatch) return rmatch;
  }
  return 0;
}


/*JSON{
  "type" : "class",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "RegExp"
}
The built-in class for handling Regular Expressions
*/

/*JSON{
  "type" : "constructor",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "RegExp",
  "name" : "RegExp",
  "generate" : "jswrap_regexp_constructor",
  "params" : [
    ["regex","JsVar","A regular expression as a string"],
    ["regex","JsVar","Flags for the regular expression as a string"]
  ],
  "return" : ["JsVar","A RegExp object"],
  "return_object" : "RegExp"
}
Creates a RegExp object
 */
JsVar *jswrap_regexp_constructor(JsVar *str, JsVar *flags) {
  if (!jsvIsString(str)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting String as first argument, got %t", str);
    return 0;
  }
  JsVar *r = jspNewObject(0,"RegExp");
  jsvObjectSetChild(r, "source", str);
  if (!jsvIsUndefined(flags)) {
    if (!jsvIsString(flags))
      jsExceptionHere(JSET_TYPEERROR, "Expecting String as first argument, got %t", str);
    else
      jsvObjectSetChild(r, "flags", flags);
  }
  return r;
}

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "RegExp",
  "name" : "exec",
  "params" : [
    ["str","JsVar","A string to match on"]
  ],
  "generate" : "jswrap_regexp_exec",
  "return" : ["JsVar","A result array, or null"]
}

 */
JsVar *jswrap_regexp_exec(JsVar *parent, JsVar *str) {
  JsVar *regex = jsvObjectGetChild(parent, "source", 0);
  if (!jsvIsString(regex)) {
    jsvUnLock(regex);
    return 0;
  }
  size_t regexLen = jsvGetStringLength(regex);
  char *regexPtr = (char *)alloca(regexLen+1);
  if (!regexPtr) {
    jsvUnLock(regex);
    return 0;
  }
  jsvGetString(regex, regexPtr, regexLen+1);
  jsvUnLock(regex);
  JsVar *rmatch = match(regexPtr, str, 0);
  if (!rmatch) return jsvNewWithFlags(JSV_NULL);
  return rmatch;
}
