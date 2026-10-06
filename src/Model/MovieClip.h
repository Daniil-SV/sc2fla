#pragma once

#include "Core/Affine.h"
#include "Model/BlendMode.h"

#include <cstdint>
#include <string>
#include <vector>

namespace sc2fla {
    struct MovieClipModifierRecord {
        enum class Type {
            Mask,
            Masked,
            Unmasked
        };

        uint32_t id = 0;
        Type type = Type::Mask;
    };

    struct MovieClipBind {
        uint32_t id = 0;
        std::string name;
        BlendMode blend = BlendMode::None;
    };

    struct MovieClipFrameElementRecord {
        uint16_t instance_index = 0;
        uint16_t matrix_index = 0xFFFF;
        uint16_t color_index = 0xFFFF;
    };

    struct MovieClipFrameRecord {
        std::string label;
        std::vector<MovieClipFrameElementRecord> elements;
    };

    struct MovieClipRecord {
        uint32_t id = 0;
        std::vector<MovieClipBind> binds;
        std::vector<MovieClipFrameRecord> frames;
        uint32_t matrix_bank = 0;
        bool has_nine_slice = false;
        Point2 nine_slice_xy {0, 0};
        Point2 nine_slice_wh {0, 0};
        uint8_t frame_rate = 24;
    };
}
