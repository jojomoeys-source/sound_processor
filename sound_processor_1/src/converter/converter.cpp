#include "converter/converter.h"

#include <stdexcept>

void CmdLineArgs2PipelineConverter::add_filter_producer(const std::string& filter_name,
                                                         FilterProducer producer) {
    if (!producer) {
        throw std::invalid_argument("Filter producer for '" + filter_name + "' is null");
    }
    producers_[filter_name] = producer;
}

FilterProducer CmdLineArgs2PipelineConverter::find_producer(const std::string& filter_name) const {
    const auto it = producers_.find(filter_name);
    if (it == producers_.end()) {
        return nullptr;
    }
    return it->second;
}

Pipeline CmdLineArgs2PipelineConverter::create_pipeline(
        const std::vector<FilterDescriptor>& descriptors) const {

    Pipeline pipeline;

    for (const auto& fd : descriptors) {
        // Поиск продюсера, проверка null, создание фильтра — всё изолировано здесь
        // (требование ТЗ: no cross-cutting concern)
        FilterProducer producer = find_producer(fd.name);

        if (!producer) {
            throw std::runtime_error("No filter producer registered for: '" + fd.name + "'");
        }

        IFilter* filter = producer(fd);

        if (!filter) {
            throw std::runtime_error("Filter producer returned null for: '" + fd.name + "'");
        }

        try {
            pipeline.add_filter(filter);
        } catch (...) {
            delete filter;
            throw;
        }
    }

    return pipeline;
}