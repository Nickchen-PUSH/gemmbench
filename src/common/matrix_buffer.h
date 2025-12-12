#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>
#include <stdexcept>

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

private:
    static float *allocate_raw(std::size_t count, std::size_t alignment)
    {
        const std::size_t bytes = count * sizeof(float);
#if defined(_MSC_VER)
        void *mem = _aligned_malloc(bytes, alignment);
        if (!mem)
        {
            throw std::bad_alloc();
        }
        return static_cast<float *>(mem);
#else
        void *mem = nullptr;
        const int rc = posix_memalign(&mem, alignment, bytes);
        if (rc != 0)
        {
            throw std::bad_alloc();
        }
        return static_cast<float *>(mem);
#endif
    }

    static void release(void *ptr) noexcept
    {
#if defined(_MSC_VER)
        _aligned_free(ptr);
#else
        std::free(ptr);
#endif
    }

    float *ptr_ = nullptr;
    std::size_t size_ = 0;
    std::size_t alignment_ = 0;
};
