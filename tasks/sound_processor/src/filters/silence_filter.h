#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

#include <string>

class SilenceFilter : public IFilter {
public:
    enum class Unit { Sec, Ms };

    SilenceFilter(Unit unit, double start, double end);
    ~SilenceFilter() override;

    static Unit parse_unit(const std::string& s);

    void apply(Waveform& waveform) override;

    Unit   get_unit()  const;
    double get_start() const;
    double get_end()   const;

private:
    Unit   unit_;
    double start_;
    double end_;
};
