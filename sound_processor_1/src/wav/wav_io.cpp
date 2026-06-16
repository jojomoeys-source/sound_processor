#include "wav/wav_io.h"
#include "wav/wav_headers.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace {

constexpr uint16_t kExpectedSampleRate = 44100;
constexpr uint16_t kExpectedChannels = 1;
constexpr uint16_t kExpectedBitsPerSample = 16;
constexpr uint16_t kExpectedBlockAlign = kExpectedChannels * (kExpectedBitsPerSample / 8);
constexpr uint32_t kExpectedByteRate = kExpectedSampleRate * kExpectedBlockAlign;

uintmax_t file_size_or_throw(const std::filesystem::path& file_path) {
    std::error_code ec;
    const uintmax_t size = std::filesystem::file_size(file_path, ec);
    if (ec) {
        throw std::runtime_error("Cannot determine file size: " + file_path.string());
    }
    return size;
}

void require_bytes_available(std::istream& file,
                             uintmax_t file_size,
                             uintmax_t bytes,
                             const std::filesystem::path& file_path) {
    const std::streampos current_pos = file.tellg();
    if (current_pos < std::streampos{0}) {
        throw std::runtime_error("Cannot determine current file position: " + file_path.string());
    }

    const auto current_offset = static_cast<std::streamoff>(current_pos);
    const auto bytes_offset = static_cast<std::streamoff>(bytes);
    if (bytes_offset < 0 || current_offset < 0 ||
        bytes > static_cast<uintmax_t>(std::numeric_limits<std::streamoff>::max())) {
        throw std::runtime_error("Invalid WAV chunk position: " + file_path.string());
    }

    const auto end_offset = current_offset + bytes_offset;
    if (end_offset < current_offset ||
        static_cast<uintmax_t>(end_offset) > file_size) {
        throw std::runtime_error("Unexpected EOF while reading WAV chunk: " + file_path.string());
    }
}

void skip_bytes(std::istream& file,
                uint32_t size,
                uintmax_t file_size,
                const std::filesystem::path& file_path) {
    require_bytes_available(file, file_size, size, file_path);
    file.seekg(size, std::ios::cur);

    if (size % 2 == 1) {
        require_bytes_available(file, file_size, 1, file_path);
        file.seekg(1, std::ios::cur);
    }
}

} // namespace

// ─── WavReader ────────────────────────────────────────────────────────────────

Waveform WavReader::read(const std::string& file_path) {
    const std::filesystem::path path(file_path);
    const uintmax_t total_size = file_size_or_throw(path);
    if (total_size < sizeof(RiffHeader)) {
        throw std::runtime_error("WAV file is too small: " + file_path);
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for reading: " + file_path);
    }

    // 1. RIFF chunk (12 байт)
    RiffHeader riff{};
    file.read(reinterpret_cast<char*>(&riff), sizeof(riff));
    if (!file) {
        throw std::runtime_error("Failed to read RIFF header: " + file_path);
    }
    if (std::strncmp(riff.chunk_id, "RIFF", 4) != 0) {
        throw std::runtime_error("Not a RIFF file: " + file_path);
    }
    if (std::strncmp(riff.format, "WAVE", 4) != 0) {
        throw std::runtime_error("Not a WAVE file: " + file_path);
    }
    if (riff.chunk_size != total_size - 8) {
        throw std::runtime_error("Invalid RIFF chunk size: " + file_path);
    }

    // 2. fmt chunk
    FmtHeader fmt{};
    require_bytes_available(file, total_size, sizeof(fmt), path);
    file.read(reinterpret_cast<char*>(&fmt), sizeof(fmt));
    if (!file) {
        throw std::runtime_error("Failed to read fmt header: " + file_path);
    }
    if (std::strncmp(fmt.subchunk_id, "fmt ", 4) != 0) {
        throw std::runtime_error("Missing fmt chunk: " + file_path);
    }
    if (fmt.subchunk_size < kExpectedBitsPerSample) {
        throw std::runtime_error("Invalid fmt chunk size: " + file_path);
    }
    if (fmt.audio_format != 1) {
        throw std::runtime_error(
            "Unsupported audio format (only PCM=1 is supported): " + file_path);
    }
    if (fmt.num_channels != kExpectedChannels) {
        throw std::runtime_error(
            "Unsupported channel count (only mono is supported): " + file_path);
    }
    if (fmt.sample_rate != kExpectedSampleRate) {
        throw std::runtime_error(
            "Unsupported sample rate (only 44100 Hz is supported): " + file_path);
    }
    if (fmt.block_align != kExpectedBlockAlign) {
        throw std::runtime_error("Invalid WAV block_align: " + file_path);
    }
    if (fmt.byte_rate != kExpectedByteRate) {
        throw std::runtime_error("Invalid WAV byte_rate: " + file_path);
    }
    if (fmt.bits_per_sample != kExpectedBitsPerSample) {
        throw std::runtime_error(
            "Unsupported bit depth (only 16-bit is supported): " + file_path);
    }

    if (fmt.subchunk_size > kExpectedBitsPerSample) {
        skip_bytes(file, fmt.subchunk_size - kExpectedBitsPerSample, total_size, path);
    }

    // 3. Ищем data chunk, пропуская любые посторонние чанки (LIST, INFO и т.п.)
    DataHeader data{};
    while (true) {
        require_bytes_available(file, total_size, sizeof(data), path);
        file.read(reinterpret_cast<char*>(&data), sizeof(data));
        if (!file) {
            throw std::runtime_error("Failed to read data header: " + file_path);
        }
        if (std::strncmp(data.subchunk_id, "data", 4) == 0) {
            break;
        }
        skip_bytes(file, data.subchunk_size, total_size, path);
    }

    const std::streampos data_payload_pos = file.tellg();
    if (data_payload_pos < std::streampos{0}) {
        throw std::runtime_error("Cannot determine data chunk position: " + file_path);
    }
    const uintmax_t remaining_after_header =
        total_size - static_cast<uintmax_t>(data_payload_pos);
    if (data.subchunk_size > remaining_after_header) {
        throw std::runtime_error("Data chunk size exceeds file size: " + file_path);
    }

    // 4. Читаем выборки
    const size_t bytes_per_sample = fmt.bits_per_sample / 8;
    if (data.subchunk_size % bytes_per_sample != 0) {
        throw std::runtime_error("Data chunk size is not aligned to sample size: " + file_path);
    }

    const size_t sample_count = data.subchunk_size / bytes_per_sample;
    Waveform waveform(sample_count, fmt.sample_rate,
                      fmt.num_channels, fmt.bits_per_sample);

    if (data.subchunk_size > 0) {
        file.read(reinterpret_cast<char*>(waveform.get_samples().data()),
                  static_cast<std::streamsize>(data.subchunk_size));
        if (!file) {
            throw std::runtime_error("Failed to read audio samples: " + file_path);
        }
    }

    return waveform;
}

// ─── WavWriter ────────────────────────────────────────────────────────────────

void WavWriter::write(const std::string& file_path, const Waveform& waveform) {
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + file_path);
    }

    const uint64_t sample_count    = waveform.get_sample_count();
    const uint16_t num_channels    = waveform.get_num_channels();
    const uint32_t sample_rate     = waveform.get_sample_rate();
    const uint16_t bits_per_sample = waveform.get_bits_per_sample();

    if (num_channels != kExpectedChannels) {
        throw std::invalid_argument("WavWriter: only mono WAV files are supported");
    }
    if (sample_rate != kExpectedSampleRate) {
        throw std::invalid_argument("WavWriter: only 44100 Hz WAV files are supported");
    }
    if (bits_per_sample != kExpectedBitsPerSample) {
        throw std::invalid_argument("WavWriter: only 16-bit WAV files are supported");
    }

    const uint32_t block_align = static_cast<uint32_t>(num_channels * (bits_per_sample / 8));
    const uint32_t byte_rate   = sample_rate * block_align;
    const uint64_t data_size   = sample_count * static_cast<uint64_t>(block_align);
    const uint64_t riff_size   = 36 + data_size;

    if (data_size > std::numeric_limits<uint32_t>::max() ||
        riff_size > std::numeric_limits<uint32_t>::max()) {
        throw std::invalid_argument("WavWriter: WAV file is too large");
    }

    // 1. RIFF chunk
    RiffHeader riff{};
    std::memcpy(riff.chunk_id, "RIFF", 4);
    riff.chunk_size = static_cast<uint32_t>(riff_size);
    std::memcpy(riff.format, "WAVE", 4);
    file.write(reinterpret_cast<const char*>(&riff), sizeof(riff));

    // 2. fmt chunk
    FmtHeader fmt{};
    std::memcpy(fmt.subchunk_id, "fmt ", 4);
    fmt.subchunk_size  = 16;
    fmt.audio_format   = 1;
    fmt.num_channels   = num_channels;
    fmt.sample_rate    = sample_rate;
    fmt.byte_rate      = byte_rate;
    fmt.block_align    = block_align;
    fmt.bits_per_sample = bits_per_sample;
    file.write(reinterpret_cast<const char*>(&fmt), sizeof(fmt));

    // 3. data chunk
    DataHeader data{};
    std::memcpy(data.subchunk_id, "data", 4);
    data.subchunk_size = static_cast<uint32_t>(data_size);
    file.write(reinterpret_cast<const char*>(&data), sizeof(data));

    // 4. Выборки
    if (data_size > 0) {
        file.write(reinterpret_cast<const char*>(waveform.get_samples().data()),
                   static_cast<std::streamsize>(data_size));
    }

    if (!file) {
        throw std::runtime_error("Failed to write WAV file: " + file_path);
    }
}
