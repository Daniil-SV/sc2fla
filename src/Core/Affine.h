#pragma once

#include <array>
#include <vector>

namespace sc2fla {

    struct AffineTransform {
        std::array<double, 6> params {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
    };

    struct Point2 {
        double x, y;
    };

    AffineTransform AffineEstimate(const std::vector<Point2>& origin, const std::vector<Point2>& convert);

}
