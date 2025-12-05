#pragma once

#include <cstddef>
#include <vector>

#include "common/dtype.h"

struct VerifyResult
{
    bool ok;
    double max_abs_error;
    double max_rel_error;
    std::size_t mismatch_index;
    int mismatch_row;
    int mismatch_col;
    float expected_value;
    float actual_value;
    double mismatch_abs_error;
    double mismatch_rel_error;
};

VerifyResult verify_result(const std::vector<float> &expected,
                           const void *actual,
                           int M,
                           int N,
                           DataType dtype,
                           double atol = 1e-3,
                           double rtol = 1e-2);
