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
#include "jswrap_object.h"
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
    ["data","JsVar","The data to be converted to a JSON string"],
    ["replacer","JsVar","This value is ignored"],
    ["space","JsVar","The number of spaces to use for padding, a string, or null/undefined for no whitespace "]
  ],
  "return" : ["JsVar","A JSON string"]
}
Convert the given object into a JSON string which can subsequently be parsed
with JSON.parse or eval.

**Note:** This differs from JavaScript's standard `JSON.stringify` in that:

* The `replacer` argument is ignored
* Typed arrays like `new Uint8Array(5)` will be dumped as if they were arrays,
  not as if they were objects (since it is more compact)
 */
JsVar *jswrap_json_stringify(JsVar *v, JsVar *replacer, JsVar *space) {
  NOT_USED(replacer);
  JSONFlags flags = JSON_IGNORE_FUNCTIONS|JSON_NO_UNDEFINED|JSON_ARRAYBUFFER_AS_ARRAY|JSON_JSON_COMPATIBILE|JSON_ALLOW_TOJSON;
  JsVar *result = jsvNewFromEmptyString();
  if (result) {// could be out of memory
    char whitespace[11] = "";
    if (jsvIsUndefined(space) || jsvIsNull(space)) {
      // nothing
    } else if (jsvIsNumeric(space)) {
      int s = (int)jsvGetInteger(space);
      if (s<0) s=0;
      if (s>10) s=10;
      whitespace[s] = 0;
      while (s) whitespace[--s]=' ';
    } else {
      size_t l = jsvGetString(space, whitespace, sizeof(whitespace)-1);
      whitespace[l]=0; // add trailing 0
    }
    if (strlen(whitespace)) flags |= JSON_ALL_NEWLINES|JSON_PRETTY;
    jsfGetJSONWhitespace(v, result, flags, whitespace);
  }
  return result;
}


JsVar *jswrap_json_parse_internal() {
  switch (lex->tk) {
  case LEX_R_TRUE:  jslGetNextToken(); return jsvNewFromBool(true);
  case LEX_R_FALSE: jslGetNextToken(); return jsvNewFromBool(false);
  case LEX_R_NULL:  jslGetNextToken(); return jsvNewWithFlags(JSV_NULL);
  case '-': {
    jslGetNextToken();
    if (lex->tk!=LEX_INT && lex->tk!=LEX_FLOAT) return 0;
    JsVar *v = jswrap_json_parse_internal();
    JsVar *zero = jsvNewFromInteger(0);
    JsVar *r = jsvMathsOp(zero, v, '-');
    jsvUnLock2(v, zero);
    return r;
  }
  case LEX_INT: {
    long long v = stringToInt(jslGetTokenValueAsString());
    jslGetNextToken();
    return jsvNewFromLongInteger(v);
  }
  case LEX_FLOAT: {
    JsVarFloat v = stringToFloat(jslGetTokenValueAsString());
    jslGetNextToken();
    return jsvNewFromFloat(v);
  }
  case LEX_STR: {
    JsVar *a = jslGetTokenValueAsVar();
    jslGetNextToken();
    return a;
  }
  case '[': {
    JsVar *arr = jsvNewEmptyArray(); if (!arr) return 0;
    jslGetNextToken(); // [
    while (lex->tk != ']' && !jspHasError()) {
      JsVar *value = jswrap_json_parse_internal();
      if (!value ||
          (lex->tk!=']' && !jslMatch(','))) {
        jsvUnLock2(value, arr);
        return 0;
      }
      jsvArrayPush(arr, value);
      jsvUnLock(value);
    }
    if (!jslMatch(']')) {
      jsvUnLock(arr);
      return 0;
    }
    return arr;
  }
  case '{': {
    JsVar *obj = jsvNewObject(); if (!obj) return 0;
    jslGetNextToken(); // {
    while (lex->tk == LEX_STR && !jspHasError()) {
      JsVar *key = jsvAsArrayIndexAndUnLock(jslGetTokenValueAsVar());
      jslGetNextToken();
      JsVar *value = 0;
      if (!jslMatch(':') ||
          !(value=jswrap_json_parse_internal()) ||
          (lex->tk!='}' && !jslMatch(','))) {
        jsvUnLock3(key, value, obj);
        return 0;
      }
      jsvAddName(obj, jsvMakeIntoVariableName(key, value));
      jsvUnLock2(value, key);
    }
    if (!jslMatch('}')) {
      jsvUnLock(obj);
      return 0;
    }
    return obj;
  }
  default: {
    char buf[32];
    jslTokenAsString(lex->tk, buf, 32);
    jsExceptionHere(JSET_SYNTAXERROR, "Expecting a valid value, got %s", buf);
    return 0; // undefined = error
  }
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

NOTE: This implementation uses eval() internally, and as such it is unsafe as it
can allow arbitrary JS commands to be executed.
 */
JsVar *jswrap_json_parse(JsVar *v) {
  JsLex lex;
  JsVar *str = jsvAsString(v);
  JsLex *oldLex = jslSetLex(&lex);
  jslInit(str);
  jsvUnLock(str);
  JsVar *res = jswrap_json_parse_internal();
  jslKill();
  jslSetLex(oldLex);
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
      JsVar *name = jsvNewFromStringVar(child, 1, JSVAPPENDSTRINGVAR_MAXLENGTH);
      cbprintf(user_callback, user_data, "%v", name);
      jsvUnLock(name);
    } else if (jsvIsString(child) && jsvIsStringEqual(child, JSPARSE_FUNCTION_CODE_NAME)) {
      codeVar = jsvObjectIteratorGetValue(&it);
    }
    jsvUnLock(child);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  cbprintf(user_callback, user_data, ") ");
#ifdef ESPR_JIT
  JsVar *jitCode = 0;
#endif
  if (jsvIsNativeFunction(var)) {
    cbprintf(user_callback, user_data, "{ [native code] }");
#ifdef ESPR_JIT
  } else if (jitCode = jsvFindChildFromString(var, JSPARSE_FUNCTION_JIT_CODE_NAME, false)) {
    jsvUnLock(jitCode);
    cbprintf(user_callback, user_data, "{ [JIT] }");
#endif
  } else {
    if (codeVar) {
      if (flags & JSON_LIMIT) {
        cbprintf(user_callback, user_data, "{%s}", JSON_LIMIT_TEXT);
      } else {
        bool hasNewLine = jsvGetStringIndexOf(codeVar,'\n')>=0;
        user_callback(hasNewLine?"{\n  ":"{", user_data);
        if (jsvIsFunctionReturn(var))
          user_callback("return ", user_data);
        // reconstruct the tokenised output into something more readable
        jslPrintTokenisedString(codeVar, user_callback, user_data);
        user_callback(hasNewLine?"\n}":"}", user_data);
      }
    } else cbprintf(user_callback, user_data, "{}");
  }
  jsvUnLock(codeVar);
}

bool jsonNeedsNewLine(JsVar *v) {
  return !(jsvIsUndefined(v) || jsvIsNull(v) || jsvIsNumeric(v));
  // we're skipping strings here because they're usually long and want printing on multiple lines
}

void jsonNewLine(JSONFlags flags, const char *whitespace, vcbprintf_callback user_callback, void *user_data) {
  user_callback("\n", user_data);
  // apply the indent
  unsigned int indent = flags / JSON_INDENT;
  while (indent--)
    user_callback(whitespace, user_data);
}

static bool jsfGetJSONForObjectItWithCallback(JsvObjectIterator *it, JSONFlags flags, const char *whitespace, JSONFlags nflags, vcbprintf_callback user_callback, void *user_data, bool first) {
  bool needNewLine = false;
  size_t sinceNewLine = 0;
  while (jsvObjectIteratorHasValue(it) && !jspIsInterrupted()) {
    JsVar *index = jsvObjectIteratorGetKey(it);
    JsVar *item = jsvGetValueOfName(index);
    bool hidden = jsvIsInternalObjectKey(index) ||
        ((flags & JSON_IGNORE_FUNCTIONS) && jsvIsFunction(item)) ||
        ((flags&JSON_NO_UNDEFINED) && jsvIsUndefined(item)) ||
        jsvIsGetterOrSetter(item);
    if (!hidden) {
      sinceNewLine++;
      if (!first) cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?", ":",");
      bool newNeedsNewLine = (flags&JSON_SOME_NEWLINES) && jsonNeedsNewLine(item);
      if ((flags&JSON_SOME_NEWLINES) && sinceNewLine>JSON_ITEMS_ON_LINE_OBJECT)
        needNewLine = true;
      if (flags&JSON_ALL_NEWLINES) {
        needNewLine = true;
        newNeedsNewLine = true;
      }
      if (needNewLine || newNeedsNewLine) {
        jsonNewLine(nflags, whitespace, user_callback, user_data);
        needNewLine = false;
        sinceNewLine = 0;
      }
      bool addQuotes = true;
      if (flags&JSON_DROP_QUOTES) {
        if (jsvIsIntegerish(index)) addQuotes = false;
        else if (jsvIsString(index) && jsvGetStringLength(index)<63) {
          char buf[64];
          jsvGetString(index,buf,sizeof(buf));
          if (isIDString(buf)) addQuotes=false;
        }
      }
      cbprintf(user_callback, user_data, addQuotes?((flags&JSON_JSON_COMPATIBILE)?"%Q%s":"%q%s"):"%v%s", index, (flags&JSON_PRETTY)?": ":":");
      if (first)
        first = false;
      jsfGetJSONWithCallback(item, index, nflags, whitespace, user_callback, user_data);
      needNewLine = newNeedsNewLine;
    }
    jsvUnLock2(index, item);
    jsvObjectIteratorNext(it);
  }
  return needNewLine;
}

void jsfGetJSONWithCallback(JsVar *var, JsVar *varName, JSONFlags flags, const char *whitespace, vcbprintf_callback user_callback, void *user_data) {
  JSONFlags nflags = flags + JSON_INDENT; // if we add a newline, make sure we indent any subsequent JSON more
  if (!whitespace) whitespace="  ";

  if (jsvIsUndefined(var)) {
    cbprintf(user_callback, user_data, (flags&JSON_NO_UNDEFINED)?"null":"undefined");
    return;
  }
  // Use IS_RECURSING flag to stop recursion
  if ((var->flags & JSV_IS_RECURSING) || (jsuGetFreeStack() < 512) || jspIsInterrupted()) {
    // also check for stack overflow/interruption
    cbprintf(user_callback, user_data, " ... ");
    return;
  }
  var->flags |= JSV_IS_RECURSING;

  if (jsvIsArray(var)) {
    JsVarInt length = jsvGetArrayLength(var);
    bool limited = (flags&JSON_LIMIT) && (length>(JsVarInt)JSON_LIMIT_AMOUNT);
    bool needNewLine = false;
    cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?"[ ":"[");
    JsVarInt lastIndex = -1;
    bool numeric = true;
    bool first = true;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, var);
    while (lastIndex+1<length && numeric && !jspIsInterrupted()) {
      JsVar *key = jsvObjectIteratorGetKey(&it);
      if (!jsvObjectIteratorHasValue(&it) || jsvIsNumeric(key)) {
        JsVarInt index = jsvObjectIteratorHasValue(&it) ? jsvGetInteger(key) : length-1;
        JsVar *item = jsvObjectIteratorGetValue(&it);
        while (lastIndex < index) {
          lastIndex++;
          if (!limited || lastIndex<(JsVarInt)JSON_LIMITED_AMOUNT || lastIndex>=length-(JsVarInt)JSON_LIMITED_AMOUNT) {
            if (!first) cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?", ":",");
            first = false;
            if (limited && lastIndex==length-(JsVarInt)JSON_LIMITED_AMOUNT) cbprintf(user_callback, user_data, JSON_LIMIT_TEXT);
            bool newNeedsNewLine = ((flags&JSON_SOME_NEWLINES) && jsonNeedsNewLine(item));
            if (flags&JSON_ALL_NEWLINES) {
              needNewLine = true;
              newNeedsNewLine = true;
            }
            if (needNewLine || newNeedsNewLine) {
              jsonNewLine(nflags, whitespace, user_callback, user_data);
              needNewLine = false;
            }
            if (lastIndex == index) {
              JsVar *indexVar = jsvNewFromInteger(index);
              jsfGetJSONWithCallback(item, indexVar, nflags, whitespace, user_callback, user_data);
              jsvUnLock(indexVar);
            } else
              cbprintf(user_callback, user_data, (flags&JSON_NO_UNDEFINED)?"null":"undefined");
            needNewLine = newNeedsNewLine;
          }
        }
        jsvUnLock(item);
        jsvObjectIteratorNext(&it);
      } else {
        numeric = false;
      }
      jsvUnLock(key);
    }

    // non-numeric  - but NOT for standard JSON
    if ((flags&JSON_PRETTY))
      jsfGetJSONForObjectItWithCallback(&it, flags, whitespace, nflags, user_callback, user_data, first);
    jsvObjectIteratorFree(&it);
    if (needNewLine) jsonNewLine(flags, whitespace, user_callback, user_data);
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
    bool asArray = flags&JSON_ARRAYBUFFER_AS_ARRAY;

    if (allZero && !asArray) {
      cbprintf(user_callback, user_data, "new %s(%d)", jswGetBasicObjectName(var), jsvGetArrayBufferLength(var));
    } else {
      const char *aname = jswGetBasicObjectName(var);
      /* You can't do `new ArrayBuffer([1,2,3])` so we have to output
       * `new Uint8Array([1,2,3]).buffer`! */
      bool isBasicArrayBuffer = strcmp(aname,"ArrayBuffer")==0;
      if (isBasicArrayBuffer) {
        aname="Uint8Array";
      }
      cbprintf(user_callback, user_data, asArray?"[":"new %s([", aname);
      if (flags&JSON_ALL_NEWLINES) jsonNewLine(nflags, whitespace, user_callback, user_data);
      size_t length = jsvGetArrayBufferLength(var);
      bool limited = (flags&JSON_LIMIT) && (length>JSON_LIMIT_AMOUNT);
      // no newlines needed for array buffers as they only contain simple stuff

      jsvArrayBufferIteratorNew(&it, var, 0);
      while (jsvArrayBufferIteratorHasElement(&it) && !jspIsInterrupted()) {
        if (!limited || it.index<JSON_LIMITED_AMOUNT || it.index>=length-JSON_LIMITED_AMOUNT) {
          if (it.index>0) cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?", ":",");
          if (flags&JSON_ALL_NEWLINES) jsonNewLine(nflags, whitespace, user_callback, user_data);
          if (limited && it.index==length-JSON_LIMITED_AMOUNT) cbprintf(user_callback, user_data, JSON_LIMIT_TEXT);
          JsVar *item = jsvArrayBufferIteratorGetValue(&it);
          jsfGetJSONWithCallback(item, NULL, nflags, whitespace, user_callback, user_data);
          jsvUnLock(item);
        }
        jsvArrayBufferIteratorNext(&it);
      }
      if (flags&JSON_ALL_NEWLINES) jsonNewLine(flags, whitespace, user_callback, user_data);
      jsvArrayBufferIteratorFree(&it);
      cbprintf(user_callback, user_data, asArray?"]":"])");
      if (isBasicArrayBuffer && !asArray) cbprintf(user_callback, user_data, ".buffer");
    }
  } else if (jsvIsObject(var)) {
    IOEventFlags device = (flags & JSON_SHOW_DEVICES) ? jsiGetDeviceFromClass(var) : EV_NONE;
    if (device!=EV_NONE) {
      cbprintf(user_callback, user_data, "%s", jshGetDeviceString(device));
    } else {
      bool showContents = true;
      if (flags & JSON_SHOW_OBJECT_NAMES) {
        JsVar *proto = jsvObjectGetChild(var, JSPARSE_INHERITS_VAR, 0);
        if (jsvHasChildren(proto)) {
          JsVar *constr = jsvObjectGetChild(proto, JSPARSE_CONSTRUCTOR_VAR, 0);
          if (constr) {
            JsVar *p = jsvGetIndexOf(execInfo.root, constr, true);
            if (p) cbprintf(user_callback, user_data, "%v: ", p);
            jsvUnLock2(p,constr);
            /* We had the constructor - now if there was a non-default toString function
             * we'll execute it and print the result */
            JsVar *toStringFn = jspGetNamedField(var, "toString", false);
            if (jsvIsFunction(toStringFn) && toStringFn->varData.native.ptr != (void (*)(void))jswrap_object_toString) {
              // Function found and it's not the default one - execute it
              JsVar *result = jspExecuteFunction(toStringFn,var,0,0);
              cbprintf(user_callback, user_data, "%v", result);
              jsvUnLock(result);
              showContents = false; // we already printed something
            }
            jsvUnLock(toStringFn);
          }
        }
        jsvUnLock(proto);
      }
      if (showContents) {
        JsVar *toStringFn = 0;
        if (flags & JSON_ALLOW_TOJSON)
          toStringFn = jspGetNamedField(var, "toJSON", false);
        if (jsvIsFunction(toStringFn)) {
          JsVar *varNameStr = varName ? jsvAsString(varName) : 0;
          JsVar *result = jspExecuteFunction(toStringFn,var,1,&varNameStr);
          jsvUnLock(varNameStr);
          if (result==var) var->flags &= ~JSV_IS_RECURSING;
          jsfGetJSONWithCallback(result, NULL, flags&~JSON_ALLOW_TOJSON, whitespace, user_callback, user_data);
          jsvUnLock(result);
        } else {
          JsvObjectIterator it;
          jsvObjectIteratorNew(&it, var);
          cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?"{ ":"{");
          bool needNewLine = jsfGetJSONForObjectItWithCallback(&it, flags, whitespace, nflags, user_callback, user_data, true);
          jsvObjectIteratorFree(&it);
          if (needNewLine) jsonNewLine(flags, whitespace, user_callback, user_data);
          cbprintf(user_callback, user_data, (flags&JSON_PRETTY)?" }":"}");
        }
        jsvUnLock(toStringFn);
      }
    }
  } else if (jsvIsFunction(var)) {
    if (flags & JSON_IGNORE_FUNCTIONS) {
      cbprintf(user_callback, user_data, "undefined");
    } else {
      cbprintf(user_callback, user_data, "function ");
      jsfGetJSONForFunctionWithCallback(var, nflags, user_callback, user_data);
    }
  } else if ((jsvIsString(var) && !jsvIsName(var)) || ((flags&JSON_JSON_COMPATIBILE)&&jsvIsPin(var))) {
    if ((flags&JSON_LIMIT) && jsvGetStringLength(var)>JSON_LIMIT_STRING_AMOUNT) {
      // if the string is too big, split it and put dots in the middle
      JsVar *var1 = jsvNewFromStringVar(var, 0, JSON_LIMITED_STRING_AMOUNT);
      JsVar *var2 = jsvNewFromStringVar(var, jsvGetStringLength(var)-JSON_LIMITED_STRING_AMOUNT, JSON_LIMITED_STRING_AMOUNT);
      cbprintf(user_callback, user_data, "%q%s%q", var1, JSON_LIMIT_TEXT, var2);
      jsvUnLock2(var1, var2);
    } else {
      cbprintf(user_callback, user_data, (flags&JSON_JSON_COMPATIBILE)?"%Q":"%q", var);
    }
  } else if ((flags&JSON_JSON_COMPATIBILE) && jsvIsFloat(var) && !isfinite(jsvGetFloat(var))) {
    cbprintf(user_callback, user_data, "null");
  } else {
    cbprintf(user_callback, user_data, "%v", var);
  }

  var->flags &= ~JSV_IS_RECURSING;
}

void jsfGetJSONWhitespace(JsVar *var, JsVar *result, JSONFlags flags, const char *whitespace) {
  assert(jsvIsString(result));
  JsvStringIterator it;
  jsvStringIteratorNew(&it, result, 0);
  jsvStringIteratorGotoEnd(&it);

  jsfGetJSONWithCallback(var, NULL, flags, whitespace, (vcbprintf_callback)&jsvStringIteratorPrintfCallback, &it);

  jsvStringIteratorFree(&it);
}

void jsfGetJSON(JsVar *var, JsVar *result, JSONFlags flags) {
  jsfGetJSONWhitespace(var, result, flags, 0);
}

void jsfPrintJSON(JsVar *var, JSONFlags flags) {
  jsfGetJSONWithCallback(var, NULL, flags, 0, (vcbprintf_callback)jsiConsolePrintString, 0);
}
void jsfPrintJSONForFunction(JsVar *var, JSONFlags flags) {
  jsfGetJSONForFunctionWithCallback(var, flags, (vcbprintf_callback)jsiConsolePrintString, 0);
}
