#pragma once

#include "filter.h"
#include "waveform.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

// Накладывает (mix) дополнительный сигнал поверх основного.
// additional — второй waveform, start_sec — момент начала наложения в основном сигнале.
// Выборки суммируются и зажимаются в [INT16_MIN, INT16_MAX].
class MixFilter : public IFilter {
public:
    MixFilter(Waveform additional, double start_sec)
        : additional_(std::move(additional)), start_sec_(start_sec) {}

    ~MixFilter() override = default;

    void apply(Waveform& waveform) override {
        if (additional_.get_sample_count() == 0) return;

        constexpr double kMin = static_cast<double>(std::numeric_limits<int16_t>::min());
        constexpr double kMax = static_cast<double>(std::numeric_limits<int16_t>::max());

        // Если основной waveform пустой, инициализируем метаданными из additional
        if (waveform.get_sample_count() == 0) {
            waveform.set_meta_info(additional_.get_sample_rate(),
                                   additional_.get_num_channels(),
                                   additional_.get_bits_per_sample());
        }

        const size_t offset = waveform.seconds_to_samples(start_sec_);
        const size_t needed = offset + additional_.get_sample_count();

        // Расширяем основной сигнал, если он короче
        if (needed > waveform.get_sample_count()) {
            waveform.resize(needed);
        }

        for (size_t i = 0; i < additional_.get_sample_count(); ++i) {
            const double sum = static_cast<double>(waveform.get_sample_at(offset + i))
                             + static_cast<double>(additional_.get_sample_at(i));
            const double clamped = std::clamp(sum, kMin, kMax);
            waveform.set_sample_at(offset + i, static_cast<int16_t>(std::round(clamped)));
        }
    }

    double get_start_sec()                    const { return start_sec_; }
    const Waveform& get_additional_waveform() const { return additional_; }

private:
    Waveform additional_;
    double   start_sec_;
};