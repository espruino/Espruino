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

#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
extern "C" {
#include "jsinteractive.h"
#include "tensorflow.h"

void DebugLog(const char* s) { jsiConsolePrint(s); }

typedef struct {
  // logging
  tflite::MicroErrorReporter micro_error_reporter;
  // This pulls in all the operation implementations we need
  tflite::ops::micro::AllOpsResolver resolver;
  // Build an interpreter to run the model with
  tflite::MicroInterpreter interpreter;
  // Create an area of memory to use for input, output, and intermediate arrays.
  // Finding the minimum value for your model may require some trial and error.
  uint8_t tensor_arena[0];
} TFData;
char tfDataPtr[sizeof(TFData)];

size_t tf_get_size(size_t arena_size, const char *model_data) {
  return sizeof(TFData) + arena_size;
}

bool tf_create(void *dataPtr, size_t arena_size, const char *model_data) {
  TFData *tf = (TFData*)dataPtr;
  new (&tf->micro_error_reporter)tflite::MicroErrorReporter();
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

  new (&tf->resolver)tflite::ops::micro::AllOpsResolver();

  // Build an interpreter to run the model with
  new (&tf->interpreter)tflite::MicroInterpreter(
      model, tf->resolver, tf->tensor_arena,
      arena_size, error_reporter);

  // Allocate memory from the tensor_arena for the model's tensors
  tf->interpreter.AllocateTensors();


  /*
   TfLiteTensor* input = tf->interpreter.input(0);
   TfLiteTensor* output = tf->interpreter.output(0);

  // Place our calculated x value in the model's input tensor
  input->data.f[0] = x_val;



  // Read the predicted y value from the model's output tensor
  float y_val = output->data.f[0];*/

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

TfLiteTensor *tf_get_input(void *dataPtr, int n) {
  TFData *tf = (TFData*)dataPtr;
  // Obtain pointers to the model's input and output tensors
  return tf->interpreter.input(0);
}

TfLiteTensor *tf_get_output(void *dataPtr, int n) {
  TFData *tf = (TFData*)dataPtr;
  // Obtain pointers to the model's input and output tensors
  return tf->interpreter.output(0);
}

} // extern "C"
