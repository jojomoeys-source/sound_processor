#pragma once

#include "args/args_parser.h"
#include "pipeline/pipeline.h"
#include "filter/filter.h"

#include <map>
#include <string>
#include <vector>

using FilterProducer = IFilter* (*)(const FilterDescriptor&);

class CmdLineArgs2PipelineConverter {
public:
    CmdLineArgs2PipelineConverter() = default;

    void add_filter_producer(const std::string& filter_name, FilterProducer producer);

    Pipeline create_pipeline(const std::vector<FilterDescriptor>& descriptors) const;

private:
    FilterProducer find_producer(const std::string& filter_name) const;

    std::map<std::string, FilterProducer> producers_;
};