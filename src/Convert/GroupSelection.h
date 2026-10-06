#pragma once

#include "Model/ResourceTable.h"

#include <cstdint>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace sc2fla {
    struct GroupSelection {
        std::unordered_map<uint32_t, std::vector<std::string>> matched_root_names;
        std::set<uint32_t> keep_movieclips;
        std::set<uint32_t> keep_shapes;
    };

    GroupSelection ComputeGroupSelection(const ResourceTable& resources, const std::vector<std::string>& patterns);
}
