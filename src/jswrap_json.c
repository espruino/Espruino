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
 * JavaScript JSON-handling Functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_json.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrapper.h"

const unsigned int JSON_LIMIT_AMOUNT = 15; // how big does an array get before we start to limit what we show
const unsigned int JSON_LIMITED_AMOUNT = 5; // When limited, how many items do we show at the beginning and end
const unsigned int JSON_LIMIT_STRING_AMOUNT = 40; // how big are strings before we limit them?
const unsigned int JSON_LIMITED_STRING_AMOUNT = 17; // When limited, how many chars do we show at the beginning and end
const unsigned int JSON_ITEMS_ON_LINE_OBJECT = 4; // How many items are allowed end to end on a line.
const char *JSON_LIMIT_TEXT = " ... ";


/*JSON{
  "type" : "class",
  "class" : "JSON"
}
An Object that handles conversion to and from the JSON data interchange format
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "JSON",
  "name" : "stringify",
  "generate" : "jswrap_json_stringify",
  "params" : [
    ["data","JsVar","The data to be converted to a JSON string"]
  ],
  "return" : ["JsVar","A JSON string"]
}
Convert the given object into a JSON string which can subsequently be parsed with JSON.parse or eval
*/
JsVar *jswrap_json_stringify(JsVar *v) {
  JsVar *result = jsvNewFromEmptyString();
  if (result) // could be out of memory
    jsfGetJSON(v, result, JSON_IGNORE_FUNCTIONS|JSON_NO_UNDEFINED);
  return result;
}


JsVar *jswrap_json_parse_internal(JsLex *lex) {
  switch (lex->tk) {
    case LEX_R_TRUE:  jslGetNextToken(lex); return jsvNewFromBool(true);
    case LEX_R_FALSE: jslGetNextToken(lex); return jsvNewFromBool(false);
    case LEX_R_NULL:  jslGetNextToken(lex); return jsvNewWithFlags(JSV_NULL);
    case '-': {
      jslGetNextToken(lex);
      if (lex->tk!=LEX_INT && lex->tk!=LEX_FLOAT) return 0;
      JsVar *v = jswrap_json_parse_internal(lex);
      JsVar *zero = jsvNewFromInteger(0);
      JsVar *r = jsvMathsOp(zero, v, '-');
      jsvUnLock(v);
      jsvUnLock(zero);
      return r;
    }
    case LEX_INT: {
      long long v = stringToInt(jslGetTokenValueAsString(lex));
      jslGetNextToken(lex);
      return jsvNewFromLongInteger(v);
    }
    case LEX_FLOAT: {
      JsVarFloat v = stringToFloat(jslGetTokenValueAsString(lex));
      jslGetNextToken(lex);
      return jsvNewFromFloat(v);
    }
    case LEX_STR: {
      JsVar *a = jslGetTokenValueAsVar(lex);
      jslGetNextToken(lex);
      return a;
    }
    case '[': {
      JsVar *arr = jsvNewWithFlags(JSV_ARRAY); if (!arr) return 0;
      jslGetNextToken(lex); // [
      while (lex->tk != ']') {
        JsVar *value = jswrap_json_parse_internal(lex);
        if (!value ||
            (lex->tk!=']' && !jslMatch(lex, ','))) {
          jsvUnLock(value);
          jsvUnLock(arr);
          return 0;
        }
        jsvArrayPush(arr, value);
        jsvUnLock(value);
      }
      if (!jslMatch(lex, ']')) {
        jsvUnLock(arr);
        return 0;
      }
      return arr;
    }
    case '{': {
      JsVar *obj = jsvNewWithFlags(JSV_OBJECT); if (!obj) return 0;
      jslGetNextToken(lex); // {
      while (lex->tk == LEX_STR) {
        JsVar *key = jsvAsArrayIndexAndUnLock(jslGetTokenValueAsVar(lex));
        jslGetNextToken(lex);
        JsVar *value = 0;
        if (!jslMatch(lex, ':') ||
            !(value=jswrap_json_parse_internal(lex)) ||
            (lex->tk!='}' && !jslMatch(lex, ','))) {
          jsvUnLock(key);
          jsvUnLock(value);
          jsvUnLock(obj);
          return 0;
        }
        jsvAddName(obj, jsvMakeIntoVariableName(key, value));
        jsvUnLock(value);
        jsvUnLock(key);
      }
      if (!jslMatch(lex, '}')) {
        jsvUnLock(obj);
        return 0;
      }
      return obj;
    }
    default: return 0; // undefined = error
  }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "JSON",
  "name" : "parse",
  "generate" : "jswrap_json_parse",
  "params" : [
    ["string","JsVar","A JSON string"]
  ],
  "return" : ["JsVar","The JavaScript object created by parsing the data string"]
}
Parse the given JSON string into a JavaScript object

NOTE: This implementation uses eval() internally, and as such it is unsafe as it can allow arbitrary JS commands to be executed.
*/
JsVar *jswrap_json_parse(JsVar *v) {
  JsLex lex;
  JsVar *str = jsvAsString(v, false);
  jslInit(&lex, str);
  jsvUnLock(str);
  JsVar *res = jswrap_json_parse_internal(&lex);
  jslKill(&lex);
  return res;
}

/* This is like jsfGetJSONWithCallback, but handles ONLY functions (and does not print the initial 'function' text) */
void jsfGetJSONForFunctionWithCallback(JsVar *var, JSONFlags flags, vcbprintf_callback user_callback, void *user_data) {
  assert(jsvIsFunction(var));
  JsVar *codeVar = 0; // TODO: this should really be in jsvAsString

  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, var);

  bool firstParm = true;
  cbprintf(user_callback, user_data, "(");
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *child = jsvObjectIteratorGetKey(&it);
    if (jsvIsFunctionParameter(child)) {
      if (firstParm)
        firstParm=false;
      else
        cbprintf(user_callback, user_data, ",");
      cbprintf(user_callback, user_data, "%v", child);
    } else if (jsvIsString(child) && jsvIsStringEqual(child, JSPARSE_FUNCTION_CODE_NAME)) {
      codeVar = jsvObjectIteratorGetValue(&it);
    }
    jsvUnLock(child);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  cbprintf(user_callback, user_data, ") ");

  if (jsvIsNative(var)) {
    cbprintf(user_callback, user_data, "{ [native code] }");
  } else {
    if (codeVar) {
      if (flags & JSON_LIMIT) {
        cbprintf(user_callback, user_data, "{%s}", JSON_LIMIT_TEXT);
      } else {
        cbprintf(user_callback, user_data, "%v", codeVar);
      }
    } else cbprintf(user_callback, user_data, "{}");
  }
  jsvUnLock(codeVar);
}

void jsfGetEscapedString(JsVar *var, vcbprintf_callback user_callback, void *user_data) {
  user_callback("\"",user_data);
  JsvStringIterator it;
  jsvStringIteratorNew(&it, var, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    user_callback(escapeCharacter(ch), user_data);
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  user_callback("\"",user_data);
}

bool jsonNeedsNewLine(JsVar *v) {
  return !(jsvIsUndefined(v) || jsvIsNull(v) || jsvIsNumeric(v));
  // we're skipping strings here because they're usually long and want printing on multiple lines
}

void jsonNewLine(JSONFlags flags, vcbprintf_callback user_callback, void *user_data) {
  cbprintf(user_callback, user_data, "\n");
  // apply the indent
  unsigned int indent = flags / JSON_INDENT;
  while (indent--)
    cbprintf(user_callback, user_data, "  ");
}

void jsfGetJSONWithCallback(JsVar *var, JSONFlags flags, vcbprintf_callback user_callback, void *user_data) {
  JSONFlags nflags = flags + JSON_INDENT; // if we add a newline, make sure we indent any subsequent JSON more

  if (jsvIsUndefined(var)) {
    cbprintf(user_callback, user_data, "undefined");
  } else {
    // Use IS_RECURSING  flag to stop recursion
    if (var->flags & JSV_IS_RECURSING) {
      cbprintf(user_callback, user_data, " ... ");
      return;
    }
    var->flags |= JSV_IS_RECURSING;

    if (jsvIsArray(var)) {
      size_t length = (size_t)jsvGetArrayLength(var);
      bool limited = (flags&JSON_LIMIT) && (length>JSON_LIMIT_AMOUNT);
      bool needNewLine = false;
      size_t i;
      cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?"[ ":"[");
      for (i=0;i<length && !jspIsInterrupted();i++) {
        if (!limited || i<JSON_LIMITED_AMOUNT || i>=length-JSON_LIMITED_AMOUNT) {
          if (i>0) cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?", ":",");
          if (limited && i==length-JSON_LIMITED_AMOUNT) {
            if (needNewLine) jsonNewLine(nflags, user_callback, user_data);
            cbprintf(user_callback, user_data, JSON_LIMIT_TEXT);
          }
          JsVar *item = jsvGetArrayItem(var, (JsVarInt)i);
          if (jsvIsUndefined(item) && (flags&JSON_NO_UNDEFINED))
              item = jsvNewWithFlags(JSV_NULL);
          bool newNeedsNewLine = (flags&JSON_NEWLINES) && jsonNeedsNewLine(item);
          if (needNewLine || newNeedsNewLine) {
            jsonNewLine(nflags, user_callback, user_data);
            needNewLine = false;
          }
          jsfGetJSONWithCallback(item, nflags, user_callback, user_data);
          needNewLine = newNeedsNewLine;
          jsvUnLock(item);
        }
      }
      if (needNewLine) jsonNewLine(flags, user_callback, user_data);
      cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?" ]":"]");
    } else if (jsvIsArrayBuffer(var)) {
      JsvArrayBufferIterator it;
      bool allZero = true;
      jsvArrayBufferIteratorNew(&it, var, 0);
      while (jsvArrayBufferIteratorHasElement(&it)) {
        if (jsvArrayBufferIteratorGetFloatValue(&it)!=0)
          allZero = false;
        jsvArrayBufferIteratorNext(&it);
      }
      jsvArrayBufferIteratorFree(&it);

      if (allZero) {
        cbprintf(user_callback, user_data, "new %s(%d)", jswGetBasicObjectName(var), jsvGetArrayBufferLength(var));
      } else {
        cbprintf(user_callback, user_data, "new %s([", jswGetBasicObjectName(var));
        size_t length = jsvGetArrayBufferLength(var);
        bool limited = (flags&JSON_LIMIT) && (length>JSON_LIMIT_AMOUNT);
        // no newlines needed for array buffers as they only contain simple stuff

        jsvArrayBufferIteratorNew(&it, var, 0);
        while (jsvArrayBufferIteratorHasElement(&it) && !jspIsInterrupted()) {
          if (!limited || it.index<JSON_LIMITED_AMOUNT || it.index>=length-JSON_LIMITED_AMOUNT) {
            if (it.index>0) cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?", ":",");
            if (limited && it.index==length-JSON_LIMITED_AMOUNT) cbprintf(user_callback, user_data, JSON_LIMIT_TEXT);
            JsVar *item = jsvArrayBufferIteratorGetValue(&it);
            jsfGetJSONWithCallback(item, nflags, user_callback, user_data);
            jsvUnLock(item);
          }
          jsvArrayBufferIteratorNext(&it);
        }
        jsvArrayBufferIteratorFree(&it);
        cbprintf(user_callback, user_data, "])");
      }
    } else if (jsvIsObject(var)) {
      IOEventFlags device = (flags & JSON_SHOW_DEVICES) ? jsiGetDeviceFromClass(var) : EV_NONE;
      if (device!=EV_NONE) {
        cbprintf(user_callback, user_data, "%s", jshGetDeviceString(device));
      } else {
        bool first = true;
        bool needNewLine = false;
        size_t sinceNewLine = 0;
        JsvObjectIterator it;
        jsvObjectIteratorNew(&it, var);
        cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?"{ ":"{");
        while (jsvObjectIteratorHasValue(&it) && !jspIsInterrupted()) {
          JsVar *index = jsvObjectIteratorGetKey(&it);
          JsVar *item = jsvObjectIteratorGetValue(&it);
          bool hidden = jsvIsInternalObjectKey(index) ||
                        ((flags & JSON_IGNORE_FUNCTIONS) && jsvIsFunction(item)) ||
                        ((flags&JSON_NO_UNDEFINED) && jsvIsUndefined(item));
          if (!hidden) {
            sinceNewLine++;
            if (!first) cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?", ":",");
            bool newNeedsNewLine = (flags&JSON_NEWLINES) && jsonNeedsNewLine(item);
            if ((flags&JSON_NEWLINES) && sinceNewLine>JSON_ITEMS_ON_LINE_OBJECT)
              needNewLine = true;
            if (needNewLine || newNeedsNewLine) {
              jsonNewLine(nflags, user_callback, user_data);
              needNewLine = false;
              sinceNewLine = 0;
            }
            cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?"%q: ":"%q:", index);
            if (first)
              first = false;
            jsfGetJSONWithCallback(item, nflags, user_callback, user_data);
            needNewLine = newNeedsNewLine;
          }
          jsvUnLock(index);
          jsvUnLock(item);
          jsvObjectIteratorNext(&it);
        }
        jsvObjectIteratorFree(&it);
        if (needNewLine) jsonNewLine(flags, user_callback, user_data);
        cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?" }":"}");
      }
    } else if (jsvIsFunction(var)) {
      if (flags & JSON_IGNORE_FUNCTIONS) {
        cbprintf(user_callback, user_data, "undefined");
      } else {
        cbprintf(user_callback, user_data, "function ");
        jsfGetJSONForFunctionWithCallback(var, nflags, user_callback, user_data);
      }
    } else if (jsvIsString(var) && !jsvIsName(var)) {
      if ((flags&JSON_LIMIT) && jsvGetStringLength(var)>JSON_LIMIT_STRING_AMOUNT) {
        // if the string is too big, split it and put dots in the middle
        JsVar *var1 = jsvNewFromStringVar(var, 0, JSON_LIMITED_STRING_AMOUNT);
        JsVar *var2 = jsvNewFromStringVar(var, jsvGetStringLength(var)-JSON_LIMITED_STRING_AMOUNT, JSON_LIMITED_STRING_AMOUNT);
        cbprintf(user_callback, user_data, "%q%s%q", var1, JSON_LIMIT_TEXT, var2);
        jsvUnLock(var1);
        jsvUnLock(var2);
      } else {
        cbprintf(user_callback, user_data, "%q", var);
      }
    } else {
      cbprintf(user_callback, user_data, "%v", var);
    }

    var->flags &= ~JSV_IS_RECURSING;
  }
}

void jsfGetJSON(JsVar *var, JsVar *result, JSONFlags flags) {
  assert(jsvIsString(result));
  JsvStringIterator it;
  jsvStringIteratorNew(&it, result, 0);
  jsvStringIteratorGotoEnd(&it);

  jsfGetJSONWithCallback(var, flags, (vcbprintf_callback)&jsvStringIteratorPrintfCallback, &it);

  jsvStringIteratorFree(&it);
}

void jsfPrintJSON(JsVar *var, JSONFlags flags) {
  jsfGetJSONWithCallback(var, flags, (vcbprintf_callback)jsiConsolePrint, 0);
}
void jsfPrintJSONForFunction(JsVar *var, JSONFlags flags) {
  jsfGetJSONForFunctionWithCallback(var, flags, (vcbprintf_callback)jsiConsolePrint, 0);
}
