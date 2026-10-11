#pragma once

#include <optional>
#include <string>
#include <vector>

namespace sc2fla {
    struct Arguments {
        std::string input;
        std::string texture;
        std::string output;
        std::string fps;
        std::vector<std::string> groups;
        bool help = false;
        bool xfl = false;
    };

    Arguments ParseArguments(const std::vector<std::string>& args);

    bool ParseFps(const std::string& text, std::optional<int>& fps);

    void PrintHelp();
}
