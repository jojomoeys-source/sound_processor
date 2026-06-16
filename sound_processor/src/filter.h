#pragma once
#include "waveform.h"
 
class IFilter {
public:
    virtual ~IFilter() = default;
    
    virtual void apply(Waveform& waveform) = 0;
};