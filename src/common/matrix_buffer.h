#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>
#include <iostream>
#if defined(_MSC_VER)
#include <malloc.h>
#endif

class MatrixBuffer
{
public:
    MatrixBuffer() noexcept = default;
    MatrixBuffer(float *ptr, std::size_t size, std::size_t alignment) noexcept
        : ptr_(ptr), size_(size), alignment_(alignment)
    {
    }

    ~MatrixBuffer()
    {
        reset();
    }

    MatrixBuffer(const MatrixBuffer &) = delete;
    MatrixBuffer &operator=(const MatrixBuffer &) = delete;

    MatrixBuffer(MatrixBuffer &&other) noexcept
        : ptr_(other.ptr_), size_(other.size_), alignment_(other.alignment_)
    {
        other.ptr_ = nullptr;
        other.size_ = 0;
        other.alignment_ = 0;
    }

    MatrixBuffer &operator=(MatrixBuffer &&other) noexcept
    {
        if (this != &other)
        {
            reset();
            ptr_ = other.ptr_;
            size_ = other.size_;
            alignment_ = other.alignment_;
            other.ptr_ = nullptr;
            other.size_ = 0;
            other.alignment_ = 0;
        }
        return *this;
    }

    static MatrixBuffer allocate(std::size_t count, std::size_t alignment = 2 * 1024 * 1024)
    {
        if (count == 0)
        {
            return MatrixBuffer(nullptr, 0, alignment);
        }
        if ((alignment & (alignment - 1)) != 0)
        {
            throw std::invalid_argument("Alignment must be a power of two");
        }
        if (alignment < alignof(float))
        {
            alignment = alignof(float);
        }
        float *ptr = allocate_raw(count, alignment);
        return MatrixBuffer(ptr, count, alignment);
    }

    float *data() noexcept { return ptr_; }
    const float *data() const noexcept { return ptr_; }
    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    float &operator[](std::size_t idx) noexcept { return ptr_[idx]; }
    const float &operator[](std::size_t idx) const noexcept { return ptr_[idx]; }

    void reset() noexcept
    {
        if (ptr_ != nullptr)
        {
            release(ptr_);
            ptr_ = nullptr;
        }
        size_ = 0;
        alignment_ = 0;
    }

    void convert_to_column_major(int M, int N)
    {
        if (ptr_ == nullptr || size_ != static_cast<std::size_t>(M) * static_cast<std::size_t>(N))
        {
            throw std::runtime_error("Invalid matrix dimensions for conversion");
        }

        MatrixBuffer temp = MatrixBuffer::allocate(size_, alignment_);

        for (int j = 0; j < N; ++j)
        {
            for (int i = 0; i < M; ++i)
            {
                temp[j * M + i] = ptr_[i * N + j];
            }
        }

        std::swap(ptr_, temp.ptr_);
        is_column_major_ = true;

        // temp will release the old memory in its destructor
    }

    void print(std::size_t rows, std::size_t cols, std::ostream &os = std::cout) const
    {
        if (ptr_ == nullptr || size_ != rows * cols)
        {
            throw std::runtime_error("Invalid matrix dimensions for printing");
        }
        if (is_column_major_)
        {
            // Print column-major matrix
            for (std::size_t i = 0; i < rows; ++i)
            {
                for (std::size_t j = 0; j < cols; ++j)
                {
                    os << ptr_[j * rows + i] << " ";
                }
                os << "\n";
            }
            return;
        }

        for (std::size_t i = 0; i < rows; ++i)
        {
            for (std::size_t j = 0; j < cols; ++j)
            {
                os << ptr_[i * cols + j] << " ";
            }
            os << "\n";
        }
    }

private:
    static float *allocate_raw(std::size_t count, std::size_t alignment)
    {
        const std::size_t bytes = count * sizeof(float);
        void *mem = nullptr;
        const int rc = posix_memalign(&mem, alignment, bytes);
        if (rc != 0)
        {
            throw std::bad_alloc();
        }
        memset(mem, 0, bytes);
        return static_cast<float *>(mem);
    }

    static void release(void *ptr) noexcept
    {
        std::free(ptr);
    }

    float *ptr_ = nullptr;
    std::size_t size_ = 0;
    std::size_t alignment_ = 0;
    bool is_column_major_ = false;
};
