#pragma once

#include <string>

#include "../common/matrix_buffer.h"
#include "sample_generator.h"

struct SampleData
{
    SampleConfig cfg{};
    MatrixBuffer A;
    MatrixBuffer B;
    MatrixBuffer C;

    void convert_to_column_major();
};

void save_sample_file(const std::string &path, const SampleData &data);
SampleData load_sample_file(const std::string &path);
