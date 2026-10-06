#pragma once

#include "Core/Affine.h"

#include <cmath>
#include <vector>

namespace sc2fla {
    inline bool XyPointsMatch(const Point2& a, const Point2& b, double eps = 0.01) {
        return std::abs(a.x - b.x) < eps && std::abs(a.y - b.y) < eps;
    }

    std::vector<Point2> UniqueXyPoints(const std::vector<Point2>& pts, double eps = 0.01);

    std::vector<Point2> PerimeterOrder(std::vector<Point2> pts);

    std::vector<Point2> PerimeterOrderStartingAfterFirst(const std::vector<Point2>& original_points);

    bool SharesVertex(const std::vector<Point2>& a, const std::vector<Point2>& b);

    bool AxisAlignedRect(const std::vector<Point2>& unique, double& x, double& y, double& w, double& h);
}
