#include "filters/ampl_filter.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

AmplFilter::AmplFilter(double factor) : factor_(factor) {
    if (!std::isfinite(factor) || factor < 0.0)
        throw std::invalid_argument("ampl: factor must be finite and >= 0");
}

AmplFilter::~AmplFilter() = default;

void AmplFilter::apply(Waveform& waveform) {
    constexpr double kMin = static_cast<double>(std::numeric_limits<int16_t>::min());
    constexpr double kMax = static_cast<double>(std::numeric_limits<int16_t>::max());

    for (size_t i = 0; i < waveform.get_sample_count(); ++i) {
        const double scaled  = static_cast<double>(waveform.get_sample_at(i)) * factor_;
        const double clamped = std::clamp(scaled, kMin, kMax);
        waveform.set_sample_at(i, static_cast<int16_t>(std::round(clamped)));
    }
}

double AmplFilter::get_factor() const {
    return factor_;
}
