#pragma once
 
#include "filter/filter.h"
#include <cmath>
#include <stdexcept>
class MuteFilter : public IFilter {
public:
    MuteFilter(double start_sec, double end_sec)
        : start_sec_(start_sec), end_sec_(end_sec) {
        if (!std::isfinite(start_sec) || start_sec < 0.0)
            throw std::invalid_argument("mute: start_sec must be finite and >= 0");
        if (!std::isfinite(end_sec) || end_sec < start_sec)
            throw std::invalid_argument("mute: end_sec must be finite and >= start_sec");
    }
 
    ~MuteFilter() override = default;
 
    void apply(Waveform& waveform) override {
        if (waveform.get_sample_count() == 0) return;
 
        const size_t start = waveform.seconds_to_samples(start_sec_);
        const size_t end   = waveform.seconds_to_samples(end_sec_);
        const size_t count = waveform.get_sample_count();
 
        for (size_t i = start; i < end && i < count; ++i) {
            waveform.set_sample_at(i, 0);
        }
    }
 
    double get_start_sec() const { return start_sec_; }
    double get_end_sec()   const { return end_sec_; }
 
private:
    double start_sec_;
    double end_sec_;
};