#pragma once
#include "filter.h"
#include "waveform.h"
#include <vector>
 
class Pipeline {
public:
    Pipeline() = default;
 
    ~Pipeline();
 
    // Копирование запрещено: Pipeline владеет фильтрами через raw ptr
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
 
    // Перемещение разрешено
    Pipeline(Pipeline&& other) noexcept;
    Pipeline& operator=(Pipeline&& other) noexcept;
 
    void add_filter(IFilter* filter);
 
    void apply(Waveform& waveform);
 
private:
    std::vector<IFilter*> filters_;
};