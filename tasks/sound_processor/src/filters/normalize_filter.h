#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

class NormalizeFilter : public IFilter {
public:
    static constexpr double kDefaultPeak = 1.0;

    explicit NormalizeFilter(double peak = kDefaultPeak);
    ~NormalizeFilter() override;

    void apply(Waveform& waveform) override;

    double get_peak() const;

private:
    double peak_;
};
