#include "sample_generator.h"

#include <random>

MatrixBuffer generate_matrix(int rows, int cols, std::uint32_t seed, int pattern)
{
    MatrixBuffer mat = MatrixBuffer::allocate(static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols));
    float *data = mat.data();

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            std::size_t idx = static_cast<std::size_t>(i) * static_cast<std::size_t>(cols) + static_cast<std::size_t>(j);
            switch (pattern)
            {
            case RANDOM:
                data[idx] = dist(rng);
                break;
            case SEQUENTIAL:
                data[idx] = static_cast<float>(idx);
                break;
            case ONES:
                data[idx] = 1.0f;
                break;
            case ZEROS:
                data[idx] = 0.0f;
                break;
            case CUSTOM:
                if(i == 0){
                    data[idx] = 2.0f;
                } else{
                    data[idx] = 1.0f;
                }
                break;
            default:
                throw std::invalid_argument("Unknown pattern type");
            }
        }
    }

    return mat;
}