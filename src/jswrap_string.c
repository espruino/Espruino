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

/*JSON{
  "type" : "class",
  "class" : "String",
  "check" : "jsvIsString(var)"
}
This is the built-in class for Text Strings.

Text Strings in Espruino are not zero-terminated, so you can store zeros in them.
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
  return jsvAsString(jsvGetArrayItem(args, 0), true);
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
Return the integer value of a single character at the given position in the String.

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
  substring = jsvAsString(substring, false);
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
  "name" : "replace",
  "generate" : "jswrap_string_replace",
  "params" : [
    ["subStr","JsVar","The string to search for"],
    ["newSubStr","JsVar","The string to replace it with"]
  ],
  "return" : ["JsVar","This string with `subStr` replaced"]
}
Search and replace ONE occurrance of `subStr` with `newSubStr` and return the result. This doesn't alter the original string. Regular expressions not supported.
*/
JsVar *jswrap_string_replace(JsVar *parent, JsVar *subStr, JsVar *newSubStr) {
  JsVar *str = jsvAsString(parent, false);
  subStr = jsvAsString(subStr, false);
  newSubStr = jsvAsString(newSubStr, false);

  int idx = jswrap_string_indexOf(parent, subStr, 0, false);
  if (idx>=0) {
    JsVar *newStr = jsvNewFromStringVar(str, 0, (size_t)idx);
    jsvAppendStringVar(newStr, newSubStr, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
    jsvAppendStringVar(newStr, str, (size_t)idx+jsvGetStringLength(subStr), JSVAPPENDSTRINGVAR_MAXLENGTH);
    jsvUnLock(str);
    str = newStr;
  }

  jsvUnLock(subStr);
  jsvUnLock(newSubStr);
  return str;
}


/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "substring",
  "generate" : "jswrap_string_substring",
  "params" : [
    ["start","int","The start character index"],
    ["end","JsVar","The end character index"]
  ],
  "return" : ["JsVar","The part of this string between start and end"]
}*/
JsVar *jswrap_string_substring(JsVar *parent, JsVarInt pStart, JsVar *vEnd) {
  JsVar *res;
  JsVarInt pEnd = jsvIsUndefined(vEnd) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vEnd);
  if (pStart<0) pStart=0;
  if (pEnd<0) pEnd=0;
  if (pEnd<pStart) {
    JsVarInt l = pStart;
    pStart = pEnd;
    pEnd = l;
  }
  res = jsvNewFromEmptyString();
  if (!res) return 0; // out of memory
  jsvAppendStringVar(res, parent, (size_t)pStart, (size_t)(pEnd-pStart));
  return res;
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
  JsVar *res;
  JsVarInt pLen = jsvIsUndefined(vLen) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vLen);
  if (pLen<0) pLen = 0;
  if (pStart<0) pStart += (JsVarInt)jsvGetStringLength(parent);
  if (pStart<0) pStart = 0;
  res = jsvNewFromEmptyString();
  if (!res) return 0; // out of memory
  jsvAppendStringVar(res, parent, (size_t)pStart, (size_t)pLen);
  return res;
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
  JsVar *res;
  JsVarInt pEnd = jsvIsUndefined(vEnd) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vEnd);
  if (pStart<0) pStart += (JsVarInt)jsvGetStringLength(parent);
  if (pEnd<0) pEnd += (JsVarInt)jsvGetStringLength(parent);
  if (pStart<0) pStart = 0;
  if (pEnd<0) pEnd = 0;
  res = jsvNewFromEmptyString();
  if (!res) return 0; // out of memory
  if (pEnd>pStart)
    jsvAppendStringVar(res, parent, (size_t)pStart, (size_t)(pEnd-pStart));
  return res;
}


/*JSON{
  "type" : "method",
  "class" : "String",
  "name" : "split",
  "generate" : "jswrap_string_split",
  "params" : [
    ["separator","JsVar","The start character index"]
  ],
  "return" : ["JsVar","Part of this string from start for len characters"]
}
Return an array made by splitting this string up by the separator. eg. ```'1,2,3'.split(',')==[1,2,3]```
*/
JsVar *jswrap_string_split(JsVar *parent, JsVar *split) {
  JsVar *array = jsvNewWithFlags(JSV_ARRAY);
  if (!array) return 0; // out of memory

  if (jsvIsUndefined(split)) {
    jsvArrayPush(array, parent);
    return array;
  }

  split = jsvAsString(split, false);

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

  JsvStringIterator itsrc, itdst;
  jsvStringIteratorNew(&itsrc, parent, 0);
  jsvStringIteratorNew(&itdst, res, 0);

  while (jsvStringIteratorHasChar(&itsrc)) {
    char ch = jsvStringIteratorGetChar(&itsrc);
    if (upper) {
      if (ch >= 97 && ch <= 122) ch = (char)(ch - 32);
    } else {
      if (ch >= 65 && ch <= 90) ch = (char)(ch + 32); // A-Z
    }
    jsvStringIteratorAppend(&itdst, ch);
    jsvStringIteratorNext(&itsrc);
  }

  jsvStringIteratorFree(&itsrc);
  jsvStringIteratorFree(&itdst);

  return res;
}

