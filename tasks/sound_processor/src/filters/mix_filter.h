#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

class MixFilter : public IFilter {
public:
    MixFilter(Waveform additional, double start_sec);
    ~MixFilter() override;

    void apply(Waveform& waveform) override;

    double get_start_sec()                    const;
    const Waveform& get_additional_waveform() const;

private:
    Waveform additional_;
    double   start_sec_;
};
