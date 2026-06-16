#include <catch2/catch_test_macros.hpp>

#include "app/application.h"
#include "filters/ampl_filter.h"
#include "converter/converter.h"
#include "filters/silence_filter.h"
#include "wav/wav_io.h"

#include <filesystem>
#include <string>
#include <vector>

static std::vector<char*> make_argv(std::vector<std::string>& args) {
    std::vector<char*> argv;
    argv.reserve(args.size());
    for (auto& arg : args) {
        argv.push_back(arg.data());
    }
    return argv;
}

TEST_CASE("CmdLineArgs2PipelineConverter: creates pipeline from descriptors", "[converter]") {
    CmdLineArgs2PipelineConverter converter;
    converter.add_filter_producer("ampl", [](const FilterDescriptor& fd) -> IFilter* {
        REQUIRE(fd.params.size() == 1);
        return new AmplFilter(2.0);
    });

    Pipeline pipeline = converter.create_pipeline({FilterDescriptor{"ampl", {"2.0"}}});
    REQUIRE(pipeline.get_filter_count() == 1);

    Waveform waveform(2, 44100, 1, 16);
    waveform.set_sample_at(0, 10);
    waveform.set_sample_at(1, -20);
    pipeline.apply(waveform);

    CHECK(waveform.get_sample_at(0) == 20);
    CHECK(waveform.get_sample_at(1) == -40);
}

TEST_CASE("CmdLineArgs2PipelineConverter: unknown filter throws", "[converter]") {
    CmdLineArgs2PipelineConverter converter;
    CHECK_THROWS_AS(
        converter.create_pipeline({FilterDescriptor{"unknown", {}}}),
        std::runtime_error);
}

TEST_CASE("CmdLineArgs2PipelineConverter: null producer is rejected", "[converter]") {
    CmdLineArgs2PipelineConverter converter;
    CHECK_THROWS_AS(
        converter.add_filter_producer("bad", nullptr),
        std::invalid_argument);
}

TEST_CASE("Application: generator ignores invalid input file", "[application]") {
    namespace fs = std::filesystem;

    const fs::path output_path =
        fs::temp_directory_path() / "sound_processor_generator_ignores_input.wav";
    fs::remove(output_path);

    Application app;
    app.configure();

    std::vector<std::string> args = {
        "sound_processor",
        "-i", "/definitely/missing/input.wav",
        "-o", output_path.string(),
        "-f", "generator", "sin", "440", "100"
    };
    std::vector<char*> argv = make_argv(args);

    const int result = app.start(static_cast<int>(argv.size()), argv.data());

    CHECK(result == 0);
    CHECK(fs::exists(output_path));

    Waveform waveform = WavReader::read(output_path.string());
    CHECK(waveform.get_sample_count() == 4410);
    CHECK(waveform.get_sample_rate() == 44100);

    fs::remove(output_path);
}
