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


/*JSON{ "type":"variable", "name" : "arguments",
         "description" : "A variable containing the arguments given to the function",
         "generate" : "jswrap_arguments",
         "return" : ["JsVar", "An array containing all the arguments given to the function"]
}*/
extern JsExecInfo execInfo;
JsVar *jswrap_arguments() {
  JsVar *scope = 0;
  if (execInfo.scopeCount>0)
    scope = jsvLock(execInfo.scopes[execInfo.scopeCount-1]);
  if (!jsvIsFunction(scope)) {
    jsvUnLock(scope);
    jsError("Can only use 'arguments' variable inside a function");
    return 0;
  }

  JsVar *args = jsvNewWithFlags(JSV_ARRAY);
  if (!args) return 0; // out of memory

  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, scope);
  while (jsvObjectIteratorHasElement(&it)) {
    JsVar *idx = jsvObjectIteratorGetKey(&it);
    if (jsvIsFunctionParameter(idx)) {
      JsVar *val = jsvSkipOneName(idx);
      jsvArrayPushAndUnLock(args, val);
    }
    jsvUnLock(idx);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  jsvUnLock(scope);

  return args;
}


/*JSON{ "type":"function", "name" : "eval",
         "description" : "Evaluate a string containing JavaScript code",
         "generate" : "jswrap_eval",
         "params" : [ [ "code", "JsVar", ""] ],
         "return" : ["JsVar", "The result of evaluating the string"]
}*/
JsVar *jswrap_eval(JsVar *v) {
  if (!v) return 0;
  JsVar *s = jsvAsString(v, false); // get as a string
  JsVar *result = jspEvaluateVar(s, 0);
  jsvUnLock(s);
  return result;
}

/*JSON{ "type":"function", "name" : "parseInt",
         "description" : "Convert a string representing a number into an integer",
         "generate" : "jswrap_parseInt",
         "params" :  [ [ "string", "JsVar", ""],
                       [ "radix", "JsVar", "The Radix of the string (optional)"] ],
         "return" : ["JsVar", "The integer value of the string (or NaN)"]
}*/
JsVar *jswrap_parseInt(JsVar *v, JsVar *radixVar) {
  int radix = 0/*don't force radix*/;
  if (jsvIsNumeric(radixVar))
    radix = (int)jsvGetInteger(radixVar);

  // shortcut for values that are already numbers
  if ((radix==0 || radix==10) && jsvIsNumeric(v))
    return jsvNewFromInteger(jsvGetInteger(v));
  // otherwise convert to string
  char buffer[JS_NUMBER_BUFFER_SIZE];
  jsvGetString(v, buffer, JS_NUMBER_BUFFER_SIZE);
  bool hasError;
  JsVarInt i = stringToIntWithRadix(buffer, radix, &hasError);
  if (hasError) return jsvNewFromFloat(NAN);
  return jsvNewFromInteger(i);
}

/*JSON{ "type":"function", "name" : "parseFloat",
         "description" : "Convert a string representing a number into an float",
         "generate" : "jswrap_parseFloat",
         "params" :  [ [ "string", "JsVar", ""] ],
         "return" : ["float", "The value of the string"]
}*/
JsVarFloat jswrap_parseFloat(JsVar *v) {
  char buffer[JS_NUMBER_BUFFER_SIZE];
  jsvGetString(v, buffer, JS_NUMBER_BUFFER_SIZE);
  return stringToFloat(buffer);
}

/*JSON{ "type":"function", "name" : "isNaN",
         "description" : "Whether the x is NaN (Not a Number) or not",
         "generate" : "jswrap_isNaN",
         "params" :  [ [ "x", "JsVar", ""] ],
         "return" : ["bool", "True is the value is NaN, false if not."]
}*/
bool jswrap_isNaN(JsVar *v) {
  if (jsvIsUndefined(v) ||
      jsvIsObject(v) ||
      (jsvIsFloat(v) && isnan(jsvGetFloat(v)))) return true;
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

/*JSON{ "type":"function", "name" : "btoa", "ifndef" : "SAVE_ON_FLASH",
         "description" : "Convert the supplied string (or array) into a base64 string",
         "generate" : "jswrap_btoa",
         "params" :  [ [ "binaryData", "JsVar", "A string of data to encode"] ],
         "return" : ["JsVar", "A base64 encoded string"]
}*/
JsVar *jswrap_btoa(JsVar *binaryData) {
  if (!jsvIsIterable(binaryData)) {
    jsError("Expecting a string or array, got %t", binaryData);
    return 0;
  }
  JsVar* base64Data = jsvNewFromEmptyString();
  if (!base64Data) return 0;
  JsvIterator itsrc;
  JsvStringIterator itdst;
  jsvIteratorNew(&itsrc, binaryData);
  jsvStringIteratorNew(&itdst, base64Data, 0);


  int padding = 0;
  while (jsvIteratorHasElement(&itsrc)) {
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


/*JSON{ "type":"function", "name" : "atob", "ifndef" : "SAVE_ON_FLASH",
         "description" : "Convert the supplied base64 string into a base64 string",
         "generate" : "jswrap_atob",
         "params" :  [ [ "binaryData", "JsVar", "A string of base64 data to decode"] ],
         "return" : ["JsVar", "A string containing the decoded data"]
}*/
JsVar *jswrap_atob(JsVar *base64Data) {
  if (!jsvIsString(base64Data)) {
    jsError("Expecting a string, got %t", base64Data);
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

  while (jsvStringIteratorHasChar(&itsrc)) {
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
