#include "benchmark.h"

#include "ops/gemm_op.h"

BenchResult bench_gemm(GemmOp *op,
                       const void *A, const void *B, void *C,
                       int M, int N, int K,
                       DataType dtype)
{
    for (int i = 0; i < 3; i++)
    {
        op->run(A, B, C, M, N, K, dtype);
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    op->run(A, B, C, M, N, K, dtype);
    auto t1 = std::chrono::high_resolution_clock::now();

    BenchResult r;
    r.ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return r;
}