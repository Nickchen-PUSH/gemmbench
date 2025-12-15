#pragma once
#define ITERATIONS 1

#include <chrono>
#include <cstring>
struct BenchResult
{
    double ms;
};

BenchResult bench_gemm(class GemmOp *op,
                       const float *A, const float *B, float *C,
                       int M, int N, int K);