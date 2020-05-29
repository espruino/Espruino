TENSOR_ROOT := $(ROOT)/libs/tensorflow
CCSOURCES += \
$(TENSOR_ROOT)/tensorflow/lite/micro/simple_memory_allocator.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/memory_helpers.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/test_helpers.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/micro_interpreter.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/micro_utils.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/micro_allocator.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/depthwise_conv.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/conv.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/pooling.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/softmax.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/fully_connected.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc \
$(TENSOR_ROOT)/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc \
$(TENSOR_ROOT)/tensorflow/lite/core/api/error_reporter.cc \
$(TENSOR_ROOT)/tensorflow/lite/core/api/flatbuffer_conversions.cc \
$(TENSOR_ROOT)/tensorflow/lite/core/api/op_resolver.cc \
$(TENSOR_ROOT)/tensorflow/lite/core/api/tensor_utils.cc \
$(TENSOR_ROOT)/tensorflow/lite/kernels/kernel_util.cc \
$(TENSOR_ROOT)/tensorflow/lite/kernels/internal/quantization_util.cc \
$(TENSOR_ROOT)/tensorflow.cc

# unused kernels
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/pack.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/logical.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/reshape.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/quantize.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/dequantize.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/logistic.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/prelu.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/ceil.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/arg_min_max.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/unpack.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/add.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/split.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/neg.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/floor.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/comparisons.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/svdf.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/all_ops_resolver.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/maximum_minimum.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/elementwise.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/strided_slice.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/round.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/concatenation.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/pad.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/mul.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/activations.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/kernels/reduce.cc \

#$(TENSOR_ROOT)/tensorflow/lite/micro/micro_mutable_op_resolver.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/debug_log_numbers.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/debug_log.cc 
#$(TENSOR_ROOT)/tensorflow/lite/micro/micro_string.cc \
#$(TENSOR_ROOT)/tensorflow/lite/micro/micro_error_reporter.cc \

SOURCES += \
$(TENSOR_ROOT)/tensorflow/lite/c/common.c \

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
