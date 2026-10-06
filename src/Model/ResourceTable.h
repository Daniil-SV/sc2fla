#pragma once

#include "Model/MatrixBank.h"
#include "Model/MovieClip.h"
#include "Model/Shape.h"
#include "Model/TextField.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace sc2fla {
    enum class ResourceKind {
        Shape,
        MovieClip,
        TextField,
        Modifier,
        None
    };

    struct ResourceTable {
        std::unordered_map<uint32_t, ShapeRecord> shapes;
        std::unordered_map<uint32_t, MovieClipRecord> movieclips;
        std::unordered_map<uint32_t, TextFieldRecord> textfields;
        std::unordered_map<uint32_t, MovieClipModifierRecord> modifiers;

        std::vector<MatrixBankRecord> matrix_banks;
        std::unordered_map<uint32_t, std::vector<std::string>> exports;

        ResourceKind KindOf(uint32_t id) const {
            if (shapes.count(id))
                return ResourceKind::Shape;
            if (movieclips.count(id))
                return ResourceKind::MovieClip;
            if (textfields.count(id))
                return ResourceKind::TextField;
            if (modifiers.count(id))
                return ResourceKind::Modifier;
            return ResourceKind::None;
        }
    };
}
