#pragma once

#include <vector>

namespace sc2fla {
    struct MatrixBankEntry {
        double a = 1, b = 0, c = 0, d = 1, tx = 0, ty = 0;
    };

    struct ColorTransformEntry {
        double r_mul = 1, g_mul = 1, b_mul = 1, a_mul = 1;
        int r_add = 0, g_add = 0, b_add = 0;
    };

    struct MatrixBankRecord {
        std::vector<MatrixBankEntry> matrices;
        std::vector<ColorTransformEntry> color_transforms;
    };
}
