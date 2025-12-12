#pragma once

#include "../common/matrix_buffer.h"
#include "sample_generator.h"

MatrixBuffer compute_reference_c(const SampleConfig &cfg,
                                 const MatrixBuffer &A,
                                 const MatrixBuffer &B);
