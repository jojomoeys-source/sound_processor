#include <catch2/catch_test_macros.hpp>

#include "filter_producers/filter_producers.h"
#include "filters/ampl_filter.h"
#include "filters/generator_filters.h"
#include "filters/highpass_filter.h"
#include "filters/lowpass_filter.h"
#include "filters/mix_filter.h"
#include "filters/mute_filter.h"
#include "filters/normalize_filter.h"
#include "filters/silence_filter.h"
#include "filters/timestretch_filter.h"
#include "waveform/waveform.h"

#include <memory>
#include <utility>
#include <vector>

static Waveform make_waveform(std::vector<int16_t> samples, uint32_t rate = 44100) {
    Waveform w(samples.size(), rate, 1, 16);
    for (size_t i = 0; i < samples.size(); ++i) {
        w.set_sample_at(i, samples[i]);
    }
    return w;
}

TEST_CASE("AmplFilter scales and clamps samples", "[filters][ampl]") {
    Waveform w = make_waveform({1000, -1000, 32767, -32768});
    AmplFilter f(0.5);
    f.apply(w);

    CHECK(w.get_sample_at(0) == 500);
    CHECK(w.get_sample_at(1) == -500);
    CHECK(w.get_sample_at(2) == 16384);
    CHECK(w.get_sample_at(3) == -16384);
}

TEST_CASE("NormalizeFilter scales samples by peak", "[filters][normalize]") {
    Waveform w = make_waveform({0, 16383, -16383});
    NormalizeFilter f;
    f.apply(w);

    CHECK(w.get_sample_at(0) == 0);
    CHECK(w.get_sample_at(1) == 32767);
    CHECK(w.get_sample_at(2) == -32767);
}

TEST_CASE("MuteFilter zeroes selected range", "[filters][mute]") {
    Waveform w(10, 10, 1, 16);
    for (size_t i = 0; i < w.get_sample_count(); ++i) {
        w.set_sample_at(i, 100);
    }

    MuteFilter f(0.2, 0.5);
    f.apply(w);

    CHECK(w.get_sample_at(0) == 100);
    CHECK(w.get_sample_at(1) == 100);
    CHECK(w.get_sample_at(2) == 0);
    CHECK(w.get_sample_at(3) == 0);
    CHECK(w.get_sample_at(4) == 0);
    CHECK(w.get_sample_at(5) == 100);
}

TEST_CASE("SilenceFilter inserts silence", "[filters][silence]") {
    Waveform w = make_waveform({100, 200, 300}, 10);

    SilenceFilter f(SilenceFilter::Unit::Sec, 0.0, 0.1);
    f.apply(w);

    REQUIRE(w.get_sample_count() == 5);
    CHECK(w.get_sample_at(0) == 0);
    CHECK(w.get_sample_at(1) == 0);
    CHECK(w.get_sample_at(2) == 100);
    CHECK(w.get_sample_at(3) == 200);
    CHECK(w.get_sample_at(4) == 300);
}

TEST_CASE("LowpassFilter smooths spike", "[filters][lowpass]") {
    Waveform w = make_waveform({0, 0, 3000, 0, 0});

    LowpassFilter f(3);
    f.apply(w);

    CHECK(w.get_sample_at(1) == 1000);
    CHECK(w.get_sample_at(2) == 1000);
    CHECK(w.get_sample_at(3) == 1000);
}

TEST_CASE("HighpassFilter removes constant signal", "[filters][highpass]") {
    Waveform w = make_waveform({1000, 1000, 1000, 1000});

    HighpassFilter f(3);
    f.apply(w);

    for (size_t i = 0; i < w.get_sample_count(); ++i) {
        CHECK(w.get_sample_at(i) == 0);
    }
}

TEST_CASE("TimestretchFilter stretches signal", "[filters][timestretch]") {
    Waveform w = make_waveform({0, 1000});

    TimestretchFilter f(2.0);
    f.apply(w);

    REQUIRE(w.get_sample_count() == 4);
    CHECK(w.get_sample_at(0) == 0);
    CHECK(w.get_sample_at(1) == 500);
    CHECK(w.get_sample_at(2) == 1000);
    CHECK(w.get_sample_at(3) == 1000);
}

TEST_CASE("MixFilter mixes overlay", "[filters][mix]") {
    Waveform base = make_waveform({1000, 1000, 1000});
    Waveform overlay = make_waveform({500});

    MixFilter f(std::move(overlay), 0.0);
    f.apply(base);

    REQUIRE(base.get_sample_count() == 3);
    CHECK(base.get_sample_at(0) == 1500);
    CHECK(base.get_sample_at(1) == 1000);
    CHECK(base.get_sample_at(2) == 1000);
}

TEST_CASE("SinGeneratorFilter replaces waveform", "[filters][generator]") {
    Waveform w = make_waveform({1, 2, 3});

    SinGeneratorFilter f(440.0, 100.0);
    f.apply(w);

    REQUIRE(w.get_sample_count() == 4410);
    CHECK(w.get_sample_rate() == 44100);
    CHECK(w.get_num_channels() == 1);
    CHECK(w.get_bits_per_sample() == 16);
    CHECK(w.get_sample_at(0) == 0);
}

TEST_CASE("FilterProducers create filters from descriptors", "[filters][producers]") {
    std::unique_ptr<IFilter> ampl(filter_producers::make_ampl(FilterDescriptor{"ampl", {"2"}}));
    Waveform ampl_waveform = make_waveform({10});
    ampl->apply(ampl_waveform);
    CHECK(ampl_waveform.get_sample_at(0) == 20);

    std::unique_ptr<IFilter> generator(
        filter_producers::make_generator(FilterDescriptor{"generator", {"sin", "440", "100"}}));
    Waveform generated;
    generator->apply(generated);
    CHECK(generated.get_sample_count() == 4410);
}
