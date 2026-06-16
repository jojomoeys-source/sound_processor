#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

// ─── Вспомогательные константы ──────────────────────────────────────────────

namespace generator_detail {

static constexpr double kPi        = 3.14159265358979323846;
static constexpr double kTwoPi     = 2.0 * kPi;
static constexpr double kSampleRate = 44100.0;  // Гц, фиксировано по ТЗ
static constexpr double kInt16Max  = static_cast<double>(
    std::numeric_limits<int16_t>::max());                       // 32767
static constexpr double kInt16Min  = static_cast<double>(
    std::numeric_limits<int16_t>::min());                       // -32768

inline int16_t clamp_sample(double v) {
    return static_cast<int16_t>(
        std::round(std::clamp(v, kInt16Min, kInt16Max)));
}

// Длительность в мс → число выборок
inline size_t ms_to_samples(double duration_ms) {
    if (duration_ms < 0.0)
        throw std::invalid_argument("generator: duration_ms must be >= 0");
    const double sample_count = duration_ms * kSampleRate / 1000.0;
    if (!std::isfinite(sample_count) ||
        sample_count > static_cast<double>(std::numeric_limits<size_t>::max())) {
        throw std::overflow_error("generator: duration_ms is too large");
    }
    return static_cast<size_t>(std::round(sample_count));
}

} // namespace generator_detail


// ─── Базовый класс генераторов ───────────────────────────────────────────────

// Генераторы игнорируют входной waveform и полностью заменяют его содержимое.
class AbstractGeneratorFilter : public IFilter {
public:
    ~AbstractGeneratorFilter() override = default;

    void apply(Waveform& waveform) override {
        // Сбрасываем метаданные под стандартный моно PCM 16 бит 44100 Гц
        waveform.set_meta_info(
            static_cast<uint32_t>(generator_detail::kSampleRate), 1, 16);

        const size_t n = sample_count();
        waveform.get_samples().resize(n);
        generate(waveform.get_samples());
    }

protected:
    // Подклассы возвращают нужное число выборок
    virtual size_t sample_count() const = 0;
    // Подклассы заполняют переданный буфер (уже нужного размера)
    virtual void generate(std::vector<int16_t>& buf) const = 0;
};


// ─── Generator sin ───────────────────────────────────────────────────────────

// -f generator sin <frequency_hz> <duration_ms>
// s[i] = clamp(32767 * sin(2π * frequency_hz * t)),  t = i / 44100
class SinGeneratorFilter : public AbstractGeneratorFilter {
public:
    SinGeneratorFilter(double frequency_hz, double duration_ms)
        : freq_(frequency_hz)
        , n_(generator_detail::ms_to_samples(duration_ms)) {
        if (!std::isfinite(frequency_hz) || frequency_hz < 0.0)
            throw std::invalid_argument("generator sin: frequency_hz must be finite and >= 0");
    }

    ~SinGeneratorFilter() override = default;

    double get_frequency_hz() const { return freq_; }

protected:
    size_t sample_count() const override { return n_; }

    void generate(std::vector<int16_t>& buf) const override {
        using namespace generator_detail;
        for (size_t i = 0; i < n_; ++i) {
            const double t = static_cast<double>(i) / kSampleRate;
            buf[i] = clamp_sample(kInt16Max * std::sin(kTwoPi * freq_ * t));
        }
    }

private:
    double freq_;
    size_t n_;
};


// ─── Generator am ────────────────────────────────────────────────────────────

// -f generator am <amplitude> <carrier_hz> <modulation_hz> <depth> <duration_ms>
// envelope = 1 + depth * sin(2π * modulation_hz * t)
// carrier  = sin(2π * carrier_hz  * t)
// s[i]     = clamp(amplitude * 32767 * envelope * carrier)
class AmGeneratorFilter : public AbstractGeneratorFilter {
public:
    AmGeneratorFilter(double amplitude, double carrier_hz,
                      double modulation_hz, double depth, double duration_ms)
        : amplitude_(amplitude), carrier_hz_(carrier_hz)
        , modulation_hz_(modulation_hz), depth_(depth)
        , n_(generator_detail::ms_to_samples(duration_ms)) {
        if (!std::isfinite(amplitude) || amplitude < 0.0 || amplitude > 1.0)
            throw std::invalid_argument("generator am: amplitude must be finite and in [0, 1]");
        if (!std::isfinite(carrier_hz) || carrier_hz < 0.0)
            throw std::invalid_argument("generator am: carrier_hz must be finite and >= 0");
        if (!std::isfinite(modulation_hz) || modulation_hz < 0.0)
            throw std::invalid_argument("generator am: modulation_hz must be finite and >= 0");
        if (!std::isfinite(depth) || depth < 0.0 || depth > 1.0)
            throw std::invalid_argument("generator am: depth must be finite and in [0, 1]");
    }

    ~AmGeneratorFilter() override = default;

protected:
    size_t sample_count() const override { return n_; }

    void generate(std::vector<int16_t>& buf) const override {
        using namespace generator_detail;
        for (size_t i = 0; i < n_; ++i) {
            const double t        = static_cast<double>(i) / kSampleRate;
            const double envelope = 1.0 + depth_ * std::sin(kTwoPi * modulation_hz_ * t);
            const double carrier  = std::sin(kTwoPi * carrier_hz_ * t);
            buf[i] = clamp_sample(amplitude_ * kInt16Max * envelope * carrier);
        }
    }

private:
    double amplitude_;
    double carrier_hz_;
    double modulation_hz_;
    double depth_;
    size_t n_;
};


// ─── Generator fm ────────────────────────────────────────────────────────────

// -f generator fm <amplitude> <carrier_hz> <modulation_hz> <deviation_hz> <duration_ms>
// phase = 2π * carrier_hz * t + (deviation_hz / modulation_hz) * sin(2π * modulation_hz * t)
// s[i]  = clamp(amplitude * 32767 * sin(phase))
class FmGeneratorFilter : public AbstractGeneratorFilter {
public:
    FmGeneratorFilter(double amplitude, double carrier_hz,
                      double modulation_hz, double deviation_hz, double duration_ms)
        : amplitude_(amplitude), carrier_hz_(carrier_hz)
        , modulation_hz_(modulation_hz), deviation_hz_(deviation_hz)
        , n_(generator_detail::ms_to_samples(duration_ms)) {
        if (!std::isfinite(amplitude) || amplitude < 0.0 || amplitude > 1.0)
            throw std::invalid_argument("generator fm: amplitude must be finite and in [0, 1]");
        if (!std::isfinite(carrier_hz) || carrier_hz < 0.0)
            throw std::invalid_argument("generator fm: carrier_hz must be finite and >= 0");
        if (!std::isfinite(modulation_hz) || modulation_hz <= 0.0)
            throw std::invalid_argument("generator fm: modulation_hz must be finite and > 0");
        if (!std::isfinite(deviation_hz) || deviation_hz < 0.0)
            throw std::invalid_argument("generator fm: deviation_hz must be finite and >= 0");
    }

    ~FmGeneratorFilter() override = default;

protected:
    size_t sample_count() const override { return n_; }

    void generate(std::vector<int16_t>& buf) const override {
        using namespace generator_detail;
        const double mod_ratio = deviation_hz_ / modulation_hz_;
        for (size_t i = 0; i < n_; ++i) {
            const double t     = static_cast<double>(i) / kSampleRate;
            const double phase = kTwoPi * carrier_hz_ * t
                               + mod_ratio * std::sin(kTwoPi * modulation_hz_ * t);
            buf[i] = clamp_sample(amplitude_ * kInt16Max * std::sin(phase));
        }
    }

private:
    double amplitude_;
    double carrier_hz_;
    double modulation_hz_;
    double deviation_hz_;
    size_t n_;
};