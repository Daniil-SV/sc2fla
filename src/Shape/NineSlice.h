#pragma once

#include "Anim/Project.h"
#include "Animate/Pic/PicGroup.h"
#include "Convert/Context.h"

#include <cstdint>
#include <vector>

namespace sc2fla {
    std::vector<Animate::Pic::Group*> PatchShapeNineSlice(anim::Project& project, ConversionContext& ctx, uint32_t id);
}
