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
 * JavaScript methods for Waveforms (eg. Audio)
 * ----------------------------------------------------------------------------
 */
#include "jswrap_waveform.h"
#include "jswrap_arraybuffer.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jstimer.h"

#define JSI_WAVEFORM_NAME JS_HIDDEN_CHAR_STR"wave"


/*JSON{ "type":"class",
        "class" : "Waveform",
        "description" : [ "This class handles waveforms. In Espruino, a Waveform is a set of data that you want to input or output.",
                          "NOTE: This API is beta and is very likely to change." ]
}*/

/*JSON{ "type":"constructor", "class": "Waveform",  "name": "Waveform",
         "description" : "Create a waveform class",
         "generate" : "jswrap_waveform_constructor",
         "params" : [ [ "output", "pin", "The pin to output on" ],
                      [ "samples", "int32", "The number of samples" ] ],
         "return" : [ "JsVar", "An Waveform object" ]

}*/
JsVar *jswrap_waveform_constructor(Pin pin, int samples) {
  if (samples<0) return 0;
  JsVar *arrayLength = jsvNewFromInteger(samples);
  JsVar *arrayBuffer = jswrap_typedarray_constructor(ARRAYBUFFERVIEW_UINT8, arrayLength, 0, 0);
  jsvUnLock(arrayLength);
  if (!arrayBuffer) return 0;

  JsVar *waveform = jspNewObject(0, "Waveform");
  if (!waveform) {
    jsvUnLock(arrayBuffer);
    return 0;
  }
  jsvUnLock(jsvObjectSetChild(waveform, "pin", jsvNewFromPin(pin)));
  jsvUnLock(jsvObjectSetChild(waveform, "buffer", arrayBuffer));
  return waveform;
}

/*JSON{ "type":"method", "class": "Waveform", "name" : "startOutput",
         "description" : "Will start outputting the waveform on the given pin. If not repeating, it'll emit a `finish` event when it is done.",
         "generate" : "jswrap_waveform_startOutput",
         "params" : [ [ "freq", "float", "The frequency to output each sample at"] ]
}*/
void jswrap_waveform_startOutput(JsVar *waveform, JsVarFloat freq) {
  bool running = jsvGetBoolAndUnLock(jsvObjectGetChild(execInfo.root, "running", 0));
  if (running) return;

  Pin pin = jshGetPinFromVarAndUnLock(jsvObjectGetChild(waveform, "pin", 0));
  JsVar *arrayBuffer = jsvObjectGetChild(waveform, "buffer", 0);
  // plough through to get array buffer data
  while (jsvIsArrayBuffer(arrayBuffer)) {
    JsVar *s = jsvLock(arrayBuffer->firstChild);
    jsvUnLock(arrayBuffer);
    arrayBuffer = s;
  }
  assert(jsvIsString(arrayBuffer));
  // And finally set it up
  jstSignalWrite(jshGetTimeFromMilliseconds(1000.0 / freq), 0, arrayBuffer);
  jsvUnLock(arrayBuffer);

  jsvUnLock(jsvObjectSetChild(waveform, "running", jsvNewFromBool(true)));
  // Add to our list of active waveforms
  JsVar *waveforms = jsvObjectGetChild(execInfo.root, JSI_WAVEFORM_NAME, JSV_ARRAY);
  if (waveforms) {
    jsvArrayPush(waveforms, waveform);
    jsvUnLock(waveforms);
  }
}

/*JSON{ "type":"idle", "generate" : "jswrap_waveform_idle" }*/
bool jswrap_waveform_idle() {
  JsVar *waveforms = jsvObjectGetChild(execInfo.root, JSI_WAVEFORM_NAME, 0);
  if (waveforms) {
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, waveforms);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *waveform = jsvArrayIteratorGetElement(&it);

      bool running = jsvGetBoolAndUnLock(jsvObjectGetChild(waveform, "running", 0));
      if (running) {
        Pin pin = jshGetPinFromVarAndUnLock(jsvObjectGetChild(waveform, "pin", 0));
        UtilTimerTask task;
        // if the timer task is now gone...
        if (!jstGetLastPinTimerTask(pin, &task)) {
          jsiQueueObjectCallbacks(waveform, "#onfinish", 0, 0);
          running = false;
          jsvUnLock(jsvObjectSetChild(waveform, "running", jsvNewFromBool(running)));
        }
      }
      jsvUnLock(waveform);
      // if not running, remove waveform from this list
      if (!running)
        jsvArrayIteratorRemoveAndGotoNext(&it, waveforms);
      else
        jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    jsvUnLock(waveforms);
  }
  return false; // no need to stay awake - an IRQ will wake us
}
