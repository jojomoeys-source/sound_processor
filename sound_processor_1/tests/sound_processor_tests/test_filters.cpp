#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "waveform/waveform.h"
#include "filters/ampl_filter.h"
#include "filter_producers/filter_producers.h"
#include "filters/normalize_filter.h"
#include "filters/mute_filter.h"
#include "filters/silence_filter.h"
#include "filters/lowpass_filter.h"
#include "filters/highpass_filter.h"
#include "filters/bandpass_filter.h"
#include "filters/reject_filter.h"
#include "filters/timestretch_filter.h"
#include "filters/mix_filter.h"
#include "filters/generator_filters.h"

#include <cmath>
#include <limits>
#include <numeric>

// ─── Вспомогательные функции ─────────────────────────────────────────────────

static Waveform make_waveform(std::vector<int16_t> samples, uint32_t rate = 44100) {
    Waveform w(samples.size(), rate, 1, 16);
    for (size_t i = 0; i < samples.size(); ++i) {
        w.set_sample_at(i, samples[i]);
    }
    return w;
}

// ─── AmplFilter ──────────────────────────────────────────────────────────────

// Проверяет: AmplFilter: multiplies samples by factor
TEST_CASE("AmplFilter: multiplies samples by factor", "[ampl]") {
    Waveform w = make_waveform({1000, -1000, 2000});
    AmplFilter f(0.5);
    f.apply(w);
    CHECK(w.get_sample_at(0) == 500);
    CHECK(w.get_sample_at(1) == -500);
    CHECK(w.get_sample_at(2) == 1000);
}

// Проверяет: AmplFilter: factor zero silences signal
TEST_CASE("AmplFilter: factor zero silences signal", "[ampl]") {
    Waveform w = make_waveform({1000, -500, 32767});
    AmplFilter f(0.0);
    f.apply(w);
    for (size_t i = 0; i < w.get_sample_count(); ++i)
        CHECK(w.get_sample_at(i) == 0);
}

// Проверяет: AmplFilter: clamps to INT16_MAX
TEST_CASE("AmplFilter: clamps to INT16_MAX", "[ampl]") {
    Waveform w = make_waveform({32767});
    AmplFilter f(2.0);
    f.apply(w);
    CHECK(w.get_sample_at(0) == 32767);
}

// Проверяет: AmplFilter: clamps to INT16_MIN
TEST_CASE("AmplFilter: clamps to INT16_MIN", "[ampl]") {
    Waveform w = make_waveform({-32768});
    AmplFilter f(2.0);
    f.apply(w);
    CHECK(w.get_sample_at(0) == -32768);
}

// Проверяет: AmplFilter: negative factor throws
TEST_CASE("AmplFilter: negative factor throws", "[ampl]") {
    CHECK_THROWS_AS(AmplFilter(-1.0), std::invalid_argument);
}

// Проверяет: FilterProducers: reject extra arguments
TEST_CASE("FilterProducers: reject extra arguments", "[producers]") {
    FilterDescriptor fd{"ampl", {"0.8", "extra"}};
    CHECK_THROWS_AS(filter_producers::make_ampl(fd), std::runtime_error);

    FilterDescriptor gen{"generator", {"sin", "440", "1000", "extra"}};
    CHECK_THROWS_AS(filter_producers::make_generator(gen), std::runtime_error);
}

// Проверяет: FilterProducers: reject non-finite arguments
TEST_CASE("FilterProducers: reject non-finite arguments", "[producers]") {
    FilterDescriptor fd{"ampl", {"nan"}};
    CHECK_THROWS_AS(filter_producers::make_ampl(fd), std::runtime_error);
}

// Проверяет: AmplFilter: empty waveform is no-op
TEST_CASE("AmplFilter: empty waveform is no-op", "[ampl]") {
    Waveform w;
    AmplFilter f(2.0);
    f.apply(w);
    CHECK(w.get_sample_count() == 0);
}

// ─── NormalizeFilter ─────────────────────────────────────────────────────────

// Проверяет: NormalizeFilter: scales max to 32767
TEST_CASE("NormalizeFilter: scales max to 32767", "[normalize]") {
    Waveform w = make_waveform({0, 16383, -16383});
    NormalizeFilter f;  // peak = 1.0
    f.apply(w);
    // Максимум должен стать 32767
    CHECK(w.get_sample_at(1) == 32767);
    CHECK(w.get_sample_at(2) == -32767);
}

// Проверяет: NormalizeFilter: scales to custom peak
TEST_CASE("NormalizeFilter: scales to custom peak", "[normalize]") {
    Waveform w = make_waveform({32767, -32767});
    NormalizeFilter f(0.5);
    f.apply(w);
    // 32767 * 0.5 = 16383.5, округление даёт 16384
    CHECK(w.get_sample_at(0) == 16384);
}

// Проверяет: NormalizeFilter: silent signal unchanged
TEST_CASE("NormalizeFilter: silent signal unchanged", "[normalize]") {
    Waveform w = make_waveform({0, 0, 0});
    NormalizeFilter f;
    f.apply(w);
    for (size_t i = 0; i < w.get_sample_count(); ++i)
        CHECK(w.get_sample_at(i) == 0);
}

// Проверяет: NormalizeFilter: empty waveform is no-op
TEST_CASE("NormalizeFilter: empty waveform is no-op", "[normalize]") {
    Waveform w;
    NormalizeFilter f;
    f.apply(w);
    CHECK(w.get_sample_count() == 0);
}

// Проверяет: NormalizeFilter: handles INT16_MIN correctly (no overflow)
TEST_CASE("NormalizeFilter: handles INT16_MIN correctly (no overflow)", "[normalize]") {
    // INT16_MIN = -32768; abs(-32768) = 32768 — не влезает в int16_t!
    // Проверяем что нет UB и результат корректен
    Waveform w = make_waveform({static_cast<int16_t>(-32768), 0});
    NormalizeFilter f(1.0);
    f.apply(w);
    // После нормализации пик должен быть 32767 по модулю
    CHECK(std::abs(static_cast<int32_t>(w.get_sample_at(0))) == 32767);
}

// Проверяет: NormalizeFilter: invalid peak throws
TEST_CASE("NormalizeFilter: invalid peak throws", "[normalize]") {
    CHECK_THROWS_AS(NormalizeFilter(-0.1), std::invalid_argument);
    CHECK_THROWS_AS(NormalizeFilter(1.5),  std::invalid_argument);
    CHECK_THROWS_AS(NormalizeFilter(std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
}

// ─── MuteFilter ──────────────────────────────────────────────────────────────

// Проверяет: MuteFilter: zeroes samples in range
TEST_CASE("MuteFilter: zeroes samples in range", "[mute]") {
    // 44100 сэмплов = 1 сек
    Waveform w(44100, 44100, 1, 16);
    for (size_t i = 0; i < w.get_sample_count(); ++i)
        w.set_sample_at(i, 1000);

    MuteFilter f(0.0, 0.5);  // [0, 0.5) сек = первые 22050 сэмплов
    f.apply(w);

    for (size_t i = 0; i < 22050; ++i)
        CHECK(w.get_sample_at(i) == 0);
    // После 0.5 сек — без изменений
    CHECK(w.get_sample_at(22050) == 1000);
}

// Проверяет: MuteFilter: invalid range throws
TEST_CASE("MuteFilter: invalid range throws", "[mute]") {
    CHECK_THROWS_AS(MuteFilter(-0.1, 1.0), std::invalid_argument);
    CHECK_THROWS_AS(MuteFilter(1.0, 0.0), std::invalid_argument);
    CHECK_THROWS_AS(MuteFilter(std::numeric_limits<double>::quiet_NaN(), 1.0), std::invalid_argument);
}

// Проверяет: MuteFilter: empty waveform is no-op
TEST_CASE("MuteFilter: empty waveform is no-op", "[mute]") {
    Waveform w;
    MuteFilter f(0.0, 1.0);
    f.apply(w);
    CHECK(w.get_sample_count() == 0);
}

// Проверяет: MuteFilter: range beyond end clamps at end
TEST_CASE("MuteFilter: range beyond end clamps at end", "[mute]") {
    Waveform w = make_waveform({100, 200, 300});
    MuteFilter f(0.0, 10.0);  // 10 сек >> длина сигнала
    f.apply(w);
    CHECK(w.get_sample_at(0) == 0);
    CHECK(w.get_sample_at(1) == 0);
    CHECK(w.get_sample_at(2) == 0);
}

// ─── SilenceFilter ───────────────────────────────────────────────────────────

// Проверяет: SilenceFilter: inserts silence (shifts tail right)
TEST_CASE("SilenceFilter: inserts silence (shifts tail right)", "[silence]") {
    // 44100 сэмплов = 1 сек, все = 500
    Waveform w(44100, 44100, 1, 16);
    for (size_t i = 0; i < w.get_sample_count(); ++i)
        w.set_sample_at(i, 500);

    // Вставляем 0.1 сек тишины в начало [0, 0.1)
    SilenceFilter f(SilenceFilter::Unit::Sec, 0.0, 0.1);
    f.apply(w);

    // Длина увеличилась на 4411 сэмплов: [0, 0.1] включительно.
    CHECK(w.get_sample_count() == 44100 + 4411);
    // Первые 4411 — нули
    CHECK(w.get_sample_at(0)     == 0);
    CHECK(w.get_sample_at(4410)  == 0);
    // После вставки — исходные данные
    CHECK(w.get_sample_at(4411)  == 500);
}

// Проверяет: SilenceFilter: ms unit
TEST_CASE("SilenceFilter: ms unit", "[silence]") {
    Waveform w(44100, 44100, 1, 16);
    for (size_t i = 0; i < w.get_sample_count(); ++i)
        w.set_sample_at(i, 100);

    SilenceFilter f(SilenceFilter::Unit::Ms, 0.0, 100.0);  // 100 ms = 4410 samples, +1 inclusive
    f.apply(w);
    CHECK(w.get_sample_count() == 44100 + 4411);
    CHECK(w.get_sample_at(0) == 0);
    CHECK(w.get_sample_at(4410) == 0);
    CHECK(w.get_sample_at(4411) == 100);
}

// Проверяет: SilenceFilter: parse_unit valid
TEST_CASE("SilenceFilter: parse_unit valid", "[silence]") {
    CHECK(SilenceFilter::parse_unit("sec") == SilenceFilter::Unit::Sec);
    CHECK(SilenceFilter::parse_unit("ms")  == SilenceFilter::Unit::Ms);
}

// Проверяет: SilenceFilter: parse_unit invalid throws
TEST_CASE("SilenceFilter: parse_unit invalid throws", "[silence]") {
    CHECK_THROWS_AS(SilenceFilter::parse_unit("s"),    std::invalid_argument);
    CHECK_THROWS_AS(SilenceFilter::parse_unit("msec"), std::invalid_argument);
}

// Проверяет: SilenceFilter: zero-length inclusive range inserts one sample
TEST_CASE("SilenceFilter: zero-length inclusive range inserts one sample", "[silence]") {
    Waveform w = make_waveform({100, 200, 300});
    SilenceFilter f(SilenceFilter::Unit::Sec, 0.0, 0.0);
    f.apply(w);
    REQUIRE(w.get_sample_count() == 4);
    CHECK(w.get_sample_at(0) == 0);
    CHECK(w.get_sample_at(1) == 100);
    CHECK(w.get_sample_at(2) == 200);
    CHECK(w.get_sample_at(3) == 300);
}

// Проверяет: SilenceFilter: negative start throws
TEST_CASE("SilenceFilter: negative start throws", "[silence]") {
    CHECK_THROWS_AS(SilenceFilter(SilenceFilter::Unit::Sec, -1.0, 0.0), std::invalid_argument);
    CHECK_THROWS_AS(SilenceFilter(SilenceFilter::Unit::Sec,
                                  std::numeric_limits<double>::quiet_NaN(), 0.0),
                    std::invalid_argument);
}

// Проверяет: SilenceFilter: end < start throws
TEST_CASE("SilenceFilter: end < start throws", "[silence]") {
    CHECK_THROWS_AS(SilenceFilter(SilenceFilter::Unit::Sec, 1.0, 0.5), std::invalid_argument);
}

// ─── LowpassFilter ───────────────────────────────────────────────────────────

// Проверяет: LowpassFilter: constant signal unchanged
TEST_CASE("LowpassFilter: constant signal unchanged", "[lowpass]") {
    Waveform w = make_waveform({1000, 1000, 1000, 1000, 1000});
    LowpassFilter f(3);
    f.apply(w);
    for (size_t i = 0; i < w.get_sample_count(); ++i)
        CHECK(w.get_sample_at(i) == 1000);
}

// Проверяет: LowpassFilter: smooths spike
TEST_CASE("LowpassFilter: smooths spike", "[lowpass]") {
    // Один большой всплеск в центре
    Waveform w = make_waveform({0, 0, 3000, 0, 0});
    LowpassFilter f(3);
    f.apply(w);
    // Центральный элемент должен уменьшиться (среднее 0+3000+0 / 3 = 1000)
    CHECK(w.get_sample_at(2) == 1000);
    // Соседние элементы получают ненулевое значение
    CHECK(w.get_sample_at(1) == 1000);
    CHECK(w.get_sample_at(3) == 1000);
}

// Проверяет: LowpassFilter: even window throws
TEST_CASE("LowpassFilter: even window throws", "[lowpass]") {
    CHECK_THROWS_AS(LowpassFilter(2), std::invalid_argument);
}

// Проверяет: LowpassFilter: zero window throws
TEST_CASE("LowpassFilter: zero window throws", "[lowpass]") {
    CHECK_THROWS_AS(LowpassFilter(0), std::invalid_argument);
}

// Проверяет: LowpassFilter: window 1 is identity
TEST_CASE("LowpassFilter: window 1 is identity", "[lowpass]") {
    Waveform w = make_waveform({100, 200, -300, 400});
    LowpassFilter f(1);
    f.apply(w);
    CHECK(w.get_sample_at(0) == 100);
    CHECK(w.get_sample_at(1) == 200);
    CHECK(w.get_sample_at(2) == -300);
    CHECK(w.get_sample_at(3) == 400);
}

// Проверяет: LowpassFilter: empty waveform is no-op
TEST_CASE("LowpassFilter: empty waveform is no-op", "[lowpass]") {
    Waveform w;
    LowpassFilter f(3);
    f.apply(w);
    CHECK(w.get_sample_count() == 0);
}

// Проверяет: HighpassFilter: constant signal becomes zero
TEST_CASE("HighpassFilter: constant signal becomes zero", "[highpass]") {
    Waveform w = make_waveform({1000, 1000, 1000, 1000});
    HighpassFilter f(3);
    f.apply(w);
    for (size_t i = 0; i < w.get_sample_count(); ++i)
        CHECK(w.get_sample_at(i) == 0);
}

// Проверяет: HighpassFilter: removes moving average from spike
TEST_CASE("HighpassFilter: removes moving average from spike", "[highpass]") {
    Waveform w = make_waveform({0, 0, 3000, 0, 0});
    HighpassFilter f(3);
    f.apply(w);
    CHECK(w.get_sample_at(0) == 0);
    CHECK(w.get_sample_at(1) == -1000);
    CHECK(w.get_sample_at(2) == 2000);
    CHECK(w.get_sample_at(3) == -1000);
    CHECK(w.get_sample_at(4) == 0);
}

// Проверяет: BandpassFilter: constant signal becomes zero
TEST_CASE("BandpassFilter: constant signal becomes zero", "[bandpass]") {
    Waveform w = make_waveform({1000, 1000, 1000, 1000});
    BandpassFilter f(3, 3);
    f.apply(w);
    for (size_t i = 0; i < w.get_sample_count(); ++i)
        CHECK(w.get_sample_at(i) == 0);
}

// Проверяет: RejectFilter: constant signal is unchanged
TEST_CASE("RejectFilter: constant signal is unchanged", "[reject]") {
    Waveform w = make_waveform({1000, 1000, 1000, 1000});
    RejectFilter f(3, 3);
    f.apply(w);
    for (size_t i = 0; i < w.get_sample_count(); ++i)
        CHECK(w.get_sample_at(i) == 1000);
}

// Проверяет: Frequency filters: invalid even windows throw
TEST_CASE("Frequency filters: invalid even windows throw", "[frequency]") {
    CHECK_THROWS_AS(HighpassFilter(2), std::invalid_argument);
    CHECK_THROWS_AS(BandpassFilter(3, 2), std::invalid_argument);
    CHECK_THROWS_AS(RejectFilter(2, 3), std::invalid_argument);
}

// ─── TimestretchFilter ───────────────────────────────────────────────────────

// Проверяет: TimestretchFilter: factor 1 produces identical signal
TEST_CASE("TimestretchFilter: factor 1 produces identical signal", "[timestretch]") {
    Waveform w = make_waveform({100, 200, 300, 400});
    TimestretchFilter f(1.0);
    f.apply(w);
    REQUIRE(w.get_sample_count() == 4);
    CHECK(w.get_sample_at(0) == 100);
    CHECK(w.get_sample_at(1) == 200);
    CHECK(w.get_sample_at(3) == 400);
}

// Проверяет: TimestretchFilter: factor 2 uses interpolation from detailed TZ
TEST_CASE("TimestretchFilter: factor 2 uses interpolation from detailed TZ", "[timestretch]") {
    Waveform w = make_waveform({0, 1000});
    TimestretchFilter f(2.0);
    f.apply(w);
    CHECK(w.get_sample_count() == 4);
    CHECK(w.get_sample_at(0) == 0);
    CHECK(w.get_sample_at(1) == 500);
    CHECK(w.get_sample_at(2) == 1000);
    CHECK(w.get_sample_at(3) == 1000);
}

// Проверяет: TimestretchFilter: factor 0.5 halves length
TEST_CASE("TimestretchFilter: factor 0.5 halves length", "[timestretch]") {
    Waveform w = make_waveform({100, 200, 300, 400});
    TimestretchFilter f(0.5);
    f.apply(w);
    CHECK(w.get_sample_count() == 2);
}

// Проверяет: TimestretchFilter: non-positive factor throws
TEST_CASE("TimestretchFilter: non-positive factor throws", "[timestretch]") {
    CHECK_THROWS_AS(TimestretchFilter(0.0),  std::invalid_argument);
    CHECK_THROWS_AS(TimestretchFilter(-1.0), std::invalid_argument);
    CHECK_THROWS_AS(TimestretchFilter(std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
}

// Проверяет: TimestretchFilter: huge factor throws instead of overflowing
TEST_CASE("TimestretchFilter: huge factor throws instead of overflowing", "[timestretch]") {
    Waveform w = make_waveform({100, 200});
    TimestretchFilter f(std::numeric_limits<double>::max());
    CHECK_THROWS_AS(f.apply(w), std::overflow_error);
}

// Проверяет: TimestretchFilter: empty waveform is no-op
TEST_CASE("TimestretchFilter: empty waveform is no-op", "[timestretch]") {
    Waveform w;
    TimestretchFilter f(2.0);
    f.apply(w);
    CHECK(w.get_sample_count() == 0);
}

// ─── MixFilter ───────────────────────────────────────────────────────────────

// Проверяет: MixFilter: mixes additional at offset 0
TEST_CASE("MixFilter: mixes additional at offset 0", "[mix]") {
    Waveform base    = make_waveform({1000, 1000, 1000, 1000});
    Waveform overlay = make_waveform({500,  500});

    MixFilter f(std::move(overlay), 0.0);
    f.apply(base);

    CHECK(base.get_sample_at(0) == 1500);
    CHECK(base.get_sample_at(1) == 1500);
    CHECK(base.get_sample_at(2) == 1000);  // наложения нет
}

// Проверяет: MixFilter: clips to INT16_MAX on overflow
TEST_CASE("MixFilter: clips to INT16_MAX on overflow", "[mix]") {
    Waveform base    = make_waveform({32000});
    Waveform overlay = make_waveform({32000});

    MixFilter f(std::move(overlay), 0.0);
    f.apply(base);

    CHECK(base.get_sample_at(0) == 32767);
}

// Проверяет: MixFilter: additional longer than base extends base
TEST_CASE("MixFilter: additional longer than base extends base", "[mix]") {
    Waveform base    = make_waveform({100});
    Waveform overlay = make_waveform({200, 300, 400});

    MixFilter f(std::move(overlay), 0.0);
    f.apply(base);

    CHECK(base.get_sample_count() == 3);
    CHECK(base.get_sample_at(0) == 300);
    CHECK(base.get_sample_at(1) == 300);
    CHECK(base.get_sample_at(2) == 400);
}

// Проверяет: MixFilter: invalid start throws
TEST_CASE("MixFilter: invalid start throws", "[mix]") {
    CHECK_THROWS_AS(MixFilter(make_waveform({100}), -0.1), std::invalid_argument);
    CHECK_THROWS_AS(MixFilter(make_waveform({100}), std::numeric_limits<double>::quiet_NaN()),
                    std::invalid_argument);
}

// Проверяет: MixFilter: empty additional is no-op
TEST_CASE("MixFilter: empty additional is no-op", "[mix]") {
    Waveform base    = make_waveform({100, 200});
    Waveform overlay;

    MixFilter f(std::move(overlay), 0.0);
    f.apply(base);

    CHECK(base.get_sample_at(0) == 100);
    CHECK(base.get_sample_at(1) == 200);
}

// ─── SinGeneratorFilter ──────────────────────────────────────────────────────

// Проверяет: SinGeneratorFilter: produces correct number of samples
TEST_CASE("SinGeneratorFilter: produces correct number of samples", "[generator][sin]") {
    Waveform w;
    SinGeneratorFilter f(440.0, 1000.0);  // 1000 ms = 44100 samples
    f.apply(w);
    CHECK(w.get_sample_count() == 44100);
}

// Проверяет: SinGeneratorFilter: replaces input signal
TEST_CASE("SinGeneratorFilter: replaces input signal", "[generator][sin]") {
    Waveform w = make_waveform({1, 2, 3, 4, 5});
    SinGeneratorFilter f(440.0, 100.0);
    f.apply(w);
    // Длина должна измениться (100ms = 4410, а не 5)
    CHECK(w.get_sample_count() == 4410);
}

// Проверяет: SinGeneratorFilter: sets correct sample rate
TEST_CASE("SinGeneratorFilter: sets correct sample rate", "[generator][sin]") {
    Waveform w;
    SinGeneratorFilter f(440.0, 100.0);
    f.apply(w);
    CHECK(w.get_sample_rate() == 44100);
    CHECK(w.get_num_channels() == 1);
    CHECK(w.get_bits_per_sample() == 16);
}

// Проверяет: SinGeneratorFilter: first sample is 0 (sin(0)=0)
TEST_CASE("SinGeneratorFilter: first sample is 0 (sin(0)=0)", "[generator][sin]") {
    Waveform w;
    SinGeneratorFilter f(440.0, 1000.0);
    f.apply(w);
    CHECK(w.get_sample_at(0) == 0);
}

// Проверяет: SinGeneratorFilter: negative frequency throws
TEST_CASE("SinGeneratorFilter: negative frequency throws", "[generator][sin]") {
    CHECK_THROWS_AS(SinGeneratorFilter(-1.0, 1000.0), std::invalid_argument);
}

// Проверяет: SinGeneratorFilter: negative duration throws
TEST_CASE("SinGeneratorFilter: negative duration throws", "[generator][sin]") {
    CHECK_THROWS_AS(SinGeneratorFilter(440.0, -1.0), std::invalid_argument);
}

// Проверяет: SinGeneratorFilter: non-finite frequency throws
TEST_CASE("SinGeneratorFilter: non-finite frequency throws", "[generator][sin]") {
    CHECK_THROWS_AS(SinGeneratorFilter(std::numeric_limits<double>::quiet_NaN(), 1000.0),
                    std::invalid_argument);
}

// Проверяет: SinGeneratorFilter: huge duration throws instead of overflowing
TEST_CASE("SinGeneratorFilter: huge duration throws instead of overflowing", "[generator][sin]") {
    CHECK_THROWS_AS(SinGeneratorFilter(440.0, std::numeric_limits<double>::max()),
                    std::overflow_error);
}

// Проверяет: SinGeneratorFilter: zero duration gives empty waveform
TEST_CASE("SinGeneratorFilter: zero duration gives empty waveform", "[generator][sin]") {
    Waveform w;
    SinGeneratorFilter f(440.0, 0.0);
    f.apply(w);
    CHECK(w.get_sample_count() == 0);
}

// ─── AmGeneratorFilter ───────────────────────────────────────────────────────

// Проверяет: AmGeneratorFilter: produces correct sample count
TEST_CASE("AmGeneratorFilter: produces correct sample count", "[generator][am]") {
    Waveform w;
    AmGeneratorFilter f(0.8, 440.0, 40.0, 0.5, 500.0);
    f.apply(w);
    CHECK(w.get_sample_count() == 22050);
}

// Проверяет: AmGeneratorFilter: invalid amplitude throws
TEST_CASE("AmGeneratorFilter: invalid amplitude throws", "[generator][am]") {
    CHECK_THROWS_AS(AmGeneratorFilter(1.5, 440.0, 40.0, 0.5, 1000.0), std::invalid_argument);
    CHECK_THROWS_AS(AmGeneratorFilter(-0.1, 440.0, 40.0, 0.5, 1000.0), std::invalid_argument);
}

// Проверяет: AmGeneratorFilter: invalid depth throws
TEST_CASE("AmGeneratorFilter: invalid depth throws", "[generator][am]") {
    CHECK_THROWS_AS(AmGeneratorFilter(0.8, 440.0, 40.0, 1.5, 1000.0), std::invalid_argument);
}

// ─── FmGeneratorFilter ───────────────────────────────────────────────────────

// Проверяет: FmGeneratorFilter: produces correct sample count
TEST_CASE("FmGeneratorFilter: produces correct sample count", "[generator][fm]") {
    Waveform w;
    FmGeneratorFilter f(0.8, 440.0, 40.0, 20.0, 500.0);
    f.apply(w);
    CHECK(w.get_sample_count() == 22050);
}

// Проверяет: FmGeneratorFilter: invalid amplitude throws
TEST_CASE("FmGeneratorFilter: invalid amplitude throws", "[generator][fm]") {
    CHECK_THROWS_AS(FmGeneratorFilter(1.5, 440.0, 40.0, 20.0, 1000.0), std::invalid_argument);
}

// Проверяет: FmGeneratorFilter: zero modulation_hz throws
TEST_CASE("FmGeneratorFilter: zero modulation_hz throws", "[generator][fm]") {
    CHECK_THROWS_AS(FmGeneratorFilter(0.8, 440.0, 0.0, 20.0, 1000.0), std::invalid_argument);
}
