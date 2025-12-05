#include "sample_io.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "../common/dtype.h"

namespace
{
constexpr std::uint32_t kSampleMagic = 0x47534d4d; // "GSMM" magic tag
constexpr std::uint32_t kSampleVersion = 2;

struct SampleFileHeaderBase
{
    std::uint32_t magic;
    std::uint32_t version;
    std::uint32_t M;
    std::uint32_t N;
    std::uint32_t K;
};

DataType decode_dtype(std::uint32_t raw)
{
    switch (static_cast<DataType>(raw))
    {
    case DataType::Float32:
    case DataType::Float16:
    case DataType::BFloat16:
        return static_cast<DataType>(raw);
    default:
        throw std::runtime_error("Sample file contains unsupported dtype tag");
    }
}

void validate_dimensions(const SampleData &data)
{
    const auto elem_size = dtype_size(data.cfg.dtype);
    const auto expectedA = static_cast<std::size_t>(data.cfg.M) * static_cast<std::size_t>(data.cfg.K) * elem_size;
    const auto expectedB = static_cast<std::size_t>(data.cfg.K) * static_cast<std::size_t>(data.cfg.N) * elem_size;
    const auto expectedC = static_cast<std::size_t>(data.cfg.M) * static_cast<std::size_t>(data.cfg.N);
    if (data.A.size() != expectedA || data.B.size() != expectedB || data.C.size() != expectedC)
    {
        throw std::runtime_error("SampleData dimensions do not match matrix sizes");
    }
}
} // namespace

void save_sample_file(const std::string &path, const SampleData &data) {
    validate_dimensions(data);

    const auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open sample file for writing: " + path);
    }

    SampleFileHeaderBase header{ kSampleMagic, kSampleVersion,
                                 static_cast<std::uint32_t>(data.cfg.M),
                                 static_cast<std::uint32_t>(data.cfg.N),
                                 static_cast<std::uint32_t>(data.cfg.K) };
    ofs.write(reinterpret_cast<const char *>(&header), sizeof(header));

    const std::uint32_t dtype_tag = static_cast<std::uint32_t>(data.cfg.dtype);
    ofs.write(reinterpret_cast<const char *>(&dtype_tag), sizeof(dtype_tag));

    const auto write_bytes = [&ofs](const void *ptr, std::size_t bytes) {
        if (bytes == 0)
            return;
        ofs.write(reinterpret_cast<const char *>(ptr), static_cast<std::streamsize>(bytes));
    };

    write_bytes(data.A.data(), data.A.size());
    write_bytes(data.B.data(), data.B.size());
    write_bytes(data.C.data(), data.C.size() * sizeof(float));
}

SampleData load_sample_file(const std::string &path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open sample file for reading: " + path);
    }

    SampleFileHeaderBase header{};
    ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
    if (!ifs || header.magic != kSampleMagic)
    {
        throw std::runtime_error("Invalid or corrupt sample file header: " + path);
    }

    std::uint32_t dtype_tag = static_cast<std::uint32_t>(DataType::Float32);
    if (header.version == 1)
    {
        // legacy files do not encode dtype, assume float32
        dtype_tag = static_cast<std::uint32_t>(DataType::Float32);
    }
    else if (header.version == kSampleVersion)
    {
        if (!ifs.read(reinterpret_cast<char *>(&dtype_tag), sizeof(dtype_tag)))
        {
            throw std::runtime_error("Sample file missing dtype information: " + path);
        }
    }
    else
    {
        throw std::runtime_error("Unsupported sample file version: " + std::to_string(header.version));
    }

    SampleData data;
    data.cfg = SampleConfig{static_cast<int>(header.M), static_cast<int>(header.N), static_cast<int>(header.K)};
    data.cfg.dtype = decode_dtype(dtype_tag);

    const auto elem_size = dtype_size(data.cfg.dtype);
    const auto a_bytes = static_cast<std::size_t>(data.cfg.M) * static_cast<std::size_t>(data.cfg.K) * elem_size;
    const auto b_bytes = static_cast<std::size_t>(data.cfg.K) * static_cast<std::size_t>(data.cfg.N) * elem_size;
    const auto c_elems = static_cast<std::size_t>(data.cfg.M) * static_cast<std::size_t>(data.cfg.N);

    data.A.resize(a_bytes);
    data.B.resize(b_bytes);
    data.C.resize(c_elems);

    const auto read_bytes = [&ifs, &path](void *dst, std::size_t bytes) {
        if (bytes == 0)
            return;
        if (!ifs.read(reinterpret_cast<char *>(dst), static_cast<std::streamsize>(bytes)))
        {
            throw std::runtime_error("Sample file is truncated: " + path);
        }
    };

    read_bytes(data.A.data(), data.A.size());
    read_bytes(data.B.data(), data.B.size());
    read_bytes(data.C.data(), data.C.size() * sizeof(float));

    return data;
}
