#include "MovieClip/Frames.h"

#include "Anim/Convert.h"
#include "Animate/Pic/PicFrame.h"
#include "MovieClip/Instances.h"

#include <algorithm>
#include <utility>

namespace sc2fla {
    void CreateVirtualLayers(Animate::Pic::Page& page, const VirtualLayout& vl, std::vector<Animate::Pic::Layer*>& layers_instance) {
        layers_instance.assign(vl.layers.size(), nullptr);
        for (int id : vl.final_order) {
            layers_instance[static_cast<size_t>(id)] = &page.AddNewLayer(vl.layers[static_cast<size_t>(id)].name, false, false);
        }
        for (int id : vl.final_order) {
            const VirtualLayer& layer = vl.layers[static_cast<size_t>(id)];
            if (layer.role == VirtualLayer::Role::Mask) {
                layers_instance[static_cast<size_t>(id)]->SetClipper();
            } else if (layer.role == VirtualLayer::Role::Masked && layer.parent >= 0) {
                Animate::Pic::Layer* mask_layer = layers_instance[static_cast<size_t>(layer.parent)];
                if (mask_layer)
                    layers_instance[static_cast<size_t>(id)]->AttachMask(*mask_layer);
            }
        }
    }

    void BuildVirtualLayerFrames(const MovieClipRecord& movieclip,
                                 const ResourceTable& resources,
                                 const MovieClipBuildState& state,
                                 const VirtualLayout& vl,
                                 const std::vector<Animate::Pic::Layer*>& layers_instance) {
        const size_t layer_count = vl.layers.size();

        std::vector<Animate::Pic::Frame*> last_frame(layer_count, nullptr);
        std::vector<bool> last_frame_empty(layer_count, false);
        std::vector<const VirtualElement*> present(layer_count, nullptr);
        std::vector<const VirtualElement*> prev_present(layer_count, nullptr);

        for (size_t i = 0; i < vl.frames.size(); ++i) {
            std::fill(present.begin(), present.end(), nullptr);
            for (const VirtualElement& e : vl.frames[i])
                present[static_cast<size_t>(e.layer)] = &e;

            for (size_t layer = 0; layer < layer_count; ++layer) {
                Animate::Pic::Layer* curr_layer = layers_instance[layer];
                if (!curr_layer)
                    continue;

                const VirtualElement* e = present[layer];
                if (!e) {
                    if (last_frame[layer] != nullptr && last_frame_empty[layer]) {
                        Animate::Pic::Frame* f = last_frame[layer];
                        f->SetDuration(f->GetDuration() + 1);
                    } else {
                        last_frame[layer] = &curr_layer->CreateFrame();
                        last_frame_empty[layer] = true;
                    }
                    continue;
                }

                const VirtualElement* pe = prev_present[layer];
                if (i > 0 && last_frame[layer] != nullptr && pe != nullptr && pe->matrix_index == e->matrix_index && pe->color_index == e->color_index) {
                    Animate::Pic::Frame* f = last_frame[layer];
                    f->SetDuration(f->GetDuration() + 1);
                    continue;
                }

                Animate::Pic::Frame& layer_frame = curr_layer->CreateFrame();
                last_frame[layer] = &layer_frame;
                last_frame_empty[layer] = false;

                const BindInstanceSpec& spec = *state.symbols_instance[static_cast<size_t>(vl.layers[layer].bind)];
                Animate::Pic::Object& instance = PlaceInstance(layer_frame, spec);

                if (e->matrix_index != 0xFFFF) {
                    const MatrixBankEntry& m = resources.matrix_banks[movieclip.matrix_bank].matrices[e->matrix_index];
                    instance.SetMatrix(anim::ToMatrix(m.a, m.b, m.c, m.d, m.tx, m.ty));
                }
                if (e->color_index != 0xFFFF) {
                    auto* effect = instance.GetGraphicEffectInstance();
                    if (effect) {
                        const ColorTransformEntry& c = resources.matrix_banks[movieclip.matrix_bank].color_transforms[e->color_index];
                        effect->SetColorTransform(anim::ToColorTransform(c));
                    }
                }
            }
            std::swap(present, prev_present);
        }
    }

    void ApplyNineSliceScaleGrid(Animate::Library::DocumentPage& movie, const MovieClipRecord& movieclip) {
        movie.SetScale9(true);
        Animate::Bound rect;
        rect.x = movieclip.nine_slice_xy.x;
        rect.y = movieclip.nine_slice_xy.y;
        rect.width = movieclip.nine_slice_xy.x + movieclip.nine_slice_wh.x;
        rect.height = movieclip.nine_slice_xy.y + movieclip.nine_slice_wh.y;

        auto keep_from_vanishing = [](float v) { return v == 0.0f ? 1e-4f : v; };
        rect.x = keep_from_vanishing(rect.x);
        rect.y = keep_from_vanishing(rect.y);
        rect.width = keep_from_vanishing(rect.width);
        rect.height = keep_from_vanishing(rect.height);

        movie.SetScale9Rect(rect);
    }

    void InsertFrameLabelsLayer(Animate::Pic::Page& page, const MovieClipRecord& movieclip) {
        bool any_named_frame = false;
        for (auto& f : movieclip.frames)
            if (!f.label.empty()) {
                any_named_frame = true;
                break;
            }
        if (!any_named_frame)
            return;

        Animate::Pic::Layer& names_layer = page.AddNewLayer("Labels", false, false);

        Animate::Pic::Frame* last = nullptr;
        for (size_t i = 0; i < movieclip.frames.size(); ++i) {
            if (movieclip.frames[i].label.empty()) {
                if (last != nullptr && last->GetLabelType() == Animate::Pic::Frame::LabelType::None) {
                    last->SetDuration(last->GetDuration() + 1);
                } else {
                    last = &names_layer.CreateFrame();
                }
                continue;
            }
            Animate::Pic::Frame& f = names_layer.CreateFrame();
            f.SetLabel(movieclip.frames[i].label, Animate::Pic::Frame::LabelType::Name);
            last = &f;
        }

        if (page.LayersCount() > 1) {
            page.MoveLayer(names_layer, page[0], true);
        }
    }
}
