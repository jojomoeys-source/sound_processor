#include "converter.h"

#include <stdexcept>

void CmdLineArgs2PipelineConverter::add_filter_producer(const std::string& filter_name,
                                                         FilterProducer producer) {
    producers_[filter_name] = std::move(producer);
}

FilterProducer CmdLineArgs2PipelineConverter::find_producer(const std::string& filter_name) const {
    auto it = producers_.find(filter_name);
    if (it == producers_.end()) {
        return nullptr;
    }
    return it->second;
}

Pipeline CmdLineArgs2PipelineConverter::create_pipeline(
        const std::vector<FilterDescriptor>& descriptors) const {

    Pipeline pipeline;

    for (const auto& fd : descriptors) {
        FilterProducer producer = find_producer(fd.name);

        if (!producer) {
            throw std::runtime_error("Unknown filter: '" + fd.name + "'");
        }

        IFilter* filter = producer(fd);

        if (!filter) {
            throw std::runtime_error("Filter producer returned null for: '" + fd.name + "'");
        }

        pipeline.add_filter(filter);
    }

    return pipeline;
}