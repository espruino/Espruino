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

#include "sine_model_data.h"
#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
extern "C" {
#include "jsinteractive.h"
void DebugLog(const char* s) { jsiConsolePrint(s); }
}

const int tensor_arena_size = 2 * 1024;

typedef struct {
  // logging
  tflite::MicroErrorReporter micro_error_reporter;
  // This pulls in all the operation implementations we need
  tflite::ops::micro::AllOpsResolver resolver;
  // Build an interpreter to run the model with
  tflite::MicroInterpreter interpreter;
  // Create an area of memory to use for input, output, and intermediate arrays.
  // Finding the minimum value for your model may require some trial and error.
  uint8_t tensor_arena[tensor_arena_size];
} TFData;
char tfDataPtr[sizeof(TFData)];


float testtensorx(float x_val) {
  TFData *tf = (TFData*)tfDataPtr;
  // Set up logging
  tflite::ErrorReporter* error_reporter = new (&tf->micro_error_reporter)tflite::MicroErrorReporter();;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  const tflite::Model* model = ::tflite::GetModel(g_sine_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return 1;
  }

  new (&tf->resolver)tflite::ops::micro::AllOpsResolver();

  // Build an interpreter to run the model with
  new (&tf->interpreter)tflite::MicroInterpreter(model, tf->resolver, tf->tensor_arena,
                                          tensor_arena_size, error_reporter);

  // Allocate memory from the tensor_arena for the model's tensors
  tf->interpreter.AllocateTensors();

  // Obtain pointers to the model's input and output tensors
  TfLiteTensor* input = tf->interpreter.input(0);
  TfLiteTensor* output = tf->interpreter.output(0);

  // Place our calculated x value in the model's input tensor
  input->data.f[0] = x_val;

  // Run inference, and report any error
  TfLiteStatus invoke_status = tf->interpreter.Invoke();
  if (invoke_status != kTfLiteOk) {
    error_reporter->Report("Invoke failed on x_val: %f",
                           static_cast<double>(x_val));
    return NAN;
  }

  // Read the predicted y value from the model's output tensor
  float y_val = output->data.f[0];

  return y_val;
}

extern "C" {
float testtensor(float x) {
  return testtensorx(x);
}
}
