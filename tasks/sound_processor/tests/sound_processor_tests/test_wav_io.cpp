#include <catch2/catch_test_macros.hpp>

#include "wav/wav_io.h"
#include "wav/wav_headers.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

const std::filesystem::path kValidWavPath =
    std::filesystem::temp_directory_path() / "sound_processor_valid.wav";
const std::filesystem::path kInvalidRatePath =
    std::filesystem::temp_directory_path() / "sound_processor_invalid_rate.wav";

void remove_if_exists(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

Waveform make_waveform(std::vector<int16_t> samples, uint32_t rate = 44100) {
    Waveform waveform(samples.size(), rate, 1, 16);
    for (size_t i = 0; i < samples.size(); ++i) {
        waveform.set_sample_at(i, samples[i]);
    }
    return waveform;
}

void write_invalid_sample_rate_wav(const std::filesystem::path& path) {
    std::ofstream file(path, std::ios::binary);
    REQUIRE(file.is_open());

    RiffHeader riff{};
    std::memcpy(riff.chunk_id, "RIFF", 4);
    riff.chunk_size = 44;
    std::memcpy(riff.format, "WAVE", 4);

    FmtHeader fmt{};
    std::memcpy(fmt.subchunk_id, "fmt ", 4);
    fmt.subchunk_size = 16;
    fmt.audio_format = 1;
    fmt.num_channels = 1;
    fmt.sample_rate = 22050;
    fmt.byte_rate = 44100;
    fmt.block_align = 2;
    fmt.bits_per_sample = 16;

    DataHeader data{};
    std::memcpy(data.subchunk_id, "data", 4);
    data.subchunk_size = 2;

    const int16_t sample = 1000;

    file.write(reinterpret_cast<const char*>(&riff), sizeof(riff));
    file.write(reinterpret_cast<const char*>(&fmt), sizeof(fmt));
    file.write(reinterpret_cast<const char*>(&data), sizeof(data));
    file.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
}

void write_basic_wav(const std::filesystem::path& path,
                     uint32_t riff_size,
                     uint32_t byte_rate,
                     uint16_t block_align,
                     uint32_t data_size,
                     bool write_data) {
    std::ofstream file(path, std::ios::binary);
    REQUIRE(file.is_open());

    RiffHeader riff{};
    std::memcpy(riff.chunk_id, "RIFF", 4);
    riff.chunk_size = riff_size;
    std::memcpy(riff.format, "WAVE", 4);

    FmtHeader fmt{};
    std::memcpy(fmt.subchunk_id, "fmt ", 4);
    fmt.subchunk_size = 16;
    fmt.audio_format = 1;
    fmt.num_channels = 1;
    fmt.sample_rate = 44100;
    fmt.byte_rate = byte_rate;
    fmt.block_align = block_align;
    fmt.bits_per_sample = 16;

    DataHeader data{};
    std::memcpy(data.subchunk_id, "data", 4);
    data.subchunk_size = data_size;

    file.write(reinterpret_cast<const char*>(&riff), sizeof(riff));
    file.write(reinterpret_cast<const char*>(&fmt), sizeof(fmt));
    file.write(reinterpret_cast<const char*>(&data), sizeof(data));

    if (write_data) {
        const int16_t sample = 1000;
        file.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
    }
}

} // namespace

// Проверяет: WavWriter записывает корректный PCM mono 44100 Hz 16-bit WAV
TEST_CASE("WavWriter: writes valid PCM mono 44100 Hz 16-bit WAV", "[wav_io]") {
    remove_if_exists(kValidWavPath);

    Waveform waveform = make_waveform({1000, -2000, 3000});
    WavWriter::write(kValidWavPath.string(), waveform);

    Waveform read_back = WavReader::read(kValidWavPath.string());
    REQUIRE(read_back.get_sample_count() == 3);
    CHECK(read_back.get_sample_rate() == 44100);
    CHECK(read_back.get_num_channels() == 1);
    CHECK(read_back.get_bits_per_sample() == 16);
    CHECK(read_back.get_sample_at(0) == 1000);
    CHECK(read_back.get_sample_at(1) == -2000);
    CHECK(read_back.get_sample_at(2) == 3000);

    remove_if_exists(kValidWavPath);
}

// Проверяет: WavWriter записывает корректный пустой WAV
TEST_CASE("WavWriter: writes valid empty WAV", "[wav_io]") {
    remove_if_exists(kValidWavPath);

    WavWriter::write(kValidWavPath.string(), Waveform{});

    Waveform read_back = WavReader::read(kValidWavPath.string());
    CHECK(read_back.get_sample_count() == 0);
    CHECK(read_back.get_sample_rate() == 44100);

    remove_if_exists(kValidWavPath);
}

// Проверяет: WavWriter отклоняет waveform не с 44100 Hz
TEST_CASE("WavWriter: rejects non-44100 Hz waveform", "[wav_io]") {
    remove_if_exists(kValidWavPath);

    Waveform waveform = make_waveform({1000}, 22050);
    CHECK_THROWS_AS(WavWriter::write(kValidWavPath.string(), waveform), std::invalid_argument);

    remove_if_exists(kValidWavPath);
}

// Проверяет: WavReader отклоняет WAV не с 44100 Hz
TEST_CASE("WavReader: rejects non-44100 Hz WAV", "[wav_io]") {
    remove_if_exists(kInvalidRatePath);
    write_invalid_sample_rate_wav(kInvalidRatePath);

    CHECK_THROWS_AS(WavReader::read(kInvalidRatePath.string()), std::runtime_error);

    remove_if_exists(kInvalidRatePath);
}

// Проверяет: WavReader отклоняет некорректный RIFF chunk size
TEST_CASE("WavReader: rejects invalid RIFF chunk size", "[wav_io]") {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "sound_processor_bad_riff_size.wav";
    remove_if_exists(path);
    write_basic_wav(path, 1, 88200, 2, 2, true);

    CHECK_THROWS_AS(WavReader::read(path.string()), std::runtime_error);

    remove_if_exists(path);
}

// Проверяет: WavReader отклоняет некорректный byte_rate
TEST_CASE("WavReader: rejects invalid byte_rate", "[wav_io]") {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "sound_processor_bad_byte_rate.wav";
    remove_if_exists(path);
    write_basic_wav(path, 44, 1, 2, 2, true);

    CHECK_THROWS_AS(WavReader::read(path.string()), std::runtime_error);

    remove_if_exists(path);
}

// Проверяет: WavReader отклоняет некорректный block_align
TEST_CASE("WavReader: rejects invalid block_align", "[wav_io]") {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "sound_processor_bad_block_align.wav";
    remove_if_exists(path);
    write_basic_wav(path, 44, 88200, 1, 2, true);

    CHECK_THROWS_AS(WavReader::read(path.string()), std::runtime_error);

    remove_if_exists(path);
}

// Проверяет: WavReader отклоняет truncated data chunk без выделения declared samples
TEST_CASE("WavReader: rejects truncated data chunk without allocating declared samples", "[wav_io]") {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "sound_processor_truncated_data.wav";
    remove_if_exists(path);
    write_basic_wav(path, 44, 88200, 2, 100000, false);

    CHECK_THROWS_AS(WavReader::read(path.string()), std::runtime_error);

    remove_if_exists(path);
}
