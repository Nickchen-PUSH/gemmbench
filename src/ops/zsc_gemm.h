#pragma once
#include "gemm_op.h"

class ZscGemmOp : public GemmOp
{
public:
    std::string name() const override { return "zsc"; }
    void run(const float *A, const float *B, float *C,
             int M, int N, int K) override;
};