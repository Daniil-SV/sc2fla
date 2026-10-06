#pragma once

#include "Convert/Context.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace sc2fla {
    struct ConvertOptions {
        std::optional<std::filesystem::path> external_texture;
        std::vector<std::string> groups;
        bool unpacked = false;
        std::optional<int> fps;
    };

    void ScToFla(const std::filesystem::path& input_path, const std::filesystem::path& output_dir, ConversionContext& ctx, const ConvertOptions& options);
}
