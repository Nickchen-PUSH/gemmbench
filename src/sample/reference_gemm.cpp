#include "reference_gemm.h"

#include <cstddef>
#include <stdexcept>

#include "../common/dtype.h"

std::vector<float> compute_reference_c(const SampleConfig &cfg,
                                       const std::vector<std::uint8_t> &A,
                                       const std::vector<std::uint8_t> &B)
{
    const auto expectedA = static_cast<std::size_t>(cfg.M) * static_cast<std::size_t>(cfg.K) * dtype_size(cfg.dtype);
    const auto expectedB = static_cast<std::size_t>(cfg.K) * static_cast<std::size_t>(cfg.N) * dtype_size(cfg.dtype);
    if (A.size() != expectedA || B.size() != expectedB)
    {
        throw std::runtime_error("Input matrices have mismatched sizes for reference GEMM");
    }

    std::vector<float> C(static_cast<std::size_t>(cfg.M) * static_cast<std::size_t>(cfg.N), 0.0f);

    for (int i = 0; i < cfg.M; ++i)
    {
        for (int j = 0; j < cfg.N; ++j)
        {
            float sum = 0.0f;
            for (int k = 0; k < cfg.K; ++k)
            {
                const float a = load_value(A.data(), static_cast<std::size_t>(i) * cfg.K + k, cfg.dtype);
                const float b = load_value(B.data(), static_cast<std::size_t>(k) * cfg.N + j, cfg.dtype);
                sum += a * b;
            }
            C[i * cfg.N + j] = sum;
        }
    }

    return C;
}
