#include <catch2/catch_test_macros.hpp>

#include "wav/wav_io.h"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace {

void remove_if_exists(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

Waveform make_waveform(std::vector<int16_t> samples) {
    Waveform waveform(samples.size(), 44100, 1, 16);
    for (size_t i = 0; i < samples.size(); ++i) {
        waveform.set_sample_at(i, samples[i]);
    }
    return waveform;
}

} // namespace

TEST_CASE("WavWriter and WavReader round-trip PCM WAV", "[wav_io]") {
    const auto path = std::filesystem::temp_directory_path() / "sound_processor_roundtrip.wav";
    remove_if_exists(path);

    WavWriter::write(path.string(), make_waveform({1000, -2000, 3000}));

    const Waveform read_back = WavReader::read(path.string());
    REQUIRE(read_back.get_sample_count() == 3);
    CHECK(read_back.get_sample_rate() == 44100);
    CHECK(read_back.get_num_channels() == 1);
    CHECK(read_back.get_bits_per_sample() == 16);
    CHECK(read_back.get_sample_at(0) == 1000);
    CHECK(read_back.get_sample_at(1) == -2000);
    CHECK(read_back.get_sample_at(2) == 3000);

    remove_if_exists(path);
}

TEST_CASE("WavWriter and WavReader round-trip empty WAV", "[wav_io]") {
    const auto path = std::filesystem::temp_directory_path() / "sound_processor_empty.wav";
    remove_if_exists(path);

    WavWriter::write(path.string(), Waveform{});

    const Waveform read_back = WavReader::read(path.string());
    CHECK(read_back.get_sample_count() == 0);
    CHECK(read_back.get_sample_rate() == 44100);

    remove_if_exists(path);
}
