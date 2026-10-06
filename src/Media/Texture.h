#pragma once

#include "Media/RgbaImage.h"

#include <filesystem>
#include <vector>

namespace sc::flash {
    class SWFTexture;
}

namespace sc2fla {
    RgbaImage DecodeTextureToRgba8(const sc::flash::SWFTexture& tex);

    std::vector<RgbaImage> LoadExternalTextures(const std::filesystem::path& path);
}
