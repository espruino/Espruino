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
 * JavaScript Stream Functions. These allow us to implement pipeable
 * streams with minimal overhead
 *
 * Currently nothing is exported, as individual classes have their
 * own wrappers that directly call read/available/etc.
 * ----------------------------------------------------------------------------
 */
#include "jswrap_stream.h"
#include "jsinteractive.h"

// force this because we don't currently export anything
/*JSON{
  "type" : "include",
  "include" : "jswrap_stream.c"
}*/

// Return how many bytes are available to read
JsVarInt jswrap_stream_available(JsVar *parent) {
  if (!jsvIsObject(parent)) return 0;
  JsVar *buf = jsvObjectGetChild(parent, STREAM_BUFFER_NAME, 0);
  JsVarInt chars = 0;
  if (jsvIsString(buf))
    chars = (JsVarInt)jsvGetStringLength(buf);
  jsvUnLock(buf);
  return chars;
}

// Return a string containing 'chars' bytes. If chars<=0 the string will be all available data
JsVar *jswrap_stream_read(JsVar *parent, JsVarInt chars) {
  if (!jsvIsObject(parent)) return 0;
  JsVar *buf = jsvObjectGetChild(parent, STREAM_BUFFER_NAME, 0);
  JsVar *data = 0;
  if (jsvIsString(buf)) {
    size_t len = jsvGetStringLength(buf);
    if (chars <= 0 || (size_t)chars>=len) {
      // return the whole buffer and remove it
      data = buf;
      buf = 0;
      jsvRemoveNamedChild(parent, STREAM_BUFFER_NAME);
    } else {
      // return just part of the buffer, and shorten it accordingly
      data = jsvNewFromStringVar(buf, 0, (size_t)chars);
      JsVar *newBuf = jsvNewFromStringVar(buf, (size_t)chars, JSVAPPENDSTRINGVAR_MAXLENGTH);
      jsvUnLock(jsvObjectSetChild(parent, STREAM_BUFFER_NAME, newBuf));
    }
  } else
    data = jsvNewFromEmptyString();
  jsvUnLock(buf);
  return data;
}

/** Push data into a stream. To be used by Espruino (not a user).
 * This either calls the on('data') handler if it exists, or it
 * puts the data in a buffer. This MAY CLAIM the string that is
 * passed in.
 *
 * This will return true on success, or false if the buffer is
 * full. Setting force=true will attempt to fill the buffer as
 * full as possible, and will raise an error flag if data is lost.
 */
bool jswrap_stream_pushData(JsVar *parent, JsVar *dataString, bool force) {
  assert(jsvIsObject(parent));
  assert(jsvIsString(dataString));
  bool ok = true;

  JsVar *callback = jsvFindChildFromString(parent, STREAM_CALLBACK_NAME, false);
  if (callback) {
    if (!jsiExecuteEventCallback(parent, callback, 1, &dataString)) {
      jsError("Error processing Serial data handler - removing it.");
      jsErrorFlags |= JSERR_CALLBACK;
      jsvRemoveNamedChild(parent, STREAM_CALLBACK_NAME);
    }
    jsvUnLock(callback);
  } else {
    // No callback - try and add buffer
    JsVar *buf = jsvObjectGetChild(parent, STREAM_BUFFER_NAME, 0);
    if (!jsvIsString(buf)) {
      // no buffer, just set this one up
      jsvObjectSetChild(parent, STREAM_BUFFER_NAME, dataString);
    } else {
      // append (if there is room!)
      size_t bufLen = jsvGetStringLength(buf);
      size_t dataLen = jsvGetStringLength(dataString);
      if (bufLen + dataLen > STREAM_MAX_BUFFER_SIZE) {
        if (force) jsErrorFlags |= JSERR_BUFFER_FULL;
        // jsWarn("String buffer overflowed maximum size (%d)", STREAM_MAX_BUFFER_SIZE);
        ok = false;
      }
      if ((ok || force) && (bufLen < STREAM_MAX_BUFFER_SIZE))
        jsvAppendStringVar(buf, dataString, 0, STREAM_MAX_BUFFER_SIZE-bufLen);
      jsvUnLock(buf);
    }
  }
  return ok;
}
