#include "Shape/Geometry.h"

#include <algorithm>
#include <cmath>

namespace sc2fla {
    std::vector<Point2> UniqueXyPoints(const std::vector<Point2>& pts, double eps) {
        std::vector<Point2> out;
        for (auto& p : pts) {
            bool dup = false;
            for (auto& q : out) {
                if (std::abs(p.x - q.x) < eps && std::abs(p.y - q.y) < eps) {
                    dup = true;
                    break;
                }
            }
            if (!dup)
                out.push_back(p);
        }
        return out;
    }

    std::vector<Point2> PerimeterOrder(std::vector<Point2> pts) {
        double cx = 0, cy = 0;
        for (auto& p : pts) {
            cx += p.x;
            cy += p.y;
        }
        cx /= static_cast<double>(pts.size());
        cy /= static_cast<double>(pts.size());
        std::sort(pts.begin(), pts.end(), [&](const Point2& a, const Point2& b) { return std::atan2(a.y - cy, a.x - cx) < std::atan2(b.y - cy, b.x - cx); });
        return pts;
    }

    std::vector<Point2> PerimeterOrderStartingAfterFirst(const std::vector<Point2>& original_points) {
        if (original_points.empty())
            return {};
        Point2 first = original_points[0];
        std::vector<Point2> unique_pts = UniqueXyPoints(original_points);
        std::vector<Point2> ordered = PerimeterOrder(unique_pts);

        size_t first_pos = 0;
        for (size_t i = 0; i < ordered.size(); ++i) {
            if (XyPointsMatch(ordered[i], first)) {
                first_pos = i;
                break;
            }
        }
        std::vector<Point2> rotated;
        rotated.reserve(ordered.size());
        for (size_t k = 1; k <= ordered.size(); ++k) {
            rotated.push_back(ordered[(first_pos + k) % ordered.size()]);
        }
        return rotated;
    }

    bool SharesVertex(const std::vector<Point2>& a, const std::vector<Point2>& b) {
        for (const Point2& x : a) {
            for (const Point2& y : b) {
                if (XyPointsMatch(x, y))
                    return true;
            }
        }
        return false;
    }

    bool AxisAlignedRect(const std::vector<Point2>& unique, double& x, double& y, double& w, double& h) {
        if (unique.size() != 4)
            return false;
        auto close = [](double a, double b) { return std::abs(a - b) < 0.01; };
        std::vector<double> xs, ys;
        for (auto& p : unique) {
            bool fx = false, fy = false;
            for (double v : xs)
                if (close(v, p.x))
                    fx = true;
            for (double v : ys)
                if (close(v, p.y))
                    fy = true;
            if (!fx)
                xs.push_back(p.x);
            if (!fy)
                ys.push_back(p.y);
        }
        if (xs.size() != 2 || ys.size() != 2)
            return false;
        x = std::min(xs[0], xs[1]);
        y = std::min(ys[0], ys[1]);
        w = std::abs(xs[0] - xs[1]);
        h = std::abs(ys[0] - ys[1]);
        return true;
    }
}
