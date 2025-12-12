#pragma once

#include "../common/matrix_buffer.h"

struct SampleConfig
{
    int M;
    int N;
    int K;
};

class SampleGenerator
{
public:
    MatrixBuffer generateA(const SampleConfig &cfg);
    MatrixBuffer generateB(const SampleConfig &cfg);
};