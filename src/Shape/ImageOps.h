#pragma once

#include "Media/RgbaImage.h"
#include "Model/Shape.h"

namespace sc2fla {
    RgbaImage RotateImage(RgbaImage src, double degrees_ccw_negated);

    void FlipHorizontal(RgbaImage& image);

    bool NormalizeSpriteOrientation(RgbaImage& sprite, ShapeBitmapCommand::MatrixResult& mres);
}
