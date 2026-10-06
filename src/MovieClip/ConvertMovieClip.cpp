#include "MovieClip/ConvertMovieClip.h"

#include "Anim/Convert.h"
#include "Animate/Pic/PicFrame.h"
#include "Animate/Pic/PicLayer.h"
#include "Animate/Pic/PicPage.h"
#include "Animate/Pic/PicSymbol.h"
#include "MovieClip/BuildState.h"
#include "MovieClip/Frames.h"
#include "MovieClip/Instances.h"
#include "MovieClip/Playback.h"
#include "MovieClip/VirtualLayout.h"

#include <algorithm>

namespace sc2fla {
    namespace {
        std::string SymbolKeyOf(const ResourceTable& resources, uint32_t id) {
            auto exports = resources.exports.find(id);
            if (exports != resources.exports.end() && !exports->second.empty())
                return "Assets/" + exports->second.front();
            return "MovieClips/MovieClip " + std::to_string(id);
        }

        bool TryEmitAsWrapper(anim::Project& project,
                              ConversionContext& ctx,
                              const ResourceTable& resources,
                              const MovieClipRecord& movieclip,
                              Animate::Library::DocumentPage& outer) {
            if (movieclip.has_nine_slice)
                return false;
            for (auto& f : movieclip.frames) {
                if (!f.label.empty())
                    return false;
            }

            std::vector<size_t> real_binds;
            for (size_t i = 0; i < movieclip.binds.size(); ++i) {
                if (resources.KindOf(movieclip.binds[i].id) != ResourceKind::Modifier)
                    real_binds.push_back(i);
            }

            if (real_binds.size() != 1)
                return false;

            if (movieclip.frames.size() != 1)
                return false;
            if (movieclip.frames[0].elements.size() != 1)
                return false;

            const MovieClipBind& bind = movieclip.binds[real_binds[0]];

            if (resources.KindOf(bind.id) != ResourceKind::MovieClip)
                return false;
            const std::string inner_key = SymbolKeyOf(resources, bind.id);
            if (!project.HasSymbol(inner_key))
                return false;

            project.document.SetCurrentDocPageForSymDependCache(&outer);

            Animate::Pic::Page& page = outer.GetPage();
            std::string layer_name = bind.name.empty() ? std::string("Layer 0") : bind.name;
            Animate::Pic::Layer& layer = page.AddNewLayer(layer_name, false, false);
            Animate::Pic::Frame& frame = layer.CreateFrame();

            Animate::Library::DocumentPage& inner_page = project.GetSymbol(inner_key);
            Animate::Pic::Symbol& inst = frame.AddSymbolChildren(inner_page);

            const bool inner_is_nine_slice =
                std::find(ctx.movies_with_nine_slices.begin(), ctx.movies_with_nine_slices.end(), bind.id) != ctx.movies_with_nine_slices.end();
            if (inner_is_nine_slice && !bind.name.empty())
                inst.SetName(bind.name);

            if (bind.blend == BlendMode::None && !inner_is_nine_slice && !PlaysOnItsOwn(resources, ctx, bind.id)) {
                inst.SetSymbolBehavior(Animate::Pic::Symbol::InstanceType::Graphic);
            }

            const MatrixBankRecord& bank = resources.matrix_banks[movieclip.matrix_bank];
            const auto& fe = movieclip.frames[0].elements[0];
            if (fe.matrix_index != 0xFFFF && fe.matrix_index < bank.matrices.size()) {
                const MatrixBankEntry& m = bank.matrices[fe.matrix_index];
                inst.SetMatrix(anim::ToMatrix(m.a, m.b, m.c, m.d, m.tx, m.ty));
            }
            if (fe.color_index != 0xFFFF && fe.color_index < bank.color_transforms.size()) {
                const ColorTransformEntry& c = bank.color_transforms[fe.color_index];
                auto* effect = inst.GetGraphicEffectInstance();
                if (effect)
                    effect->SetColorTransform(anim::ToColorTransform(c));
            }
            if (bind.blend != BlendMode::None)
                inst.GetGraphicEffectInstance()->SetBlendMode(anim::ToBlendMode(bind.blend));

            project.document.AddSymbolReferenceToSymDependCache(inner_page);
            project.document.SetCurrentDocPageForSymDependCache(nullptr);
            return true;
        }

        void BuildMovieClip(anim::Project& project,
                            ConversionContext& ctx,
                            const ResourceTable& resources,
                            const MovieClipRecord& movieclip,
                            Animate::Library::DocumentPage& movie) {
            MovieClipBuildState state;
            state.layers_instance.resize(movieclip.binds.size(), nullptr);
            state.symbols_instance.resize(movieclip.binds.size());
            state.uses.assign(movieclip.binds.size(), BindUse::Skip);

            project.document.SetCurrentDocPageForSymDependCache(&movie);

            PrepareLayersAndInstances(project, ctx, resources, movieclip, movie, state);

            VirtualLayout layout = BuildVirtualLayout(movieclip, state.uses);

            Animate::Pic::Page& page = movie.GetPage();
            CreateVirtualLayers(page, layout, state.layers_instance);

            BuildVirtualLayerFrames(movieclip, resources, state, layout, state.layers_instance);

            if (movieclip.has_nine_slice)
                ApplyNineSliceScaleGrid(movie, movieclip);
            InsertFrameLabelsLayer(page, movieclip);

            project.document.SetCurrentDocPageForSymDependCache(nullptr);
        }

        void ExtendHold(
            ConversionContext& ctx, const ResourceTable& resources, uint32_t id, const MovieClipRecord& movieclip, Animate::Library::DocumentPage& symbol) {
            if (!ctx.extend_single_frame_clips || movieclip.frames.size() != 1)
                return;
            const uint32_t target = PlayableLength(resources, ctx, id);
            if (target > 1)
                HoldLayersUntil(symbol.GetPage(), target);
        }
    }

    void ConvertMovieClip(anim::Project& project,
                          ConversionContext& ctx,
                          const ResourceTable& resources,
                          uint32_t id,
                          const MovieClipRecord& movieclip,
                          const std::vector<std::string>* export_names) {
        if (export_names != nullptr) {
            for (auto& exp : *export_names) {
                std::string key = "Assets/" + exp;
                if (project.HasSymbol(key))
                    continue;
                auto& movie = project.MakeSymbol(key, Animate::Library::DocumentPage::SymbolType::MovieClip);
                movie.GetLinkageProps().SetIdentifier(exp);
                movie.GetLinkageProps().SetClass(exp);
                if (!TryEmitAsWrapper(project, ctx, resources, movieclip, movie))
                    BuildMovieClip(project, ctx, resources, movieclip, movie);
                ExtendHold(ctx, resources, id, movieclip, movie);
            }
            return;
        }

        std::string key = "MovieClips/MovieClip " + std::to_string(id);
        if (project.HasSymbol(key))
            return;
        auto& movie = project.MakeSymbol(key, Animate::Library::DocumentPage::SymbolType::MovieClip);
        if (!TryEmitAsWrapper(project, ctx, resources, movieclip, movie))
            BuildMovieClip(project, ctx, resources, movieclip, movie);
        ExtendHold(ctx, resources, id, movieclip, movie);
    }
}
