#pragma once
#include <vector>
#include <cstdint>

#include "../common/dtype.h"

struct SampleConfig
{
    int M;
    int N;
    int K;
    DataType dtype = DataType::Float32;
};

class SampleGenerator
{
public:
    std::vector<std::uint8_t> generateA(const SampleConfig &cfg);
    std::vector<std::uint8_t> generateB(const SampleConfig &cfg);
};