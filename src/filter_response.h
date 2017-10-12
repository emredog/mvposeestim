#ifndef FILTER_RESPONSE_H
#define FILTER_RESPONSE_H

#include "flags.h"
#include "myarray.hpp"


/*
 * Compute filter response using CPU.
 */
myArray<FPTYPE>** fconvMT_C(const myArray<FPTYPE> *pyra_feat_level, myArray<FPTYPE> **filters, int start, int end);


#ifdef USE_CUDA

/*
 * Initialize some resources to use CUDA.
 */
void initCuda(void);

/*
 * release resources used by CUDA.
 */
void releaseCuda(void);

/*
 * Compute filter response using GPU.
 */
myArray<FPTYPE>** fconv_cuda(const myArray<FPTYPE> *pyra_feat_level, myArray<FPTYPE> **filters, int start, int end);

/*
 * Wake up CUDA driver.
 */
void cudaWakeUp(void);

#endif


#endif // FILTER_RESPONSE_H

