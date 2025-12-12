// #include "openblas.h"

// #include <cblas.h>

// #include "registry.h"

// void OpenBLASGemmOp::run(const float *A, const float *B, float *C,
//                          int M, int N, int K)
// {
//     cblas_sgemm(CblasRowMajor,
//                 CblasNoTrans, CblasNoTrans,
//                 M, N, K,
//                 1.0f,
//                 A, K,
//                 B, N,
//                 0.0f,
//                 C, N);
// }

// REGISTER_GEMM_OP(OpenBLASGemmOp)