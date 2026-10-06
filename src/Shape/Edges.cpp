#include "Shape/Edges.h"

#include "Core/Format.h"

#include <charconv>
#include <cmath>

namespace sc2fla {
    namespace {
        void AppendEdgeCoord(std::string& out, double value) {
            double rounded = std::round(value);
            if (std::abs(value - rounded) < 1e-6) {
                char digits[24];
                const auto result = std::to_chars(digits, digits + sizeof(digits), static_cast<long long>(rounded));
                out.append(digits, static_cast<size_t>(result.ptr - digits));
            } else {
                AppendRepr(out, value);
            }
        }
    }

    std::string LoopToEdgesXml(const std::vector<Point2>& loop) {
        std::string edges;
        const size_t n = loop.size();
        edges.reserve(n * 32);
        for (size_t i = 0; i < n; ++i) {
            const Point2& curr = loop[i];
            const Point2& nxt = loop[(i + 1) % n];
            edges.push_back('!');
            AppendEdgeCoord(edges, curr.x * 20.0);
            edges.push_back(' ');
            AppendEdgeCoord(edges, curr.y * 20.0);
            edges.push_back('|');
            AppendEdgeCoord(edges, nxt.x * 20.0);
            edges.push_back(' ');
            AppendEdgeCoord(edges, nxt.y * 20.0);
        }
        return edges;
    }
}
