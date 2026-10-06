#include "Media/Texture.h"

#include <cctype>
#include <core/image/raw_image.h>
#include <core/io/file_stream.h>
#include <core/io/shared_memory_stream.h>
#include <cstring>
#include <flash/objects/SWFTexture.h>
#include <flash/objects/SupercellSWF.h>
#include <string>
#include <texture/texture.h>

namespace sc2fla {

    namespace {

        RgbaImage RawImageToRgba8(wk::RawImage& raw) {
            RgbaImage out;
            out.width = raw.width();
            out.height = raw.height();
            out.pixels.resize(static_cast<size_t>(out.width) * out.height * 4);

            if (raw.depth() == wk::Image::PixelDepth::RGBA8) {
                std::memcpy(out.pixels.data(), raw.data(), out.pixels.size());
            } else {
                wk::Image::remap(raw.data(), out.pixels.data(), raw.width(), raw.height(), raw.depth(), wk::Image::PixelDepth::RGBA8);
            }
            return out;
        }

    }

    RgbaImage DecodeTextureToRgba8(const sc::flash::SWFTexture& tex) {
        wk::Ref<wk::RawImage> raw = tex.raw_image();
        if (!raw)
            return RgbaImage {};
        return RawImageToRgba8(*raw);
    }

    namespace {

        RgbaImage DecodeSctxFile(const std::filesystem::path& path) {
            using namespace sc::texture;

            wk::InputFileStream file(path);
            SupercellTexture texture(file);
            texture.read_data();

            wk::Image::PixelDepth depth = texture.depth();
            wk::Image::ColorSpace space = texture.colorspace();

            wk::Ref<wk::RawImage> image;
            if (texture.is_compressed()) {
                image = wk::CreateRef<wk::RawImage>(texture.width(), texture.height(), depth, space);
                wk::SharedMemoryStream stream(image->data(), image->data_length());
                texture.decompress_data(stream);
            } else {
                image = wk::CreateRef<wk::RawImage>(texture.data(), texture.width(), texture.height(), depth, space);
            }
            return RawImageToRgba8(*image);
        }

        RgbaImage DecodeKtxFile(const std::filesystem::path& path) {
            using namespace sc::texture;

            wk::InputFileStream file(path);
            auto ktx = KhronosTexture::load_texture(file);

            wk::Ref<wk::RawImage> image = wk::CreateRef<wk::RawImage>(ktx->width(), ktx->height(), ktx->depth(), ktx->colorspace());
            wk::SharedMemoryStream stream(image->data(), image->data_length());
            ktx->decompress_data(stream);
            return RawImageToRgba8(*image);
        }

    }

    std::vector<RgbaImage> LoadExternalTextures(const std::filesystem::path& path) {
        std::string ext = path.extension().string();
        for (auto& c : ext)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        std::vector<RgbaImage> out;

        if (ext == ".sctx") {
            out.push_back(DecodeSctxFile(path));
            return out;
        }
        if (ext == ".ktx" || ext == ".ktx2") {
            out.push_back(DecodeKtxFile(path));
            return out;
        }

        sc::flash::SupercellSWF swf;
        swf.load(path);
        out.reserve(swf.textures.size());
        for (auto& tex : swf.textures) {
            out.push_back(DecodeTextureToRgba8(tex));
        }
        return out;
    }

}
