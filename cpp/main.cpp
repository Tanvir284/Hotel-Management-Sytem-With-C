#include "HotelApp.hpp"
#include "Terminal.hpp"

#include <exception>
#include <iostream>

// Entry point. All fatal errors bubble up here and are reported cleanly,
// satisfying the "modern C++ error handling" requirement at the top level.
int main() {
    try {
        HotelApp app;
        return app.run();
    } catch (const std::exception& e) {
        Terminal::resetColor();
        std::cerr << "\n Fatal error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        Terminal::resetColor();
        std::cerr << "\n Fatal unknown error.\n";
        return 1;
    }
}
