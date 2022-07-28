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
 * This file is designed to be parsed during the build process
 *
 * JavaScript String Functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_string.h"
#include "jsvariterator.h"
#include "jsparse.h"
#ifndef SAVE_ON_FLASH
#include "jswrap_regexp.h"
#endif

/*JSON{
  "type" : "class",
  "class" : "String",
  "check" : "jsvIsString(var)"
}
This is the built-in class for Text Strings.

Text Strings in Espruino are not zero-terminated, so you can store zeros in
them.
 */

/*JSON{
  "type" : "constructor",
  "class" : "String",
  "name" : "String",
  "generate" : "jswrap_string_constructor",
  "params" : [
    ["str","JsVarArray","A value to turn into a string. If undefined or not supplied, an empty String is created."]
  ],
  "return" : ["JsVar","A String"]
}
Create a new String
 */
JsVar *jswrap_string_constructor(JsVar *args) {
  if (jsvGetArrayLength(args)==0)
    return jsvNewFromEmptyString(); // no argument - return an empty string
  return jsvAsStringAndUnLock(jsvGetArrayItem(args, 0));
}

/*JSON{
  "type" : "property",
  "class" : "String",
  "name" : "length",
  "generate" : "jswrap_object_length",
  "return" : ["JsVar","The value of the string"]
}
Find the length of the string
 */

/*JSON{
  "type" : "staticmethod",
  "class" : "String",
  "name" : "fromCharCode",
  "generate" : "jswrap_string_fromCharCode",
  "params" : [
    ["code","JsVarArray","One or more character codes to create a string from (range 0-255)."]
  ],
  "return" : ["JsVar","The character"]
}
Return the character(s) represented by the given character code(s).
 */
JsVar *jswrap_string_fromCharCode(JsVar *arr) {
  assert(jsvIsArray(arr));

  JsVar *r = jsvNewFromEmptyString();
  if (!r) return 0;

  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, arr);
  while (jsvObjectIteratorHasValue(&it)) {
    char ch = (char)jsvGetIntegerAndUnLock(jsvObjectIteratorGetValue(&it));
    jsvAppendStringBuf(r, &ch, 1);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);

  return r;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "charAt",
  "generate" : "jswrap_string_charAt",
  "params" : [
    ["pos","int","The character number in the string. Negative values return characters from end of string (-1 = last char)"]
  ],
  "return" : ["JsVar","The character in the string"]
}
Return a single character at the given position in the String.
 */
JsVar *jswrap_string_charAt(JsVar *parent, JsVarInt idx) {
  // We do this so we can handle '/0' in a string
  JsVar *r = jsvNewFromEmptyString();
  if (r && jsvIsString(parent) && idx>=0) {
    JsvStringIterator it;
    jsvStringIteratorNew(&it, parent, (size_t)idx);
    if (jsvStringIteratorHasChar(&it)) {
      char ch = jsvStringIteratorGetChar(&it);
      jsvAppendStringBuf(r, &ch, 1);
    }
    jsvStringIteratorFree(&it);
  }
  return r;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "charCodeAt",
  "generate" : "jswrap_string_charCodeAt",
  "params" : [
    ["pos","int","The character number in the string. Negative values return characters from end of string (-1 = last char)"]
  ],
  "return" : ["int32","The integer value of a character in the string"]
}
Return the integer value of a single character at the given position in the
String.

Note that this returns 0 not 'NaN' for out of bounds characters
 */
int jswrap_string_charCodeAt(JsVar *parent, JsVarInt idx) {
  return (unsigned char)jsvGetCharInString(parent, (size_t)idx);
}


/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "indexOf",
  "generate_full" : "jswrap_string_indexOf(parent, substring, fromIndex, false)",
  "params" : [
    ["substring","JsVar","The string to search for"],
    ["fromIndex","JsVar","Index to search from"]
  ],
  "return" : ["int32","The index of the string, or -1 if not found"]
}
Return the index of substring in this string, or -1 if not found
 */
/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "lastIndexOf",
  "generate_full" : "jswrap_string_indexOf(parent, substring, fromIndex, true)",
  "params" : [
    ["substring","JsVar","The string to search for"],
    ["fromIndex","JsVar","Index to search from"]
  ],
  "return" : ["int32","The index of the string, or -1 if not found"]
}
Return the last index of substring in this string, or -1 if not found
 */
int jswrap_string_indexOf(JsVar *parent, JsVar *substring, JsVar *fromIndex, bool lastIndexOf) {
  if (!jsvIsString(parent)) return 0;
  // slow, but simple!
  substring = jsvAsString(substring);
  if (!substring) return 0; // out of memory
  int parentLength = (int)jsvGetStringLength(parent);
  int subStringLength = (int)jsvGetStringLength(substring);
  if (subStringLength > parentLength) {
    jsvUnLock(substring);
    return -1;
  }
  int lastPossibleSearch = parentLength - subStringLength;
  int idx, dir, end;
  if (!lastIndexOf) { // normal indexOf
    dir = 1;
    end = lastPossibleSearch+1;
    idx = 0;
    if (jsvIsNumeric(fromIndex)) {
      idx = (int)jsvGetInteger(fromIndex);
      if (idx<0) idx=0;
      if (idx>end) idx=end;
    }
  } else {
    dir = -1;
    end = -1;
    idx = lastPossibleSearch;
    if (jsvIsNumeric(fromIndex)) {
      idx = (int)jsvGetInteger(fromIndex);
      if (idx<0) idx=0;
      if (idx>lastPossibleSearch) idx=lastPossibleSearch;
    }
  }

  for (;idx!=end;idx+=dir) {
    if (jsvCompareString(parent, substring, (size_t)idx, 0, true)==0) {
      jsvUnLock(substring);
      return idx;
    }
  }
  jsvUnLock(substring);
  return -1;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "match",
  "generate" : "jswrap_string_match",
  "params" : [
    ["substr","JsVar","Substring or RegExp to match"]
  ],
  "return" : ["JsVar","A match array or `null` (see below):"]
}
Matches an occurrence `subStr` in the string.

Returns `null` if no match, or:

```
"abcdef".match("b") == [
  "b",         // array index 0 - the matched string
  index: 1,    // the start index of the match
  input: "b"   // the input string
 ]
"abcdefabcdef".match(/bcd/) == [
  "bcd", index: 1,
  input: "abcdefabcdef"
 ]
```

'Global' RegExp matches just return an array of matches (with no indices):

```
"abcdefabcdef".match(/bcd/g) = [
  "bcd",
  "bcd"
 ]
```
 */
JsVar *jswrap_string_match(JsVar *parent, JsVar *subStr) {
  if (!jsvIsString(parent)) return 0;
  if (jsvIsUndefined(subStr)) return 0;

#ifndef SAVE_ON_FLASH
  // Use RegExp if one is passed in
  if (jsvIsInstanceOf(subStr, "RegExp")) {
    jsvObjectSetChildAndUnLock(subStr, "lastIndex", jsvNewFromInteger(0));
    JsVar *match;
    match = jswrap_regexp_exec(subStr, parent);
    if (!jswrap_regexp_hasFlag(subStr,'g')) {
      return match;
    }

    // global
    JsVar *array = jsvNewEmptyArray();
    if (!array) return 0; // out of memory
    while (match && !jsvIsNull(match)) {
      // get info about match
      JsVar *matchStr = jsvGetArrayItem(match,0);
      JsVarInt idx = jsvGetIntegerAndUnLock(jsvObjectGetChild(match,"index",0));
      JsVarInt len = (JsVarInt)jsvGetStringLength(matchStr);
      int last = idx+len;
      jsvArrayPushAndUnLock(array, matchStr);
      // search again
      jsvUnLock(match);
      jsvObjectSetChildAndUnLock(subStr, "lastIndex", jsvNewFromInteger(last + (len?0:1)));
      match = jswrap_regexp_exec(subStr, parent);
    }
    jsvUnLock(match);
    jsvObjectSetChildAndUnLock(subStr, "lastIndex", jsvNewFromInteger(0));
    return array;
  }
#endif

  subStr = jsvAsString(subStr);

  int idx = jswrap_string_indexOf(parent, subStr, 0, false);
  if (idx>=0) {
      JsVar *array = jsvNewEmptyArray();
      if (!array) {
        jsvUnLock(subStr);
        return 0; // out of memory
      }

      jsvArrayPush(array, subStr);
      jsvObjectSetChildAndUnLock(array, "index", jsvNewFromInteger(idx));
      jsvObjectSetChildAndUnLock(array, "input", subStr);
      return array;
  }
  jsvUnLock(subStr);
  return jsvNewNull();
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "replace",
  "generate" : "jswrap_string_replace",
  "params" : [
    ["subStr","JsVar","The string to search for"],
    ["newSubStr","JsVar","The string to replace it with"]
  ],
  "return" : ["JsVar","This string with `subStr` replaced"]
}
Search and replace ONE occurrence of `subStr` with `newSubStr` and return the
result. This doesn't alter the original string. Regular expressions not
supported.
 */
JsVar *jswrap_string_replace(JsVar *parent, JsVar *subStr, JsVar *newSubStr) {
  JsVar *str = jsvAsString(parent);
#ifndef SAVE_ON_FLASH
  // Use RegExp if one is passed in
  if (jsvIsInstanceOf(subStr, "RegExp")) {
    JsVar *replace;
    if (jsvIsFunction(newSubStr) || jsvIsString(newSubStr))
      replace = jsvLockAgain(newSubStr);
    else
      replace = jsvAsString(newSubStr);
    jsvObjectSetChildAndUnLock(subStr, "lastIndex", jsvNewFromInteger(0));
    bool global = jswrap_regexp_hasFlag(subStr,'g');
    JsVar *newStr = jsvNewFromEmptyString();
    JsvStringIterator dst;
    jsvStringIteratorNew(&dst, newStr, 0);
    JsVarInt lastIndex = 0;
    JsVar *match;
    match = jswrap_regexp_exec(subStr, str);
    while (match && !jsvIsNull(match) && !jspIsInterrupted()) {
      // get info about match
      JsVar *matchStr = jsvGetArrayItem(match,0);
      JsVarInt idx = jsvGetIntegerAndUnLock(jsvObjectGetChild(match,"index",0));
      JsVarInt len = (JsVarInt)jsvGetStringLength(matchStr);
      // do the replacement
      jsvStringIteratorAppendString(&dst, str, (size_t)lastIndex, (idx-lastIndex)); // the string before the match
      if (jsvIsFunction(replace)) {
        unsigned int argCount = 0;
        JsVar *args[13];
        args[argCount++] = jsvLockAgain(matchStr);
        JsVar *v;
        while ((v = jsvGetArrayItem(match, (JsVarInt)argCount)))
          args[argCount++] = v;
        args[argCount++] = jsvObjectGetChild(match,"index",0);
        args[argCount++] = jsvObjectGetChild(match,"input",0);
        JsVar *result = jsvAsStringAndUnLock(jspeFunctionCall(replace, 0, 0, false, (JsVarInt)argCount, args));
        jsvUnLockMany(argCount, args);
        jsvStringIteratorAppendString(&dst, result, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
        jsvUnLock(result);
      } else {
        JsvStringIterator src;
        jsvStringIteratorNew(&src, replace, 0);
        while (jsvStringIteratorHasChar(&src)) {
          char ch = jsvStringIteratorGetCharAndNext(&src);
          if (ch=='$') {
            ch = jsvStringIteratorGetCharAndNext(&src);
            JsVar *group = 0;
            if (ch>'0' && ch<='9')
              group = jsvGetArrayItem(match, ch-'0');
            if (group) {
              jsvStringIteratorAppendString(&dst, group, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
              jsvUnLock(group);
            } else {
              jsvStringIteratorAppend(&dst, '$');
              jsvStringIteratorAppend(&dst, ch);
            }
          } else {
            jsvStringIteratorAppend(&dst, ch);
          }
        }
        jsvStringIteratorFree(&src);
      }
      lastIndex = idx+len;
      jsvUnLock(matchStr);
      // search again if global
      jsvUnLock(match);
      match = 0;
      if (global) {
        jsvObjectSetChildAndUnLock(subStr, "lastIndex", jsvNewFromInteger(lastIndex + (len?0:1)));
        match = jswrap_regexp_exec(subStr, str);
      }
    }
    jsvStringIteratorAppendString(&dst, str, (size_t)lastIndex, JSVAPPENDSTRINGVAR_MAXLENGTH); // append the rest of the string
    jsvStringIteratorFree(&dst);
    jsvUnLock3(match,replace,str);
    // reset lastIndex if global
    if (global)
      jsvObjectSetChildAndUnLock(subStr, "lastIndex", jsvNewFromInteger(0));
    return newStr;
  }
#endif

  newSubStr = jsvAsString(newSubStr);
  subStr = jsvAsString(subStr);

  int idx = jswrap_string_indexOf(parent, subStr, 0, false);
  if (idx>=0) {
    JsVar *newStr = jsvNewFromStringVar(str, 0, (size_t)idx);
    jsvAppendStringVar(newStr, newSubStr, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
    jsvAppendStringVar(newStr, str, (size_t)idx+jsvGetStringLength(subStr), JSVAPPENDSTRINGVAR_MAXLENGTH);
    jsvUnLock(str);
    str = newStr;
  }

  jsvUnLock2(subStr, newSubStr);
  return str;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "substring",
  "generate" : "jswrap_string_substring",
  "params" : [
    ["start","int","The start character index (inclusive)"],
    ["end","JsVar","The end character index (exclusive)"]
  ],
  "return" : ["JsVar","The part of this string between start and end"]
}*/
JsVar *jswrap_string_substring(JsVar *parent, JsVarInt pStart, JsVar *vEnd) {
  JsVarInt pEnd = jsvIsUndefined(vEnd) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vEnd);
  if (pStart<0) pStart=0;
  if (pEnd<0) pEnd=0;
  if (pEnd<pStart) {
    JsVarInt l = pStart;
    pStart = pEnd;
    pEnd = l;
  }
  return jsvNewFromStringVar(parent, (size_t)pStart, (size_t)(pEnd-pStart));
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "substr",
  "generate" : "jswrap_string_substr",
  "params" : [
    ["start","int","The start character index"],
    ["len","JsVar","The number of characters"]
  ],
  "return" : ["JsVar","Part of this string from start for len characters"]
}*/
JsVar *jswrap_string_substr(JsVar *parent, JsVarInt pStart, JsVar *vLen) {
  JsVarInt pLen = jsvIsUndefined(vLen) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vLen);
  if (pLen<0) pLen = 0;
  if (pStart<0) pStart += (JsVarInt)jsvGetStringLength(parent);
  if (pStart<0) pStart = 0;
  return jsvNewFromStringVar(parent, (size_t)pStart, (size_t)pLen);
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "slice",
  "generate" : "jswrap_string_slice",
  "params" : [
    ["start","int","The start character index, if negative it is from the end of the string"],
    ["end","JsVar","The end character index, if negative it is from the end of the string, and if omitted it is the end of the string"]
  ],
  "return" : ["JsVar","Part of this string from start for len characters"]
}*/
JsVar *jswrap_string_slice(JsVar *parent, JsVarInt pStart, JsVar *vEnd) {
  JsVarInt pEnd = jsvIsUndefined(vEnd) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vEnd);
  if (pStart<0) pStart += (JsVarInt)jsvGetStringLength(parent);
  if (pEnd<0) pEnd += (JsVarInt)jsvGetStringLength(parent);
  if (pStart<0) pStart = 0;
  if (pEnd<0) pEnd = 0;
  if (pEnd<=pStart) return jsvNewFromEmptyString();
  return jsvNewFromStringVar(parent, (size_t)pStart, (size_t)(pEnd-pStart));
}


/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "split",
  "generate" : "jswrap_string_split",
  "params" : [
    ["separator","JsVar","The separator `String` or `RegExp` to use"]
  ],
  "return" : ["JsVar","Part of this string from start for len characters"]
}
Return an array made by splitting this string up by the separator. e.g.
```'1,2,3'.split(',')==['1', '2', '3']```

Regular Expressions can also be used to split strings, e.g. `'1a2b3
4'.split(/[^0-9]/)==['1', '2', '3', '4']`.
 */
JsVar *jswrap_string_split(JsVar *parent, JsVar *split) {
  if (!jsvIsString(parent)) return 0;
  JsVar *array = jsvNewEmptyArray();
  if (!array) return 0; // out of memory

  if (jsvIsUndefined(split)) {
    jsvArrayPush(array, parent);
    return array;
  }


#ifndef SAVE_ON_FLASH
  // Use RegExp if one is passed in
  if (jsvIsInstanceOf(split, "RegExp")) {
    int last = 0;
    JsVar *match;
    jsvObjectSetChildAndUnLock(split, "lastIndex", jsvNewFromInteger(0));
    match = jswrap_regexp_exec(split, parent);
    while (match && !jsvIsNull(match)) {
      // get info about match
      JsVar *matchStr = jsvGetArrayItem(match,0);
      JsVarInt idx = jsvGetIntegerAndUnLock(jsvObjectGetChild(match,"index",0));
      int len = (int)jsvGetStringLength(matchStr);
      jsvUnLock(matchStr);
      // do the replacement
      jsvArrayPushAndUnLock(array, jsvNewFromStringVar(parent, (size_t)last, (size_t)(idx-last)));
      last = idx+len;
      // search again
      jsvUnLock(match);
      jsvObjectSetChildAndUnLock(split, "lastIndex", jsvNewFromInteger(last));
      match = jswrap_regexp_exec(split, parent);
    }
    jsvUnLock(match);
    jsvObjectSetChildAndUnLock(split, "lastIndex", jsvNewFromInteger(0));
    // add remaining string after last match
    if (last <= (int)jsvGetStringLength(parent))
      jsvArrayPushAndUnLock(array, jsvNewFromStringVar(parent, (size_t)last, JSVAPPENDSTRINGVAR_MAXLENGTH));
    return array;
  }
#endif

  split = jsvAsString(split);

  int idx, last = 0;
  int splitlen = jsvIsUndefined(split) ? 0 : (int)jsvGetStringLength(split);
  int l = (int)jsvGetStringLength(parent) + 1 - splitlen;

  for (idx=0;idx<=l;idx++) {
    if (splitlen==0 && idx==0) continue; // special case for where split string is ""
    if (idx==l || splitlen==0 || jsvCompareString(parent, split, (size_t)idx, 0, true)==0) {
      if (idx==l) {
        idx=l+splitlen; // if the last element, do to the end of the string
        if (splitlen==0) break;
      }

      JsVar *part = jsvNewFromStringVar(parent, (size_t)last, (size_t)(idx-last));
      if (!part) break; // out of memory
      jsvArrayPush(array, part);
      jsvUnLock(part);
      last = idx+splitlen;
    }
  }
  jsvUnLock(split);
  return array;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "toLowerCase",
  "generate_full" : "jswrap_string_toUpperLowerCase(parent, false)",
  "params" : [

  ],
  "return" : ["JsVar","The lowercase version of this string"]
}*/
/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "toUpperCase",
  "generate_full" : "jswrap_string_toUpperLowerCase(parent, true)",
  "params" : [

  ],
  "return" : ["JsVar","The uppercase version of this string"]
}*/
JsVar *jswrap_string_toUpperLowerCase(JsVar *parent, bool upper) {
  JsVar *res = jsvNewFromEmptyString();
  if (!res) return 0; // out of memory
  JsVar *parentStr = jsvAsString(parent);

  JsvStringIterator itsrc, itdst;
  jsvStringIteratorNew(&itsrc, parentStr, 0);
  jsvStringIteratorNew(&itdst, res, 0);

  while (jsvStringIteratorHasChar(&itsrc)) {
    char ch = jsvStringIteratorGetCharAndNext(&itsrc);
    ch = upper ? charToUpperCase(ch) : charToLowerCase(ch);
    jsvStringIteratorAppend(&itdst, ch);
  }

  jsvStringIteratorFree(&itsrc);
  jsvStringIteratorFree(&itdst);
  jsvUnLock(parentStr);

  return res;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "trim",
  "generate" : "jswrap_string_trim",
  "return" : ["JsVar","A String with Whitespace removed from the beginning and end"],
  "return_object" : "String"
}
Return a new string with any whitespace (tabs, space, form feed, newline,
carriage return, etc) removed from the beginning and end.
 */
JsVar *jswrap_string_trim(JsVar *parent) {
  JsVar *s = jsvAsString(parent);
  if (!s) return s;
  unsigned int start = 0;
  int end = -1;

  // work out beginning and end
  JsvStringIterator it;
  jsvStringIteratorNew(&it, s, 0);
  while (jsvStringIteratorHasChar(&it)) {
    size_t idx = jsvStringIteratorGetIndex(&it);
    bool ws = isWhitespace(jsvStringIteratorGetCharAndNext(&it));
    if (!ws) {
      if (end<0) start = (unsigned int)idx;
      end = (int)idx; // last
    }
  }
  jsvStringIteratorFree(&it);
  // work out length
  unsigned int len = 0;
  if (end>=(int)start) len = 1+(unsigned int)end-start;
  JsVar *res = jsvNewFromStringVar(s, start, len);
  jsvUnLock(s);
  return res;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "concat",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_string_concat",
  "params" : [
    ["args","JsVarArray","Strings to append"]
  ],
  "return" : ["JsVar","The result of appending all arguments to this string"]
}
Append all arguments to this `String` and return the result. Does not modify the
original `String`.
*/
JsVar *jswrap_string_concat(JsVar *parent, JsVar *args) {
  if (!jsvIsString(parent)) return 0;
  JsVar *str = jsvNewFromStringVar(parent, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
  JsVar *extra = jsvArrayJoin(args, NULL/*filler*/, false/*ignoreNull*/);
  jsvAppendStringVarComplete(str, extra);
  jsvUnLock(extra);
  return str;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "startsWith",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_string_startsWith",
  "params" : [
    ["searchString","JsVar","The string to search for"],
    ["position","int","The start character index (or 0 if not defined)"]
  ],
  "return" : ["bool","`true` if the given characters are found at the beginning of the string, otherwise, `false`."]
}
*/
bool jswrap_string_startsWith(JsVar *parent, JsVar *search, int position) {
  if (!jsvIsString(parent)) return false;
  JsVar *searchStr = jsvAsString(search);
  bool match = false;
  if (position >= 0 &&
      (int)jsvGetStringLength(searchStr)+position <= (int)jsvGetStringLength(parent))
   match = jsvCompareString(parent, searchStr, (size_t)position,0,true)==0;
  jsvUnLock(searchStr);
  return match;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "endsWith",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_string_endsWith",
  "params" : [
    ["searchString","JsVar","The string to search for"],
    ["length","JsVar","The 'end' of the string - if left off the actual length of the string is used"]
  ],
  "return" : ["bool","`true` if the given characters are found at the end of the string, otherwise, `false`."]
}
*/
bool jswrap_string_endsWith(JsVar *parent, JsVar *search, JsVar *length) {
  if (!jsvIsString(parent)) return false;
  int position = jsvIsNumeric(length) ? jsvGetInteger(length) : (int)jsvGetStringLength(parent);
  JsVar *searchStr = jsvAsString(search);
  position -= (int)jsvGetStringLength(searchStr);
  bool match = false;
  if (position >= 0 &&
      (int)jsvGetStringLength(searchStr)+position <= (int)jsvGetStringLength(parent))
    match = jsvCompareString(parent, searchStr, (size_t)position,0,true)==0;
  jsvUnLock(searchStr);
  return match;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "includes",
  "ifndef" : "SAVE_ON_FLASH",
  "generate_full" : "jswrap_string_indexOf(parent, substring, fromIndex, false)>=0",
  "params" : [
    ["substring","JsVar","The string to search for"],
    ["fromIndex","JsVar","The start character index (or 0 if not defined)"]
  ],
  "return" : ["bool","`true` if the given characters are in the string, otherwise, `false`."]
}
*/


/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "repeat",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_string_repeat",
  "params" : [
    ["count","int","An integer with the amount of times to repeat this String"]
  ],
  "return" : ["JsVar","A string containing repetitions of this string"],
  "return_object" : "String"
}
Repeat this string the given number of times.
*/
JsVar *jswrap_string_repeat(JsVar *parent, int count) {
  if (count<0) {
    jsExceptionHere(JSET_ERROR, "Invalid count value");
    return 0;
  }
  JsVar *result = jsvNewFromEmptyString();
  while (count-- && !jspIsInterrupted())
    jsvAppendStringVarComplete(result, parent);
  return result;
}

/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "padStart",
  "ifndef" : "SAVE_ON_FLASH",
  "generate_full" : "jswrap_string_padX(parent, targetLength, padString, true)",
  "params" : [
    ["targetLength","int","The length to pad this string to"],
    ["padString","JsVar","[optional] The string to pad with, default is `' '`"]
  ],
  "return" : ["JsVar","A string containing this string padded to the correct length"],
  "return_object" : "String"
}
Pad this string at the beginning to the required number of characters

```
"Hello".padStart(10) == "     Hello"
"123".padStart(10,".-") == ".-.-.-.123"
```
*/
/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "padEnd",
  "ifndef" : "SAVE_ON_FLASH",
  "generate_full" : "jswrap_string_padX(parent, targetLength, padString, false)",
  "params" : [
    ["targetLength","int","The length to pad this string to"],
    ["padString","JsVar","[optional] The string to pad with, default is `' '`"]
  ],
  "return" : ["JsVar","A string containing this string padded to the correct length"],
  "return_object" : "String"
}
Pad this string at the end to the required number of characters

```
"Hello".padEnd(10) == "Hello     "
"123".padEnd(10,".-") == "123.-.-.-."
```
*/
JsVar *jswrap_string_padX(JsVar *str, int targetLength, JsVar *padString, bool padStart) {
  if (!jsvIsString(str) || (int)jsvGetStringLength(str)>=targetLength)
    return jsvLockAgain(str);

  int padChars = targetLength - (int)jsvGetStringLength(str);

  JsVar *result = padStart ? jsvNewFromEmptyString() : jsvNewFromStringVar(str,0,JSVAPPENDSTRINGVAR_MAXLENGTH);
  if (!result) return 0;

  padString = padString ? jsvAsString(padString) : jsvNewFromString(" ");
  int padLength = (int)jsvGetStringLength(padString);
  while (padChars > 0) {
    jsvAppendStringVar(result, padString, 0, (size_t)((padLength > padChars) ? padChars : padLength));
    padChars -= padLength;
  }
  if (padStart) jsvAppendStringVarComplete(result, str);
  jsvUnLock(padString);
  return result;
}
