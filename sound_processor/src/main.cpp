#include "application.h"

#include <iostream>

int main(int argc, char* argv[]) {
    static constexpr int kExitOk              = 0;
    static constexpr int kExitStdException    = 1;
    static constexpr int kExitUnknownException = 2;

    Application app;
    try {
        app.configure();
        return app.start(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return kExitStdException;
    } catch (...) {
        std::cerr << "Error: unknown exception.\n";
        return kExitUnknownException;
    }
}
