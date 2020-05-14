TENSOR_ROOT := $(ROOT)/libs/tensorflow
CCSOURCES += \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/simple_memory_allocator.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/memory_helpers.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/micro_error_reporter.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/micro_mutable_op_resolver.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/debug_log_numbers.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/test_helpers.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/micro_interpreter.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/micro_utils.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/micro_allocator.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/pack.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/logical.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/reshape.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/depthwise_conv.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/quantize.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/dequantize.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/logistic.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/conv.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/pooling.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/softmax.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/prelu.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/ceil.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/arg_min_max.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/unpack.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/add.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/split.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/neg.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/floor.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/comparisons.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/svdf.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/all_ops_resolver.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/fully_connected.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/maximum_minimum.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/elementwise.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/strided_slice.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/kernels/round.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/memory_planner/linear_memory_planner.cc \
$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/memory_planner/greedy_memory_planner.cc \
$(TENSOR_ROOT)/tensorflow/lite/core/api/error_reporter.cc \
$(TENSOR_ROOT)/tensorflow/lite/core/api/flatbuffer_conversions.cc \
$(TENSOR_ROOT)/tensorflow/lite/core/api/op_resolver.cc \
$(TENSOR_ROOT)/tensorflow/lite/core/api/tensor_utils.cc \
$(TENSOR_ROOT)/tensorflow/lite/kernels/kernel_util.cc \
$(TENSOR_ROOT)/tensorflow/lite/kernels/internal/quantization_util.cc \
$(TENSOR_ROOT)/tensorflow.cc

#$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/debug_log.cc 

SOURCES += \
$(TENSOR_ROOT)/tensorflow/lite/c/c_api_internal.c \

WRAPPERSOURCES += \
$(TENSOR_ROOT)/jswrap_tensorflow.c

INCLUDE += \
-I$(TENSOR_ROOT) \
-I$(TENSOR_ROOT)/tensorflow \
-I$(TENSOR_ROOT)/third_party/gemmlowp \
-I$(TENSOR_ROOT)/third_party/kissfft \
-I$(TENSOR_ROOT)/third_party/flatbuffers/include

CCFLAGS += -DNDEBUG -g -DTF_LITE_STATIC_MEMORY --std=c++11 -g -fno-rtti -fpermissive -Wno-sign-compare -Wno-conversion -Wno-sign-conversion -Wno-missing-field-initializers -Wno-type-limits -Wno-unused-parameter -Wno-unused-variable
DEFINES += -DUSE_TENSORFLOW=1
