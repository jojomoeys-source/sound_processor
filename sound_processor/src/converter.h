#pragma once

#include "args_parser.h"
#include "pipeline.h"
#include "filter.h"

#include <map>
#include <string>
#include <vector>

// Тип продюсера: указатель на функцию, принимающую дескриптор и возвращающую IFilter*.
// Использование указателя на функцию (а не std::function) согласуется с требованиями ТЗ:
// типоунифицированный, пригодный как значение ассоциативного массива.
using FilterProducer = IFilter* (*)(const FilterDescriptor&);

class CmdLineArgs2PipelineConverter {
public:
    CmdLineArgs2PipelineConverter() = default;

    // Регистрация продюсера для заданного имени фильтра
    void add_filter_producer(const std::string& filter_name, FilterProducer producer);

    // Построение пайплайна из списка дескрипторов.
    // Вся логика поиска продюсера, проверки ненулевых указателей и создания фильтров
    // изолирована внутри этого метода (требование ТЗ: no cross-cutting concern).
    Pipeline create_pipeline(const std::vector<FilterDescriptor>& descriptors) const;

private:
    // Поиск продюсера по имени; возвращает nullptr если не найден
    FilterProducer find_producer(const std::string& filter_name) const;

    std::map<std::string, FilterProducer> producers_;
};