#pragma once
#include "gemm_op.h"

class OpenBLASGemmOp : public GemmOp
{
public:
    std::string name() const override { return "openblas"; }
    void run(const float *A, const float *B, float *C,
             int M, int N, int K) override;
};