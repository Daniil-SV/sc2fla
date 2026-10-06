#pragma once

#include <filesystem>
#include <vector>

namespace sc2fla {
    bool CollectInputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& files);
}
