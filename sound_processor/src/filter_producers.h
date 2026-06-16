#pragma once

#include "args_parser.h"
#include "filter.h"
#include "wav_io.h"

#include "mute_filter.h"
#include "gain_filter.h"
#include "mix_filter.h"

#include <stdexcept>
#include <string>

// Продюсеры фильтров.
// Каждый продюсер: валидирует параметры из дескриптора, создаёт фильтр, возвращает IFilter*.
// При ошибке бросает std::runtime_error с понятным сообщением.

namespace filter_producers {

inline IFilter* make_mute(const FilterDescriptor& fd) {
    // -f mute <start_sec> <end_sec>
    if (fd.params.size() < 2) {
        throw std::runtime_error(
            "Filter 'mute' requires 2 arguments: <start_sec> <end_sec>");
    }
    double start = 0.0;
    double end   = 0.0;
    try {
        start = std::stod(fd.params[0]);
        end   = std::stod(fd.params[1]);
    } catch (const std::exception&) {
        throw std::runtime_error(
            "Filter 'mute': arguments must be numbers, got: '"
            + fd.params[0] + "' '" + fd.params[1] + "'");
    }
    return new MuteFilter(start, end);
}

inline IFilter* make_gain(const FilterDescriptor& fd) {
    // -f gain <factor>
    if (fd.params.size() < 1) {
        throw std::runtime_error(
            "Filter 'gain' requires 1 argument: <factor>");
    }
    double factor = 0.0;
    try {
        factor = std::stod(fd.params[0]);
    } catch (const std::exception&) {
        throw std::runtime_error(
            "Filter 'gain': argument must be a number, got: '" + fd.params[0] + "'");
    }
    return new GainFilter(factor);
}

inline IFilter* make_mix(const FilterDescriptor& fd) {
    // -f mix <filename> <start_sec>
    if (fd.params.size() < 2) {
        throw std::runtime_error(
            "Filter 'mix' requires 2 arguments: <filename> <start_sec>");
    }
    double start = 0.0;
    try {
        start = std::stod(fd.params[1]);
    } catch (const std::exception&) {
        throw std::runtime_error(
            "Filter 'mix': start_sec must be a number, got: '" + fd.params[1] + "'");
    }
    Waveform additional = WavReader::read(fd.params[0]);
    return new MixFilter(additional, start);
}

} // namespace filter_producers