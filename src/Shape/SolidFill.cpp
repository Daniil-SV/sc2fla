#include "Shape/SolidFill.h"

#include "Animate/Fill/Style/SolidColor.h"
#include "Animate/Pic/PicFrame.h"
#include "Animate/Pic/PicLayer.h"
#include "Animate/Pic/PicRect.h"
#include "Animate/Pic/PicShape.h"
#include "Shape/Edges.h"
#include "Shape/Geometry.h"

#include <algorithm>
#include <string>

namespace sc2fla {
    namespace {
        bool IsAllSameUv(const ShapeBitmapCommand& bitmap) {
            for (const Point2& c : bitmap.uv_coords) {
                if (c.x != bitmap.uv_coords[0].x || c.y != bitmap.uv_coords[0].y)
                    return false;
            }
            return true;
        }

        bool FrontIndexLess(const std::vector<size_t>& a, const std::vector<size_t>& b) {
            return a.front() < b.front();
        }

        Animate::Fill::SolidFillStyle MakeSolidFill(const std::vector<RgbaImage>& textures, const ShapeBitmapCommand& bitmap) {
            const RgbaImage& texture_image = textures.at(static_cast<size_t>(bitmap.texture_index));
            long x = static_cast<long>(bitmap.uv_coords[0].x);
            long y = static_cast<long>(bitmap.uv_coords[0].y);
            uint8_t r = 0, g = 0, b = 0, a = 255;
            if (x >= 0 && y >= 0 && x < texture_image.width && y < texture_image.height) {
                size_t off = (static_cast<size_t>(y) * texture_image.width + x) * 4;
                r = texture_image.pixels[off];
                g = texture_image.pixels[off + 1];
                b = texture_image.pixels[off + 2];
                a = texture_image.pixels[off + 3];
            }
            Animate::Fill::SolidFillStyle solid;
            solid.fill_color = Animate::ColorAlpha {r, g, b, a};
            return solid;
        }
    }

    SolidPlan PlanSolidFills(const ShapeRecord& shape) {
        const std::vector<ShapeBitmapCommand>& bitmaps = shape.bitmaps;

        SolidPlan plan;
        plan.is_solid.assign(bitmaps.size(), 0);
        plan.unique.resize(bitmaps.size());

        std::vector<size_t> remaining;
        for (size_t bi = 0; bi < bitmaps.size(); ++bi) {
            if (!IsAllSameUv(bitmaps[bi]))
                continue;
            plan.is_solid[bi] = 1;
            plan.unique[bi] = UniqueXyPoints(bitmaps[bi].xy_coords);
            remaining.push_back(bi);
        }

        bool peel_stopped_on_triangle = false;
        while (!remaining.empty()) {
            size_t last = remaining.back();
            bool isolated = true;
            for (size_t k = 0; k + 1 < remaining.size(); ++k) {
                if (SharesVertex(plan.unique[last], plan.unique[remaining[k]])) {
                    isolated = false;
                    break;
                }
            }
            if (!isolated)
                break;
            if (plan.unique[last].size() < 4) {
                peel_stopped_on_triangle = true;
                break;
            }
            plan.peels.push_back(last);
            remaining.pop_back();
        }

        if (!remaining.empty())
            plan.components = ConnectedComponents(plan.unique, remaining);
        for (auto& comp : plan.components)
            std::sort(comp.begin(), comp.end());

        if (!remaining.empty() && !peel_stopped_on_triangle) {
            plan.have_anchor = true;
            plan.anchor = remaining.back();
            std::stable_partition(plan.components.begin(), plan.components.end(), [&](const std::vector<size_t>& comp) {
                return std::find(comp.begin(), comp.end(), plan.anchor) != comp.end();
            });
            std::sort(plan.components.begin() + 1, plan.components.end(), FrontIndexLess);
        } else if (!remaining.empty()) {
            std::sort(plan.components.begin(), plan.components.end(), FrontIndexLess);
        }
        return plan;
    }

    void AddColorfillLayers(Animate::Pic::Page& page, const std::vector<RgbaImage>& textures, const ShapeRecord& shape, const SolidPlan& plan) {
        int colorfill_n = 0;
        for (size_t idx : plan.peels) {
            Animate::Pic::Layer& layer = page.AddNewLayer("Colorfill " + std::to_string(colorfill_n++), false, false);
            Animate::Pic::Frame& frame = layer.CreateFrame();

            double rx, ry, rw, rh;
            if (AxisAlignedRect(plan.unique[idx], rx, ry, rw, rh)) {
                Animate::Pic::Rectangle& rect = frame.AddFrameElement<Animate::Pic::Rectangle>();
                rect.SetPrimitiveBound(Animate::Bound(static_cast<float>(rx), static_cast<float>(ry), static_cast<float>(rw), static_cast<float>(rh)));
                rect.SetPrimitiveFill(MakeSolidFill(textures, shape.bitmaps[idx]));
                continue;
            }

            std::string edges_str = shape.bitmaps.size() == 1 ? LoopToEdgesXml(PerimeterOrderStartingAfterFirst(shape.bitmaps[idx].xy_coords))
                                                              : LoopToEdgesXml(PerimeterOrder(plan.unique[idx]));
            Animate::Pic::Shape& color_fill = frame.AddFrameElement<Animate::Pic::Shape>();
            Animate::Fill::SolidFillStyle solid = MakeSolidFill(textures, shape.bitmaps[idx]);
            color_fill.CreateEdge(solid, edges_str);
        }
    }

    void AddFillLayer(Animate::Pic::Page& page, const std::vector<RgbaImage>& textures, const ShapeRecord& shape, const SolidPlan& plan) {
        if (plan.components.empty())
            return;

        Animate::Pic::Layer& layer = page.AddNewLayer("Fill " + std::to_string(plan.peels.size()), false, false);
        Animate::Pic::Frame& frame = layer.CreateFrame();

        for (const auto& comp : plan.components) {
            std::string edges_str;
            std::vector<std::string> extra_outer_edges;
            std::vector<std::string> hole_edges;
            const ShapeBitmapCommand& representative = shape.bitmaps[comp.front()];

            if (comp.size() == 1) {
                edges_str = LoopToEdgesXml(PerimeterOrderStartingAfterFirst(representative.xy_coords));
            } else {
                std::vector<std::vector<Point2>> loops = TraceBoundary(plan.unique, comp);

                if (!loops.empty()) {
                    ClassifiedLoops classified = ClassifyLoops(loops);
                    auto rotate_to_anchor = [&](std::vector<Point2>& loop) {
                        if (!plan.have_anchor || std::find(comp.begin(), comp.end(), plan.anchor) == comp.end())
                            return;
                        const std::vector<Point2>& anchor_unique = plan.unique[plan.anchor];
                        if (!anchor_unique.empty())
                            loop = RotateLoopToStartAt(loop, anchor_unique.back());
                    };
                    for (size_t li = 0; li < classified.outers.size(); ++li) {
                        std::vector<Point2> loop = classified.outers[li];
                        rotate_to_anchor(loop);
                        if (li == 0)
                            edges_str = LoopToEdgesXml(loop);
                        else
                            extra_outer_edges.push_back(LoopToEdgesXml(loop));
                    }
                    for (auto& hole : classified.holes)
                        hole_edges.push_back(LoopToEdgesXml(hole));
                }
            }

            Animate::Pic::Shape& color_fill = frame.AddFrameElement<Animate::Pic::Shape>();

            if (plan.components.size() > 1)
                color_fill.ConvertToDrawingObject();
            Animate::Fill::SolidFillStyle solid = MakeSolidFill(textures, representative);
            color_fill.CreateEdge(solid, edges_str);
            for (const std::string& extra : extra_outer_edges)
                color_fill.CreateEdge(solid, extra, false);

            for (const std::string& hole : hole_edges)
                color_fill.CreateEdge(solid, hole, true);
        }
    }
}
