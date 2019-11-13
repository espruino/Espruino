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
 * Contains JavaScript interface for Neopixel/WS281x/APA10x devices
 * ----------------------------------------------------------------------------
 */

#include <jswrap_tensorflow.h>
#include "jsvariterator.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrap_arraybuffer.h"
#include "tensorflow.h"

/*JSON{
  "type" : "library",
  "class" : "tensorflow"
}
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "tensorflow",
  "name" : "create",
  "generate" : "jswrap_tensorflow_create",
  "params" : [
    ["arenaSize","int","The TensorFlow Arena size"],
    ["model","JsVar","The model to use - this should be a flat array/string"]
  ],
  "return" : ["JsVar","A tensorflow instance"],
  "return_object" : "TFMicroInterpreter"
}
*/
JsVar *jswrap_tensorflow_create(int arena_size, JsVar *model) {
  if (arena_size<512) {
    jsExceptionHere(JSET_ERROR, "Invalid Arena Size");
    return 0;
  }

  size_t modelSize = 0;
  char *modelPtr = jsvGetDataPointer(model, &modelSize);
  if (!modelPtr) {
    jsExceptionHere(JSET_TYPEERROR, "Model is not a Flat String/ArrayBuffer");
    return 0;
  }

  JsVar *tfmi = jspNewObject(NULL,"TFMicroInterpreter");
  if (!tfmi) return 0;

  size_t tfSize = tf_get_size((size_t)arena_size, modelSize);
  JsVar *mi = jsvNewFlatStringOfLength(tfSize);
  if (!mi) {
    jsExceptionHere(JSET_ERROR, "Unable to allocate enough RAM for TensorFlow");
    jsvUnLock(tfmi);
    return 0;
  }
  char *tfPtr = jsvGetDataPointer(mi, &tfSize);
  if (!tfPtr) {
    assert(0);
    return 0; // should never get here
  }

  if (!tf_create(tfPtr, (size_t)arena_size, modelPtr)) {
    jsExceptionHere(JSET_ERROR, "MicroInterpreter creation failed");
    jsvUnLock2(tfmi, mi);
  }
  jsvObjectSetChild(tfmi, "model", model); // so we keep a reference
  jsvObjectSetChildAndUnLock(tfmi, "mi", mi); // so we keep a reference
  return tfmi;
}

/*JSON{
  "type" : "class",
  "library" : "tensorflow",
  "class" : "TFMicroInterpreter",
  "ifdef" : "USE_TENSORFLOW"
}
Class containing an instance of TFMicroInterpreter
*/
void *jswrap_tfmicrointerpreter_getTFMI(JsVar *parent) {
  JsVar *mi = jsvObjectGetChild(parent, "mi", 0);
  size_t tfSize;
  char *tfPtr = jsvGetDataPointer(mi, &tfSize);
  jsvUnLock(mi);
  if (!tfPtr)
    jsExceptionHere(JSET_ERROR, "TFMicroInterpreter structure corrupted");
  return tfPtr;
}

JsVar *jswrap_tfmicrointerpreter_tensorToArrayBuffer(JsVar *parent, TfLiteTensor *tensor) {
  void *tfmi = jswrap_tfmicrointerpreter_getTFMI(parent);
  JsVar *mi = jsvObjectGetChild(parent, "mi", 0);
  if (!tensor && !mi) {
    jsExceptionHere(JSET_ERROR, "Unable to get tensor");
    return 0;
  }
  JsVarDataArrayBufferViewType abType = ARRAYBUFFERVIEW_UNDEFINED;
  switch (tensor->type) {
  case kTfLiteFloat32 :
    abType = ARRAYBUFFERVIEW_FLOAT32; break;
  case kTfLiteInt32 :
    abType = ARRAYBUFFERVIEW_INT32; break;
  case kTfLiteUInt8 :
    abType = ARRAYBUFFERVIEW_UINT8; break;
  case kTfLiteInt16 :
    abType = ARRAYBUFFERVIEW_INT16; break;
  case kTfLiteInt8 :
    abType = ARRAYBUFFERVIEW_INT8; break;
  default:
    jsExceptionHere(JSET_TYPEERROR, "Unsupported Tensor format TfLiteType:%d", tensor->type);
    return 0;
  }

  JsVar *ab = jsvNewArrayBufferFromString(mi,0);
  JsVar *b = jswrap_typedarray_constructor(abType, ab, ((size_t)&tensor->data.f[0])-(size_t)tfmi, tensor->bytes / JSV_ARRAYBUFFER_GET_SIZE(abType));
  jsvUnLock2(ab,mi);
  return b;
}
/*JSON{
  "type" : "method",
  "class" : "TFMicroInterpreter",
  "name" : "getInput",
  "generate" : "jswrap_tfmicrointerpreter_getInput",
  "return" : ["JsVar","An arraybuffer referencing the input data"],
  "return_object" : "ArrayBufferView"
}
*/
JsVar *jswrap_tfmicrointerpreter_getInput(JsVar *parent) {
  void *tfmi = jswrap_tfmicrointerpreter_getTFMI(parent);
  if (!tfmi) return 0;
  return jswrap_tfmicrointerpreter_tensorToArrayBuffer(parent, tf_get_input(tfmi, 0));
}
/*JSON{
  "type" : "method",
  "class" : "TFMicroInterpreter",
  "name" : "getOutput",
  "generate" : "jswrap_tfmicrointerpreter_getOutput",
  "return" : ["JsVar","An arraybuffer referencing the output data"],
  "return_object" : "ArrayBufferView"
}
*/
JsVar *jswrap_tfmicrointerpreter_getOutput(JsVar *parent) {
  void *tfmi = jswrap_tfmicrointerpreter_getTFMI(parent);
  if (!tfmi) return 0;
  return jswrap_tfmicrointerpreter_tensorToArrayBuffer(parent, tf_get_output(tfmi, 0));
}
/*JSON{
  "type" : "method",
  "class" : "TFMicroInterpreter",
  "name" : "invoke",
  "generate" : "jswrap_tfmicrointerpreter_invoke"
}
*/
void jswrap_tfmicrointerpreter_invoke(JsVar *parent) {
  void *tfmi = jswrap_tfmicrointerpreter_getTFMI(parent);
  if (!tfmi) return;
  if (!tf_invoke(tfmi)) {
    jsExceptionHere(JSET_TYPEERROR, "TFMicroInterpreter invoke failed");
  }
}

// FIXME: what about tf_destroy?
