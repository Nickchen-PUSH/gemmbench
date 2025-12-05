#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

enum class DataType : std::uint32_t
{
    Float32 = 0,
    Float16 = 1,
    BFloat16 = 2
};

inline constexpr std::size_t dtype_size(DataType dt)
{
    switch (dt)
    {
    case DataType::Float32:
        return sizeof(float);
    case DataType::Float16:
        return sizeof(std::uint16_t);
    case DataType::BFloat16:
        return sizeof(std::uint16_t);
    }
    return 0;
}

inline std::string dtype_to_string(DataType dt)
{
    switch (dt)
    {
    case DataType::Float32:
        return "float32";
    case DataType::Float16:
        return "float16";
    case DataType::BFloat16:
        return "bfloat16";
    }
    return "unknown";
}

inline DataType parse_dtype(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    if (value == "float32" || value == "fp32" || value == "f32")
    {
        return DataType::Float32;
    }
    if (value == "float16" || value == "fp16" || value == "f16" || value == "half")
    {
        return DataType::Float16;
    }
    if (value == "bfloat16" || value == "bf16")
    {
        return DataType::BFloat16;
    }

    throw std::invalid_argument("Unsupported dtype: " + value);
}

namespace detail
{
inline std::uint32_t float_to_bits(float value)
{
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

inline float bits_to_float(std::uint32_t bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

inline std::uint16_t float_to_fp16_bits(float value)
{
    const std::uint32_t f = float_to_bits(value);
    const std::uint32_t sign = (f >> 16U) & 0x8000U;
    std::int32_t exp = static_cast<std::int32_t>((f >> 23U) & 0xFFU) - 127 + 15;
    std::uint32_t mant = f & 0x7FFFFFU;

    if (exp <= 0)
    {
        if (exp < -10)
        {
            return static_cast<std::uint16_t>(sign);
        }
        mant |= 0x800000U;
        const std::uint32_t shift = static_cast<std::uint32_t>(1 - exp);
        std::uint32_t mant16 = mant >> (shift + 13U);
        if ((mant >> (shift + 12U)) & 1U)
        {
            mant16 += 1U;
        }
        return static_cast<std::uint16_t>(sign | mant16);
    }

    if (exp >= 31)
    {
        if (mant == 0)
        {
            return static_cast<std::uint16_t>(sign | 0x7C00U);
        }
        return static_cast<std::uint16_t>(sign | 0x7C00U | (mant >> 13U));
    }

    std::uint16_t half = static_cast<std::uint16_t>(sign | (static_cast<std::uint32_t>(exp) << 10U) | (mant >> 13U));
    if (mant & 0x00001000U)
    {
        half = static_cast<std::uint16_t>(half + 1U);
    }
    return half;
}

inline float fp16_bits_to_float(std::uint16_t value)
{
    const int sign = (value & 0x8000U) ? -1 : 1;
    const int exp = (value >> 10U) & 0x1FU;
    const int mant = value & 0x03FFU;

    if (exp == 0)
    {
        if (mant == 0)
        {
            return sign < 0 ? -0.0f : 0.0f;
        }
        const float frac = static_cast<float>(mant) / 1024.0f;
        const float scaled = std::ldexp(frac, -14);
        return sign < 0 ? -scaled : scaled;
    }

    if (exp == 31)
    {
        if (mant == 0)
        {
            return sign < 0 ? -std::numeric_limits<float>::infinity()
                            : std::numeric_limits<float>::infinity();
        }
        return std::numeric_limits<float>::quiet_NaN();
    }

    const float frac = 1.0f + static_cast<float>(mant) / 1024.0f;
    const float scaled = std::ldexp(frac, exp - 15);
    return sign < 0 ? -scaled : scaled;
}

inline std::uint16_t float_to_bf16_bits(float value)
{
    const std::uint32_t bits = float_to_bits(value);
    const std::uint32_t rounding = ((bits >> 16U) & 1U) + 0x7FFFU;
    return static_cast<std::uint16_t>((bits + rounding) >> 16U);
}

inline float bf16_bits_to_float(std::uint16_t value)
{
    const std::uint32_t bits = static_cast<std::uint32_t>(value) << 16U;
    return bits_to_float(bits);
}
} // namespace detail

inline float load_value(const void *base, std::size_t idx, DataType dtype)
{
    const auto *bytes = static_cast<const std::uint8_t *>(base);
    switch (dtype)
    {
    case DataType::Float32:
    {
        float value;
        std::memcpy(&value, bytes + idx * sizeof(float), sizeof(float));
        return value;
    }
    case DataType::Float16:
    {
        std::uint16_t value16;
        std::memcpy(&value16, bytes + idx * sizeof(std::uint16_t), sizeof(std::uint16_t));
        return detail::fp16_bits_to_float(value16);
    }
    case DataType::BFloat16:
    {
        std::uint16_t value16;
        std::memcpy(&value16, bytes + idx * sizeof(std::uint16_t), sizeof(std::uint16_t));
        return detail::bf16_bits_to_float(value16);
    }
    }
    return 0.0f;
}

inline void store_value(void *base, std::size_t idx, DataType dtype, float value)
{
    auto *bytes = static_cast<std::uint8_t *>(base);
    switch (dtype)
    {
    case DataType::Float32:
    {
        std::memcpy(bytes + idx * sizeof(float), &value, sizeof(float));
        break;
    }
    case DataType::Float16:
    {
        const std::uint16_t packed = detail::float_to_fp16_bits(value);
        std::memcpy(bytes + idx * sizeof(std::uint16_t), &packed, sizeof(packed));
        break;
    }
    case DataType::BFloat16:
    {
        const std::uint16_t packed = detail::float_to_bf16_bits(value);
        std::memcpy(bytes + idx * sizeof(std::uint16_t), &packed, sizeof(packed));
        break;
    }
    }
}