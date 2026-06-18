#include <catch2/catch_test_macros.hpp>

#include "app/application.h"
#include "wav/wav_io.h"
#include "waveform/waveform.h"

#include <filesystem>
#include <string>
#include <vector>

namespace {

void remove_if_exists(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

int run_application(const std::vector<std::string>& args) {
    std::vector<std::string> storage = args;
    std::vector<char*> argv;
    argv.reserve(storage.size());
    for (auto& arg : storage) {
        argv.push_back(arg.data());
    }

    Application app;
    app.configure();
    return app.start(static_cast<int>(argv.size()), argv.data());
}

} // namespace

TEST_CASE("Application returns success without arguments", "[application]") {
    CHECK(run_application({"sound_processor"}) == 0);
}

TEST_CASE("Application returns error for bad arguments", "[application]") {
    CHECK(run_application({"sound_processor", "-f", "-i"}) == 1);
}

TEST_CASE("Application generates WAV without input file", "[application]") {
    const auto output = std::filesystem::temp_directory_path() / "sound_processor_app_generator.wav";
    remove_if_exists(output);

    CHECK(run_application({"sound_processor", "-o", output.string(),
                           "-f", "generator", "sin", "440", "100"}) == 0);

    const Waveform waveform = WavReader::read(output.string());
    CHECK(waveform.get_sample_count() == 4410);
    CHECK(waveform.get_sample_rate() == 44100);
    CHECK(waveform.get_num_channels() == 1);
    CHECK(waveform.get_bits_per_sample() == 16);

    remove_if_exists(output);
}

TEST_CASE("Application copies input to output without filters", "[application]") {
    const auto input = std::filesystem::temp_directory_path() / "sound_processor_app_input.wav";
    const auto output = std::filesystem::temp_directory_path() / "sound_processor_app_output.wav";
    remove_if_exists(input);
    remove_if_exists(output);

    Waveform source(3, 44100, 1, 16);
    source.set_sample_at(0, 1000);
    source.set_sample_at(1, -2000);
    source.set_sample_at(2, 3000);
    WavWriter::write(input.string(), source);

    CHECK(run_application({"sound_processor", "-i", input.string(),
                           "-o", output.string()}) == 0);

    const Waveform copied = WavReader::read(output.string());
    REQUIRE(copied.get_sample_count() == 3);
    CHECK(copied.get_sample_at(0) == 1000);
    CHECK(copied.get_sample_at(1) == -2000);
    CHECK(copied.get_sample_at(2) == 3000);

    remove_if_exists(input);
    remove_if_exists(output);
}
