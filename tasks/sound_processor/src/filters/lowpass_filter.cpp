#include "filters/lowpass_filter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

LowpassFilter::LowpassFilter(int window_size) : window_size_(window_size) {
    if (window_size < 1 || window_size % 2 == 0)
        throw std::invalid_argument(
            "lowpass: window_size must be odd and >= 1");
}

LowpassFilter::~LowpassFilter() = default;

void LowpassFilter::apply(Waveform& waveform) {
    const size_t count = waveform.get_sample_count();
    if (count == 0) return;

    const int half = window_size_ / 2;
    const auto& src = waveform.get_samples();
    std::vector<int16_t> dst(count);

    for (size_t i = 0; i < count; ++i) {
        double sum = 0.0;
        for (int k = -half; k <= half; ++k) {
            const int idx_raw = static_cast<int>(i) + k;
            const size_t idx  = static_cast<size_t>(
                std::clamp(idx_raw, 0, static_cast<int>(count) - 1));
            sum += static_cast<double>(src[idx]);
        }
        dst[i] = static_cast<int16_t>(std::round(sum / window_size_));
    }

    waveform.get_samples() = std::move(dst);
}

int LowpassFilter::get_window_size() const {
    return window_size_;
}
