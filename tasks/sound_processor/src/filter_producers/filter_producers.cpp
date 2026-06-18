#include "filter_producers/filter_producers.h"

#include "filters/ampl_filter.h"
#include "filters/normalize_filter.h"
#include "filters/silence_filter.h"
#include "filters/timestretch_filter.h"
#include "filters/lowpass_filter.h"
#include "filters/highpass_filter.h"
#include "filters/bandpass_filter.h"
#include "filters/reject_filter.h"
#include "filters/generator_filters.h"
#include "filters/mute_filter.h"
#include "filters/mix_filter.h"
#include "wav/wav_io.h"

#include <cmath>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace filter_producers {

namespace detail {

double parse_double(const std::string& s, const char* filter, const char* param) {
    try {
        size_t pos = 0;
        const double value = std::stod(s, &pos);
        if (pos != s.size() || !std::isfinite(value)) {
            throw std::invalid_argument("invalid number");
        }
        return value;
    } catch (const std::exception&) {
        throw std::runtime_error(
            std::string("Filter '") + filter + "': " + param +
            " must be a valid number, got: '" + s + "'");
    }
}

int parse_int(const std::string& s, const char* filter, const char* param) {
    try {
        size_t pos = 0;
        const int value = std::stoi(s, &pos);
        if (pos != s.size()) {
            throw std::invalid_argument("invalid integer");
        }
        return value;
    } catch (const std::exception&) {
        throw std::runtime_error(
            std::string("Filter '") + filter + "': " + param +
            " must be a valid integer, got: '" + s + "'");
    }
}

void require_params(const FilterDescriptor& fd, size_t n) {
    if (fd.params.size() != n) {
        throw std::runtime_error(
            "Filter '" + fd.name + "' requires " + std::to_string(n) +
            " argument(s), got " + std::to_string(fd.params.size()));
    }
}

void require_params_range(const FilterDescriptor& fd, size_t min_n, size_t max_n) {
    if (fd.params.size() < min_n || fd.params.size() > max_n) {
        throw std::runtime_error(
            "Filter '" + fd.name + "' requires from " + std::to_string(min_n) +
            " to " + std::to_string(max_n) + " argument(s), got " +
            std::to_string(fd.params.size()));
    }
}

} // namespace detail

IFilter* make_ampl(const FilterDescriptor& fd) {
    detail::require_params(fd, 1);
    const double factor = detail::parse_double(fd.params[0], "ampl", "factor");
    return new AmplFilter(factor);
}

IFilter* make_normalize(const FilterDescriptor& fd) {
    detail::require_params_range(fd, 0, 1);
    if (fd.params.empty()) {
        return new NormalizeFilter();
    }
    const double peak = detail::parse_double(fd.params[0], "normalize", "peak");
    return new NormalizeFilter(peak);
}

IFilter* make_silence(const FilterDescriptor& fd) {
    detail::require_params(fd, 3);
    const SilenceFilter::Unit unit = SilenceFilter::parse_unit(fd.params[0]);
    const double start = detail::parse_double(fd.params[1], "silence", "start");
    const double end   = detail::parse_double(fd.params[2], "silence", "end");
    return new SilenceFilter(unit, start, end);
}

IFilter* make_timestretch(const FilterDescriptor& fd) {
    detail::require_params(fd, 1);
    const double factor = detail::parse_double(fd.params[0], "timestretch", "factor");
    return new TimestretchFilter(factor);
}

IFilter* make_lowpass(const FilterDescriptor& fd) {
    detail::require_params(fd, 1);
    const int window = detail::parse_int(fd.params[0], "lowpass", "window_size");
    return new LowpassFilter(window);
}

IFilter* make_highpass(const FilterDescriptor& fd) {
    detail::require_params(fd, 1);
    const int window = detail::parse_int(fd.params[0], "highpass", "window_size");
    return new HighpassFilter(window);
}

IFilter* make_bandpass(const FilterDescriptor& fd) {
    detail::require_params(fd, 2);
    const int low_window = detail::parse_int(fd.params[0], "bandpass", "low_window_size");
    const int high_window = detail::parse_int(fd.params[1], "bandpass", "high_window_size");
    return new BandpassFilter(low_window, high_window);
}

IFilter* make_reject(const FilterDescriptor& fd) {
    detail::require_params(fd, 2);
    const int low_window = detail::parse_int(fd.params[0], "reject", "low_window_size");
    const int high_window = detail::parse_int(fd.params[1], "reject", "high_window_size");
    return new RejectFilter(low_window, high_window);
}

IFilter* make_mute(const FilterDescriptor& fd) {
    detail::require_params(fd, 2);
    const double start = detail::parse_double(fd.params[0], "mute", "start_sec");
    const double end   = detail::parse_double(fd.params[1], "mute", "end_sec");
    return new MuteFilter(start, end);
}

IFilter* make_mix(const FilterDescriptor& fd) {
    detail::require_params(fd, 2);
    const double start = detail::parse_double(fd.params[1], "mix", "start_sec");
    Waveform additional = WavReader::read(fd.params[0]);
    return new MixFilter(std::move(additional), start);
}

IFilter* make_generator_sin(const FilterDescriptor& fd, size_t offset) {
    detail::require_params(fd, offset + 2);
    const double freq = detail::parse_double(fd.params[offset],     "generator sin", "frequency_hz");
    const double dur  = detail::parse_double(fd.params[offset + 1], "generator sin", "duration_ms");
    return new SinGeneratorFilter(freq, dur);
}

IFilter* make_generator_am(const FilterDescriptor& fd, size_t offset) {
    detail::require_params(fd, offset + 5);
    const double amplitude     = detail::parse_double(fd.params[offset],     "generator am", "amplitude");
    const double carrier_hz    = detail::parse_double(fd.params[offset + 1], "generator am", "carrier_hz");
    const double modulation_hz = detail::parse_double(fd.params[offset + 2], "generator am", "modulation_hz");
    const double depth         = detail::parse_double(fd.params[offset + 3], "generator am", "depth");
    const double duration_ms   = detail::parse_double(fd.params[offset + 4], "generator am", "duration_ms");
    return new AmGeneratorFilter(amplitude, carrier_hz, modulation_hz, depth, duration_ms);
}

IFilter* make_generator_fm(const FilterDescriptor& fd, size_t offset) {
    detail::require_params(fd, offset + 5);
    const double amplitude     = detail::parse_double(fd.params[offset],     "generator fm", "amplitude");
    const double carrier_hz    = detail::parse_double(fd.params[offset + 1], "generator fm", "carrier_hz");
    const double modulation_hz = detail::parse_double(fd.params[offset + 2], "generator fm", "modulation_hz");
    const double deviation_hz  = detail::parse_double(fd.params[offset + 3], "generator fm", "deviation_hz");
    const double duration_ms   = detail::parse_double(fd.params[offset + 4], "generator fm", "duration_ms");
    return new FmGeneratorFilter(amplitude, carrier_hz, modulation_hz, deviation_hz, duration_ms);
}

IFilter* make_generator(const FilterDescriptor& fd) {
    if (fd.params.empty()) {
        throw std::runtime_error(
            "Filter 'generator' requires subtype: sin, am, or fm");
    }
    const std::string& subtype = fd.params[0];
    if (subtype == "sin") return make_generator_sin(fd, 1);
    if (subtype == "am")  return make_generator_am(fd,  1);
    if (subtype == "fm")  return make_generator_fm(fd,  1);
    throw std::runtime_error(
        "Filter 'generator': unknown subtype '" + subtype +
        "'. Expected: sin, am, fm");
}

} // namespace filter_producers
