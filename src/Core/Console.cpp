#include "Core/Console.h"

#include <iostream>

namespace sc2fla {
    namespace {
        constexpr const char* GREEN = "\x1b[32m";
        constexpr const char* MAGENTA = "\x1b[35m";
        constexpr const char* RED = "\x1b[31m";
        constexpr const char* RESET = "\x1b[0m";
    }

    void Console::Info(const std::string& message) {
        std::cout << GREEN << "[INFO] " << message << RESET << std::endl;
    }

    void Console::Warning(const std::string& message) {
        std::cout << MAGENTA << "[WARN] " << message << RESET << std::endl;
    }

    void Console::Error(const std::string& message) {
        std::cerr << RED << "[ERROR] " << message << RESET << std::endl;
    }
}
