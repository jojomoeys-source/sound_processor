#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

class RejectFilter : public IFilter {
public:
    RejectFilter(int low_window_size, int high_window_size);
    ~RejectFilter() override;

    void apply(Waveform& waveform) override;

    int get_low_window_size() const;
    int get_high_window_size() const;

private:
    int low_window_size_;
    int high_window_size_;
};
