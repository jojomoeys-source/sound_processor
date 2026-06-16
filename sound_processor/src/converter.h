#pragma once

#include "args_parser.h"
#include "pipeline.h"
#include "filter.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

// Тип продюсера: функция, принимающая дескриптор фильтра и возвращающая IFilter*
using FilterProducer = std::function<IFilter*(const FilterDescriptor&)>;

class CmdLineArgs2PipelineConverter {
public:
    CmdLineArgs2PipelineConverter() = default;

    // Регистрация продюсера для заданного имени фильтра
    void add_filter_producer(const std::string& filter_name, FilterProducer producer);

    // Построение пайплайна из списка дескрипторов
    Pipeline create_pipeline(const std::vector<FilterDescriptor>& descriptors) const;

private:
    // Поиск продюсера по имени; возвращает nullptr если не найден
    FilterProducer find_producer(const std::string& filter_name) const;

    std::map<std::string, FilterProducer> producers_;
};