#include "sample_generator.h"

#include <random>

namespace
{
MatrixBuffer generate_matrix(int rows, int cols, std::uint32_t seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    const std::size_t size = static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols);
    auto buffer = MatrixBuffer::allocate(size);

    for (int i = 0; i < rows * cols; ++i)
    {
        buffer.data()[i] = dist(rng);
    }
    return buffer;
}
} // namespace

MatrixBuffer SampleGenerator::generateA(const SampleConfig &cfg)
{
    return generate_matrix(cfg.M, cfg.K, 123);
}

MatrixBuffer SampleGenerator::generateB(const SampleConfig &cfg)
{
    return generate_matrix(cfg.K, cfg.N, 456);
}