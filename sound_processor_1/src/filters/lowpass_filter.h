#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

#include <cmath>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <vector>

// -f lowpass <window_size>
// Сглаживает сигнал скользящим средним по окну нечётного размера.
// На краях сигнала выходящие за границу индексы заменяются крайними значениями.
class LowpassFilter : public IFilter {
public:
    explicit LowpassFilter(int window_size) : window_size_(window_size) {
        if (window_size < 1 || window_size % 2 == 0)
            throw std::invalid_argument(
                "lowpass: window_size must be odd and >= 1");
    }

    ~LowpassFilter() override = default;

    void apply(Waveform& waveform) override {
        const size_t count = waveform.get_sample_count();
        if (count == 0) return;

        const int half = window_size_ / 2;
        const auto& src = waveform.get_samples();
        std::vector<int16_t> dst(count);

        for (size_t i = 0; i < count; ++i) {
            double sum = 0.0;
            for (int k = -half; k <= half; ++k) {
                // Индекс с клэмпингом на края
                const int idx_raw = static_cast<int>(i) + k;
                const size_t idx  = static_cast<size_t>(
                    std::clamp(idx_raw, 0, static_cast<int>(count) - 1));
                sum += static_cast<double>(src[idx]);
            }
            dst[i] = static_cast<int16_t>(std::round(sum / window_size_));
        }

        waveform.get_samples() = std::move(dst);
    }

    int get_window_size() const { return window_size_; }

private:
    int window_size_;
};