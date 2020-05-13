#include "tensorflow/lite/c/common.h"

size_t tf_get_size(size_t arena_size, const char *model_data);
bool tf_create(void *dataPtr, size_t arena_size, const char *model_data);
void tf_destroy(void *dataPtr);
bool tf_invoke(void *dataPtr);
TfLiteTensor *tf_get_input(void *dataPtr, int n);
TfLiteTensor *tf_get_output(void *dataPtr, int n);
