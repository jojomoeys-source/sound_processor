#pragma once

#include "filter.h"
#include "waveform.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

// -f timestretch <factor>
// Изменяет длительность сигнала в factor раз линейной интерполяцией.
// factor > 1 — растяжение, 0 < factor < 1 — сжатие.
class TimestretchFilter : public IFilter {
public:
    explicit TimestretchFilter(double factor) : factor_(factor) {
        if (factor <= 0.0)
            throw std::invalid_argument("timestretch: factor must be > 0");
    }

    ~TimestretchFilter() override = default;

    void apply(Waveform& waveform) override {
        const size_t old_size = waveform.get_sample_count();
        if (old_size == 0) return;

        const size_t new_size = static_cast<size_t>(std::round(
            static_cast<double>(old_size) * factor_));
        if (new_size == 0) {
            waveform.get_samples().clear();
            return;
        }

        constexpr double kMin = static_cast<double>(std::numeric_limits<int16_t>::min());
        constexpr double kMax = static_cast<double>(std::numeric_limits<int16_t>::max());

        const auto& src = waveform.get_samples();
        std::vector<int16_t> dst(new_size);

        for (size_t i = 0; i < new_size; ++i) {
            // Дробная позиция в исходном сигнале
            const double pos  = static_cast<double>(i) / factor_;
            const size_t l    = static_cast<size_t>(pos);
            const double frac = pos - static_cast<double>(l);

            double value;
            if (l + 1 < old_size) {
                // Линейная интерполяция между l и l+1
                value = static_cast<double>(src[l]) * (1.0 - frac)
                      + static_cast<double>(src[l + 1]) * frac;
            } else {
                // Последний отсчёт — правого соседа нет
                value = static_cast<double>(src[l]);
            }

            dst[i] = static_cast<int16_t>(
                std::round(std::clamp(value, kMin, kMax)));
        }

        waveform.get_samples() = std::move(dst);
    }

    double get_factor() const { return factor_; }

private:
    double factor_;
};