#pragma once

#include <chrono>

struct BenchResult
{
    double ms;
};

BenchResult bench_gemm(class GemmOp *op,
                       const float *A, const float *B, float *C,
                       int M, int N, int K);