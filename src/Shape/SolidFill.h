#pragma once

#include "Animate/Pic/PicPage.h"
#include "Media/RgbaImage.h"
#include "Model/Shape.h"
#include "Shape/Boundary.h"

#include <cstddef>
#include <vector>

namespace sc2fla {
    struct SolidPlan {
        std::vector<char> is_solid;
        UniquePoints unique;
        std::vector<size_t> peels;
        std::vector<std::vector<size_t>> components;
        size_t anchor = 0;
        bool have_anchor = false;
    };

    SolidPlan PlanSolidFills(const ShapeRecord& shape);

    void AddColorfillLayers(Animate::Pic::Page& page, const std::vector<RgbaImage>& textures, const ShapeRecord& shape, const SolidPlan& plan);

    void AddFillLayer(Animate::Pic::Page& page, const std::vector<RgbaImage>& textures, const ShapeRecord& shape, const SolidPlan& plan);

    void AddMaskFillLayer(Animate::Pic::Page& page, const std::vector<RgbaImage>& textures, const ShapeRecord& shape, const SolidPlan& plan);
}
