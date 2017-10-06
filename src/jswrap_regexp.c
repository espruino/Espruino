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
 * Handling of 'global' flag especially
 * lastIndex support?
 */

// Based on
// http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
// - this is a super basic regex parser

#define MAX_GROUPS 8

typedef struct {
  JsVar *sourceStr;
  size_t startIndex;
  int groups;
  size_t groupStart[MAX_GROUPS];
  size_t groupEnd[MAX_GROUPS];
} matchInfo;

JsVar *matchhere(char *regexp, JsvStringIterator *txtIt, matchInfo info);

JsVar *matchfound(JsvStringIterator *txtIt, matchInfo info) {
  JsVar *rmatch = jsvNewEmptyArray();
  size_t endIndex = jsvStringIteratorGetIndex(txtIt);
  JsVar *matchStr = jsvNewFromStringVar(info.sourceStr, info.startIndex, endIndex-info.startIndex);
  jsvSetArrayItem(rmatch, 0, matchStr);
  jsvUnLock(matchStr);
  int i;
  for (i=0;i<info.groups;i++) {
    matchStr = jsvNewFromStringVar(info.sourceStr, info.groupStart[i], info.groupEnd[i]-info.groupStart[i]);
    jsvSetArrayItem(rmatch, i+1, matchStr);
    jsvUnLock(matchStr);
  }
  jsvObjectSetChildAndUnLock(rmatch, "index", jsvNewFromInteger((JsVarInt)info.startIndex));
  jsvObjectSetChild(rmatch, "input", info.sourceStr);

  return rmatch;
}

/* match: search for regexp anywhere in text */
JsVar *match(char *regexp, JsVar *str, size_t startIndex) {
  matchInfo info;
  info.sourceStr = str;
  info.startIndex = startIndex;
  info.groups = 0;

  JsvStringIterator txtIt;
  jsvStringIteratorNew(&txtIt, str, startIndex);
  if (regexp[0] == '^')
    return matchhere(regexp+1, &txtIt, info);
  /* must look even if string is empty */
  JsVar *rmatch = matchhere(regexp, &txtIt, info);
  while (!rmatch && jsvStringIteratorHasChar(&txtIt)) {
    jsvStringIteratorNext(&txtIt);
    info.startIndex++;
    JsvStringIterator txtIt2 = jsvStringIteratorClone(&txtIt);
    rmatch = matchhere(regexp, &txtIt2, info);
    jsvStringIteratorFree(&txtIt2);
  }
  jsvStringIteratorFree(&txtIt);
  return rmatch;
}

bool matchcharacter(char *regexp, JsvStringIterator *txtIt, int *length) {
  *length = 1;
  char ch = jsvStringIteratorGetChar(txtIt);
  if (regexp[0]=='.') return true;
  if (regexp[0]=='[') { // Character set (any char inside '[]')
    bool inverted = regexp[1]=='^';
    if (inverted) (*length)++;
    bool matchAny = false;
    while (regexp[*length] && regexp[*length]!=']') {
      int matchLen;
      matchAny |= matchcharacter(&regexp[*length], txtIt, &matchLen);
      (*length) += matchLen;
    }
    if (regexp[*length]==']') {
      (*length)++;
    } else {
      jsExceptionHere(JSET_ERROR, "Unfinished character set in RegEx");
      return false;
    }
    return matchAny != inverted;
  }
  if (regexp[0]=='\\') { // escape character
    *length = 2;
    // missing quite a few here
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Regular_Expressions
    if (regexp[1]=='\\') return ch=='\\';
    if (regexp[1]=='d') return isNumeric(ch);
    if (regexp[1]=='D') return !isNumeric(ch);
    if (regexp[1]=='f') return ch==0x0C;
    if (regexp[1]=='n') return ch==0x0A;
    if (regexp[1]=='r') return ch==0x0D;
    if (regexp[1]=='s') return isWhitespace(ch);
    if (regexp[1]=='S') return !isWhitespace(ch);
    if (regexp[1]=='t') return ch==0x09;
    if (regexp[1]=='v') return ch==0x0B;
    if (regexp[1]=='w') return isNumeric(ch) || isAlpha(ch) || ch=='_';
    if (regexp[1]=='W') return !(isNumeric(ch) || isAlpha(ch) || ch=='_');
    if (regexp[1]=='0') return ch==0x00;
    if (regexp[1]=='x' && regexp[2] && regexp[3]) {
      *length = 4;
      char code = (char)((chtod(regexp[2])<<4) | chtod(regexp[3]));
      return ch==code;
    }
    jsExceptionHere(JSET_ERROR, "Unknown escape character %d in RegEx", (int)regexp[1]);
    return false;
  }
  return regexp[0]==ch;
}

/* matchhere: search for regexp at beginning of text */
JsVar *matchhere(char *regexp, JsvStringIterator *txtIt, matchInfo info) {
  if (jspIsInterrupted()) return 0;
  if (regexp[0] == '\0')
    return matchfound(txtIt, info);
  if (regexp[0] == '(') {
    info.groupStart[info.groups] = jsvStringIteratorGetIndex(txtIt);
    info.groupEnd[info.groups] = info.groupStart[info.groups];
    if (info.groups<MAX_GROUPS) info.groups++;
    return matchhere(regexp+1, txtIt, info);
  }
  if (regexp[0] == ')') {
    info.groupEnd[info.groups-1] = jsvStringIteratorGetIndex(txtIt);
    return matchhere(regexp+1, txtIt, info);
  }
  int charLength;
  bool charMatched = matchcharacter(regexp, txtIt, &charLength);
  if (regexp[charLength] == '*') {
    char *regexpAfterStar = regexp+charLength+1;
    // Check ZERO instances
    JsvStringIterator txtIt2 = jsvStringIteratorClone(txtIt);
    JsVar *rmatch = matchhere(regexpAfterStar, &txtIt2, info);
    jsvStringIteratorFree(&txtIt2);
    if (rmatch) return rmatch;
    // Match more than one instances
    while (jsvStringIteratorHasChar(txtIt) && charMatched) {
      jsvStringIteratorNext(txtIt);
      charMatched = matchcharacter(regexp, txtIt, &charLength);
      txtIt2 = jsvStringIteratorClone(txtIt);
      rmatch = matchhere(regexpAfterStar, &txtIt2, info);
      jsvStringIteratorFree(&txtIt2);
      if (rmatch) return rmatch;
    }
    return 0;
  }
  // End of regex
  if (regexp[0] == '$' && regexp[1] == '\0') {
    if (!jsvStringIteratorHasChar(txtIt))
      return matchfound(txtIt, info);
    else
      return 0;
  }
  //
  if (jsvStringIteratorHasChar(txtIt) && charMatched) {
    jsvStringIteratorNext(txtIt);
    return matchhere(regexp+charLength, txtIt, info);
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
Test this regex on a string - returns a result array on success, or `null` otherwise.

```
>/Wo/.exec("Hello World");
=[
  "Wo",
  "index": 6,
  "input": "Hello World"
 ]
```

Or with groups:

```
>/W(o)rld/.exec("Hello World");
=[
  "World",
  "o", "index": 6,
  "input": "Hello World"
 ]
```
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

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "RegExp",
  "name" : "test",
  "params" : [
    ["str","JsVar","A string to match on"]
  ],
  "generate" : "jswrap_regexp_test",
  "return" : ["bool","true for a match, or false"]
}
Test this regex on a string - returns `true` on a successful match, or `false` otherwise
 */
bool jswrap_regexp_test(JsVar *parent, JsVar *str) {
  JsVar *v = jswrap_regexp_exec(parent, str);
  bool r = v && !jsvIsNull(v);
  jsvUnLock(v);
  return r;
}
