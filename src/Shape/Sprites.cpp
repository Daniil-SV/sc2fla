#include "Shape/Sprites.h"

#include "Anim/Convert.h"
#include "Animate/Pic/PicBitmap.h"
#include "Animate/Pic/PicFrame.h"
#include "Animate/Pic/PicLayer.h"
#include "Shape/ImageOps.h"

#include <array>
#include <string>

namespace sc2fla {
    namespace {
        struct PendingCommand {
            long sprite_index;
            std::array<double, 6> params;
        };

        size_t RegisterSprite(anim::Project& project,
                              ConversionContext& ctx,
                              const std::vector<RgbaImage>& textures,
                              const ShapeBitmapCommand& bitmap,
                              ShapeBitmapCommand::MatrixResult& mres) {
            mres = bitmap.GetMatrix(nullptr, true);

            const RgbaImage& texture_image = textures.at(static_cast<size_t>(bitmap.texture_index));
            RgbaImage sprite = bitmap.GetImage(texture_image);
            sprite = RotateImage(std::move(sprite), -mres.rotation);
            if (mres.mirroring)
                FlipHorizontal(sprite);
            NormalizeSpriteOrientation(sprite, mres);

            const size_t index = ctx.AddSprite(bitmap.uv_coords, mres.sprite_box, sprite.width, sprite.height);

            auto& bits = project.MakeBitmap("Sprites/Sprite " + std::to_string(index));
            bits.CreateBits(*ToRawImage(sprite));
            bits.SetAllowSmooth(true);
            return index;
        }
    }

    void AddSpriteLayers(anim::Project& project,
                         ConversionContext& ctx,
                         Animate::Pic::Page& page,
                         const std::vector<RgbaImage>& textures,
                         const ShapeRecord& shape,
                         const std::vector<char>& is_solid) {
        std::vector<PendingCommand> pending;
        for (size_t bi = 0; bi < shape.bitmaps.size(); ++bi) {
            if (is_solid[bi])
                continue;
            const ShapeBitmapCommand& bitmap = shape.bitmaps[bi];

            long sprite_index;
            ShapeBitmapCommand::MatrixResult mres;

            const long existing = ctx.FindSprite(bitmap.uv_coords);
            if (existing < 0) {
                sprite_index = static_cast<long>(RegisterSprite(project, ctx, textures, bitmap, mres));
            } else {
                sprite_index = existing;
                mres = bitmap.GetMatrix(&ctx.sprites[static_cast<size_t>(existing)].sprite_box, false);
            }

            pending.push_back({sprite_index, mres.transform.params});
        }

        int command_number = 0;
        for (auto it = pending.rbegin(); it != pending.rend(); ++it) {
            Animate::Pic::Layer& layer = page.AddNewLayer("Command " + std::to_string(command_number++), false, false);
            Animate::Pic::Frame& frame = layer.CreateFrame();

            auto& bits = *project.media.at("Sprites/Sprite " + std::to_string(it->sprite_index));
            Animate::Pic::Bitmap& bitmap_instance = frame.AddBitmapChildren(bits);

            const auto& p = it->params;
            bitmap_instance.SetMatrix(anim::ToMatrix(p[0], p[2], p[1], p[3], p[4], p[5]));
        }
    }
}
