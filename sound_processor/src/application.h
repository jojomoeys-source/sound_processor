#pragma once

#include "args_parser.h"
#include "converter.h"

class Application {
public:
    Application() = default;

    // Регистрирует все известные продюсеры фильтров.
    // Вызывать один раз до start().
    void configure();

    // Запускает основной цикл: парсинг АКС → построение пайплайна → I/O → apply.
    // Возвращает код завершения для main().
    int start(int argc, char* argv[]);

private:
    void print_help() const;

    CmdLineArgs2PipelineConverter converter_;
};