#include "Shape/ConvertShape.h"

#include "Shape/SolidFill.h"
#include "Shape/Sprites.h"

#include <string>

namespace sc2fla {
    void ConvertShape(anim::Project& project, ConversionContext& ctx, const std::vector<RgbaImage>& textures, uint32_t id, const ShapeRecord& shape) {
        auto& graphic = project.MakeSymbol("Shapes/Shape " + std::to_string(id), Animate::Library::DocumentPage::SymbolType::Graphic);
        Animate::Pic::Page& page = graphic.GetPage();

        const SolidPlan plan = PlanSolidFills(shape);
        AddColorfillLayers(page, textures, shape, plan);
        AddFillLayer(page, textures, shape, plan);
        AddSpriteLayers(project, ctx, page, textures, shape, plan.is_solid);
    }
}
