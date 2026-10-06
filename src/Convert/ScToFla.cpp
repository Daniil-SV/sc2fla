#include "Convert/ScToFla.h"

#include "Anim/Project.h"
#include "Animate/Document/SketchDocument.h"
#include "Convert/Extract.h"
#include "Convert/FrameRate.h"
#include "Convert/GroupSelection.h"
#include "Core/Console.h"
#include "Media/Texture.h"
#include "MovieClip/ConvertMovieClip.h"
#include "Shape/ConvertShape.h"

#include <flash/objects/SWFTexture.h>
#include <flash/objects/SupercellSWF.h>
#include <map>
#include <stdexcept>

namespace sc2fla {
    namespace {
        namespace fs = std::filesystem;

        using ResourceIds = std::map<uint32_t, ResourceKind>;

        void LoadSwf(sc::flash::SupercellSWF& swf, const fs::path& input_path, bool external_texture) {
            if (external_texture) {
                swf.current_file = input_path;
                swf.load_internal(input_path, false);
                swf.stream.clear();
                return;
            }

            try {
                swf.load(input_path);
            } catch (const std::exception& e) {
                std::string msg = e.what();
                if (msg.find("Failed to load external texture") != std::string::npos) {
                    throw std::runtime_error(msg +
                                             "\n"
                                             "  The file references an external texture companion but it wasn't\n"
                                             "  found next to the input file. Use -t to specify it explicitly:\n"
                                             "    sc2fla " +
                                             input_path.filename().string() + " -t <path/to/" + input_path.stem().string() + "_tex.sc>");
                }
                throw;
            }
        }

        std::vector<RgbaImage> LoadTextures(sc::flash::SupercellSWF& swf, const std::optional<fs::path>& external_texture) {
            std::vector<RgbaImage> textures;
            textures.reserve(swf.textures.size());
            for (auto& tex : swf.textures)
                textures.push_back(DecodeTextureToRgba8(tex));

            if (external_texture) {
                Console::Info("Loading external texture(s) from " + external_texture->string());
                std::vector<RgbaImage> external = LoadExternalTextures(*external_texture);

                for (size_t i = 0; i < external.size(); ++i) {
                    if (i < textures.size())
                        textures[i] = std::move(external[i]);
                    else
                        textures.push_back(std::move(external[i]));
                }
            }
            return textures;
        }

        ResourceTable ExtractResources(sc::flash::SupercellSWF& swf, const std::vector<RgbaImage>& textures) {
            ResourceTable resources;

            for (auto& bank : swf.matrixBanks)
                resources.matrix_banks.push_back(ExtractMatrixBank(bank));
            for (auto& shape : swf.shapes)
                resources.shapes[shape.id] = ExtractShape(shape, textures);
            for (auto& mc : swf.movieclips)
                resources.movieclips[mc.id] = ExtractMovieClip(mc);
            for (auto& tf : swf.textfields)
                resources.textfields[tf.id] = ExtractTextField(tf);
            for (auto& mod : swf.movieclip_modifiers)
                resources.modifiers[mod.id] = ExtractModifier(mod);
            for (auto& exp : swf.exports)
                resources.exports[exp.id].push_back(exp.name.string());

            return resources;
        }

        void ConfigureDocument(Animate::Document::SketchDocument& document) {
            document.SetBackgroundColor(wk::ColorRGB {0x66, 0x66, 0x66});

            Animate::DocumentType doc_type;
            doc_type.name = "org.scWorkshop.SupercellSWF.Publisher";
            doc_type.documentId = Animate::GUID {0xA03733F0, 0x0799, 0x11EE, {0xBE, 0x56, 0x02, 0x42, 0xAC, 0x12, 0x00, 0x02}};
            document.SetDocumentType(doc_type);

            auto& controller = document.GetController();
            controller.MakeFolders("Sprites");
            controller.MakeFolders("Shapes");
            controller.MakeFolders("MovieClips");
            controller.MakeFolders("Assets");
        }

        ResourceIds AllResourceIds(const ResourceTable& resources) {
            ResourceIds ids;
            for (auto& kv : resources.shapes)
                ids[kv.first] = ResourceKind::Shape;
            for (auto& kv : resources.movieclips)
                ids[kv.first] = ResourceKind::MovieClip;
            for (auto& kv : resources.textfields)
                ids[kv.first] = ResourceKind::TextField;
            for (auto& kv : resources.modifiers)
                ids[kv.first] = ResourceKind::Modifier;
            return ids;
        }

        ResourceIds SelectedResourceIds(const GroupSelection& selection) {
            ResourceIds ids;
            for (uint32_t id : selection.keep_shapes)
                ids[id] = ResourceKind::Shape;
            for (uint32_t id : selection.keep_movieclips)
                ids[id] = ResourceKind::MovieClip;
            return ids;
        }

        const std::vector<std::string>* ExportNamesOf(const ResourceTable& resources, const GroupSelection& selection, bool group_active, uint32_t id) {
            if (group_active) {
                auto matched = selection.matched_root_names.find(id);
                return matched != selection.matched_root_names.end() ? &matched->second : nullptr;
            }
            auto exported = resources.exports.find(id);
            return exported != resources.exports.end() ? &exported->second : nullptr;
        }
    }

    void ScToFla(const fs::path& input_path, const fs::path& output_dir, ConversionContext& ctx, const ConvertOptions& options) {
        Console::Info("Loading " + input_path.filename().string());

        ctx.playable_length_cache.clear();
        ctx.plays_on_its_own_cache.clear();

        sc::flash::SupercellSWF swf;
        LoadSwf(swf, input_path, options.external_texture.has_value());

        const std::vector<RgbaImage> textures = LoadTextures(swf, options.external_texture);
        const ResourceTable resources = ExtractResources(swf, textures);

        Console::Info("Creating SketchDocument for Adobe Animate");
        Animate::Document::SketchDocument document;
        ConfigureDocument(document);

        anim::Project project(document);

        ResourceIds ids = AllResourceIds(resources);

        const bool group_active = !options.groups.empty();
        GroupSelection selection;
        if (group_active) {
            selection = ComputeGroupSelection(resources, options.groups);
            if (selection.matched_root_names.empty()) {
                Console::Warning("-g: no exported object name matched any of the given pattern(s); nothing to convert.");
            } else {
                ids = SelectedResourceIds(selection);
            }
        }

        const int frame_rate = options.fps ? *options.fps : DetectFrameRate(resources, group_active, selection);
        Console::Info("Frame Rate: " + std::to_string(frame_rate));
        document.SetFrameRate(frame_rate);

        for (auto& kv : ids) {
            if (kv.second == ResourceKind::MovieClip && resources.movieclips.at(kv.first).has_nine_slice) {
                ctx.movies_with_nine_slices.push_back(kv.first);
            }
        }

        Console::Info("Converting SupercellFlash resources to Adobe Animate");
        for (auto& kv : ids) {
            const uint32_t id = kv.first;
            if (kv.second == ResourceKind::Shape) {
                ConvertShape(project, ctx, textures, id, resources.shapes.at(id));
            } else if (kv.second == ResourceKind::MovieClip) {
                ConvertMovieClip(project, ctx, resources, id, resources.movieclips.at(id), ExportNamesOf(resources, selection, group_active, id));
            }
        }

        const std::string project_name = input_path.stem().string();
        const std::string ext = options.unpacked ? ".xfl" : ".fla";
        Console::Info("Writing " + project_name + ext);
        document.WriteXFL(output_dir / (project_name + ext));
    }
}
