#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

class LowpassFilter : public IFilter {
public:
    explicit LowpassFilter(int window_size);
    ~LowpassFilter() override;

    void apply(Waveform& waveform) override;

    int get_window_size() const;

private:
    int window_size_;
};
