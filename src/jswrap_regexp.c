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
#include "jsinteractive.h"

/* TODO:
 *
 * Handling of 'global' flag especially
 * lastIndex support?
 */

#define MAX_GROUPS 9
#define NO_RANGE  256

typedef struct {
  JsVar *sourceStr;
  size_t startIndex;
  bool ignoreCase;
  bool rangeMatch;
  short rangeFirstChar;
  int groups;
  size_t groupStart[MAX_GROUPS];
  size_t groupEnd[MAX_GROUPS];
} matchInfo;

static JsVar *matchhere(char *regexp, JsvStringIterator *txtIt, matchInfo info);

static JsVar *matchfound(JsvStringIterator *txtIt, matchInfo info) {
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

// No match found, so search to end of regex to see if there's an '|'
static JsVar *nomatchfound(char *regexp, matchInfo info) {
  if (!jspCheckStackPosition()) return 0;
  // TODO: | inside a group?
  while (*regexp && *regexp!='|') {
    if (*regexp=='\\') {
      regexp++; // end of regex after escape char!
      if (!*regexp) return 0;
    }
    regexp++;
  }
  if (*regexp != '|') return 0;
  regexp++;

  JsvStringIterator txtIt;
  jsvStringIteratorNew(&txtIt,  info.sourceStr, info.startIndex);
  JsVar *rmatch = matchhere(regexp, &txtIt, info);
  jsvStringIteratorFree(&txtIt);
  return rmatch;
}

/* match: search for regexp anywhere in text */
static JsVar *match(char *regexp, JsVar *str, size_t startIndex, bool ignoreCase) {
  matchInfo info;
  info.sourceStr = str;
  info.startIndex = startIndex;
  info.ignoreCase = ignoreCase;
  info.rangeMatch = false;
  info.rangeFirstChar = NO_RANGE;
  info.groups = 0;

  JsVar *rmatch;
  JsvStringIterator txtIt, txtIt2;
  jsvStringIteratorNew(&txtIt, str, startIndex);
  /* must look even if string is empty */
  jsvStringIteratorClone(&txtIt2, &txtIt);
  rmatch = matchhere(regexp, &txtIt2, info);
  jsvStringIteratorFree(&txtIt2);
  jsvStringIteratorNext(&txtIt);
  while (!rmatch && jsvStringIteratorHasChar(&txtIt)) {
    info.startIndex++;
    jsvStringIteratorClone(&txtIt2, &txtIt);
    rmatch = matchhere(regexp, &txtIt2, info);
    jsvStringIteratorFree(&txtIt2);
    jsvStringIteratorNext(&txtIt);
  }
  jsvStringIteratorFree(&txtIt);
  return rmatch;
}

/* Match one character. Doesn't modify txtIt, returns length of regexp char in *length  */
bool matchcharacter(char *regexp, JsvStringIterator *txtIt, int *length, matchInfo *info) {
  *length = 1;
  char ch = jsvStringIteratorGetChar(txtIt);
  if (regexp[0]=='.') return true;
  if (regexp[0]=='[') { // Character set (any char inside '[]')
    info->rangeMatch = true;
    bool inverted = regexp[1]=='^';
    if (inverted) (*length)++;
    bool matchAny = false;
    while (regexp[*length] && regexp[*length]!=']') {
      int matchLen;
      if (regexp[*length]=='.') { // https://github.com/espruino/Espruino/issues/1948
        matchAny |= ch=='.';
        (*length)++;
      } else {
        matchAny |= matchcharacter(&regexp[*length], txtIt, &matchLen, info);
        (*length) += matchLen;
      }
    }
    if (regexp[*length]==']') {
      (*length)++;
    } else {
      jsExceptionHere(JSET_ERROR, "Unfinished character set in RegEx");
      return false;
    }
    info->rangeMatch = false;
    return matchAny != inverted;
  }
  char cH = regexp[0];
  if (cH=='\\') { // escape character
    *length = 2;
    // fallback to the quoted character (e.g. /,-,? etc.)
    cH = regexp[1];
    // missing quite a few here
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Regular_Expressions
    if (cH=='d') return isNumeric(ch);
    if (cH=='D') return !isNumeric(ch);
    if (cH=='f') { cH=0x0C; goto haveCode; }
    if (cH=='b') { cH=0x08; goto haveCode; }
    if (cH=='n') { cH=0x0A; goto haveCode; }
    if (cH=='r') { cH=0x0D; goto haveCode; }
    if (cH=='s') return isWhitespace(ch);
    if (cH=='S') return !isWhitespace(ch);
    if (cH=='t') { cH=0x09; goto haveCode; }
    if (cH=='v') { cH=0x0B; goto haveCode; }
    if (cH=='w') return isNumeric(ch) || isAlpha(ch) || ch=='_';
    if (cH=='W') return !(isNumeric(ch) || isAlpha(ch) || ch=='_');
    if (cH=='0') { cH=0; goto haveCode; }
    if (cH>='1' && cH<='9') {
      jsExceptionHere(JSET_ERROR, "Backreferences not supported");
      return false;
      // we could implement this if needed by matching against info->groupStart/End
    }
    if (cH=='x' && regexp[2] && regexp[3]) {
      *length = 4;
      cH = (char)hexToByte(regexp[2],regexp[3]);
      goto haveCode;
    }
  }
haveCode:
  if (info->rangeMatch && regexp[*length] == '-' && regexp[1+*length] != ']') { // Character set range start
    info->rangeFirstChar = cH;
    (*length)++;
    int matchLen;
    bool match = matchcharacter(&regexp[*length], txtIt, &matchLen, info);
    (*length)+=matchLen;
    return match;
  }
  if (info->ignoreCase) {
    ch = charToLowerCase(ch);
    cH = charToLowerCase(cH);
  }
  if (info->rangeFirstChar != NO_RANGE) { // Character set range
    char cL = (char)info->rangeFirstChar;
    if (info->ignoreCase) {
      cL = charToLowerCase(cL);
    }
    info->rangeFirstChar = NO_RANGE;
    return (ch >= cL && ch <= cH && cL < cH);
  }
  return cH==ch;
}

/* matchhere: search for regexp at beginning of text. Only handles up to '|' character. Modifies txtIt */
static JsVar *matchhere(char *regexp, JsvStringIterator *txtIt, matchInfo info) {
  if (jspIsInterrupted()) return 0;
  if (regexp[0] == '\0' || // end of regex
      regexp[0] == '|') // end of this 'or' section of regex
    return matchfound(txtIt, info);
  if (regexp[0] == '^') { // must be beginning of String
    if (jsvStringIteratorGetIndex(txtIt)!=0)
      return 0; // no match
    if (!jspCheckStackPosition()) return 0;
    return matchhere(regexp+1, txtIt, info);
  }
  // Marker for end of String
  if (regexp[0] == '$') {
    if (!jsvStringIteratorHasChar(txtIt))
      return matchhere(regexp+1, txtIt, info);
    else
      return nomatchfound(regexp+1, info); // not the end, it's a fail
  }
  if (regexp[0] == '(') {
    info.groupStart[info.groups] = jsvStringIteratorGetIndex(txtIt);
    info.groupEnd[info.groups] = info.groupStart[info.groups];
    if (info.groups<MAX_GROUPS) info.groups++;
    if (!jspCheckStackPosition()) return 0;
    return matchhere(regexp+1, txtIt, info);
  }
  if (regexp[0] == ')') {
    if (info.groups>0)
      info.groupEnd[info.groups-1] = jsvStringIteratorGetIndex(txtIt);
    if (!jspCheckStackPosition()) return 0;
    return matchhere(regexp+1, txtIt, info);
  }
  int charLength;
  bool charMatched = matchcharacter(regexp, txtIt, &charLength, &info);
  if (regexp[charLength] == '*' || regexp[charLength] == '+') {
    char op = regexp[charLength];
    if (!charMatched && op=='+') {
      // with '+' operator it has to match at least once
      return nomatchfound(&regexp[charLength+1], info);
    }
    char *regexpAfterStar = regexp+charLength+1;
    JsvStringIterator txtIt2;
    // Try and match everything after right now
    jsvStringIteratorClone(&txtIt2, txtIt);
    JsVar *lastrmatch = matchhere(regexpAfterStar, &txtIt2, info);
    jsvStringIteratorFree(&txtIt2);
    // Otherwise try and match more than one
    while (jsvStringIteratorHasChar(txtIt) && charMatched) {
      // We had this character matched, so move on and see if we can match with the new one
      jsvStringIteratorNext(txtIt);
      charMatched = matchcharacter(regexp, txtIt, &charLength, &info);
      // See if we can match after the character...
      jsvStringIteratorClone(&txtIt2, txtIt);
      JsVar *rmatch = matchhere(regexpAfterStar, &txtIt2, info);
      jsvStringIteratorFree(&txtIt2);
      // can't match with this - use the last one
      if (rmatch) {
        jsvUnLock(lastrmatch);
        lastrmatch = rmatch;
      }
    }
    return lastrmatch;
  }

  // This character is matched
  if (jsvStringIteratorHasChar(txtIt) && charMatched) {
    jsvStringIteratorNext(txtIt);
    if (!jspCheckStackPosition()) return 0;
    return matchhere(regexp+charLength, txtIt, info);
  }
  // No match
  return nomatchfound(&regexp[charLength], info);
}

/*JSON{
  "type" : "class",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "RegExp"
}
The built-in class for handling Regular Expressions

**Note:** Espruino's regular expression parser does not contain all the features
present in a full ES6 JS engine. However it does contain support for the all the
basics.
*/

/*JSON{
  "type" : "constructor",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "RegExp",
  "name" : "RegExp",
  "generate" : "jswrap_regexp_constructor",
  "params" : [
    ["regex","JsVar","A regular expression as a string"],
    ["flags","JsVar","Flags for the regular expression as a string"]
  ],
  "return" : ["JsVar","A RegExp object"],
  "return_object" : "RegExp"
}
Creates a RegExp object, for handling Regular Expressions
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
  jsvObjectSetChildAndUnLock(r, "lastIndex", jsvNewFromInteger(0));
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
Test this regex on a string - returns a result array on success, or `null`
otherwise.


`/Wo/.exec("Hello World")` will return:

```
[
 "Wo",
 "index": 6,
 "input": "Hello World"
]
```

Or with groups `/W(o)rld/.exec("Hello World")` returns:

```
[
 "World",
 "o", "index": 6,
 "input": "Hello World"
]
```

 */
JsVar *jswrap_regexp_exec(JsVar *parent, JsVar *arg) {
  JsVar *str = jsvAsString(arg);
  JsVarInt lastIndex = jsvGetIntegerAndUnLock(jsvObjectGetChild(parent, "lastIndex", 0));
  JsVar *regex = jsvObjectGetChild(parent, "source", 0);
  if (!jsvIsString(regex) || lastIndex>jsvGetStringLength(str)) {
    jsvUnLock2(str,regex);
    return 0;
  }
  size_t regexLen = jsvGetStringLength(regex);
  char *regexPtr = (char *)alloca(regexLen+1);
  if (!regexPtr) {
    jsvUnLock2(str,regex);
    return 0;
  }
  jsvGetString(regex, regexPtr, regexLen+1);
  jsvUnLock(regex);
  JsVar *rmatch = match(regexPtr, str, (size_t)lastIndex, jswrap_regexp_hasFlag(parent,'i'));
  jsvUnLock(str);
  if (!rmatch) {
    rmatch = jsvNewWithFlags(JSV_NULL);
    lastIndex = 0;
  } else {
    // if it's global, set lastIndex
    if (jswrap_regexp_hasFlag(parent,'g')) {
      JsVar *matchStr = jsvGetArrayItem(rmatch,0);
      lastIndex = jsvGetIntegerAndUnLock(jsvObjectGetChild(rmatch, "index", 0)) +
                  (JsVarInt)jsvGetStringLength(matchStr);
      jsvUnLock(matchStr);
    } else
      lastIndex = 0;
  }
  jsvObjectSetChildAndUnLock(parent, "lastIndex", jsvNewFromInteger(lastIndex));
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
Test this regex on a string - returns `true` on a successful match, or `false`
otherwise
 */
bool jswrap_regexp_test(JsVar *parent, JsVar *str) {
  JsVar *v = jswrap_regexp_exec(parent, str);
  bool r = v && !jsvIsNull(v);
  jsvUnLock(v);
  return r;
}

/// Does this regex have the given flag?
bool jswrap_regexp_hasFlag(JsVar *parent, char flag) {
  JsVar *flags = jsvObjectGetChild(parent, "flags", 0);
  bool has = false;
  if (jsvIsString(flags)) {
    JsvStringIterator it;
    jsvStringIteratorNew(&it, flags, 0);
    while (jsvStringIteratorHasChar(&it)) {
      has |= jsvStringIteratorGetCharAndNext(&it)==flag;
    }
    jsvStringIteratorFree(&it);
  }
  jsvUnLock(flags);
  return has;
}
