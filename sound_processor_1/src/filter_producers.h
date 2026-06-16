#pragma once

#include "args_parser.h"
#include "filter.h"
#include "wav_io.h"

#include "filters/ampl_filter.h"
#include "filters/normalize_filter.h"
#include "filters/silence_filter.h"
#include "filters/timestretch_filter.h"
#include "filters/lowpass_filter.h"
#include "filters/generator_filters.h"
#include "filters/mute_filter.h"
#include "filters/gain_filter.h"
#include "filters/mix_filter.h"

#include <stdexcept>
#include <string>

// Продюсеры фильтров.
// Каждый продюсер: валидирует параметры из дескриптора, создаёт фильтр, возвращает IFilter*.
// При ошибке бросает std::runtime_error с понятным сообщением.

namespace filter_producers {

// ─── Вспомогательные функции ─────────────────────────────────────────────────

namespace detail {

inline double parse_double(const std::string& s, const char* filter, const char* param) {
    try {
        return std::stod(s);
    } catch (const std::exception&) {
        throw std::runtime_error(
            std::string("Filter '") + filter + "': " + param +
            " must be a number, got: '" + s + "'");
    }
}

inline int parse_int(const std::string& s, const char* filter, const char* param) {
    try {
        return std::stoi(s);
    } catch (const std::exception&) {
        throw std::runtime_error(
            std::string("Filter '") + filter + "': " + param +
            " must be an integer, got: '" + s + "'");
    }
}

inline void require_params(const FilterDescriptor& fd, size_t n) {
    if (fd.params.size() < n) {
        throw std::runtime_error(
            "Filter '" + fd.name + "' requires " + std::to_string(n) +
            " argument(s), got " + std::to_string(fd.params.size()));
    }
}

} // namespace detail


// ─── Преобразующие фильтры ───────────────────────────────────────────────────

// -f ampl <factor>
inline IFilter* make_ampl(const FilterDescriptor& fd) {
    detail::require_params(fd, 1);
    const double factor = detail::parse_double(fd.params[0], "ampl", "factor");
    return new AmplFilter(factor);
}

// -f normalize [peak]
inline IFilter* make_normalize(const FilterDescriptor& fd) {
    if (fd.params.empty()) {
        return new NormalizeFilter();  // peak = 1.0 по умолчанию
    }
    const double peak = detail::parse_double(fd.params[0], "normalize", "peak");
    return new NormalizeFilter(peak);
}

// -f silence <unit> <start> <end>
inline IFilter* make_silence(const FilterDescriptor& fd) {
    detail::require_params(fd, 3);
    const SilenceFilter::Unit unit = SilenceFilter::parse_unit(fd.params[0]);
    const double start = detail::parse_double(fd.params[1], "silence", "start");
    const double end   = detail::parse_double(fd.params[2], "silence", "end");
    return new SilenceFilter(unit, start, end);
}

// -f timestretch <factor>
inline IFilter* make_timestretch(const FilterDescriptor& fd) {
    detail::require_params(fd, 1);
    const double factor = detail::parse_double(fd.params[0], "timestretch", "factor");
    return new TimestretchFilter(factor);
}

// -f lowpass <window_size>
inline IFilter* make_lowpass(const FilterDescriptor& fd) {
    detail::require_params(fd, 1);
    const int window = detail::parse_int(fd.params[0], "lowpass", "window_size");
    return new LowpassFilter(window);
}

// -f mute <start_sec> <end_sec>
inline IFilter* make_mute(const FilterDescriptor& fd) {
    detail::require_params(fd, 2);
    const double start = detail::parse_double(fd.params[0], "mute", "start_sec");
    const double end   = detail::parse_double(fd.params[1], "mute", "end_sec");
    return new MuteFilter(start, end);
}

// -f gain <factor>
inline IFilter* make_gain(const FilterDescriptor& fd) {
    detail::require_params(fd, 1);
    const double factor = detail::parse_double(fd.params[0], "gain", "factor");
    return new GainFilter(factor);
}

// -f mix <filename> <start_sec>
inline IFilter* make_mix(const FilterDescriptor& fd) {
    detail::require_params(fd, 2);
    const double start = detail::parse_double(fd.params[1], "mix", "start_sec");
    Waveform additional = WavReader::read(fd.params[0]);
    return new MixFilter(std::move(additional), start);
}


// ─── Фильтры-генераторы ──────────────────────────────────────────────────────

// -f generator sin <frequency_hz> <duration_ms>
inline IFilter* make_generator_sin(const FilterDescriptor& fd, size_t offset) {
    if (fd.params.size() < offset + 2) {
        throw std::runtime_error(
            "Filter 'generator sin' requires frequency_hz and duration_ms");
    }
    const double freq = detail::parse_double(fd.params[offset],     "generator sin", "frequency_hz");
    const double dur  = detail::parse_double(fd.params[offset + 1], "generator sin", "duration_ms");
    return new SinGeneratorFilter(freq, dur);
}

// -f generator am <amplitude> <carrier_hz> <modulation_hz> <depth> <duration_ms>
inline IFilter* make_generator_am(const FilterDescriptor& fd, size_t offset) {
    if (fd.params.size() < offset + 5) {
        throw std::runtime_error(
            "Filter 'generator am' requires amplitude carrier_hz modulation_hz depth duration_ms");
    }
    const double amplitude     = detail::parse_double(fd.params[offset],     "generator am", "amplitude");
    const double carrier_hz    = detail::parse_double(fd.params[offset + 1], "generator am", "carrier_hz");
    const double modulation_hz = detail::parse_double(fd.params[offset + 2], "generator am", "modulation_hz");
    const double depth         = detail::parse_double(fd.params[offset + 3], "generator am", "depth");
    const double duration_ms   = detail::parse_double(fd.params[offset + 4], "generator am", "duration_ms");
    return new AmGeneratorFilter(amplitude, carrier_hz, modulation_hz, depth, duration_ms);
}

// -f generator fm <amplitude> <carrier_hz> <modulation_hz> <deviation_hz> <duration_ms>
inline IFilter* make_generator_fm(const FilterDescriptor& fd, size_t offset) {
    if (fd.params.size() < offset + 5) {
        throw std::runtime_error(
            "Filter 'generator fm' requires amplitude carrier_hz modulation_hz deviation_hz duration_ms");
    }
    const double amplitude     = detail::parse_double(fd.params[offset],     "generator fm", "amplitude");
    const double carrier_hz    = detail::parse_double(fd.params[offset + 1], "generator fm", "carrier_hz");
    const double modulation_hz = detail::parse_double(fd.params[offset + 2], "generator fm", "modulation_hz");
    const double deviation_hz  = detail::parse_double(fd.params[offset + 3], "generator fm", "deviation_hz");
    const double duration_ms   = detail::parse_double(fd.params[offset + 4], "generator fm", "duration_ms");
    return new FmGeneratorFilter(amplitude, carrier_hz, modulation_hz, deviation_hz, duration_ms);
}

// -f generator <sin|am|fm> ...
// Диспетчер: первый параметр — разновидность генератора.
inline IFilter* make_generator(const FilterDescriptor& fd) {
    if (fd.params.empty()) {
        throw std::runtime_error(
            "Filter 'generator' requires subtype: sin, am, or fm");
    }
    const std::string& subtype = fd.params[0];
    // offset=1 — параметры после названия подтипа
    if (subtype == "sin") return make_generator_sin(fd, 1);
    if (subtype == "am")  return make_generator_am(fd,  1);
    if (subtype == "fm")  return make_generator_fm(fd,  1);
    throw std::runtime_error(
        "Filter 'generator': unknown subtype '" + subtype +
        "'. Expected: sin, am, fm");
}

} // namespace filter_producers