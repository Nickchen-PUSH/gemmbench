#pragma once

#include "../common/matrix_buffer.h"
#define RANDOM 0
#define SEQUENTIAL 1
#define ONES 2
#define ZEROS 3
#define CUSTOM 4

MatrixBuffer generate_matrix(int rows, int cols, std::uint32_t seed, int pattern = RANDOM);

struct SampleConfig
{
    int M;
    int N;
    int K;
};