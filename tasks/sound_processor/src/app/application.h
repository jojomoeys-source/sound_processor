#pragma once

#include "converter/converter.h"

class Application {
public:
    Application() = default;

    void configure();

    int start(int argc, char* argv[]);

private:
    void print_help() const;

    CmdLineArgs2PipelineConverter converter_;
};