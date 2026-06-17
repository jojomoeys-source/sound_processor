#include "pipeline/pipeline.h"

#include <stdexcept>
#include <utility>

void Pipeline::add_filter(IFilter* filter) {
    if (!filter) {
        throw std::invalid_argument("Pipeline: cannot add null filter");
    }
    filters_.push_back(filter);
}

void Pipeline::apply(Waveform& waveform) {
    for (IFilter* filter : filters_) {
        filter->apply(waveform);
    }
}

Pipeline::~Pipeline() {
    for (IFilter* filter : filters_) {
        delete filter;
    }
    filters_.clear();
}

Pipeline::Pipeline(Pipeline&& other) noexcept
    : filters_(std::move(other.filters_)) {}

Pipeline& Pipeline::operator=(Pipeline&& other) noexcept {
    if (this != &other) {
        for (IFilter* filter : filters_) {
            delete filter;
        }
        filters_ = std::move(other.filters_);
    }
    return *this;
}