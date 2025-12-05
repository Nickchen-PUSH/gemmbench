#pragma once
#include "gemm_op.h"

class NaiveGemmOp : public GemmOp
{
public:
    std::string name() const override { return "naive"; }
    bool supports_dtype(DataType) const override { return true; }

    void run(const void *A, const void *B, void *C,
             int M, int N, int K, DataType dtype) override;
};