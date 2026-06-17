#include "filters/silence_filter.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

SilenceFilter::SilenceFilter(Unit unit, double start, double end)
    : unit_(unit), start_(start), end_(end) {
    if (!std::isfinite(start) || start < 0.0)
        throw std::invalid_argument("silence: start must be finite and >= 0");
    if (!std::isfinite(end) || end < start)
        throw std::invalid_argument("silence: end must be finite and >= start");
}

SilenceFilter::~SilenceFilter() = default;

SilenceFilter::Unit SilenceFilter::parse_unit(const std::string& s) {
    if (s == "sec") return Unit::Sec;
    if (s == "ms")  return Unit::Ms;
    throw std::invalid_argument("silence: unit must be 'sec' or 'ms', got: '" + s + "'");
}

void SilenceFilter::apply(Waveform& waveform) {
    const double to_sec = (unit_ == Unit::Ms) ? 0.001 : 1.0;
    const double start_sec = start_ * to_sec;
    const double end_sec   = end_   * to_sec;

    const size_t start_idx = waveform.seconds_to_samples(start_sec);
    const size_t end_idx   = waveform.seconds_to_samples(end_sec);

    if (end_idx < start_idx) return;
    const size_t silence_count = end_idx - start_idx + 1;

    auto& samples = waveform.get_samples();
    const size_t old_size = samples.size();

    if (start_idx >= old_size) {
        samples.resize(start_idx + silence_count, 0);
        return;
    }

    samples.insert(samples.begin() + static_cast<std::ptrdiff_t>(start_idx),
                   silence_count, 0);
}

SilenceFilter::Unit SilenceFilter::get_unit() const {
    return unit_;
}

double SilenceFilter::get_start() const {
    return start_;
}

double SilenceFilter::get_end() const {
    return end_;
}
