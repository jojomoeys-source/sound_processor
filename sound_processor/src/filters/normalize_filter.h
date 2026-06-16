#pragma once

#include "filter.h"
#include "waveform.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

// -f normalize [peak]
// Масштабирует сигнал так, чтобы максимальный по модулю отсчёт стал равен
// peak * 32767. peak необязателен, по умолчанию 1.0.
class NormalizeFilter : public IFilter {
public:
    static constexpr double kDefaultPeak = 1.0;

    explicit NormalizeFilter(double peak = kDefaultPeak) : peak_(peak) {
        if (peak < 0.0 || peak > 1.0)
            throw std::invalid_argument("normalize: peak must be in [0, 1]");
    }

    ~NormalizeFilter() override = default;

    void apply(Waveform& waveform) override {
        const size_t count = waveform.get_sample_count();
        if (count == 0) return;

        // 1. Найти текущий максимальный модуль.
        // Храним пик в int32_t: abs(INT16_MIN) = 32768, что не влезает в int16_t.
        int32_t current_peak = 0;
        for (size_t i = 0; i < count; ++i) {
            const int32_t abs_s = std::abs(static_cast<int32_t>(waveform.get_sample_at(i)));
            if (abs_s > current_peak) current_peak = abs_s;
        }

        // 2. Если сигнал тихий — ничего не делаем
        if (current_peak == 0) return;

        // 3. Масштабируем
        constexpr double kMax = static_cast<double>(std::numeric_limits<int16_t>::max());
        constexpr double kMin = static_cast<double>(std::numeric_limits<int16_t>::min());
        const double scale = peak_ * 32767.0 / static_cast<double>(current_peak);

        for (size_t i = 0; i < count; ++i) {
            const double scaled  = static_cast<double>(waveform.get_sample_at(i)) * scale;
            const double clamped = std::clamp(scaled, kMin, kMax);
            waveform.set_sample_at(i, static_cast<int16_t>(std::round(clamped)));
        }
    }

    double get_peak() const { return peak_; }

private:
    double peak_;
};