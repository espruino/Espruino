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
#include "jswrap_stream.h"

static JsVar* pipeGetArray(bool create) {
  return jsvObjectGetChild(execInfo.hiddenRoot, "pipes", create ? JSV_ARRAY : 0);
}


static void handlePipeClose(JsVar *arr, JsvObjectIterator *it, JsVar* pipe) {
  jsiQueueObjectCallbacks(pipe, JS_EVENT_PREFIX"complete", &pipe, 1);
  // Check the source to see if there was more data... It may not be a stream,
  // but if it is and it has data it should have a a STREAM_BUFFER_NAME field
  JsVar *source = jsvObjectGetChild(pipe,"source",0);
  JsVar *destination = jsvObjectGetChild(pipe,"destination",0);
  if (source && destination) {
    JsVar *buffer = jsvObjectGetChild(source, STREAM_BUFFER_NAME, 0);
    if (buffer && jsvGetStringLength(buffer)) {
      jsvObjectRemoveChild(source, STREAM_BUFFER_NAME); // remove outstanding data
      /* call write fn - we ignore drain/etc here because the source has
      just closed and we want to get this sorted quickly */
      JsVar *writeFunc = jspGetNamedField(destination, "write", false);
      if (jsvIsFunction(writeFunc)) { // do the objects have the necessary methods on them?
        jsvUnLock(jspExecuteFunction(writeFunc, destination, 1, &buffer));
      }
      jsvUnLock(writeFunc);
      // update position
      JsVar *position = jsvObjectGetChild(pipe,"position",0);
      jsvSetInteger(position, jsvGetInteger(position) + (JsVarInt)jsvGetStringLength(buffer));
      jsvUnLock(position);
    }
    jsvUnLock(buffer);
  }
  // also call 'end' if 'end' was passed as an initialisation option
  if (jsvGetBoolAndUnLock(jsvObjectGetChild(pipe,"end",0))) {
    // call destination.end if available
    if (destination) {
      // remove our drain and close listeners.
      // TODO: This removes ALL listeners. Maybe we should just remove ours?
      jswrap_object_removeAllListeners_cstr(destination, "drain");
      jswrap_object_removeAllListeners_cstr(destination, "close");
      // execute the 'end' function
      JsVar *endFunc = jspGetNamedField(destination, "end", false);
      if (endFunc) {
        jsvUnLock2(jspExecuteFunction(endFunc, destination, 0, 0), endFunc);
      }
      // execute the 'close' function
      JsVar *closeFunc = jspGetNamedField(destination, "close", false);
      if (closeFunc) {
        jsvUnLock2(jspExecuteFunction(closeFunc, destination, 0, 0), closeFunc);
      }
    }
    /* call source.close if available - probably not what node does
    but seems very sensible in this case. If you don't want it,
    set end:false */
    if (source) {
      // TODO: This removes ALL listeners. Maybe we should just remove ours?
      jswrap_object_removeAllListeners_cstr(source, "close");
      // execute the 'close' function
      JsVar *closeFunc = jspGetNamedField(source, "close", false);
      if (closeFunc) {
        jsvUnLock2(jspExecuteFunction(closeFunc, source, 0, 0), closeFunc);
      }
    }
  }
  jsvUnLock2(source, destination);
  JsVar *idx = jsvObjectIteratorGetKey(it);
  jsvRemoveChild(arr,idx);
  jsvUnLock(idx);
}

static bool handlePipe(JsVar *arr, JsvObjectIterator *it, JsVar* pipe) {
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
            jsvObjectSetChildAndUnLock(pipe,"drainWait",jsvNewFromBool(true));
          }
          jsvUnLock(response);
          jsvSetInteger(position, jsvGetInteger(position) + bufferSize);
        }
        jsvUnLock(buffer);
        dataTransferred = true; // so we don't close the pipe if we get an empty string
      }
    } else {
      if(!jsvIsFunction(readFunc))
        jsExceptionHere(JSET_ERROR, "Source Stream does not implement the required read(length) method.");
      if(!jsvIsFunction(writeFunc))
        jsExceptionHere(JSET_ERROR, "Destination Stream does not implement the required write(buffer) method.");
    }
    jsvUnLock2(readFunc, writeFunc);
  }

  if(!dataTransferred) { // when no more chunks are possible, execute the callback
    handlePipeClose(arr, it, pipe);
  }
  jsvUnLock3(source, destination, chunkSize);
  jsvUnLock(position);
  return dataTransferred;
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_pipe_idle",
  "ifndef" : "SAVE_ON_FLASH"
}*/
bool jswrap_pipe_idle() {
  bool wasBusy = false;
  JsVar *arr = pipeGetArray(false);
  if (arr) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, arr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *pipe = jsvObjectIteratorGetValue(&it);
      wasBusy |= handlePipe(arr, &it, pipe);
      jsvUnLock(pipe);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    jsvUnLock(arr);
  }
  return wasBusy;
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_pipe_kill",
  "ifndef" : "SAVE_ON_FLASH"
}*/
void jswrap_pipe_kill() {
  // now remove all pipes...
  JsVar *arr = pipeGetArray(false);
  if (arr) {
    jsvRemoveAllChildren(arr);
    jsvUnLock(arr);
  }
}

/** This gets called when a pipe destination drains itself */
static void jswrap_pipe_drain_listener(JsVar *destination) {
  if (!jsvIsObject(destination)) return;
  // try and find it...
  JsVar *arr = pipeGetArray(false);
  if (arr) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, arr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *pipe = jsvObjectIteratorGetValue(&it);
      JsVar *dst = jsvObjectGetChild(pipe,"destination",0);
      if (dst == destination) {
        // found it! said wait to false
        jsvObjectSetChildAndUnLock(pipe,"drainWait",jsvNewFromBool(false));
      }
      jsvUnLock2(dst, pipe);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    jsvUnLock(arr);
  }
}

/** This gets called when the destination closes and we need to clean up */
static void jswrap_pipe_close_listener(JsVar *destination, const char *name) {
  if (!jsvIsObject(destination)) return;
  // try and find it...
  JsVar *arr = pipeGetArray(false);
  if (arr) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, arr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *pipe = jsvObjectIteratorGetValue(&it);
      JsVar *dst = jsvObjectGetChild(pipe,name,0);
      if (dst == destination) {
        // found it! said wait to false
        handlePipeClose(arr, &it, pipe);
      }
      jsvUnLock2(dst, pipe);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    jsvUnLock(arr);
  }
}

static void jswrap_pipe_src_close_listener(JsVar *source) {
  jswrap_pipe_close_listener(source, "source");
}
static void jswrap_pipe_dst_close_listener(JsVar *destination) {
  jswrap_pipe_close_listener(destination, "destination");
}

/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "pipe",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_pipe",
  "params" : [
    ["source","JsVar","The source file/stream that will send content."],
    ["destination","JsVar","The destination file/stream that will receive content from the source."],
    ["options","JsVar",["An optional object `{ chunkSize : int=64, end : bool=true, complete : function }`","chunkSize : The amount of data to pipe from source to destination at a time","complete : a function to call when the pipe activity is complete","end : call the 'end' function on the destination when the source is finished"]]
  ]
}*/
void jswrap_pipe(JsVar* source, JsVar* dest, JsVar* options) {
  if (!source || !dest) return;
  JsVar *pipe = jspNewObject(0, "Pipe");
  JsVar *arr = pipeGetArray(true);
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
            jsvObjectSetChild(pipe, JS_EVENT_PREFIX"complete", c);
            jsvUnLock(c);
          }
          c = jsvObjectGetChild(options, "end", false);
          if (c) callEnd = jsvGetBoolAndUnLock(c);
          c = jsvObjectGetChild(options, "chunkSize", false);
          if (c) {
            if (jsvIsNumeric(c) && jsvGetInteger(c)>0)
              chunkSize = jsvGetInteger(c);
            else
              jsExceptionHere(JSET_TYPEERROR, "chunkSize must be an integer > 0");
            jsvUnLock(c);
          }
        } else if (!jsvIsUndefined(options)) {
          jsExceptionHere(JSET_TYPEERROR, "'options' must be an object, or undefined");
        }
        // set up our event listeners
        jswrap_object_addEventListener(source, "close", jswrap_pipe_src_close_listener, JSWAT_THIS_ARG);
        jswrap_object_addEventListener(dest, "drain", jswrap_pipe_drain_listener, JSWAT_VOID | (JSWAT_JSVAR << (JSWAT_BITS*1)));
        jswrap_object_addEventListener(dest, "close", jswrap_pipe_dst_close_listener, JSWAT_THIS_ARG);
        // set up the rest of the pipe
        jsvObjectSetChildAndUnLock(pipe, "chunkSize", jsvNewFromInteger(chunkSize));
        jsvObjectSetChildAndUnLock(pipe, "end", jsvNewFromBool(callEnd));
        jsvUnLock3(jsvAddNamedChild(pipe, position, "position"), 
                   jsvAddNamedChild(pipe, source, "source"), 
                   jsvAddNamedChild(pipe, dest, "destination"));
        // add the pipe to our list
        jsvArrayPush(arr, pipe);
      } else {
        jsExceptionHere(JSET_ERROR, "Destination object does not implement the required write(buffer, length, position) method.");
      }
    } else {
      jsExceptionHere(JSET_ERROR, "Source object does not implement the required read(buffer, length, position) method.");
    }
    jsvUnLock2(readFunc, writeFunc);
  }
  jsvUnLock3(arr, pipe, position);
}

