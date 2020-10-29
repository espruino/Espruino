#include "tensorflow/lite/c/common.h"

typedef struct {
  void *data; // pointer to data
  TfLiteType type; // data type
  int bytes; // how big is the tensor
} tf_tensorfinfo;


size_t tf_get_size(size_t arena_size, const char *model_data);
bool tf_create(void *dataPtr, size_t arena_size, const char *model_data);
void tf_destroy(void *dataPtr);
bool tf_invoke(void *dataPtr);
tf_tensorfinfo tf_get(void *dataPtr, bool isInput);
