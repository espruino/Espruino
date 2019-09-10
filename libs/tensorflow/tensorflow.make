TENSOR_ROOT := $(ROOT)/libs/tensorflow
CCSOURCES += \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/simple_memory_allocator.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/micro_mutable_op_resolver.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/micro_allocator.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/debug_log.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/debug_log_numbers.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/micro_interpreter.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/kernels/depthwise_conv.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/kernels/softmax.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/kernels/all_ops_resolver.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/kernels/fully_connected.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/kernels/elementwise.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/kernels/arg_min_max.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/core/api/error_reporter.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/core/api/flatbuffer_conversions.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/core/api/op_resolver.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/memory_planner/greedy_memory_planner.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/experimental/micro/memory_helpers.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/activations.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/add.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/add_n.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/arg_min_max.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/assign_variable.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/audio_spectrogram.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/basic_rnn.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/batch_to_space_nd.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/bidirectional_sequence_lstm.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/bidirectional_sequence_rnn.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/cast.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/ceil.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/comparisons.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/concatenation.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/conv.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/cpu_backend_context.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/cpu_backend_gemm_eigen.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/depth_to_space.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/depthwise_conv.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/dequantize.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/detection_postprocess.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/div.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/eigen_support.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/elementwise.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/embedding_lookup.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/embedding_lookup_sparse.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/expand_dims.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/exp.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/fake_quant.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/fill.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/floor.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/floor_div.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/floor_mod.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/fully_connected.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/gather.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/gather_nd.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/hashtable_lookup.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/if.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/kernel_util.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/l2norm.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/local_response_norm.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/logical.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/lsh_projection.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/lstm.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/lstm_eval.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/matrix_diag.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/matrix_set_diag.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/maximum_minimum.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/mfcc.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/mirror_pad.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/mul.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/neg.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/non_max_suppression.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/one_hot.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/pack.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/pad.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/pooling.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/pow.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/quantize.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/range.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/rank.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/read_variable.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/reduce.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/register.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/register_ref.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/reshape.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/resize_bilinear.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/resize_nearest_neighbor.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/reverse.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/reverse_sequence.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/rfft2d.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/round.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/select.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/shape.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/skip_gram.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/slice.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/space_to_batch_nd.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/space_to_depth.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/sparse_to_dense.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/split.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/split_v.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/squared_difference.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/squeeze.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/strided_slice.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/sub.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/svdf.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/tile.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/topk_v2.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/transpose.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/transpose_conv.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/unidirectional_sequence_lstm.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/unidirectional_sequence_rnn.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/unique.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/unpack.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/where.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/while.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/zeros_like.cc \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/internal/quantization_util.cc \
$(TENSOR_ROOT)/main.cc \
$(TENSOR_ROOT)/sine_model_data.cc

#$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/pooling.cc \
#$(TENSOR_ROOT)/tensorflow/lite/experimental/micro/micro_error_reporter.cc \
#$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/test_main.cc \
#$(TENSOR_ROOT)/tensorflow/tensorflow/lite/kernels/test_util.cc \


SOURCES += \
$(TENSOR_ROOT)/tensorflow/tensorflow/lite/c/c_api_internal.c \

WRAPPERSOURCES += \
$(TENSOR_ROOT)/jswrap_tensorflow.c

INCLUDE += \
-I$(TENSOR_ROOT) \
-I$(TENSOR_ROOT)/third_party/gemmlowp \
-I$(TENSOR_ROOT)/third_party/flatbuffers/include \
-I$(TENSOR_ROOT)/third_party/farmhash/src \
-I$(TENSOR_ROOT)/eigen-git-mirror \
-I$(TENSOR_ROOT)/tensorflow \
-I$(TENSOR_ROOT)/tensorflow/tensorflow/lite \

CCFLAGS += -DNDEBUG --std=c++11 -g -fno-rtti -fpermissive -DTF_LITE_STATIC_MEMORY


