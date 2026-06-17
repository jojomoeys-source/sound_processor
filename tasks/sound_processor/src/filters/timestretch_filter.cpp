#include "filters/timestretch_filter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

TimestretchFilter::TimestretchFilter(double factor) : factor_(factor) {
    if (!std::isfinite(factor) || factor <= 0.0)
        throw std::invalid_argument("timestretch: factor must be finite and > 0");
}

TimestretchFilter::~TimestretchFilter() = default;

void TimestretchFilter::apply(Waveform& waveform) {
    const size_t old_size = waveform.get_sample_count();
    if (old_size == 0) return;

    const double new_size_d = static_cast<double>(old_size) * factor_;
    if (!std::isfinite(new_size_d) ||
        new_size_d > static_cast<double>(std::numeric_limits<size_t>::max())) {
        throw std::overflow_error("timestretch: result is too large");
    }
    const size_t new_size = static_cast<size_t>(std::round(new_size_d));
    if (new_size == 0) {
        waveform.get_samples().clear();
        return;
    }

    constexpr double kMin = static_cast<double>(std::numeric_limits<int16_t>::min());
    constexpr double kMax = static_cast<double>(std::numeric_limits<int16_t>::max());

    const auto& src = waveform.get_samples();
    std::vector<int16_t> dst(new_size);

    for (size_t i = 0; i < new_size; ++i) {
        const double pos = static_cast<double>(i) / factor_;
        const size_t left = static_cast<size_t>(std::floor(pos));
        const double frac = pos - static_cast<double>(left);

        double value = 0.0;
        if (left + 1 < old_size) {
            value = static_cast<double>(src[left]) * (1.0 - frac) +
                    static_cast<double>(src[left + 1]) * frac;
        } else {
            value = static_cast<double>(src[left]);
        }

        dst[i] = static_cast<int16_t>(std::round(std::clamp(value, kMin, kMax)));
    }

    waveform.get_samples() = std::move(dst);
}

double TimestretchFilter::get_factor() const {
    return factor_;
}
