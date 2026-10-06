#pragma once

#include "Animate/Pic/PicPage.h"
#include "Convert/Context.h"
#include "Model/ResourceTable.h"

#include <cstdint>

namespace sc2fla {
    bool PlaysOnItsOwn(const ResourceTable& resources, ConversionContext& ctx, uint32_t id);

    uint32_t PlayableLength(const ResourceTable& resources, ConversionContext& ctx, uint32_t id);

    void HoldLayersUntil(Animate::Pic::Page& page, uint32_t target);
}
