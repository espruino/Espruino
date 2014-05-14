/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2014 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Handling of the 'pipe' function - to pipe one stream to another
 * ----------------------------------------------------------------------------
 */

#include "jswrap_pipe.h"

/*JSON{ "type":"library",
        "class" : "Pipe",
        "description" : ["This is the Pipe container for async related IO." ]
}*/

JsVar* PipeGetArray(const char *name, bool create) {
  JsVar *arrayName = jsvFindChildFromString(execInfo.root, name, create);
  JsVar *arr = jsvSkipName(arrayName);
  if (!arr && create) {
    arr = jsvNewWithFlags(JSV_ARRAY);
    jsvSetValueOfName(arrayName, arr);
  }
  jsvUnLock(arrayName);
  return arr;
}

static bool _pipe(JsVar* source, JsVar* destination, JsVar* chunkSize, JsVar* position) {
  bool dataTransferred = false;
  if(source && destination && chunkSize && position) {
    JsVar *readFunc = jspGetNamedField(source, "read");
    JsVar *writeFunc = jspGetNamedField(destination, "write");
    if (jsvIsFunction(readFunc) && jsvIsFunction(writeFunc)) { // do the objects have the necessary methods on them?
      JsVar *buffer = jspExecuteFunction(readFunc, source, 1, &chunkSize);
      if(buffer) {
        JsVarInt bufferSize = jsvGetLength(buffer);
        if (bufferSize>0) {
          jsvUnLock(jspExecuteFunction(writeFunc, destination, 1, &buffer));
          jsvSetInteger(position, jsvGetInteger(position) + bufferSize);
          dataTransferred = true;
        }
        jsvUnLock(buffer);
      }
    } else {
      if(!jsvIsFunction(readFunc))
        jsError("Source Stream does not implement the required read(length) method.");
      if(!jsvIsFunction(writeFunc))
        jsError("Destination Stream does not implement the required write(buffer) method.");
    }
    jsvUnLock(readFunc);
    jsvUnLock(writeFunc);
  }
  return dataTransferred;
}

/*JSON{ "type":"idle", "generate" : "jswrap_pipe_idle" }*/
bool jswrap_pipe_idle() {
  bool ret = false;
  JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes",false);
  if (arr) {
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, arr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *pipe = jsvArrayIteratorGetElement(&it);
      JsVar *position = jsvObjectGetChild(pipe,"position",0);
      JsVar *chunkSize = jsvObjectGetChild(pipe,"chunkSize",0);
      JsVar *source = jsvObjectGetChild(pipe,"source",0);
      JsVar *destination = jsvObjectGetChild(pipe,"destination",0);
      if(!_pipe(source, destination, chunkSize, position)) { // when no more chunks are possible, execute the callback.
        jsiQueueObjectCallbacks(pipe, "#oncomplete", &pipe, 1);
        JsVar *idx = jsvArrayIteratorGetIndex(&it);
        jsvRemoveChild(arr,idx);
        jsvUnLock(idx);
      } else {
        ret = true;
      }
      jsvUnLock(source);
      jsvUnLock(destination);
      jsvUnLock(pipe);
      jsvUnLock(chunkSize);
      jsvUnLock(position);
      jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    jsvUnLock(arr);
  }
  return ret;
}

/*JSON{ "type":"kill", "generate" : "jswrap_pipe_kill" }*/
void jswrap_pipe_kill() {
  // now remove all pipes...
  {
    JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes", false);
    if (arr) {
      jsvRemoveAllChildren(arr);
      jsvUnLock(arr);
    }
  }
}

/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "pipe",
         "generate" : "jswrap_pipe",
         "params" : [ ["source", "JsVar", "The source file/stream that will send content."],
                      ["destination", "JsVar", "The destination file/stream that will receive content from the source."],
                      ["options", "JsVar", [ "An optional object `{ chunkSize : int=32, complete : function }`",
                                             "chunkSize : The amount of data to pipe from source to destination at a time",
                                             "complete : a function to call when the pipe activity is complete"] ] ]
}*/
void jswrap_pipe(JsVar* source, JsVar* dest, JsVar* options) {
  if (!source || !dest) return;
  JsVar *pipe = jspNewObject(0, "Pipe");
  JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes", true);
  JsVar* chunkSize = jsvNewFromInteger(32);
  JsVar* position = jsvNewFromInteger(0);
  if (pipe && arr && chunkSize && position) {// out of memory?
    JsVar *readFunc = jspGetNamedField(source, "read");
    JsVar *writeFunc = jspGetNamedField(dest, "write");
    if(jsvIsFunction(readFunc)) {
      if(jsvIsFunction(writeFunc)) {
        // parse Options Object
        if (jsvIsObject(options)) {
          JsVar *c;
          c = jsvObjectGetChild(options, "complete", false);
          if(c) {
            jsvAddNamedChild(pipe, c, "#oncomplete");
            jsvUnLock(c);
          }
          c = jsvObjectGetChild(options, "chunkSize", false);
          if(c) {
            if (jsvIsNumeric(c) && jsvGetInteger(c)>0)
              jsvSetInteger(chunkSize, jsvGetInteger(c));
            else
              jsWarn("chunkSize must be an integer > 0");
          }
        } else if (!jsvIsUndefined(options)) {
          jsWarn("'options' must be an object, or undefined");
        }
        // set up the rest of the pipe
        jsvUnLock(jsvAddNamedChild(pipe, position, "position"));
        jsvUnLock(jsvAddNamedChild(pipe, chunkSize, "chunkSize"));
        jsvUnLock(jsvAddNamedChild(pipe, source, "source"));
        jsvUnLock(jsvAddNamedChild(pipe, dest, "destination"));
        // add the pipe to our list
        jsvArrayPush(arr, pipe);
      } else {
        jsError("Destination object does not implement the required write(buffer, length, position) method.");
      }
    } else {
      jsError("Source object does not implement the required read(buffer, length, position) method.");
    }
    jsvUnLock(readFunc);
    jsvUnLock(writeFunc);
  }
  jsvUnLock(arr);
  jsvUnLock(pipe);
  jsvUnLock(chunkSize);
  jsvUnLock(position);
}

