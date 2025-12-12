#pragma once

#include <string>

class GemmOp
{
public:
    virtual std::string name() const = 0;
    virtual void run(const float *A, const float *B, float *C,
                     int M, int N, int K) = 0;
    virtual ~GemmOp() {}
};