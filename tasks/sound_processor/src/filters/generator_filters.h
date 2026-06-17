#pragma once

#include "filter/filter.h"
#include "waveform/waveform.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class AbstractGeneratorFilter : public IFilter {
public:
    ~AbstractGeneratorFilter() override;

    void apply(Waveform& waveform) override;

protected:
    virtual size_t sample_count() const = 0;
    virtual void generate(std::vector<int16_t>& buf) const = 0;
};

class SinGeneratorFilter : public AbstractGeneratorFilter {
public:
    SinGeneratorFilter(double frequency_hz, double duration_ms);
    ~SinGeneratorFilter() override;

    double get_frequency_hz() const;

protected:
    size_t sample_count() const override;
    void generate(std::vector<int16_t>& buf) const override;

private:
    double freq_;
    size_t n_;
};

class AmGeneratorFilter : public AbstractGeneratorFilter {
public:
    AmGeneratorFilter(double amplitude, double carrier_hz,
                      double modulation_hz, double depth, double duration_ms);
    ~AmGeneratorFilter() override;

protected:
    size_t sample_count() const override;
    void generate(std::vector<int16_t>& buf) const override;

private:
    double amplitude_;
    double carrier_hz_;
    double modulation_hz_;
    double depth_;
    size_t n_;
};

class FmGeneratorFilter : public AbstractGeneratorFilter {
public:
    FmGeneratorFilter(double amplitude, double carrier_hz,
                      double modulation_hz, double deviation_hz, double duration_ms);
    ~FmGeneratorFilter() override;

protected:
    size_t sample_count() const override;
    void generate(std::vector<int16_t>& buf) const override;

private:
    double amplitude_;
    double carrier_hz_;
    double modulation_hz_;
    double deviation_hz_;
    size_t n_;
};
