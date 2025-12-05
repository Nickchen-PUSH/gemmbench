#pragma once

#include <chrono>

#include "common/dtype.h"

struct BenchResult
{
    double ms;
};

BenchResult bench_gemm(class GemmOp *op,
                       const void *A, const void *B, void *C,
                       int M, int N, int K,
                       DataType dtype);