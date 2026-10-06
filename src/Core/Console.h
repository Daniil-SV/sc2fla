#pragma once

#include <string>

namespace sc2fla {
    class Console {
    public:
        static void Info(const std::string& message);
        static void Warning(const std::string& message);
        static void Error(const std::string& message);
    };
}
