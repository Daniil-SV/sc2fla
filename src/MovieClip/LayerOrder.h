#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace sc2fla {
    using LayerEdge = std::pair<int, int>;

    std::vector<LayerEdge> DecideLayerOrderPairs(size_t frame_count, const std::vector<std::vector<int>>& frame_layers);

    std::vector<int> TopologicalSortLayerOrder(const std::vector<int>& layer_ids, const std::vector<LayerEdge>& edges);
}
