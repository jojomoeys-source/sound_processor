#include "filters/highpass_filter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr double kInt16Min = static_cast<double>(std::numeric_limits<int16_t>::min());
constexpr double kInt16Max = static_cast<double>(std::numeric_limits<int16_t>::max());

void validate_odd_window(int window_size, const char* filter_name) {
    if (window_size < 1 || window_size % 2 == 0) {
        throw std::invalid_argument(
            std::string(filter_name) + ": window_size must be odd and >= 1");
    }
}

int16_t clamp_sample(double value) {
    return static_cast<int16_t>(std::round(std::clamp(value, kInt16Min, kInt16Max)));
}

double moving_average(
        const std::vector<int16_t>& samples,
        size_t index,
        int half,
        int window_size) {
    double sum = 0.0;
    for (int k = -half; k <= half; ++k) {
        const int idx_raw = static_cast<int>(index) + k;
        const size_t idx = static_cast<size_t>(
            std::clamp(idx_raw, 0, static_cast<int>(samples.size()) - 1));
        sum += static_cast<double>(samples[idx]);
    }
    return sum / static_cast<double>(window_size);
}

} // namespace

HighpassFilter::HighpassFilter(int window_size) : window_size_(window_size) {
    validate_odd_window(window_size_, "highpass");
}

HighpassFilter::~HighpassFilter() = default;

void HighpassFilter::apply(Waveform& waveform) {
    auto& samples = waveform.get_samples();
    if (samples.empty()) return;

    const int half = window_size_ / 2;
    std::vector<int16_t> result(samples.size());

    for (size_t i = 0; i < samples.size(); ++i) {
        result[i] = clamp_sample(static_cast<double>(samples[i]) -
                                 moving_average(samples, i, half, window_size_));
    }

    samples = std::move(result);
}

int HighpassFilter::get_window_size() const {
    return window_size_;
}
