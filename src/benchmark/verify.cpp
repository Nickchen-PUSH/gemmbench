#include "verify.h"

#include <cstddef>
#include <cmath>
#include <limits>
#include <stdexcept>

VerifyResult verify_result(const float *expected,
                           const float *actual,
                           int M,
                           int N,
                           double atol,
                           double rtol)
{
    const std::size_t total = static_cast<std::size_t>(M) * static_cast<std::size_t>(N);
    if ((total > 0) && (!expected || !actual))
    {
        throw std::runtime_error("Null matrix pointer provided for verification");
    }

    VerifyResult result{};
    result.ok = true;
    result.max_abs_error = 0.0;
    result.max_rel_error = 0.0;
    result.mismatch_index = std::numeric_limits<std::size_t>::max();
    result.mismatch_row = -1;
    result.mismatch_col = -1;
    result.expected_value = 0.0f;
    result.actual_value = 0.0f;
    result.mismatch_abs_error = 0.0;
    result.mismatch_rel_error = 0.0;

    for (std::size_t idx = 0; idx < total; ++idx)
    {
        const float exp_val = expected[idx];
        const float act_val = actual[idx];
        const double abs_err = std::abs(static_cast<double>(exp_val) - static_cast<double>(act_val));
        const double rel_err = abs_err / (std::abs(static_cast<double>(exp_val)) + 1e-12);

        if (abs_err > result.max_abs_error)
        {
            result.max_abs_error = abs_err;
        }
        if (rel_err > result.max_rel_error)
        {
            result.max_rel_error = rel_err;
        }

        const double tolerance = atol + rtol * std::abs(static_cast<double>(exp_val));
        if (abs_err > tolerance && result.ok)
        {
            result.ok = false;
            result.mismatch_index = idx;
            result.mismatch_row = static_cast<int>(idx / static_cast<std::size_t>(N));
            result.mismatch_col = static_cast<int>(idx % static_cast<std::size_t>(N));
            result.expected_value = exp_val;
            result.actual_value = act_val;
            result.mismatch_abs_error = abs_err;
            result.mismatch_rel_error = rel_err;
        }
    }

    return result;
}
