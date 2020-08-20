/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/lite/micro/micro_interpreter.h"
#ifdef TENSORFLOW_ALL_OPS
#include "tensorflow/lite/micro/kernels/all_ops_resolver.h"
#else
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#endif
#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/micro/compatibility.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
extern "C" {
#include "jsinteractive.h"
#include "tensorflow.h"

void DebugLog(const char* s) { jsiConsolePrint("TF:");jsiConsolePrint(s); }

namespace tflite {

class EspruinoErrorReporter : public ErrorReporter {
 public:
  ~EspruinoErrorReporter() {}
  int Report(const char* format, va_list args) override {
    char log_buffer[256];
    espruino_snprintf_va(log_buffer, sizeof(log_buffer), format, args);
    jsExceptionHere(JSET_ERROR, "%s", log_buffer);
    return 0;
  }
  TF_LITE_REMOVE_VIRTUAL_DELETE
};
}  // namespace tflite

typedef struct {
  // logging
  tflite::EspruinoErrorReporter micro_error_reporter;
  // This pulls in the operation implementations we need
#ifdef TENSORFLOW_ALL_OPS
  tflite::ops::micro::AllOpsResolver resolver;
#else
#define TENSORFLOW_OP_COUNT 6
  tflite::MicroMutableOpResolver<TENSORFLOW_OP_COUNT> resolver;
#endif
  // Build an interpreter to run the model with
  tflite::MicroInterpreter interpreter;
  // Create an area of memory to use for input, output, and intermediate arrays.
  // Finding the minimum value for your model may require some trial and error.
  alignas(16) uint8_t tensor_arena[0]; // the arena must now be 16 byte aligned
} TFData;
char tfDataPtr[sizeof(TFData)];

size_t tf_get_size(size_t arena_size, const char *model_data) {
  return sizeof(TFData) + arena_size;
}

bool tf_create(void *dataPtr, size_t arena_size, const char *model_data) {
  TFData *tf = (TFData*)dataPtr;
  new (&tf->micro_error_reporter)tflite::EspruinoErrorReporter();
  // Set up logging
  tflite::ErrorReporter* error_reporter = &tf->micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  const tflite::Model* model = ::tflite::GetModel(model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return false;
  }

#ifdef TENSORFLOW_ALL_OPS
  new (&tf->resolver)tflite::ops::micro::AllOpsResolver();
#else
  // Pull in only the operation implementations we need.
  new (&tf->resolver)tflite::MicroMutableOpResolver<TENSORFLOW_OP_COUNT>();
  tf->resolver.AddDepthwiseConv2D();
  tf->resolver.AddConv2D();
  tf->resolver.AddAveragePool2D();
  tf->resolver.AddMaxPool2D();
  tf->resolver.AddFullyConnected();
  tf->resolver.AddSoftmax();
#endif

  // Build an interpreter to run the model with
  new (&tf->interpreter)tflite::MicroInterpreter(
      model, tf->resolver, tf->tensor_arena,
      arena_size, error_reporter);

  // Allocate memory from the tensor_arena for the model's tensors
  tf->interpreter.AllocateTensors();
  return true;
}

void tf_destroy(void *dataPtr) {
  TFData *tf = (TFData*)dataPtr;

  tf->interpreter.~MicroInterpreter();
}

bool tf_invoke(void *dataPtr) {
  TFData *tf = (TFData*)dataPtr;
  tflite::ErrorReporter* error_reporter = &tf->micro_error_reporter;
  // Run inference, and report any error
  //jsiConsolePrintf("in %f\n",tf->interpreter.input(0)->data.f[0]);
  TfLiteStatus invoke_status = tf->interpreter.Invoke();
  //jsiConsolePrintf("out %f\n",tf->interpreter.output(0)->data.f[0]);
  if (invoke_status != kTfLiteOk) {
    error_reporter->Report("Invoke failed");
    return false;
  }
  return true;
}

tf_tensorfinfo tf_get(void *dataPtr, bool isInput) {
  TFData *tf = (TFData*)dataPtr;
  tf_tensorfinfo inf;
  inf.data = 0;
  TfLiteTensor *tensor = isInput ? tf->interpreter.input(0) : tf->interpreter.output(0);
  if (tensor) {
    inf.type = tensor->type;
    inf.data = &tensor->data.f[0];
    inf.bytes = tensor->bytes;
  }
  return inf;
}

} // extern "C"
