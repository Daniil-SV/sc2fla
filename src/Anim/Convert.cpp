#include "Anim/Convert.h"

#include <cmath>

namespace sc2fla::anim {
    Animate::ColorTransform ToColorTransform(const ColorTransformEntry& c) {
        Animate::ColorTransform ct;
        ct.transform_type = Animate::ColorTransform::Type::Special;
        ct.multiply_red = static_cast<int16_t>(std::lround(c.r_mul * 255.0));
        ct.multiply_green = static_cast<int16_t>(std::lround(c.g_mul * 255.0));
        ct.multiply_blue = static_cast<int16_t>(std::lround(c.b_mul * 255.0));
        ct.multiply_alpha = static_cast<int16_t>(std::lround(c.a_mul * 255.0));
        ct.offset_red = static_cast<int16_t>(c.r_add);
        ct.offset_green = static_cast<int16_t>(c.g_add);
        ct.offset_blue = static_cast<int16_t>(c.b_add);
        ct.offset_alpha = 0;
        return ct;
    }
}
