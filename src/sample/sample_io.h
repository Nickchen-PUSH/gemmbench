#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "sample_generator.h"

struct SampleData
{
    SampleConfig cfg;
    std::vector<std::uint8_t> A;
    std::vector<std::uint8_t> B;
    std::vector<float> C;
};

void save_sample_file(const std::string &path, const SampleData &data);
SampleData load_sample_file(const std::string &path);
