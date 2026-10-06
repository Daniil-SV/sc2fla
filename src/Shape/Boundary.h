#pragma once

#include "Core/Affine.h"

#include <cstddef>
#include <vector>

namespace sc2fla {
    using UniquePoints = std::vector<std::vector<Point2>>;

    struct ClassifiedLoops {
        std::vector<std::vector<Point2>> outers;
        std::vector<std::vector<Point2>> holes;
    };

    std::vector<std::vector<size_t>> ConnectedComponents(const UniquePoints& unique, const std::vector<size_t>& indices);

    std::vector<std::vector<Point2>> TraceBoundary(const UniquePoints& unique, const std::vector<size_t>& group);

    ClassifiedLoops ClassifyLoops(const std::vector<std::vector<Point2>>& loops);

    std::vector<Point2> RotateLoopToStartAt(std::vector<Point2> loop, const Point2& start_point);
}
