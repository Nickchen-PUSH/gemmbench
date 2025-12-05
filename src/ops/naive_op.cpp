#include "naive_op.h"
#include "registry.h"

void NaiveGemmOp::run(const void *A, const void *B, void *C,
                      int M, int N, int K, DataType dtype)
{
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            float sum = 0.f;
            for (int k = 0; k < K; k++)
            {
                const auto idx_a = static_cast<std::size_t>(i) * static_cast<std::size_t>(K) + static_cast<std::size_t>(k);
                const auto idx_b = static_cast<std::size_t>(k) * static_cast<std::size_t>(N) + static_cast<std::size_t>(j);
                sum += load_value(A, idx_a, dtype) * load_value(B, idx_b, dtype);
            }
            const auto idx_c = static_cast<std::size_t>(i) * static_cast<std::size_t>(N) + static_cast<std::size_t>(j);
            store_value(C, idx_c, dtype, sum);
        }
    }
}

// 一行注册
REGISTER_GEMM_OP(NaiveGemmOp)