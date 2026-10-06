#pragma once

#include "Core/Affine.h"
#include "Media/RgbaImage.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace sc2fla {
    struct ShapeBitmapCommand {
        struct MatrixResult {
            AffineTransform transform;
            std::vector<Point2> sprite_box;
            double rotation = 0.0;
            bool mirroring = false;
        };

        int texture_index = -1;
        std::vector<Point2> uv_coords;
        std::vector<Point2> xy_coords;

        std::pair<double, bool> GetRotation(bool nearest) const;
        MatrixResult GetMatrix(const std::vector<Point2>* custom_uv_coords = nullptr, bool use_nearest = false) const;
        RgbaImage GetImage(const RgbaImage& texture_image) const;
    };

    struct ShapeRecord {
        uint32_t id = 0;
        std::vector<ShapeBitmapCommand> bitmaps;
    };
}
