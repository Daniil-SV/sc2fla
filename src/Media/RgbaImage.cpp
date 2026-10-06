#include "Media/RgbaImage.h"

#include <core/image/raw_image.h>
#include <cstring>

namespace sc2fla {
    wk::Ref<wk::RawImage> ToRawImage(const RgbaImage& image) {
        auto raw = wk::CreateRef<wk::RawImage>(static_cast<uint16_t>(image.width),
                                               static_cast<uint16_t>(image.height),
                                               wk::Image::PixelDepth::RGBA8,
                                               wk::Image::ColorSpace::Linear);
        std::memcpy(raw->data(), image.pixels.data(), image.pixels.size());
        return raw;
    }
}
