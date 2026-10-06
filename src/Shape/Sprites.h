#pragma once

#include "Anim/Project.h"
#include "Animate/Pic/PicPage.h"
#include "Convert/Context.h"
#include "Media/RgbaImage.h"
#include "Model/Shape.h"

#include <vector>

namespace sc2fla {
    void AddSpriteLayers(anim::Project& project,
                         ConversionContext& ctx,
                         Animate::Pic::Page& page,
                         const std::vector<RgbaImage>& textures,
                         const ShapeRecord& shape,
                         const std::vector<char>& is_solid);
}
