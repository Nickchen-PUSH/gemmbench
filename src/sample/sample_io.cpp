#include "sample_io.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace
{
constexpr std::uint32_t kSampleMagic = 0x47534d4d; // "GSMM"
constexpr std::uint32_t kSampleVersion = 1;

struct SampleFileHeader
{
    std::uint32_t magic;
    std::uint32_t version;
    std::uint32_t M;
    std::uint32_t N;
    std::uint32_t K;
};

void validate_dimensions(const SampleData &data)
{
    const auto expectedA = static_cast<std::size_t>(data.cfg.M) * static_cast<std::size_t>(data.cfg.K);
    const auto expectedB = static_cast<std::size_t>(data.cfg.K) * static_cast<std::size_t>(data.cfg.N);
    const auto expectedC = static_cast<std::size_t>(data.cfg.M) * static_cast<std::size_t>(data.cfg.N);
    if (data.A.size() != expectedA || data.B.size() != expectedB || data.C.size() != expectedC)
    {
        throw std::runtime_error("SampleData dimensions do not match matrix sizes");
    }
}
} // namespace

void save_sample_file(const std::string &path, const SampleData &data)
{
    validate_dimensions(data);

    const auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty())
    {
        std::filesystem::create_directories(parent);
    }

    std::ofstream ofs(path, std::ios::binary);
    if (!ofs)
    {
        throw std::runtime_error("Failed to open sample file for writing: " + path);
    }

    SampleFileHeader header{ kSampleMagic, kSampleVersion,
                             static_cast<std::uint32_t>(data.cfg.M),
                             static_cast<std::uint32_t>(data.cfg.N),
                             static_cast<std::uint32_t>(data.cfg.K) };
    ofs.write(reinterpret_cast<const char *>(&header), sizeof(header));

    const auto write_buffer = [&ofs](const MatrixBuffer &matrix) {
        if (matrix.empty())
            return;
        ofs.write(reinterpret_cast<const char *>(matrix.data()),
                  static_cast<std::streamsize>(matrix.size() * sizeof(float)));
    };

    write_buffer(data.A);
    write_buffer(data.B);
    write_buffer(data.C);
}

SampleData load_sample_file(const std::string &path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
    {
        throw std::runtime_error("Failed to open sample file for reading: " + path);
    }

    SampleFileHeader header{};
    ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
    if (!ifs || header.magic != kSampleMagic || header.version != kSampleVersion)
    {
        throw std::runtime_error("Invalid or corrupt sample file header: " + path);
    }

    SampleData data;
    data.cfg = SampleConfig{static_cast<int>(header.M), static_cast<int>(header.N), static_cast<int>(header.K)};

    const auto read_buffer = [&ifs, &path](std::size_t count) {
        MatrixBuffer buffer = MatrixBuffer::allocate(count);
        if (count == 0)
        {
            return buffer;
        }
        const auto bytes = static_cast<std::streamsize>(count * sizeof(float));
        if (!ifs.read(reinterpret_cast<char *>(buffer.data()), bytes))
        {
            throw std::runtime_error("Sample file is truncated: " + path);
        }
        return buffer;
    };

    const auto a_size = static_cast<std::size_t>(data.cfg.M) * static_cast<std::size_t>(data.cfg.K);
    const auto b_size = static_cast<std::size_t>(data.cfg.K) * static_cast<std::size_t>(data.cfg.N);
    const auto c_size = static_cast<std::size_t>(data.cfg.M) * static_cast<std::size_t>(data.cfg.N);

    data.A = read_buffer(a_size);
    data.B = read_buffer(b_size);
    data.C = read_buffer(c_size);

    return data;
}
