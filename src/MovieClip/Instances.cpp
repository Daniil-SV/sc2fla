#include "MovieClip/Instances.h"

#include "Anim/Convert.h"
#include "Anim/Text.h"
#include "Animate/Pic/PicFrame.h"
#include "Animate/Pic/PicGroup.h"
#include "Animate/Pic/PicPage.h"
#include "Animate/Pic/PicSymbol.h"
#include "Animate/Pic/PicText.h"
#include "MovieClip/ConvertMovieClip.h"
#include "MovieClip/Playback.h"
#include "Shape/NineSlice.h"

#include <algorithm>
#include <string>

namespace sc2fla {
    namespace {
        BindUse ModifierUse(MovieClipModifierRecord::Type type) {
            switch (type) {
                case MovieClipModifierRecord::Type::Mask:
                    return BindUse::MaskModifier;
                case MovieClipModifierRecord::Type::Masked:
                    return BindUse::MaskedModifier;
                case MovieClipModifierRecord::Type::Unmasked:
                    return BindUse::UnmaskedModifier;
            }
            return BindUse::Skip;
        }
    }

    void PrepareLayersAndInstances(anim::Project& project,
                                   ConversionContext& ctx,
                                   const ResourceTable& resources,
                                   const MovieClipRecord& movieclip,
                                   Animate::Library::DocumentPage& movie_page,
                                   MovieClipBuildState& state) {
        for (size_t i = 0; i < movieclip.binds.size(); ++i) {
            const MovieClipBind& bind = movieclip.binds[i];
            ResourceKind kind = resources.KindOf(bind.id);

            if (kind == ResourceKind::Modifier) {
                state.uses[i] = ModifierUse(resources.modifiers.at(bind.id).type);
                continue;
            }

            BindInstanceSpec spec;

            if (kind == ResourceKind::Shape) {
                if (movieclip.has_nine_slice) {
                    spec.kind = BindInstanceSpec::Kind::NineSliceGroup;
                    spec.nine_slice_template = PatchShapeNineSlice(project, ctx, bind.id);
                    state.symbols_instance[i] = std::move(spec);
                    state.uses[i] = BindUse::Symbol;
                    continue;
                }

                spec.kind = BindInstanceSpec::Kind::ShapeSymbol;
                spec.page = &project.GetSymbol("Shapes/Shape " + std::to_string(bind.id));

                spec.force_graphic = bind.blend == BlendMode::None;
                spec.blend = bind.blend;
            } else if (kind == ResourceKind::MovieClip) {
                auto exp_it = resources.exports.find(bind.id);
                bool has_export = (exp_it != resources.exports.end() && !exp_it->second.empty());
                std::string mc_key = has_export ? ("Assets/" + exp_it->second.front()) : ("MovieClips/MovieClip " + std::to_string(bind.id));
                if (!project.HasSymbol(mc_key)) {
                    ConvertMovieClip(project, ctx, resources, bind.id, resources.movieclips.at(bind.id), has_export ? &exp_it->second : nullptr);

                    project.document.SetCurrentDocPageForSymDependCache(&movie_page);
                }

                spec.kind = BindInstanceSpec::Kind::MovieClipSymbol;
                spec.page = &project.GetSymbol(mc_key);
                if (!bind.name.empty())
                    spec.instance_name = bind.name;

                bool in_movies_nine =
                    std::find(ctx.movies_with_nine_slices.begin(), ctx.movies_with_nine_slices.end(), bind.id) != ctx.movies_with_nine_slices.end();
                bool in_shapes_nine = ctx.shapes_with_nine_slices.count(bind.id) != 0;

                spec.force_graphic = (!in_movies_nine && !in_shapes_nine) && bind.blend == BlendMode::None && !PlaysOnItsOwn(resources, ctx, bind.id);
                spec.blend = bind.blend;

                project.document.AddSymbolReferenceToSymDependCache(*spec.page);
            } else if (kind == ResourceKind::TextField) {
                spec.kind = BindInstanceSpec::Kind::TextField;
                spec.textfield = &resources.textfields.at(bind.id);
                if (!bind.name.empty())
                    spec.text_instance_name = bind.name;
            } else {
                continue;
            }

            if (spec.kind == BindInstanceSpec::Kind::ShapeSymbol) {
                project.document.AddSymbolReferenceToSymDependCache(*spec.page);
            }

            state.symbols_instance[i] = std::move(spec);
            state.uses[i] = BindUse::Symbol;
        }
    }

    Animate::Pic::Object& PlaceInstance(Animate::Pic::Frame& frame, const BindInstanceSpec& spec) {
        using namespace Animate;

        switch (spec.kind) {
            case BindInstanceSpec::Kind::ShapeSymbol:
            case BindInstanceSpec::Kind::MovieClipSymbol: {
                Pic::Symbol& inst = frame.AddSymbolChildren(*spec.page);
                if (spec.instance_name)
                    inst.SetName(*spec.instance_name);
                if (spec.force_graphic)
                    inst.SetSymbolBehavior(Pic::Symbol::InstanceType::Graphic);
                if (spec.blend != BlendMode::None)
                    inst.GetGraphicEffectInstance()->SetBlendMode(anim::ToBlendMode(spec.blend));
                return inst;
            }
            case BindInstanceSpec::Kind::NineSliceGroup: {
                Animate::Pic::Object* first = nullptr;
                for (Animate::Pic::Group* piece : spec.nine_slice_template) {
                    Animate::Pic::Object& placed = frame.AddElementReference(*piece);
                    if (!first)
                        first = &placed;
                }
                if (first)
                    return *first;
                Animate::Pic::Group empty_fallback;
                return frame.AddElementReference(empty_fallback);
            }
            case BindInstanceSpec::Kind::TextField: {
                const TextFieldRecord& tf = *spec.textfield;
                Pic::Text& inst = frame.AddFrameElement<Pic::Text>();
                inst.SetTextType(Pic::Text::TextType::Dynamic);
                if (spec.text_instance_name)
                    inst.SetPIInstanceName(*spec.text_instance_name);

                inst.SetSelectable(spec.text_instance_name.has_value());
                inst.SetWidth(static_cast<float>(tf.right - tf.left));
                inst.SetHeight(static_cast<float>(tf.bottom - tf.top));

                inst.SetText(tf.text);

                Point2D position {static_cast<float>(tf.top), static_cast<float>(tf.left)};
                inst.PositionText(position);
                inst.ApplyStyle(anim::BuildTextStyle(tf));
                anim::ApplyTextFilters(inst, tf);
                return inst;
            }
        }

        return frame.AddFrameElement<Animate::Pic::Group>();
    }
}
