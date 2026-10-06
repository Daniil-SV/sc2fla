#include "Shape/NineSlice.h"

#include "Anim/Convert.h"
#include "Animate/Fill/Style/Bitmap.h"
#include "Animate/Pic/PicBitmap.h"
#include "Animate/Pic/PicFrame.h"
#include "Animate/Pic/PicLayer.h"
#include "Animate/Pic/PicPage.h"
#include "Animate/Pic/PicShape.h"
#include "Core/Math.h"

#include <cmath>
#include <sstream>
#include <string>

namespace sc2fla {
    std::vector<Animate::Pic::Group*> PatchShapeNineSlice(anim::Project& project, ConversionContext& ctx, uint32_t id) {
        auto cached = ctx.shapes_with_nine_slices.find(id);
        if (cached != ctx.shapes_with_nine_slices.end()) {
            std::vector<Animate::Pic::Group*> out;
            for (auto& g : cached->second)
                out.push_back(g.get());
            return out;
        }

        std::vector<std::unique_ptr<Animate::Pic::Group>> pieces;

        std::string key = "Shapes/Shape " + std::to_string(id);
        if (!project.HasSymbol(key)) {
            std::vector<Animate::Pic::Group*> out;
            ctx.shapes_with_nine_slices[id] = std::move(pieces);
            return out;
        }

        Animate::Library::DocumentPage& shape_symbol = project.GetSymbol(key);
        Animate::Pic::Page& page = shape_symbol.GetPage();

        for (Animate::Pic::Layer& layer : page) {
            for (Animate::Pic::Frame& frame : layer) {
                for (Animate::Pic::Object& element : frame) {
                    auto new_piece = std::make_unique<Animate::Pic::Group>();

                    if (!element.IsPicBitmap()) {
                        new_piece->AddElementReference(element);
                        pieces.push_back(std::move(new_piece));
                        continue;
                    }

                    auto& bmp_inst = static_cast<Animate::Pic::Bitmap&>(element);
                    Animate::Library::MediaBits& media = bmp_inst.GetBits();

                    std::string lin = media.GetItemPath().string();
                    size_t space = lin.find_last_of(' ');
                    std::string idx_str = (space != std::string::npos) ? lin.substr(space + 1) : lin;
                    size_t uvs_index = 0;
                    bool have_index = true;
                    try {
                        uvs_index = static_cast<size_t>(std::stoul(idx_str));
                    } catch (...) {
                        have_index = false;
                    }

                    if (!have_index || uvs_index >= ctx.sprites.size()) {
                        new_piece->AddElementReference(element);
                        pieces.push_back(std::move(new_piece));
                        continue;
                    }

                    long w = ctx.sprites[uvs_index].width;
                    long h = ctx.sprites[uvs_index].height;
                    std::ostringstream edge_shape;

                    Animate::Matrix inst_matrix;
                    bool got_matrix = bmp_inst.GetMatrix(inst_matrix);
                    if (!got_matrix)
                        inst_matrix = Animate::Matrix::Identity();

                    auto absolute = [&](double px, double py) -> Point2 {
                        return {inst_matrix.a * px + inst_matrix.c * py + inst_matrix.tx, inst_matrix.b * px + inst_matrix.d * py + inst_matrix.ty};
                    };
                    std::vector<Point2> corners_px = {{0, 0},
                                                      {static_cast<double>(w), 0},
                                                      {static_cast<double>(w), static_cast<double>(h)},
                                                      {0, static_cast<double>(h)}};
                    std::vector<Point2> corners_abs;
                    for (auto& c : corners_px)
                        corners_abs.push_back(absolute(c.x, c.y));

                    edge_shape.str("");
                    for (size_t x = 0; x < corners_abs.size(); ++x) {
                        const Point2& curr = corners_abs[x];
                        const Point2& nxt = corners_abs[(x + 1) % corners_abs.size()];
                        edge_shape << "!" << Round0(curr.x * 20) << " " << Round0(curr.y * 20) << "|" << Round0(nxt.x * 20) << " "
                                   << Round0(nxt.y * 20);
                    }

                    Animate::Pic::Shape& slice = new_piece->AddElement<Animate::Pic::Shape>();

                    Animate::Fill::BitmapFillStyle slice_fill;
                    slice_fill.bitmap = &media;

                    auto snap0 = [](double v) { return std::fabs(v) < 1e-6 ? 0.0 : v; };
                    slice_fill.transform = anim::ToMatrix(inst_matrix.a,
                                                          snap0(inst_matrix.b * 20.0),
                                                          snap0(inst_matrix.c * 20.0),
                                                          inst_matrix.d,
                                                          snap0(inst_matrix.tx),
                                                          snap0(inst_matrix.ty));
                    slice.CreateEdge(slice_fill, edge_shape.str());

                    pieces.push_back(std::move(new_piece));
                }
            }
        }

        std::vector<Animate::Pic::Group*> out;
        for (auto& g : pieces)
            out.push_back(g.get());
        ctx.shapes_with_nine_slices[id] = std::move(pieces);
        return out;
    }
}
