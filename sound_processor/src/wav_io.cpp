#include "wav_io.h"
#include "wav_headers.h"

#include <cstring>
#include <fstream>
#include <stdexcept>

// ─── WavReader ────────────────────────────────────────────────────────────────

Waveform WavReader::read(const std::string& file_path) {
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

    // 2. fmt chunk (24 байт)
    FmtHeader fmt{};
    file.read(reinterpret_cast<char*>(&fmt), sizeof(fmt));
    if (!file) {
        throw std::runtime_error("Failed to read fmt header: " + file_path);
    }
    if (std::strncmp(fmt.subchunk_id, "fmt ", 4) != 0) {
        throw std::runtime_error("Missing fmt chunk: " + file_path);
    }
    if (fmt.audio_format != 1) {
        throw std::runtime_error(
            "Unsupported audio format (only PCM=1 is supported): " + file_path);
    }
    if (fmt.num_channels != 1) {
        throw std::runtime_error(
            "Unsupported channel count (only mono is supported): " + file_path);
    }
    if (fmt.bits_per_sample != 16) {
        throw std::runtime_error(
            "Unsupported bit depth (only 16-bit is supported): " + file_path);
    }

    // Если subchunk_size > 16 — пропускаем лишние байты расширения fmt
    if (fmt.subchunk_size > 16) {
        file.seekg(fmt.subchunk_size - 16, std::ios::cur);
    }

    // 3. Ищем data chunk, пропуская любые посторонние чанки (LIST, INFO и т.п.)
    DataHeader data{};
    while (true) {
        file.read(reinterpret_cast<char*>(&data), sizeof(data));
        if (!file) {
            throw std::runtime_error("Failed to read data header: " + file_path);
        }
        if (std::strncmp(data.subchunk_id, "data", 4) == 0) {
            break; // нашли нужный чанк
        }
        // Неизвестный чанк — пропускаем его тело
        file.seekg(data.subchunk_size, std::ios::cur);
        if (!file) {
            throw std::runtime_error("Unexpected EOF while searching for data chunk: " + file_path);
        }
    }

    // 4. Читаем выборки
    const size_t bytes_per_sample = fmt.bits_per_sample / 8; // = 2
    const size_t sample_count     = data.subchunk_size / bytes_per_sample;

    Waveform waveform(sample_count, fmt.sample_rate,
                      fmt.num_channels, fmt.bits_per_sample);

    file.read(reinterpret_cast<char*>(waveform.get_samples().data()),
              static_cast<std::streamsize>(data.subchunk_size));
    if (!file) {
        throw std::runtime_error("Failed to read audio samples: " + file_path);
    }

    return waveform;
}

// ─── WavWriter ────────────────────────────────────────────────────────────────

void WavWriter::write(const std::string& file_path, const Waveform& waveform) {
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + file_path);
    }

    const uint32_t sample_count    = static_cast<uint32_t>(waveform.get_sample_count());
    const uint16_t num_channels    = waveform.get_num_channels();
    const uint32_t sample_rate     = waveform.get_sample_rate();
    const uint16_t bits_per_sample = waveform.get_bits_per_sample();
    const uint16_t block_align     = static_cast<uint16_t>(num_channels * (bits_per_sample / 8));
    const uint32_t byte_rate       = sample_rate * block_align;
    const uint32_t data_size       = sample_count * block_align;

    // 1. RIFF chunk
    RiffHeader riff{};
    std::memcpy(riff.chunk_id, "RIFF", 4);
    riff.chunk_size = 36 + data_size;   // размер файла минус 8 байт
    std::memcpy(riff.format, "WAVE", 4);
    file.write(reinterpret_cast<const char*>(&riff), sizeof(riff));

    // 2. fmt chunk
    FmtHeader fmt{};
    std::memcpy(fmt.subchunk_id, "fmt ", 4);
    fmt.subchunk_size  = 16;        // для PCM всегда 16
    fmt.audio_format   = 1;         // PCM
    fmt.num_channels   = num_channels;
    fmt.sample_rate    = sample_rate;
    fmt.byte_rate      = byte_rate;
    fmt.block_align    = block_align;
    fmt.bits_per_sample = bits_per_sample;
    file.write(reinterpret_cast<const char*>(&fmt), sizeof(fmt));

    // 3. data chunk
    DataHeader data{};
    std::memcpy(data.subchunk_id, "data", 4);
    data.subchunk_size = data_size;
    file.write(reinterpret_cast<const char*>(&data), sizeof(data));

    // 4. Выборки
    file.write(reinterpret_cast<const char*>(waveform.get_samples().data()),
               static_cast<std::streamsize>(data_size));

    if (!file) {
        throw std::runtime_error("Failed to write WAV file: " + file_path);
    }
}