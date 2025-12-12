#include "reference_gemm.h"

#include <cstddef>
#include <stdexcept>

MatrixBuffer compute_reference_c(const SampleConfig &cfg,
                                 const MatrixBuffer &A,
                                 const MatrixBuffer &B)
{
    const auto expectedA = static_cast<std::size_t>(cfg.M) * static_cast<std::size_t>(cfg.K);
    const auto expectedB = static_cast<std::size_t>(cfg.K) * static_cast<std::size_t>(cfg.N);
    if (A.size() != expectedA || B.size() != expectedB)
    {
        throw std::runtime_error("Input matrices have mismatched sizes for reference GEMM");
    }

    MatrixBuffer C = MatrixBuffer::allocate(static_cast<std::size_t>(cfg.M) * static_cast<std::size_t>(cfg.N));
    const float *a_ptr = A.data();
    const float *b_ptr = B.data();
    float *c_ptr = C.data();

    for (int i = 0; i < cfg.M; ++i)
    {
        for (int j = 0; j < cfg.N; ++j)
        {
            float sum = 0.0f;
            for (int k = 0; k < cfg.K; ++k)
            {
                sum += a_ptr[i * cfg.K + k] * b_ptr[k * cfg.N + j];
            }
            c_ptr[i * cfg.N + j] = sum;
        }
    }

    return C;
}
