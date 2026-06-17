#include "filters/normalize_filter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

NormalizeFilter::NormalizeFilter(double peak) : peak_(peak) {
    if (!std::isfinite(peak) || peak < 0.0 || peak > 1.0)
        throw std::invalid_argument("normalize: peak must be finite and in [0, 1]");
}

NormalizeFilter::~NormalizeFilter() = default;

void NormalizeFilter::apply(Waveform& waveform) {
    const size_t count = waveform.get_sample_count();
    if (count == 0) return;

    int32_t current_peak = 0;
    for (size_t i = 0; i < count; ++i) {
        const int32_t abs_s = std::abs(static_cast<int32_t>(waveform.get_sample_at(i)));
        if (abs_s > current_peak) current_peak = abs_s;
    }

    if (current_peak == 0) return;

    constexpr double kMax = static_cast<double>(std::numeric_limits<int16_t>::max());
    constexpr double kMin = static_cast<double>(std::numeric_limits<int16_t>::min());
    const double scale = peak_ * 32767.0 / static_cast<double>(current_peak);

    for (size_t i = 0; i < count; ++i) {
        const double scaled  = static_cast<double>(waveform.get_sample_at(i)) * scale;
        const double clamped = std::clamp(scaled, kMin, kMax);
        waveform.set_sample_at(i, static_cast<int16_t>(std::round(clamped)));
    }
}

double NormalizeFilter::get_peak() const {
    return peak_;
}
