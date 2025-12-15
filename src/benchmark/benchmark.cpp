#include "benchmark.h"
#include "../common/matrix_buffer.h"
#include "ops/gemm_op.h"

BenchResult bench_gemm(GemmOp *op,
                       const float *A, const float *B, float *C,
                       int M, int N, int K)
{
    printf("Benchmarking operator: %s\n", op->name().c_str());
    double total_ms = 0.0;

    for (int iter = 0; iter < ITERATIONS; ++iter)
    {
        memset(C, 0, static_cast<std::size_t>(M) * static_cast<std::size_t>(N) * sizeof(float));
        auto t0 = std::chrono::high_resolution_clock::now();
        op->run(A, B, C, M, N, K);
        auto t1 = std::chrono::high_resolution_clock::now();
        total_ms += std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    BenchResult r;
    r.ms = total_ms / ITERATIONS;
    return r;
}