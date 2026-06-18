#pragma once

#include "args/args_parser.h"
#include "filter/filter.h"

#include <cstddef>

namespace filter_producers {

namespace detail {

double parse_double(const std::string& s, const char* filter, const char* param);
int parse_int(const std::string& s, const char* filter, const char* param);
void require_params(const FilterDescriptor& fd, size_t n);
void require_params_range(const FilterDescriptor& fd, size_t min_n, size_t max_n);

} // namespace detail

IFilter* make_ampl(const FilterDescriptor& fd);
IFilter* make_normalize(const FilterDescriptor& fd);
IFilter* make_silence(const FilterDescriptor& fd);
IFilter* make_timestretch(const FilterDescriptor& fd);
IFilter* make_lowpass(const FilterDescriptor& fd);
IFilter* make_highpass(const FilterDescriptor& fd);
IFilter* make_bandpass(const FilterDescriptor& fd);
IFilter* make_reject(const FilterDescriptor& fd);
IFilter* make_mute(const FilterDescriptor& fd);
IFilter* make_mix(const FilterDescriptor& fd);

IFilter* make_generator_sin(const FilterDescriptor& fd, size_t offset);
IFilter* make_generator_am(const FilterDescriptor& fd, size_t offset);
IFilter* make_generator_fm(const FilterDescriptor& fd, size_t offset);
IFilter* make_generator(const FilterDescriptor& fd);

} // namespace filter_producers
