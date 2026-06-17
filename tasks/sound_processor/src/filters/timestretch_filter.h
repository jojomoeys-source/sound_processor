#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

class TimestretchFilter : public IFilter {
public:
    explicit TimestretchFilter(double factor);
    ~TimestretchFilter() override;

    void apply(Waveform& waveform) override;

    double get_factor() const;

private:
    double factor_;
};
