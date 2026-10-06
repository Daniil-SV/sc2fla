#pragma once

#include "Anim/Project.h"
#include "Convert/Context.h"
#include "Media/RgbaImage.h"
#include "Model/Shape.h"

#include <cstdint>
#include <vector>

namespace sc2fla {
    void ConvertShape(anim::Project& project, ConversionContext& ctx, const std::vector<RgbaImage>& textures, uint32_t id, const ShapeRecord& shape);
}
