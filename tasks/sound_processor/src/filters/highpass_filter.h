#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

class HighpassFilter : public IFilter {
public:
    explicit HighpassFilter(int window_size);
    ~HighpassFilter() override;

    void apply(Waveform& waveform) override;

    int get_window_size() const;

private:
    int window_size_;
};
