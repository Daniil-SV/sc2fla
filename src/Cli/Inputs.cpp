#include "Cli/Inputs.h"

#include "Core/Console.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace sc2fla {
    namespace {
        namespace fs = std::filesystem;

        bool IsScFile(const fs::path& p) {
            std::string ext = p.extension().string();
            for (auto& c : ext)
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (ext != ".sc")
                return false;

            const std::string suffix = "_tex";
            const std::string stem = p.stem().string();
            return !(stem.size() >= suffix.size() && stem.compare(stem.size() - suffix.size(), suffix.size(), suffix) == 0);
        }
    }

    bool CollectInputs(const fs::path& input, std::vector<fs::path>& files) {
        if (fs::is_regular_file(input)) {
            if (IsScFile(input))
                files.push_back(input);
            else
                Console::Warning("Invalid file (expected .sc): " + input.string());
            return true;
        }

        if (fs::is_directory(input)) {
            for (auto& entry : fs::directory_iterator(input)) {
                if (entry.is_regular_file() && IsScFile(entry.path()))
                    files.push_back(entry.path());
            }
            std::sort(files.begin(), files.end());
            return true;
        }

        Console::Error("Path not found: " + input.string());
        return false;
    }
}
