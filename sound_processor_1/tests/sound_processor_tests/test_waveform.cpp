#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "waveform/waveform.h"

#include <limits>

// Проверяет: Waveform: default construction
TEST_CASE("Waveform: default construction", "[waveform]") {
    Waveform w;
    CHECK(w.get_sample_count() == 0);
    CHECK(w.get_sample_rate() == 44100);
    CHECK(w.get_num_channels() == 1);
    CHECK(w.get_bits_per_sample() == 16);
}

// Проверяет: Waveform: construction with sample count
TEST_CASE("Waveform: construction with sample count", "[waveform]") {
    Waveform w(1000);
    CHECK(w.get_sample_count() == 1000);
    // Все сэмплы инициализированы нулём
    for (size_t i = 0; i < w.get_sample_count(); ++i) {
        CHECK(w.get_sample_at(i) == 0);
    }
}

// Проверяет: Waveform: full construction
TEST_CASE("Waveform: full construction", "[waveform]") {
    Waveform w(500, 44100, 1, 16);
    CHECK(w.get_sample_count() == 500);
    CHECK(w.get_sample_rate() == 44100);
    CHECK(w.get_num_channels() == 1);
    CHECK(w.get_bits_per_sample() == 16);
}

// Проверяет: Waveform: set_sample_at / get_sample_at
TEST_CASE("Waveform: set_sample_at / get_sample_at", "[waveform]") {
    Waveform w(10);
    w.set_sample_at(0, 1000);
    w.set_sample_at(9, -32768);
    CHECK(w.get_sample_at(0) == 1000);
    CHECK(w.get_sample_at(9) == -32768);
}

// Проверяет: Waveform: out of bounds throws
TEST_CASE("Waveform: out of bounds throws", "[waveform]") {
    Waveform w(5);
    CHECK_THROWS(w.get_sample_at(5));
    CHECK_THROWS(w.set_sample_at(10, 0));
}

// Проверяет: Waveform: get_duration_seconds
TEST_CASE("Waveform: get_duration_seconds", "[waveform]") {
    Waveform w(44100, 44100, 1, 16);
    CHECK(w.get_duration_seconds() == Catch::Approx(1.0));

    Waveform w2(22050, 44100, 1, 16);
    CHECK(w2.get_duration_seconds() == Catch::Approx(0.5));
}

// Проверяет: Waveform: seconds_to_samples
TEST_CASE("Waveform: seconds_to_samples", "[waveform]") {
    Waveform w(44100, 44100, 1, 16);
    CHECK(w.seconds_to_samples(1.0) == 44100);
    CHECK(w.seconds_to_samples(0.5) == 22050);
    CHECK(w.seconds_to_samples(0.0) == 0);
}

// Проверяет: Waveform: seconds_to_samples negative throws
TEST_CASE("Waveform: seconds_to_samples negative throws", "[waveform]") {
    Waveform w(100, 44100, 1, 16);
    CHECK_THROWS(w.seconds_to_samples(-1.0));
    CHECK_THROWS(w.seconds_to_samples(std::numeric_limits<double>::max()));
}

// Проверяет: Waveform: resize
TEST_CASE("Waveform: resize", "[waveform]") {
    Waveform w(100);
    w.resize(200);
    CHECK(w.get_sample_count() == 200);
    // новые сэмплы — нули
    CHECK(w.get_sample_at(150) == 0);

    w.resize(50);
    CHECK(w.get_sample_count() == 50);
}

// Проверяет: Waveform: clear resets to default
TEST_CASE("Waveform: clear resets to default", "[waveform]") {
    Waveform w(1000, 44100, 1, 16);
    w.set_sample_at(0, 999);
    w.clear();
    CHECK(w.get_sample_count() == 0);
    CHECK(w.get_sample_rate() == 44100);  // дефолт
}

// Проверяет: Waveform: set_meta_info
TEST_CASE("Waveform: set_meta_info", "[waveform]") {
    Waveform w;
    w.set_meta_info(22050, 1, 16);
    CHECK(w.get_sample_rate() == 22050);
}

// Проверяет: Waveform: samples_index_to_seconds
TEST_CASE("Waveform: samples_index_to_seconds", "[waveform]") {
    Waveform w(44100, 44100, 1, 16);
    CHECK(w.samples_index_to_seconds(44100) == Catch::Approx(1.0));
    CHECK(w.samples_index_to_seconds(0) == Catch::Approx(0.0));
}
