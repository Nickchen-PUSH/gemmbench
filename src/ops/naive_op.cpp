#include "naive_op.h"
#include "registry.h"

void NaiveGemmOp::run(const float *A, const float *B, float *C,
                      int M, int N, int K)
{
    for (int i = 0; i < M; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            float sum = 0.f;
            for (int k = 0; k < K; ++k)
            {
                sum += A[i * K + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

// 一行注册
REGISTER_GEMM_OP(NaiveGemmOp)