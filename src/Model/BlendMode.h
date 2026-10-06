#pragma once

#include <cstdint>

namespace sc2fla {
    enum class BlendMode : uint8_t {
        None,
        Layer,
        Multiply,
        Screen,
        Lighten,
        Darken,
        Difference,
        Add,
        Subtract,
        Invert,
        Alpha,
        Erase,
        Overlay,
        HardLight,
    };

    inline BlendMode ResolveBlendMode(uint8_t raw) {
        return raw >= 2 && raw <= 14 ? static_cast<BlendMode>(raw - 1) : BlendMode::None;
    }
}
