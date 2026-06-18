#pragma once
#include "filter/filter.h"
#include "waveform/waveform.h"
#include <vector>
 
class Pipeline {
public:
    Pipeline() = default;
 
    ~Pipeline();
 
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
 
    Pipeline(Pipeline&& other) noexcept;
    Pipeline& operator=(Pipeline&& other) noexcept;
 
    void add_filter(IFilter* filter);

    size_t get_filter_count() const { return filters_.size(); }

    IFilter* operator[](size_t index) const { return filters_.at(index); }

    void apply(Waveform& waveform);
 
private:
    std::vector<IFilter*> filters_;
};