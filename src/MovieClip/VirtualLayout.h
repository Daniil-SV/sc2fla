#pragma once

#include "Model/MovieClip.h"

#include <cstdint>
#include <string>
#include <vector>

namespace sc2fla {
    enum class BindUse : uint8_t {
        Skip,
        Symbol,
        MaskModifier,
        MaskedModifier,
        UnmaskedModifier,
    };

    struct VirtualLayer {
        enum class Role {
            Normal,
            Mask,
            Masked
        };

        int bind = -1;
        Role role = Role::Normal;
        int parent = -1;
        std::string name;
    };

    struct VirtualElement {
        int layer;
        uint16_t matrix_index;
        uint16_t color_index;
    };

    struct VirtualLayout {
        std::vector<VirtualLayer> layers;
        std::vector<std::vector<VirtualElement>> frames;
        std::vector<int> final_order;
    };

    VirtualLayout BuildVirtualLayout(const MovieClipRecord& movieclip, const std::vector<BindUse>& uses);
}
