#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

class AmplFilter : public IFilter {
public:
    explicit AmplFilter(double factor);
    ~AmplFilter() override;

    void apply(Waveform& waveform) override;

    double get_factor() const;

private:
    double factor_;
};
