/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

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
#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
extern "C" {
#include <jsinteractive.h>
}

namespace tflite {
namespace {
void DebugLogPrintf(const char* format, va_list args) {
  const int output_cache_size = 64;
  char output_cache[output_cache_size + 1];
  int output_cache_index = 0;
  const char* current = format;
  while (*current != 0) {
    if (*current == '%') {
      const char next = *(current + 1);
      if ((next == 'd') || (next == 's')) {
        current += 1;
        if (output_cache_index > 0) {
          output_cache[output_cache_index] = 0;
          jsiConsolePrint(output_cache);
          output_cache_index = 0;
        }
        if (next == 'd') {
          jsiConsolePrintf("%d",va_arg(args, int));
        } else if (next == 's') {
          jsiConsolePrint(va_arg(args, char*));
        }
      }
    } else {
      output_cache[output_cache_index] = *current;
      output_cache_index += 1;
    }
    if (output_cache_index >= output_cache_size) {
      output_cache[output_cache_index] = 0;
      jsiConsolePrint(output_cache);
      output_cache_index = 0;
    }
    current += 1;
  }
  if (output_cache_index > 0) {
    output_cache[output_cache_index] = 0;
    jsiConsolePrint(output_cache);
    output_cache_index = 0;
  }
  jsiConsolePrint("\n");
}
}  // namespace

class MicroErrorReporter : public ErrorReporter {
 public:
  ~MicroErrorReporter() {}
  int Report(const char* format, va_list args) override {
    DebugLogPrintf(format, args);
    return 0;
  }

 private:
  TF_LITE_REMOVE_VIRTUAL_DELETE
};

}// namespace tflite

float testtensorx(float x_val) {
  // Set up logging.
  tflite::MicroErrorReporter micro_error_reporter;
  tflite::ErrorReporter* error_reporter = &micro_error_reporter;

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

  // This pulls in all the operation implementations we need
  tflite::ops::micro::AllOpsResolver resolver;


  // Create an area of memory to use for input, output, and intermediate arrays.
  // The size of this will depend on the model you're using, and may need to be
  // determined by experimentation.
  const int tensor_arena_size = 4 * 1024;
  uint8_t tensor_arena[tensor_arena_size];

  // Build an interpreter to run the model with.
  tflite::MicroInterpreter interpreter(model, resolver, tensor_arena,
                                       tensor_arena_size, error_reporter);

  // Obtain pointers to the model's input and output tensors
  TfLiteTensor* input = interpreter.input(0);
  TfLiteTensor* output = interpreter.output(0);


    // Place our calculated x value in the model's input tensor
    input->data.f[0] = x_val;

    // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter.Invoke();
    if (invoke_status != kTfLiteOk) {
      error_reporter->Report("Invoke failed on x_val: %f",
                             static_cast<double>(x_val));
      return 42;
    }

    // Read the predicted y value from the model's output tensor
    float y_val = output->data.f[0];

    // Output the results. A custom HandleOutput function can be implemented
    // for each supported hardware target.
    //HandleOutput(error_reporter, x_val, y_val);

  
  return x_val;
}

extern "C" {
float testtensor(float x) {
  return testtensorx(x);
}
}
