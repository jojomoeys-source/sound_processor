#include "application.h"

#include <iostream>

int main(int argc, char* argv[]) {
    Application app;
    try {
        app.configure();
        return app.start(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Error: unknown exception.\n";
        return 2;
    }
}
