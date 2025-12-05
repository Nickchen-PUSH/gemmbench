#include "sample_generator.h"

#include <random>

#include "../common/dtype.h"

namespace
{
std::vector<std::uint8_t> generate_matrix(const SampleConfig &cfg,
                                          std::uint32_t seed)
{
    const std::size_t elements = static_cast<std::size_t>(cfg.M) * static_cast<std::size_t>(cfg.K);
    const std::size_t bytes = elements * dtype_size(cfg.dtype);
    std::vector<std::uint8_t> buffer(bytes);

    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (std::size_t idx = 0; idx < elements; ++idx)
    {
        store_value(buffer.data(), idx, cfg.dtype, dist(gen));
    }

    return buffer;
}
} // namespace

std::vector<std::uint8_t> SampleGenerator::generateA(const SampleConfig &cfg)
{
    return generate_matrix(cfg, 123);
}

std::vector<std::uint8_t> SampleGenerator::generateB(const SampleConfig &cfg)
{
    SampleConfig transposed = cfg;
    transposed.M = cfg.K;
    transposed.K = cfg.N;
    return generate_matrix(transposed, 456);
}