#include "benchmark.h"

#include "ops/gemm_op.h"

BenchResult bench_gemm(GemmOp *op,
                       const float *A, const float *B, float *C,
                       int M, int N, int K)
{
    for (int i = 0; i < 3; i++)
        op->run(A, B, C, M, N, K);

    auto t0 = std::chrono::high_resolution_clock::now();
    op->run(A, B, C, M, N, K);
    auto t1 = std::chrono::high_resolution_clock::now();

    BenchResult r;
    r.ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return r;
}