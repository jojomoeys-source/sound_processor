#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "waveform/waveform.h"

TEST_CASE("Waveform stores metadata and samples", "[waveform]") {
    Waveform w(2, 44100, 2, 16);
    w.set_sample_at(0, 1000);
    w.set_sample_at(1, -2000);

    CHECK(w.get_sample_count() == 2);
    CHECK(w.get_sample_rate() == 44100);
    CHECK(w.get_num_channels() == 2);
    CHECK(w.get_bits_per_sample() == 16);
    CHECK(w.get_sample_at(0) == 1000);
    CHECK(w.get_sample_at(1) == -2000);
    CHECK(w.get_duration_seconds() == Catch::Approx(2.0 / 44100.0));
}

TEST_CASE("Waveform converts time and resizes", "[waveform]") {
    Waveform w(10, 44100, 1, 16);
    w.resize(20);

    CHECK(w.seconds_to_samples(0.5) == 22050);
    CHECK(w.samples_index_to_seconds(44100) == Catch::Approx(1.0));
    CHECK(w.get_sample_count() == 20);
    CHECK(w.get_sample_at(15) == 0);
}
