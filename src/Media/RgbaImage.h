#pragma once

#include <core/memory/ref.h>
#include <cstdint>
#include <vector>

namespace wk {
    class RawImage;
}

namespace sc2fla {
    struct RgbaImage {
        int width = 0;
        int height = 0;
        std::vector<uint8_t> pixels;
    };

    wk::Ref<wk::RawImage> ToRawImage(const RgbaImage& image);
}
