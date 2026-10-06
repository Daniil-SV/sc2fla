#include "MovieClip/LayerOrder.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <set>
#include <unordered_set>

namespace sc2fla {
    namespace {
        constexpr size_t DENSE_LIMIT = 4096;

        template <typename FirstSeen>
        void VisitFrames(size_t frame_count, const std::vector<std::vector<int>>& frame_layers, FirstSeen&& first_seen, std::vector<LayerEdge>& edges) {
            auto consider_frame = [&](size_t fi) {
                const std::vector<int>& ids = frame_layers[fi];
                for (size_t i = 0; i < ids.size(); ++i) {
                    for (size_t j = i + 1; j < ids.size(); ++j) {
                        if (first_seen(ids[i], ids[j]))
                            edges.emplace_back(ids[i], ids[j]);
                    }
                }
            };

            const size_t last = frame_count - 1;
            for (size_t checkpoint : {size_t {0}, last / 2, last})
                consider_frame(checkpoint);
            for (size_t fi = 0; fi < frame_count; ++fi)
                consider_frame(fi);
        }
    }

    std::vector<LayerEdge> DecideLayerOrderPairs(size_t frame_count, const std::vector<std::vector<int>>& frame_layers) {
        std::vector<LayerEdge> edges;
        if (frame_count == 0)
            return edges;

        int max_id = -1;
        for (const auto& ids : frame_layers) {
            for (int id : ids)
                max_id = std::max(max_id, id);
        }
        const size_t span = static_cast<size_t>(max_id + 1);

        if (span <= DENSE_LIMIT) {
            std::vector<uint8_t> decided(span * span, 0);
            VisitFrames(
                frame_count,
                frame_layers,
                [&](int a, int b) {
                    const size_t lo = static_cast<size_t>(std::min(a, b));
                    const size_t hi = static_cast<size_t>(std::max(a, b));
                    uint8_t& cell = decided[lo * span + hi];
                    if (cell)
                        return false;
                    cell = 1;
                    return true;
                },
                edges);
        } else {
            std::unordered_set<uint64_t> decided;
            VisitFrames(
                frame_count,
                frame_layers,
                [&](int a, int b) {
                    const uint64_t lo = static_cast<uint32_t>(std::min(a, b));
                    const uint64_t hi = static_cast<uint32_t>(std::max(a, b));
                    return decided.insert((lo << 32) | hi).second;
                },
                edges);
        }
        return edges;
    }

    std::vector<int> TopologicalSortLayerOrder(const std::vector<int>& layer_ids, const std::vector<LayerEdge>& edges) {
        const size_t count = layer_ids.size();
        const int max_id = layer_ids.empty() ? -1 : layer_ids.back();

        std::vector<int> index_of(static_cast<size_t>(max_id + 1), -1);
        for (size_t i = 0; i < count; ++i)
            index_of[static_cast<size_t>(layer_ids[i])] = static_cast<int>(i);

        auto lookup = [&](int id) { return id >= 0 && id <= max_id ? index_of[static_cast<size_t>(id)] : -1; };

        std::vector<int> in_degree(count, 0);
        std::vector<size_t> out_begin(count + 1, 0);
        for (const LayerEdge& e : edges) {
            const int from = lookup(e.first);
            const int to = lookup(e.second);
            if (from >= 0)
                ++out_begin[static_cast<size_t>(from) + 1];
            if (to >= 0)
                ++in_degree[static_cast<size_t>(to)];
        }
        for (size_t i = 0; i < count; ++i)
            out_begin[i + 1] += out_begin[i];

        std::vector<int> out_edges(out_begin[count]);
        std::vector<size_t> cursor(out_begin.begin(), out_begin.end() - 1);
        for (const LayerEdge& e : edges) {
            const int from = lookup(e.first);
            if (from >= 0)
                out_edges[cursor[static_cast<size_t>(from)]++] = lookup(e.second);
        }

        std::vector<char> alive(count, 1);
        size_t alive_count = count;
        size_t top = count;

        std::set<int> ready;
        for (size_t i = 0; i < count; ++i) {
            if (in_degree[i] == 0)
                ready.insert(static_cast<int>(i));
        }

        std::vector<int> order;
        order.reserve(count);
        while (alive_count > 0) {
            int v;
            if (!ready.empty()) {
                auto last = std::prev(ready.end());
                v = *last;
                ready.erase(last);
            } else {
                while (!alive[top - 1])
                    --top;
                v = static_cast<int>(top - 1);
            }
            order.push_back(layer_ids[static_cast<size_t>(v)]);
            alive[static_cast<size_t>(v)] = 0;
            --alive_count;

            for (size_t k = out_begin[static_cast<size_t>(v)]; k < out_begin[static_cast<size_t>(v) + 1]; ++k) {
                const int next = out_edges[k];
                if (next >= 0 && alive[static_cast<size_t>(next)] && --in_degree[static_cast<size_t>(next)] <= 0)
                    ready.insert(next);
            }
        }
        return order;
    }
}
