#include "Shape/Boundary.h"

#include "Shape/Geometry.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <unordered_map>
#include <utility>

namespace sc2fla {
    namespace {
        std::vector<std::pair<Point2, Point2>> CommandPerimeterEdges(const std::vector<Point2>& unique_points) {
            std::vector<Point2> pts = PerimeterOrder(unique_points);
            std::vector<std::pair<Point2, Point2>> edges;
            size_t n = pts.size();
            for (size_t i = 0; i < n; ++i)
                edges.emplace_back(pts[i], pts[(i + 1) % n]);
            return edges;
        }

        using RoundedPoint = std::pair<long long, long long>;

        RoundedPoint RoundPoint(const Point2& p, double eps = 0.01) {
            return {std::llround(p.x / eps), std::llround(p.y / eps)};
        }

        bool PointInLoop(const Point2& p, const std::vector<Point2>& loop) {
            bool inside = false;
            for (size_t i = 0, n = loop.size(), j = n - 1; i < n; j = i++) {
                const Point2& a = loop[i];
                const Point2& b = loop[j];
                if ((a.y > p.y) != (b.y > p.y) && p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x)
                    inside = !inside;
            }
            return inside;
        }

        double LoopSignedArea(const std::vector<Point2>& loop) {
            double twice = 0.0;
            for (size_t i = 0, n = loop.size(); i < n; ++i) {
                const Point2& a = loop[i];
                const Point2& b = loop[(i + 1) % n];
                twice += a.x * b.y - b.x * a.y;
            }
            return twice * 0.5;
        }
    }

    std::vector<std::vector<size_t>> ConnectedComponents(const UniquePoints& unique, const std::vector<size_t>& indices) {
        std::vector<size_t> parent(indices.size());
        for (size_t i = 0; i < parent.size(); ++i)
            parent[i] = i;

        auto find = [&](size_t x) {
            while (parent[x] != x) {
                parent[x] = parent[parent[x]];
                x = parent[x];
            }
            return x;
        };

        for (size_t ai = 0; ai < indices.size(); ++ai) {
            for (size_t bi = ai + 1; bi < indices.size(); ++bi) {
                if (SharesVertex(unique[indices[ai]], unique[indices[bi]])) {
                    size_t ra = find(ai), rb = find(bi);
                    if (ra != rb)
                        parent[ra] = rb;
                }
            }
        }

        std::unordered_map<size_t, std::vector<size_t>> groups;
        for (size_t i = 0; i < indices.size(); ++i)
            groups[indices[find(i)]].push_back(indices[i]);

        std::vector<std::vector<size_t>> result;
        for (auto& kv : groups)
            result.push_back(kv.second);
        return result;
    }

    std::vector<std::vector<Point2>> TraceBoundary(const UniquePoints& unique, const std::vector<size_t>& group) {
        std::map<std::pair<RoundedPoint, RoundedPoint>, int> edge_count;
        std::map<std::pair<RoundedPoint, RoundedPoint>, std::pair<Point2, Point2>> edge_value;

        for (size_t idx : group) {
            for (auto& pr : CommandPerimeterEdges(unique[idx])) {
                RoundedPoint ra = RoundPoint(pr.first), rb = RoundPoint(pr.second);
                auto key = (ra < rb) ? std::make_pair(ra, rb) : std::make_pair(rb, ra);
                edge_count[key] += 1;
                edge_value[key] = pr;
            }
        }

        std::vector<std::pair<Point2, Point2>> boundary;
        for (auto& kv : edge_count) {
            if (kv.second % 2 == 1)
                boundary.push_back(edge_value[kv.first]);
        }

        std::unordered_map<long long, std::vector<size_t>> adj;
        auto point_bucket = [](const RoundedPoint& p) { return p.first * 1000003469LL + p.second; };
        for (size_t i = 0; i < boundary.size(); ++i) {
            adj[point_bucket(RoundPoint(boundary[i].first))].push_back(i);
            adj[point_bucket(RoundPoint(boundary[i].second))].push_back(i);
        }

        std::vector<bool> used(boundary.size(), false);
        std::vector<std::vector<Point2>> loops;
        for (size_t start = 0; start < boundary.size(); ++start) {
            if (used[start])
                continue;
            used[start] = true;
            std::vector<Point2> loop = {boundary[start].first, boundary[start].second};
            RoundedPoint begin = RoundPoint(boundary[start].first);
            RoundedPoint cur = RoundPoint(boundary[start].second);
            while (!(cur == begin)) {
                bool found = false;
                for (size_t e : adj[point_bucket(cur)]) {
                    if (used[e])
                        continue;
                    bool endpoint_is_first = (RoundPoint(boundary[e].first) == cur);
                    Point2 to = endpoint_is_first ? boundary[e].second : boundary[e].first;
                    used[e] = true;
                    loop.push_back(to);
                    cur = RoundPoint(to);
                    found = true;
                    break;
                }
                if (!found)
                    break;
            }
            if (loop.size() > 1 && RoundPoint(loop.front()) == RoundPoint(loop.back()))
                loop.pop_back();
            loops.push_back(loop);
        }
        return loops;
    }

    std::vector<Point2> RotateLoopToStartAt(std::vector<Point2> loop, const Point2& start_point) {
        for (size_t i = 0; i < loop.size(); ++i) {
            if (XyPointsMatch(loop[i], start_point)) {
                std::rotate(loop.begin(), loop.begin() + static_cast<long>(i), loop.end());
                break;
            }
        }
        return loop;
    }

    ClassifiedLoops ClassifyLoops(const std::vector<std::vector<Point2>>& loops) {
        ClassifiedLoops result;
        for (size_t i = 0; i < loops.size(); ++i) {
            if (loops[i].size() < 3)
                continue;

            Point2 probe {(loops[i][0].x + loops[i][1].x) * 0.5, (loops[i][0].y + loops[i][1].y) * 0.5};
            int depth = 0;
            for (size_t j = 0; j < loops.size(); ++j) {
                if (j != i && loops[j].size() >= 3 && PointInLoop(probe, loops[j]))
                    ++depth;
            }
            (depth % 2 == 0 ? result.outers : result.holes).push_back(loops[i]);
        }
        if (result.outers.empty() && !loops.empty()) {
            result.outers.push_back(loops.front());
            result.holes.clear();
        }
        if (!result.outers.empty()) {
            const bool outer_negative = LoopSignedArea(result.outers.front()) < 0;
            for (auto& hole : result.holes) {
                if ((LoopSignedArea(hole) < 0) != outer_negative)
                    std::reverse(hole.begin(), hole.end());
            }
        }
        return result;
    }
}
