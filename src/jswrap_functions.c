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
 * JavaScript methods and functions in the global namespace
 * ----------------------------------------------------------------------------
 */
#include "jswrap_functions.h"
#include "jslex.h"
#include "jsparse.h"
#include "jsinteractive.h"


/*JSON{
  "type" : "variable",
  "name" : "arguments",
  "generate" : "jswrap_arguments",
  "return" : ["JsVar","An array containing all the arguments given to the function"]
}
A variable containing the arguments given to the function
 */
extern JsExecInfo execInfo;
JsVar *jswrap_arguments() {
  JsVar *scope = 0;
  if (execInfo.scopeCount>0)
    scope = execInfo.scopes[execInfo.scopeCount-1];
  if (!jsvIsFunction(scope)) {
    jsExceptionHere(JSET_ERROR, "Can only use 'arguments' variable inside a function");
    return 0;
  }

  return jsvGetFunctionArgumentLength(scope);
}



/*JSON{
  "type" : "constructor",
  "class" : "Function",
  "name" : "Function",
  "generate" : "jswrap_function_constructor",
  "params" : [
    ["args","JsVarArray","Zero or more arguments (as strings), followed by a string representing the code to run"]
  ],
  "return" : ["JsVar","A Number object"]
}
Creates a function
 */
JsVar *jswrap_function_constructor(JsVar *args) {
  JsVar *fn = jsvNewWithFlags(JSV_FUNCTION);
  if (!fn) return 0;

  /* Slightly odd form because we want to iterate
   * over all items, but leave the final one as
   * that will be for code. */
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, args);
  JsVar *v = jsvObjectIteratorGetValue(&it);
  jsvObjectIteratorNext(&it);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *s = jsvAsString(v, false);
    if (s) {
      // copy the string - if a string was supplied already we want to make
      // sure we have a new (unreferenced) string
      JsVar *paramName = jsvNewFromStringVar(s, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
      jsvUnLock(s);
      if (paramName) {
        jsvMakeFunctionParameter(paramName); // force this to be called a function parameter
        jsvAddName(fn, paramName);
        jsvUnLock(paramName);
      }
    }

    jsvUnLock(v);
    v = jsvObjectIteratorGetValue(&it);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  jsvObjectSetChildAndUnLock(fn, JSPARSE_FUNCTION_CODE_NAME, v);
  return fn;
}

/*JSON{
  "type" : "function",
  "name" : "eval",
  "generate" : "jswrap_eval",
  "params" : [
    ["code","JsVar",""]
  ],
  "return" : ["JsVar","The result of evaluating the string"]
}
Evaluate a string containing JavaScript code
 */
JsVar *jswrap_eval(JsVar *v) {
  if (!v) return 0;
  JsVar *s = jsvAsString(v, false); // get as a string
  JsVar *result = jspEvaluateVar(s, execInfo.thisVar, 0);
  jsvUnLock(s);
  return result;
}

/*JSON{
  "type" : "function",
  "name" : "parseInt",
  "generate" : "jswrap_parseInt",
  "params" : [
    ["string","JsVar",""],
    ["radix","JsVar","The Radix of the string (optional)"]
  ],
  "return" : ["JsVar","The integer value of the string (or NaN)"]
}
Convert a string representing a number into an integer
 */
JsVar *jswrap_parseInt(JsVar *v, JsVar *radixVar) {
  int radix = 0/*don't force radix*/;
  if (jsvIsNumeric(radixVar))
    radix = (int)jsvGetInteger(radixVar);

  if (jsvIsFloat(v) && !isfinite(jsvGetFloat(v)))
    return jsvNewFromFloat(NAN);

  // otherwise convert to string
  char buffer[JS_NUMBER_BUFFER_SIZE];
  if (jsvGetString(v, buffer, JS_NUMBER_BUFFER_SIZE)==JS_NUMBER_BUFFER_SIZE) {
    jsExceptionHere(JSET_ERROR, "String too big to convert to integer\n");
    return jsvNewFromFloat(NAN);
  }
  bool hasError = false;
  if (!radix && buffer[0]=='0' && isNumeric(buffer[1]))
    radix = 10; // DON'T assume a number is octal if it starts with 0
  long long i = stringToIntWithRadix(buffer, radix, &hasError);
  if (hasError) return jsvNewFromFloat(NAN);
  return jsvNewFromLongInteger(i);
}

/*JSON{
  "type" : "function",
  "name" : "parseFloat",
  "generate" : "jswrap_parseFloat",
  "params" : [
    ["string","JsVar",""]
  ],
  "return" : ["float","The value of the string"]
}
Convert a string representing a number into an float
 */
JsVarFloat jswrap_parseFloat(JsVar *v) {
  char buffer[JS_NUMBER_BUFFER_SIZE];
  if (jsvGetString(v, buffer, JS_NUMBER_BUFFER_SIZE)==JS_NUMBER_BUFFER_SIZE) {
    jsExceptionHere(JSET_ERROR, "String too big to convert to float\n");
    return NAN;
  }
  if (!strcmp(buffer, "Infinity")) return INFINITY;
  if (!strcmp(buffer, "-Infinity")) return -INFINITY;
  return stringToFloat(buffer);
}

/*JSON{
  "type" : "function",
  "name" : "isNaN",
  "generate" : "jswrap_isNaN",
  "params" : [
    ["x","JsVar",""]
  ],
  "return" : ["bool","True is the value is NaN, false if not."]
}
Whether the x is NaN (Not a Number) or not
 */
bool jswrap_isNaN(JsVar *v) {
  if (jsvIsUndefined(v) ||
      jsvIsObject(v) ||
      ((jsvIsFloat(v)||jsvIsArray(v)) && isnan(jsvGetFloat(v)))) return true;
  if (jsvIsString(v)) {
    // this is where is can get a bit crazy
    bool allWhiteSpace = true;
    JsvStringIterator it;
    jsvStringIteratorNew(&it,v,0);
    while (jsvStringIteratorHasChar(&it)) {
      if (!isWhitespace(jsvStringIteratorGetChar(&it))) {
        allWhiteSpace = false;
        break;
      }
      jsvStringIteratorNext(&it);
    }
    jsvStringIteratorFree(&it);
    if (allWhiteSpace) return false;
    return isnan(jsvGetFloat(v));
  }
  return false;
}


NO_INLINE static int jswrap_btoa_encode(int c) {
  c = c & 0x3F;
  if (c<26) return 'A'+c;
  if (c<52) return 'a'+c-26;
  if (c<62) return '0'+c-52;
  if (c==62) return '+';
  return '/'; // c==63
}

NO_INLINE static int jswrap_atob_decode(int c) {
  c = c & 0xFF;
  if (c>='A' && c<='Z') return c-'A';
  if (c>='a' && c<='z') return 26+c-'a';
  if (c>='0' && c<='9') return 52+c-'0';
  if (c=='+') return 62;
  if (c=='/') return 63;
  return -1; // not found
}

/*JSON{
  "type" : "function",
  "name" : "btoa",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_btoa",
  "params" : [
    ["binaryData","JsVar","A string of data to encode"]
  ],
  "return" : ["JsVar","A base64 encoded string"]
}
Encode the supplied string (or array) into a base64 string
 */
JsVar *jswrap_btoa(JsVar *binaryData) {
  if (!jsvIsIterable(binaryData)) {
    jsExceptionHere(JSET_ERROR, "Expecting a string or array, got %t", binaryData);
    return 0;
  }
  JsVar* base64Data = jsvNewFromEmptyString();
  if (!base64Data) return 0;
  JsvIterator itsrc;
  JsvStringIterator itdst;
  jsvIteratorNew(&itsrc, binaryData);
  jsvStringIteratorNew(&itdst, base64Data, 0);


  int padding = 0;
  while (jsvIteratorHasElement(&itsrc) && !jspIsInterrupted()) {
    int octet_a = (unsigned char)jsvIteratorGetIntegerValue(&itsrc)&255;
    jsvIteratorNext(&itsrc);
    int octet_b = 0, octet_c = 0;
    if (jsvIteratorHasElement(&itsrc)) {
      octet_b = jsvIteratorGetIntegerValue(&itsrc)&255;
      jsvIteratorNext(&itsrc);
      if (jsvIteratorHasElement(&itsrc)) {
        octet_c = jsvIteratorGetIntegerValue(&itsrc)&255;
        jsvIteratorNext(&itsrc);
        padding = 0;
      } else
        padding = 1;
    } else
      padding = 2;

    int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    jsvStringIteratorAppend(&itdst, (char)jswrap_btoa_encode(triple >> 18));
    jsvStringIteratorAppend(&itdst, (char)jswrap_btoa_encode(triple >> 12));
    jsvStringIteratorAppend(&itdst, (char)((padding>1)?'=':jswrap_btoa_encode(triple >> 6)));
    jsvStringIteratorAppend(&itdst, (char)((padding>0)?'=':jswrap_btoa_encode(triple)));
  }

  jsvIteratorFree(&itsrc);
  jsvStringIteratorFree(&itdst);

  return base64Data;
}

/*JSON{
  "type" : "function",
  "name" : "atob",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_atob",
  "params" : [
    ["binaryData","JsVar","A string of base64 data to decode"]
  ],
  "return" : ["JsVar","A string containing the decoded data"]
}
Decode the supplied base64 string into a normal string
 */
JsVar *jswrap_atob(JsVar *base64Data) {
  if (!jsvIsString(base64Data)) {
    jsExceptionHere(JSET_ERROR, "Expecting a string, got %t", base64Data);
    return 0;
  }
  JsVar* binaryData = jsvNewFromEmptyString();
  if (!binaryData) return 0;
  JsvStringIterator itsrc;
  JsvStringIterator itdst;
  jsvStringIteratorNew(&itsrc, base64Data, 0);
  jsvStringIteratorNew(&itdst, binaryData, 0);
  // skip whitespace
  while (jsvStringIteratorHasChar(&itsrc) &&
      isWhitespace(jsvStringIteratorGetChar(&itsrc)))
    jsvStringIteratorNext(&itsrc);

  while (jsvStringIteratorHasChar(&itsrc) && !jspIsInterrupted()) {
    uint32_t triple = 0;
    int i, valid=0;
    for (i=0;i<4;i++) {
      if (jsvStringIteratorHasChar(&itsrc)) {
        int sextet = jswrap_atob_decode(jsvStringIteratorGetChar(&itsrc));
        jsvStringIteratorNext(&itsrc);
        if (sextet>=0) {
          triple |= (unsigned int)(sextet) << ((3-i)*6);
          valid=i;
        }
      }
    }

    if (valid>0) jsvStringIteratorAppend(&itdst, (char)(triple >> 16));
    if (valid>1) jsvStringIteratorAppend(&itdst, (char)(triple >> 8));
    if (valid>2) jsvStringIteratorAppend(&itdst, (char)(triple));
  }

  jsvStringIteratorFree(&itsrc);
  jsvStringIteratorFree(&itdst);

  return binaryData;
}

/*JSON{
  "type" : "function",
  "name" : "encodeURIComponent",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_encodeURIComponent",
  "params" : [
    ["str","JsVar","A string to encode as a URI"]
  ],
  "return" : ["JsVar","A string containing the encoded data"]
}
Convert a string with any character not alphanumeric or `- _ . ! ~ * ' ( )` converted to the form `%XY` where `XY` is its hexadecimal representation
 */
JsVar *jswrap_encodeURIComponent(JsVar *arg) {
  JsVar *v = jsvAsString(arg, false);
  if (!v) return 0;
  JsVar *result = jsvNewFromEmptyString();
  if (result) {
    JsvStringIterator it;
    jsvStringIteratorNew(&it, v, 0);
    JsvStringIterator dst;
    jsvStringIteratorNew(&dst, result, 0);
    while (jsvStringIteratorHasChar(&it)) {
      char ch = jsvStringIteratorGetChar(&it);
      if (isAlpha(ch) || isNumeric(ch) ||
          ch=='-' || // _ in isAlpha
          ch=='.' ||
          ch=='!' ||
          ch=='~' ||
          ch=='*' ||
          ch=='\'' ||
          ch=='(' ||
          ch==')') {
        jsvStringIteratorAppend(&dst, ch);
      } else {
        jsvStringIteratorAppend(&dst, '%');
        unsigned int d = ((unsigned)ch)>>4;
        jsvStringIteratorAppend(&dst, (char)((d>9)?('A'+d-10):('0'+d)));
        d = ((unsigned)ch)&15;
        jsvStringIteratorAppend(&dst, (char)((d>9)?('A'+d-10):('0'+d)));
      }
      jsvStringIteratorNext(&it);
    }
    jsvStringIteratorFree(&dst);
    jsvStringIteratorFree(&it);
  }
  jsvUnLock(v);
  return result;
}

/*JSON{
  "type" : "function",
  "name" : "decodeURIComponent",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_decodeURIComponent",
  "params" : [
    ["str","JsVar","A string to decode from a URI"]
  ],
  "return" : ["JsVar","A string containing the decoded data"]
}
Convert any groups of characters of the form '%ZZ', into characters with hex code '0xZZ'
 */
JsVar *jswrap_decodeURIComponent(JsVar *arg) {
  JsVar *v = jsvAsString(arg, false);
  if (!v) return 0;
  JsVar *result = jsvNewFromEmptyString();
  if (result) {
    JsvStringIterator it;
    jsvStringIteratorNew(&it, v, 0);
    JsvStringIterator dst;
    jsvStringIteratorNew(&dst, result, 0);
    while (jsvStringIteratorHasChar(&it)) {
      char ch = jsvStringIteratorGetChar(&it);
      if (ch>>7) {
        jsExceptionHere(JSET_ERROR, "ASCII only\n");
        break;
      }
      if (ch!='%') {
        jsvStringIteratorAppend(&dst, ch);
      } else {
        jsvStringIteratorNext(&it);
        int hi = chtod(jsvStringIteratorGetChar(&it));
        jsvStringIteratorNext(&it);
        int lo = chtod(jsvStringIteratorGetChar(&it));
        ch = (char)((hi<<4)|lo);
        if (hi<0 || lo<0 || ch>>7) {
          jsExceptionHere(JSET_ERROR, "Invalid URI\n");
          break;
        }
        jsvStringIteratorAppend(&dst, ch);
      }
      jsvStringIteratorNext(&it);
    }
    jsvStringIteratorFree(&dst);
    jsvStringIteratorFree(&it);
  }
  jsvUnLock(v);
  return result;
}
