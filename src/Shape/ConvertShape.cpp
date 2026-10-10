#include "Shape/ConvertShape.h"

#include "Shape/SolidFill.h"
#include "Shape/Sprites.h"

#include <algorithm>
#include <string>

namespace sc2fla {
    namespace {
        bool NeedsMergedMask(const SolidPlan& plan) {
            if (plan.peels.size() + plan.components.size() < 2)
                return false;
            return std::all_of(plan.is_solid.begin(), plan.is_solid.end(), [](char solid) { return solid != 0; });
        }
    }

    void ConvertShape(anim::Project& project, ConversionContext& ctx, const std::vector<RgbaImage>& textures, uint32_t id, const ShapeRecord& shape) {
        auto& graphic = project.MakeSymbol("Shapes/Shape " + std::to_string(id), Animate::Library::DocumentPage::SymbolType::Graphic);
        Animate::Pic::Page& page = graphic.GetPage();

        const SolidPlan plan = PlanSolidFills(shape);
        AddColorfillLayers(page, textures, shape, plan);
        AddFillLayer(page, textures, shape, plan);
        AddSpriteLayers(project, ctx, page, textures, shape, plan.is_solid);

        if (ctx.mask_shapes.count(id) && NeedsMergedMask(plan)) {
            auto& mask_graphic = project.MakeSymbol("Shapes/Shape " + std::to_string(id) + " Mask", Animate::Library::DocumentPage::SymbolType::Graphic);
            AddMaskFillLayer(mask_graphic.GetPage(), textures, shape, plan);
        }
    }
}
