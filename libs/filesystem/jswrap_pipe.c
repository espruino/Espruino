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
 *
 * Piping works as follows:
 *   On idle, data is read from the source stream
 *    * If this returns undefined, the stream is considered finished and is closed
 *    * If this returns "" we just assume that it's waiting for more data
 *    * If it returns some data, 'write' is called with it
 *    * And if 'write' returns the boolean false then we stall the pipe until
 *       the destination emits a 'drain' signal
 *    * If the destination emits a 'close' signal we close the pipe
 *    * When the pipe closes, unless 'end=false' on initialisation, we call
 *      'end' on destination, and 'close' on source.
 *
 * ----------------------------------------------------------------------------
 */

#include "jswrap_pipe.h"
#include "jswrap_object.h"

/*JSON{ "type":"library", "ifndef" : "SAVE_ON_FLASH",
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

static void handlePipeClose(JsVar *arr, JsvArrayIterator *it, JsVar* pipe) {
  jsiQueueObjectCallbacks(pipe, "#oncomplete", &pipe, 1);
  // also call 'end' if 'end' was passed as an initialisation option
  if (jsvGetBoolAndUnLock(jsvObjectGetChild(pipe,"end",0))) {
    // call destination.end if available
    JsVar *destination = jsvObjectGetChild(pipe,"destination",0);
    // TODO: we should probably remove our drain+close listeners
    JsVar *endFunc = jspGetNamedField(destination, "end", false);
    if (endFunc) {
      jsvUnLock(jspExecuteFunction(endFunc, destination, 0, 0));
      jsvUnLock(endFunc);
    }
    jsvUnLock(destination);
    /* call source.close if available - probably not what node does
    but seems very sensible in this case. If you don't want it,
    set end:false */
    JsVar *source = jsvObjectGetChild(pipe,"source",0);
    JsVar *closeFunc = jspGetNamedField(source, "close", false);
    if (closeFunc) {
      jsvUnLock(jspExecuteFunction(closeFunc, source, 0, 0));
      jsvUnLock(closeFunc);
    }
    jsvUnLock(source);
  }
  JsVar *idx = jsvArrayIteratorGetIndex(it);
  jsvRemoveChild(arr,idx);
  jsvUnLock(idx);
}

static bool handlePipe(JsVar *arr, JsvArrayIterator *it, JsVar* pipe) {
  bool paused = jsvGetBoolAndUnLock(jsvObjectGetChild(pipe,"drainWait",0));
  if (paused) return false;

  JsVar *position = jsvObjectGetChild(pipe,"position",0);
  JsVar *chunkSize = jsvObjectGetChild(pipe,"chunkSize",0);
  JsVar *source = jsvObjectGetChild(pipe,"source",0);
  JsVar *destination = jsvObjectGetChild(pipe,"destination",0);

  bool dataTransferred = false;
  if(source && destination && chunkSize && position) {
    JsVar *readFunc = jspGetNamedField(source, "read", false);
    JsVar *writeFunc = jspGetNamedField(destination, "write", false);
    if (jsvIsFunction(readFunc) && jsvIsFunction(writeFunc)) { // do the objects have the necessary methods on them?
      JsVar *buffer = jspExecuteFunction(readFunc, source, 1, &chunkSize);
      if(buffer) {
        JsVarInt bufferSize = jsvGetLength(buffer);
        if (bufferSize>0) {
          JsVar *response = jspExecuteFunction(writeFunc, destination, 1, &buffer);
          if (jsvIsBoolean(response) && jsvGetBool(response)==false) {
            // If boolean false was returned, wait for drain event (http://nodejs.org/api/stream.html#stream_writable_write_chunk_encoding_callback)
            jsvUnLock(jsvObjectSetChild(pipe,"drainWait",jsvNewFromBool(true)));
          }
          jsvUnLock(response);
          jsvSetInteger(position, jsvGetInteger(position) + bufferSize);
        }
        jsvUnLock(buffer);
        dataTransferred = true; // so we don't close the pipe if we get an empty string
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

  if(!dataTransferred) { // when no more chunks are possible, execute the callback
    handlePipeClose(arr, it, pipe);
  }
  jsvUnLock(source);
  jsvUnLock(destination);
  jsvUnLock(chunkSize);
  jsvUnLock(position);
  return dataTransferred;
}

/*JSON{ "type":"idle", "generate" : "jswrap_pipe_idle", "ifndef" : "SAVE_ON_FLASH" }*/
bool jswrap_pipe_idle() {
  bool wasBusy = false;
  JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes",false);
  if (arr) {
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, arr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *pipe = jsvArrayIteratorGetElement(&it);
      wasBusy |= handlePipe(arr, &it, pipe);
      jsvUnLock(pipe);
      jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    jsvUnLock(arr);
  }
  return wasBusy;
}

/*JSON{ "type":"kill", "generate" : "jswrap_pipe_kill", "ifndef" : "SAVE_ON_FLASH" }*/
void jswrap_pipe_kill() {
  // now remove all pipes...
  JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes", false);
  if (arr) {
    jsvRemoveAllChildren(arr);
    jsvUnLock(arr);
  }
}

/** This gets called when a pipe destination drains itself */
static void jswrap_pipe_drain_listener(JsVar *destination) {
  if (!jsvIsObject(destination)) return;
  // try and find it...
  JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes",false);
  if (arr) {
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, arr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *pipe = jsvArrayIteratorGetElement(&it);
      JsVar *dst = jsvObjectGetChild(pipe,"destination",0);
      if (dst == destination) {
        // found it! said wait to false
        jsvUnLock(jsvObjectSetChild(pipe,"drainWait",jsvNewFromBool(false)));
      }
      jsvUnLock(dst);
      jsvUnLock(pipe);
      jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    jsvUnLock(arr);
  }
}

/** This gets called when the destination closes and we need to clean up */
static void jswrap_pipe_close_listener(JsVar *destination) {
  if (!jsvIsObject(destination)) return;
  // try and find it...
  JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes",false);
  if (arr) {
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, arr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *pipe = jsvArrayIteratorGetElement(&it);
      JsVar *dst = jsvObjectGetChild(pipe,"destination",0);
      if (dst == destination) {
        // found it! said wait to false
        handlePipeClose(arr, &it, pipe);
      }
      jsvUnLock(dst);
      jsvUnLock(pipe);
      jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    jsvUnLock(arr);
  }
}

/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "pipe", "ifndef" : "SAVE_ON_FLASH",
         "generate" : "jswrap_pipe",
         "params" : [ ["source", "JsVar", "The source file/stream that will send content."],
                      ["destination", "JsVar", "The destination file/stream that will receive content from the source."],
                      ["options", "JsVar", [ "An optional object `{ chunkSize : int=64, end : bool=true, complete : function }`",
                                             "chunkSize : The amount of data to pipe from source to destination at a time",
                                             "complete : a function to call when the pipe activity is complete",
                                             "end : call the 'end' function on the destination when the source is finished"] ] ]
}*/
void jswrap_pipe(JsVar* source, JsVar* dest, JsVar* options) {
  if (!source || !dest) return;
  JsVar *pipe = jspNewObject(0, "Pipe");
  JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes", true);
  JsVar* position = jsvNewFromInteger(0);
  if (pipe && arr && position) {// out of memory?
    JsVar *readFunc = jspGetNamedField(source, "read", false);
    JsVar *writeFunc = jspGetNamedField(dest, "write", false);
    if(jsvIsFunction(readFunc)) {
      if(jsvIsFunction(writeFunc)) {
        JsVarInt chunkSize = 64;
        bool callEnd = true;
        // parse Options Object
        if (jsvIsObject(options)) {
          JsVar *c;
          c = jsvObjectGetChild(options, "complete", false);
          if (c) {
            jsvObjectSetChild(pipe, "#oncomplete", c);
            jsvUnLock(c);
          }
          c = jsvObjectGetChild(options, "end", false);
          if (c) callEnd = jsvGetBoolAndUnLock(c);
          c = jsvObjectGetChild(options, "chunkSize", false);
          if (c) {
            if (jsvIsNumeric(c) && jsvGetInteger(c)>0)
              chunkSize = jsvGetInteger(c);
            else
              jsWarn("chunkSize must be an integer > 0");
            jsvUnLock(c);
          }
        } else if (!jsvIsUndefined(options)) {
          jsWarn("'options' must be an object, or undefined");
        }
        // set up our drain and close event listeners
        jswrap_object_addEventListener(dest, "drain", jswrap_pipe_drain_listener, JSWAT_VOID | (JSWAT_JSVAR << (JSWAT_BITS*1)));
        jswrap_object_addEventListener(dest, "close", jswrap_pipe_close_listener, JSWAT_VOID | (JSWAT_JSVAR << (JSWAT_BITS*1)));
        // set up the rest of the pipe
        jsvUnLock(jsvObjectSetChild(pipe, "chunkSize", jsvNewFromInteger(chunkSize)));
        jsvUnLock(jsvObjectSetChild(pipe, "end", jsvNewFromBool(callEnd)));
        jsvUnLock(jsvAddNamedChild(pipe, position, "position"));
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
  jsvUnLock(position);
}

