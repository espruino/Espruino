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

/*JSON{ "type":"class",
        "class" : "String",
        "check" : "jsvIsString(var)",
        "description" : ["This is the built-in class for Text Strings.",
                         "Text Strings in Espruino are not zero-terminated, so you can store zeros in them." ]
}*/

/*JSON{ "type":"constructor", "class": "String",  "name": "String",
         "description" : "Create a new String",
         "generate" : "jswrap_string_constructor",
         "params" : [ [ "str", "JsVar", "A value to turn into a string. If undefined or not supplied, an empty String is created." ] ],
         "return" : [ "JsVar", "A String" ]

}*/
JsVar *jswrap_string_constructor(JsVar *a) {
  if (!a) return jsvNewFromEmptyString(); // no argument - return an empty string
  return jsvAsString(a, false);
}

/*JSON{ "type":"staticmethod", "class": "String", "name" : "fromCharCode",
         "description" : "Return the character(s) represented by the given character code(s).",
         "generate" : "jswrap_string_fromCharCode",
         "params" : [ [ "code", "JsVarArray", "One or more character codes to create a string from (range 0-255)."] ],
         "return" : ["JsVar", "The character"]
}*/
JsVar *jswrap_string_fromCharCode(JsVar *arr) {
  assert(jsvIsArray(arr));

  JsVar *r = jsvNewFromEmptyString();
  if (!r) return 0;

  JsvArrayIterator it;
  jsvArrayIteratorNew(&it, arr);
  while (jsvArrayIteratorHasElement(&it)) {
    char ch = (char)jsvGetIntegerAndUnLock(jsvArrayIteratorGetElement(&it));
    jsvAppendStringBuf(r, &ch, 1);
    jsvArrayIteratorNext(&it);
  }
  jsvArrayIteratorFree(&it);

  return r;
}

/*JSON{ "type":"method", "class": "String", "name" : "charAt",
         "description" : "Return a single character at the given position in the String.",
         "generate" : "jswrap_string_charAt",
         "params" : [ [ "pos", "int", "The character number in the string. Negative values return characters from end of string (-1 = last char)"] ],
         "return" : ["JsVar", "The character in the string"]
}*/
JsVar *jswrap_string_charAt(JsVar *parent, JsVarInt idx) {
  // We do this so we can handle '/0' in a string
  JsVar *r = jsvNewFromEmptyString();
  if (r && jsvIsString(parent)) {
    JsvStringIterator it;
    jsvStringIteratorNew(&it, parent, idx);
    if (jsvStringIteratorHasChar(&it)) {
      char ch = jsvStringIteratorGetChar(&it);
      jsvAppendStringBuf(r, &ch, 1);
    }
    jsvStringIteratorFree(&it);
  }
  return r;
}

/*JSON{ "type":"method", "class": "String", "name" : "charCodeAt",
         "description" : ["Return the integer value of a single character at the given position in the String.",
                          "Note that this returns 0 not 'NaN' for out of bounds characters"],
         "generate" : "jswrap_string_charCodeAt",
         "params" : [ [ "pos", "int", "The character number in the string. Negative values return characters from end of string (-1 = last char)"] ],
         "return" : ["int32", "The integer value of a character in the string"]
}*/
int jswrap_string_charCodeAt(JsVar *parent, JsVarInt idx) {
  return (unsigned char)jsvGetCharInString(parent, (size_t)idx);
}


/*JSON{ "type":"method", "class": "String", "name" : "indexOf",
         "description" : "Return the index of substring in this string, or -1 if not found",
         "generate_full" : "jswrap_string_indexOf(parent, substring, fromIndex, false)",
         "params" : [ [ "substring", "JsVar", "The string to search for"],
                      [ "fromIndex", "JsVar", "Index to search from"] ],
         "return" : ["int32", "The index of the string, or -1 if not found"]
}*/
/*JSON{ "type":"method", "class": "String", "name" : "lastIndexOf",
         "description" : "Return the last index of substring in this string, or -1 if not found",
         "generate_full" : "jswrap_string_indexOf(parent, substring, fromIndex, true)",
         "params" : [ [ "substring", "JsVar", "The string to search for"],
                      [ "fromIndex", "JsVar", "Index to search from"] ],
         "return" : ["int32", "The index of the string, or -1 if not found"]
}*/
int jswrap_string_indexOf(JsVar *parent, JsVar *substring, JsVar *fromIndex, bool lastIndexOf) {
  // slow, but simple!
  substring = jsvAsString(substring, false);
  if (!substring) return 0; // out of memory
  int parentLength = (int)jsvGetStringLength(parent);
  int lastPossibleSearch = parentLength - (int)jsvGetStringLength(substring);
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

/*JSON{ "type":"method", "class": "String", "name" : "substring",
         "generate" : "jswrap_string_substring",
         "params" : [ [ "start", "int", "The start character index"],
                      [ "end", "JsVar", "The end character index"] ],
         "return" : ["JsVar", "The part of this string between start and end"]
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
  res = jsvNewWithFlags(JSV_STRING);
  if (!res) return 0; // out of memory
  jsvAppendStringVar(res, parent, (size_t)pStart, (size_t)(pEnd-pStart));
  return res;
}

/*JSON{ "type":"method", "class": "String", "name" : "substr",
         "generate" : "jswrap_string_substr",
         "params" : [ [ "start", "int", "The start character index"],
                      [ "len", "JsVar", "The number of characters"] ],
         "return" : ["JsVar", "Part of this string from start for len characters"]
}*/
JsVar *jswrap_string_substr(JsVar *parent, JsVarInt pStart, JsVar *vLen) {
  JsVar *res;
  JsVarInt pLen = jsvIsUndefined(vLen) ? JSVAPPENDSTRINGVAR_MAXLENGTH : (int)jsvGetInteger(vLen);
  if (pLen<0) pLen = 0;
  if (pStart<0) pStart += (JsVarInt)jsvGetStringLength(parent);
  if (pStart<0) pStart = 0;
  res = jsvNewWithFlags(JSV_STRING);
  if (!res) return 0; // out of memory
  jsvAppendStringVar(res, parent, (size_t)pStart, (size_t)pLen);
  return res;
}

/*JSON{ "type":"method", "class": "String", "name" : "split",
         "description" : "Return an array made by splitting this string up by the separator. eg. ```'1,2,3'.split(',')==[1,2,3]```",
         "generate" : "jswrap_string_split",
         "params" : [ [ "separator", "JsVar", "The start character index"] ],
         "return" : ["JsVar", "Part of this string from start for len characters"]
}*/
JsVar *jswrap_string_split(JsVar *parent, JsVar *split) {
  JsVar *array = jsvNewWithFlags(JSV_ARRAY);
  if (!array) return 0; // out of memory

  if (jsvIsUndefined(split)) {
    jsvArrayPush(array, parent);
    return array;
  }

  split = jsvAsString(split, true);


  int idx, last = 0;
  int splitlen = jsvIsUndefined(split) ? 0 : (int)jsvGetStringLength(split);
  int l = (int)jsvGetStringLength(parent) - splitlen;

  for (idx=0;idx<=l;idx++) {
    if (splitlen==0 &&idx==0) continue; // special case for where split string is ""
    if (idx==l || splitlen==0 || jsvCompareString(parent, split, (size_t)idx, 0, true)==0) {
      if (idx==l) idx=l+splitlen; // if the last element, do to the end of the string
      JsVar *part = jsvNewFromStringVar(parent, (size_t)last, (size_t)(idx-last));
      if (!part) break; // out of memory
      jsvArrayPush(array, part);
      jsvUnLock(part);
      last = idx+splitlen;
    }
  }
  return array;
}

/*JSON { "type":"method", "class": "String", "name": "toLowerCase",
         "generate_full": "jswrap_string_toUpperLowerCase(parent, false)",
         "params": [],
         "return": ["JsVar", "The lowercase version of this string"]
}*/
/*JSON { "type":"method", "class": "String", "name": "toUpperCase",
         "generate_full": "jswrap_string_toUpperLowerCase(parent, true)",
         "params": [],
         "return": ["JsVar", "The uppercase version of this string"]
}*/
JsVar *jswrap_string_toUpperLowerCase(JsVar *parent, bool upper) {
  JsVar *res = jsvNewWithFlags(JSV_STRING);
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

