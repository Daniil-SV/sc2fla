#pragma once

#include <cstdint>
#include <string>

namespace sc2fla {
    enum class TextAlign {
        Left,
        Right,
        Center,
        Justify,
    };

    struct TextFieldRecord {
        uint32_t id = 0;
        int left = 0, top = 0, right = 0, bottom = 0;
        TextAlign alignment = TextAlign::Left;
        std::string text;
        std::string font_name;
        uint8_t font_size = 0;
        bool bold = false, italic = false;
        uint32_t font_color = 0x000000FF;
        bool has_outline = false;
        bool auto_kern = true;
        int16_t bend_angle = 0;
    };
}
