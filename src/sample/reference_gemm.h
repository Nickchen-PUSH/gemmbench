#pragma once

#include <vector>
#include <cstdint>

#include "sample_generator.h"

std::vector<float> compute_reference_c(const SampleConfig &cfg,
                                       const std::vector<std::uint8_t> &A,
                                       const std::vector<std::uint8_t> &B);
