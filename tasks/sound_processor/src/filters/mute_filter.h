#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

class MuteFilter : public IFilter {
public:
    MuteFilter(double start_sec, double end_sec);
    ~MuteFilter() override;

    void apply(Waveform& waveform) override;

    double get_start_sec() const;
    double get_end_sec()   const;

private:
    double start_sec_;
    double end_sec_;
};
