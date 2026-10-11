#pragma once

#include "Animate/Pic/PicGroup.h"
#include "Core/Affine.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sc2fla {
    struct SharedSprite {
        std::vector<Point2> uv_coords;
        std::vector<Point2> sprite_box;
        int width = 0;
        int height = 0;
    };

    struct ConversionContext {
        std::vector<SharedSprite> sprites;

        std::unordered_map<uint32_t, std::vector<std::unique_ptr<Animate::Pic::Group>>> shapes_with_nine_slices;
        std::vector<uint32_t> movies_with_nine_slices;
        std::unordered_set<uint32_t> mask_shapes;

        bool animated_clips_play = false;
        bool extend_single_frame_clips = true;

        std::unordered_map<uint32_t, bool> plays_on_its_own_cache;
        std::unordered_map<uint32_t, uint32_t> playable_length_cache;

        long FindSprite(const std::vector<Point2>& uv_coords) const;
        size_t AddSprite(std::vector<Point2> uv_coords, std::vector<Point2> sprite_box, int width, int height);

    private:
        static uint64_t HashUv(const std::vector<Point2>& uv_coords);

        std::unordered_map<uint64_t, std::vector<size_t>> sprites_by_hash;
    };
}
