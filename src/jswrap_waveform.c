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

#define JSI_WAVEFORM_NAME "wave"

#ifndef SAVE_ON_FLASH

/*JSON{
  "type" : "class",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Waveform"
}
This class handles waveforms. In Espruino, a Waveform is a set of data that you
want to input or output.
 */
/*JSON{
  "type" : "event",
  "class" : "Waveform",
  "name" : "finish",
  "params" : [["buffer","JsVar","the last played buffer"]],
  "ifdef" : "BANGLEJS"
}
Event emitted when playback has finished
 */
/*JSON{
  "type" : "event",
  "class" : "Waveform",
  "name" : "buffer",
  "params" : [["buffer","JsVar","the last played buffer (which now needs to be filled ready for playback)"]],
  "ifdef" : "BANGLEJS"
}
When in double-buffered mode, this event is emitted when the `Waveform` class swaps to playing a new buffer - so you should then fill this current buffer up with new data.
 */

static JsVar *jswrap_waveform_getBuffer(JsVar *waveform, int bufferNumber, bool *is16Bit) {
  JsVar *buffer = jsvObjectGetChildIfExists(waveform, (bufferNumber==0)?"buffer":"buffer2");
  if (!buffer) return 0;
  if (is16Bit) {
    *is16Bit = false;
    if (jsvIsArrayBuffer(buffer) && JSV_ARRAYBUFFER_GET_SIZE(buffer->varData.arraybuffer.type)==2)
      *is16Bit = true;
  }
  // plough through to get array buffer data
  JsVar *backingString = jsvGetArrayBufferBackingString(buffer, NULL);
  jsvUnLock(buffer);
  return backingString;
}


/*JSON{
  "type" : "kill",
  "generate" : "jswrap_waveform_kill",
  "ifndef" : "SAVE_ON_FLASH"
}*/
void jswrap_waveform_kill() { // be sure to remove all waveforms...
  JsVar *waveforms = jsvObjectGetChildIfExists(execInfo.hiddenRoot, JSI_WAVEFORM_NAME);
  if (waveforms) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, waveforms);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *waveform = jsvObjectIteratorGetValue(&it);
      bool running = jsvObjectGetBoolChild(waveform, "running");
      if (running) {
        JsVar *buffer = jswrap_waveform_getBuffer(waveform,0,0);
        if (!jstStopBufferTimerTask(buffer)) {
          jsExceptionHere(JSET_ERROR, "Waveform couldn't be stopped");
        }
        jsvUnLock(buffer);
      }
      jsvUnLock(waveform);
      // if not running, remove waveform from this list
      jsvObjectIteratorRemoveAndGotoNext(&it, waveforms);
    }
    jsvObjectIteratorFree(&it);
    jsvUnLock(waveforms);
  }
}


/*JSON{
  "type" : "constructor",
  "class" : "Waveform",
  "name" : "Waveform",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_waveform_constructor",
  "params" : [
    ["samples","JsVar","The number of samples to allocate as an integer, *or* an arraybuffer (2v25+) containing the samples"],
    ["options","JsVar","[optional] options struct `{ doubleBuffer:bool, bits : 8/16 }` (see below)"]
  ],
  "return" : ["JsVar","An Waveform object"]
}
Create a waveform class. This allows high speed input and output of waveforms.
It has an internal variable called `buffer` (as well as `buffer2` when
double-buffered - see `options` below) which contains the data to input/output.

Options can contain:

```
{
  doubleBuffer : bool   // whether to allocate two buffers or not (default false)
  bits         : 8/16   // the amount of bits to use (default 8).
}
```

When double-buffered, a 'buffer' event will be emitted each time a buffer is
finished with (the argument is that buffer). When the recording stops, a
'finish' event will be emitted (with the first argument as the buffer).

```
// Output a sine wave
var w = new Waveform(1000);
for (var i=0;i<1000;i++) w.buffer[i]=128+120*Math.sin(i/2);
analogWrite(H0, 0.5, {freq:80000}); // set up H0 to output an analog value by PWM
w.on("finish", () => print("Done!"))
w.startOutput(H0,8000); // start playback
```

```
// On 2v25, from Storage
var f = require("Storage").read("sound.pcm");
var w = new Waveform(E.toArrayBuffer(f));
w.on("finish", () => print("Done!"))
w.startOutput(H0,8000); // start playback
```

See https://www.espruino.com/Waveform for more examples.
 */
JsVar *jswrap_waveform_constructor(JsVar *_samples, JsVar *options) {
  int samples = 0;
  JsVar *arrayBuffer = NULL;
  if (jsvIsIntegerish(_samples)) {
    samples = jsvGetInteger(_samples);
    if (samples<=0) {
      jsExceptionHere(JSET_ERROR, "Samples must be greater than 0");
      return 0;
    }
  } else if (jsvIsArrayBuffer(_samples)) {
    arrayBuffer = jsvLockAgain(_samples);
    samples = jsvGetLength(arrayBuffer);
  } else {
    jsExceptionHere(JSET_ERROR, "'samples' should be a integer or ArrayBuffer");
    return 0;
  }

  bool doubleBuffer = false;
  bool use16bit = false;
  if (jsvIsObject(options)) {
    doubleBuffer = jsvObjectGetBoolChild(options, "doubleBuffer");

    int bits = (int)jsvObjectGetIntegerChild(options, "bits");
    if (bits!=0 && bits!=8 && bits!=16) {
      jsExceptionHere(JSET_ERROR, "Invalid number of bits");
      return 0;
    } else if (bits==16) use16bit = true;
  } else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_ERROR, "Expecting options to be undefined or an Object, not %t", options);
  }

  JsVarDataArrayBufferViewType bufferType = use16bit ? ARRAYBUFFERVIEW_UINT16 : ARRAYBUFFERVIEW_UINT8;
  if (!arrayBuffer) arrayBuffer = jsvNewTypedArray(bufferType, samples);
  JsVar *arrayBuffer2 = 0;
  if (doubleBuffer) arrayBuffer2 = jsvNewTypedArray(bufferType, samples);
  JsVar *waveform = jspNewObject(0, "Waveform");

  if (!waveform || !arrayBuffer || (doubleBuffer && !arrayBuffer2)) {
    jsvUnLock3(waveform,arrayBuffer,arrayBuffer2); // out of memory
    return 0;
  }
  jsvObjectSetChildAndUnLock(waveform, "buffer", arrayBuffer);
  if (arrayBuffer2) jsvObjectSetChildAndUnLock(waveform, "buffer2", arrayBuffer2);

  return waveform;
}

static void jswrap_waveform_start(JsVar *waveform, Pin pin, JsVarFloat freq, JsVar *options, bool isWriting) {
  bool running = jsvObjectGetBoolChild(waveform, "running");
  if (running) {
    jsExceptionHere(JSET_ERROR, "Waveform is already running");
    return;
  }
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Invalid pin");
    return;
  }
  if (!isfinite(freq) || freq<0.001) {
    jsExceptionHere(JSET_ERROR, "Frequency must be above 0.001Hz");
    return;
  }

  JsSysTime startTime = 0;
  bool repeat = false;
  Pin npin = PIN_UNDEFINED;
  if (jsvIsObject(options)) {
    JsVarFloat t = jsvObjectGetFloatChild(options, "time");
    if (isfinite(t) && t>0)
      startTime = jshGetTimeFromMilliseconds(t*1000) - jshGetSystemTime();
    repeat = jsvObjectGetBoolChild(options, "repeat");
    npin = jshGetPinFromVarAndUnLock(jsvObjectGetChildIfExists(options, "npin"));
  } else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_ERROR, "Expecting options to be undefined or an Object, not %t", options);
  }

  bool is16Bit = false;
  JsVar *buffer = jswrap_waveform_getBuffer(waveform,0, &is16Bit);
  JsVar *buffer2 = jswrap_waveform_getBuffer(waveform,1,0);

  UtilTimerEventType eventType;

  if (is16Bit) {
    eventType = isWriting ? UET_WRITE_SHORT : UET_READ_SHORT;
  } else {
    eventType = isWriting ? UET_WRITE_BYTE : UET_READ_BYTE;
  }


  // And finally set it up
  int timerId = jstStartSignal(startTime, jshGetTimeFromMilliseconds(1000.0 / freq), pin, npin, buffer, repeat?(buffer2?buffer2:buffer):0, eventType);
  if (timerId<0)
    jsWarn("Unable to schedule a timer");
  else
    jsvObjectSetChildAndUnLock(waveform, "freq", jsvNewFromInteger(timerId));
  jsvUnLock2(buffer,buffer2);

  jsvObjectSetChildAndUnLock(waveform, "running", jsvNewFromBool(true));
  jsvObjectSetChildAndUnLock(waveform, "freq", jsvNewFromFloat(freq));
  // Add to our list of active waveforms
  JsVar *waveforms = jsvObjectGetChild(execInfo.hiddenRoot, JSI_WAVEFORM_NAME, JSV_ARRAY);
  if (waveforms) {
    jsvSetArrayItem(waveforms, timerId, waveform);
    jsvUnLock(waveforms);
  }
}

/*JSON{
  "type" : "method",
  "class" : "Waveform",
  "name" : "startOutput",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_waveform_startOutput",
  "params" : [
    ["output","pin","The pin to output on"],
    ["freq","float","The frequency to output each sample at"],
    ["options","JsVar","[optional] options struct `{time:float, repeat:bool, npin:Pin}` (see below)"]
  ]
}
Will start outputting the waveform on the given pin - the pin must have
previously been initialised with analogWrite. If not repeating, it'll emit a
`finish` event when it is done.

```
{
  time : float,        // the that the waveform with start output at, e.g. `getTime()+1` (otherwise it is immediate)
  repeat : bool,       // whether to repeat the given sample
  npin : Pin,          // If specified, the waveform is output across two pins (see below)
}
```

Using `npin` allows you to split the Waveform output between two pins and hence avoid
any DC bias (or need to capacitor), for instance you could attach a speaker to `H0` and
`H1` on Jolt.js. When the value in the waveform was at 50% both outputs would be 0,
below 50% the signal would be on `npin` with `pin` as 0, and above 50% it would be on `pin` with `npin` as 0.
*/
void jswrap_waveform_startOutput(JsVar *waveform, Pin pin, JsVarFloat freq, JsVar *options) {
  jswrap_waveform_start(waveform, pin, freq, options, true/*write*/);
}

/*JSON{
  "type" : "method",
  "class" : "Waveform",
  "name" : "startInput",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_waveform_startInput",
  "params" : [
    ["output","pin","The pin to output on"],
    ["freq","float","The frequency to output each sample at"],
    ["options","JsVar","[optional] options struct `{time:float,repeat:bool}` where: `time` is the that the waveform with start output at, e.g. `getTime()+1` (otherwise it is immediate), `repeat` is a boolean specifying whether to repeat the give sample"]
  ]
}
Will start inputting the waveform on the given pin that supports analog. If not
repeating, it'll emit a `finish` event when it is done.
 */
void jswrap_waveform_startInput(JsVar *waveform, Pin pin, JsVarFloat freq, JsVar *options) {
  // Setup analog, and also bail out on failure
  if (jshPinAnalog(pin)<0) return;
  // start!
  jswrap_waveform_start(waveform, pin, freq, options, false/*read*/);
}

/*JSON{
  "type" : "method",
  "class" : "Waveform",
  "name" : "stop",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_waveform_stop"
}
Stop a waveform that is currently outputting
 */
void jswrap_waveform_stop(JsVar *waveform) {
  bool running = jsvObjectGetBoolChild(waveform, "running");
  if (!running) {
    jsExceptionHere(JSET_ERROR, "Waveform is not running");
    return;
  }
  JsVar *buffer = jswrap_waveform_getBuffer(waveform,0,0);
  if (!jstStopBufferTimerTask(buffer)) {
    jsExceptionHere(JSET_ERROR, "Waveform couldn't be stopped");
  }
  jsvUnLock(buffer);
}

JsVar *_jswrap_waveform_getById(int id) {
  JsVar *waveforms = jsvObjectGetChild(execInfo.hiddenRoot, JSI_WAVEFORM_NAME, JSV_ARRAY);
  if (!waveforms) return 0;
  return jsvGetArrayItem(waveforms, id);
}

/*JSON{
  "type" : "EV_CUSTOM",
  "#if" : "!defined(SAVE_ON_FLASH)",
  "generate" : "jswrap_waveform_eventHandler"
}
*/
void jswrap_waveform_eventHandler(IOEventFlags eventFlags, uint8_t *data, int length) {
  IOCustomEventFlags customFlags = *(IOCustomEventFlags*)data;
  int id = customFlags >> EVC_DATA_SHIFT;
  if ((customFlags&EVC_TYPE_MASK)==EVC_TIMER_FINISHED) {
    JsVar *waveform = _jswrap_waveform_getById(id);
    if (waveform) {
      jsvObjectSetChildAndUnLock(waveform, "running", jsvNewFromBool(false));
      JsVar *arrayBuffer = jsvObjectGetChildIfExists(waveform, "buffer");
      jsiQueueObjectCallbacks(waveform, JS_EVENT_PREFIX"finish", &arrayBuffer, 1);
      jsvUnLock(arrayBuffer);
      JsVar *waveforms = jsvObjectGetChild(execInfo.hiddenRoot, JSI_WAVEFORM_NAME, JSV_ARRAY);
      if (waveforms) jsvRemoveArrayItem(waveforms, id);
      jsvUnLock2(waveform, waveforms);
    }
  }
  if ((customFlags&EVC_TYPE_MASK) == EVC_TIMER_BUFFER_FLIP) {
    JsVar *waveform = _jswrap_waveform_getById(id);
    if (waveform) {
      UtilTimerTask *task = &utilTimerTaskInfo[id];
      if (task->data.buffer.nextBuffer &&
          task->data.buffer.nextBuffer != task->data.buffer.currentBuffer) {
        // if it is a double-buffered task
        JsVar *buffer = jswrap_waveform_getBuffer(waveform,0,0);
        int currentBuffer = (jsvGetRef(buffer)==task->data.buffer.currentBuffer) ? 0 : 1;
        jsvUnLock(buffer);
        int oldBuffer = jsvGetIntegerAndUnLock(jsvObjectGetChild(waveform, "currentBuffer", JSV_INTEGER));
        if (oldBuffer != currentBuffer) {
          // buffers have changed - fire off a 'buffer' event with the buffer that needs to be filled
          jsvObjectSetChildAndUnLock(waveform, "currentBuffer", jsvNewFromInteger(currentBuffer));
          JsVar *arrayBuffer = jsvObjectGetChildIfExists(waveform, (currentBuffer==0) ? "buffer2" : "buffer");
          jsiQueueObjectCallbacks(waveform, JS_EVENT_PREFIX"buffer", &arrayBuffer, 1);
          jsvUnLock(arrayBuffer);
        }
      }
      jsvUnLock(waveform);
    }
  }
}
#endif

