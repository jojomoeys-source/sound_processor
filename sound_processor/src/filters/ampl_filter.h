#pragma once
 
#include "filter.h"
#include "waveform.h"
 
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
 
// -f ampl <factor>
// Умножает каждый отсчёт на factor, результат зажимается в [INT16_MIN, INT16_MAX].
class AmplFilter : public IFilter {
public:
    explicit AmplFilter(double factor) : factor_(factor) {
        if (factor < 0.0)
            throw std::invalid_argument("ampl: factor must be >= 0");
    }
 
    ~AmplFilter() override = default;
 
    void apply(Waveform& waveform) override {
        constexpr double kMin = static_cast<double>(std::numeric_limits<int16_t>::min());
        constexpr double kMax = static_cast<double>(std::numeric_limits<int16_t>::max());
 
        for (size_t i = 0; i < waveform.get_sample_count(); ++i) {
            const double scaled  = static_cast<double>(waveform.get_sample_at(i)) * factor_;
            const double clamped = std::clamp(scaled, kMin, kMax);
            waveform.set_sample_at(i, static_cast<int16_t>(std::round(clamped)));
        }
    }
 
    double get_factor() const { return factor_; }
 
private:
    double factor_;
};