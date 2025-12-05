#pragma once

#include <string>

#include "common/dtype.h"

class GemmOp
{
public:
    virtual std::string name() const = 0;
    virtual bool supports_dtype(DataType dtype) const
    {
        return dtype == DataType::Float32;
    }
    virtual void run(const void *A, const void *B, void *C,
                     int M, int N, int K, DataType dtype) = 0;
    virtual ~GemmOp() {}
};