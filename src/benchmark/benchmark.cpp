#include "benchmark.h"
#include "../common/matrix_buffer.h"
#include "ops/gemm_op.h"
#define ITERATIONS 10

BenchResult bench_gemm(GemmOp *op,
                       const float *A, const float *B, float *C,
                       int M, int N, int K)
{
    printf("Benchmarking operator: %s\n", op->name().c_str());

    auto t0 = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < ITERATIONS; ++iter)
    {
        op->run(A, B, C, M, N, K);
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    BenchResult r;
    r.ms = std::chrono::duration<double, std::milli>(t1 - t0).count() / ITERATIONS;
    return r;
}