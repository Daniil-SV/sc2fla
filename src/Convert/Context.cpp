#include "Convert/Context.h"

#include <cstring>

namespace sc2fla {
    namespace {
        uint64_t MixDouble(uint64_t seed, double value) {
            value += 0.0;
            uint64_t bits;
            std::memcpy(&bits, &value, sizeof(bits));
            seed ^= bits + 0x9E3779B97F4A7C15ULL + (seed << 6) + (seed >> 2);
            return seed;
        }
    }

    uint64_t ConversionContext::HashUv(const std::vector<Point2>& uv_coords) {
        uint64_t seed = uv_coords.size();
        for (const Point2& p : uv_coords) {
            seed = MixDouble(seed, p.x);
            seed = MixDouble(seed, p.y);
        }
        return seed;
    }

    long ConversionContext::FindSprite(const std::vector<Point2>& uv_coords) const {
        auto bucket = sprites_by_hash.find(HashUv(uv_coords));
        if (bucket == sprites_by_hash.end())
            return -1;

        for (size_t index : bucket->second) {
            const std::vector<Point2>& candidate = sprites[index].uv_coords;
            if (candidate.size() != uv_coords.size())
                continue;
            bool equal = true;
            for (size_t i = 0; i < candidate.size(); ++i) {
                if (candidate[i].x != uv_coords[i].x || candidate[i].y != uv_coords[i].y) {
                    equal = false;
                    break;
                }
            }
            if (equal)
                return static_cast<long>(index);
        }
        return -1;
    }

    size_t ConversionContext::AddSprite(std::vector<Point2> uv_coords, std::vector<Point2> sprite_box, int width, int height) {
        const size_t index = sprites.size();
        sprites_by_hash[HashUv(uv_coords)].push_back(index);
        sprites.push_back({std::move(uv_coords), std::move(sprite_box), width, height});
        return index;
    }
}
