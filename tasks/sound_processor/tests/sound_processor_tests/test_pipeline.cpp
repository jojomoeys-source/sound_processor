#include <catch2/catch_test_macros.hpp>

#include "converter/converter.h"
#include "filters/ampl_filter.h"
#include "waveform/waveform.h"

#include <cstdint>
#include <vector>

static Waveform make_waveform(std::vector<int16_t> samples) {
    Waveform waveform(samples.size(), 44100, 1, 16);
    for (size_t i = 0; i < samples.size(); ++i) {
        waveform.set_sample_at(i, samples[i]);
    }
    return waveform;
}

TEST_CASE("Converter creates and applies pipeline", "[converter]") {
    CmdLineArgs2PipelineConverter converter;
    converter.add_filter_producer("ampl", [](const FilterDescriptor& fd) -> IFilter* {
        (void)fd;
        return new AmplFilter(2.0);
    });

    Pipeline pipeline = converter.create_pipeline({FilterDescriptor{"ampl", {"2.0"}}});
    REQUIRE(pipeline.get_filter_count() == 1);

    Waveform waveform = make_waveform({10, -20});
    pipeline.apply(waveform);

    CHECK(waveform.get_sample_at(0) == 20);
    CHECK(waveform.get_sample_at(1) == -40);
}
