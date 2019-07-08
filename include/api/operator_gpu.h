//
// Created by kier on 19-5-18.
//

#ifndef TENSORSTACK_API_OPERATOR_GPU_H
#define TENSORSTACK_API_OPERATOR_GPU_H

#include "operator.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <driver_types.h>

/**
 * get cuda stream in gpu operator
 * @param context instance of ts_OperatorContext
 * @raturn pointer value of cudaStream_t @sa TS_CUDA_STREAM
 */
TENSOR_STACK_C_API void *ts_OperatorContext_cuda_stream(ts_OperatorContext *context);

/**
 * get cuda stream in gpu operator
 * @param context instance of ts_OperatorContext
 */
#define TS_CUDA_STREAM(context) ((cudaStream_t)(ts_OperatorContext_cuda_stream(context)))

#ifdef __cplusplus
}
#endif

#endif //TENSORSTACK_API_OPERATOR_GPU_H
