#pragma once

#include "filter.h"
#include "waveform.h"

#include <algorithm>
#include <stdexcept>
#include <string>

// -f silence <unit> <start> <end>
// Вставляет нулевые отсчёты в диапазон [start, end]; отсчёты после start сдвигаются вправо.
// unit: "sec" или "ms"
class SilenceFilter : public IFilter {
public:
    enum class Unit { Sec, Ms };

    SilenceFilter(Unit unit, double start, double end)
        : unit_(unit), start_(start), end_(end) {
        if (start < 0.0)
            throw std::invalid_argument("silence: start must be >= 0");
        if (end < start)
            throw std::invalid_argument("silence: end must be >= start");
    }

    ~SilenceFilter() override = default;

    static Unit parse_unit(const std::string& s) {
        if (s == "sec") return Unit::Sec;
        if (s == "ms")  return Unit::Ms;
        throw std::invalid_argument("silence: unit must be 'sec' or 'ms', got: '" + s + "'");
    }

    void apply(Waveform& waveform) override {
        const double to_sec = (unit_ == Unit::Ms) ? 0.001 : 1.0;
        const double start_sec = start_ * to_sec;
        const double end_sec   = end_   * to_sec;

        const size_t start_idx = waveform.seconds_to_samples(start_sec);
        const size_t end_idx   = waveform.seconds_to_samples(end_sec);

        // Число нулевых отсчётов для вставки
        if (end_idx <= start_idx) return;
        const size_t silence_count = end_idx - start_idx;

        auto& samples = waveform.get_samples();
        const size_t old_size = samples.size();

        // Если точка вставки за концом — просто дописываем нули
        if (start_idx >= old_size) {
            samples.resize(start_idx + silence_count, 0);
            return;
        }

        // Вставляем нули в позицию start_idx, сдвигая хвост вправо
        samples.insert(samples.begin() + static_cast<std::ptrdiff_t>(start_idx),
                       silence_count, 0);
    }

    Unit   get_unit()  const { return unit_; }
    double get_start() const { return start_; }
    double get_end()   const { return end_; }

private:
    Unit   unit_;
    double start_;
    double end_;
};