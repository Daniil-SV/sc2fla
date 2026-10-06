#include "Convert/FrameRate.h"

#include <utility>
#include <vector>

namespace sc2fla {
    int DetectFrameRate(const ResourceTable& resources, bool group_active, const GroupSelection& group_sel) {
        std::vector<uint8_t> candidates;
        auto collect_from_id = [&](uint32_t id) {
            auto it = resources.movieclips.find(id);
            if (it != resources.movieclips.end())
                candidates.push_back(it->second.frame_rate);
        };

        if (group_active) {
            for (auto& kv : group_sel.matched_root_names)
                collect_from_id(kv.first);
        } else {
            for (auto& kv : resources.exports)
                collect_from_id(kv.first);
        }

        if (candidates.empty()) {
            for (auto& kv : resources.movieclips)
                candidates.push_back(kv.second.frame_rate);
        }

        if (candidates.empty())
            return 30;

        std::vector<std::pair<uint8_t, int>> counts;
        for (uint8_t v : candidates) {
            bool found = false;
            for (auto& c : counts) {
                if (c.first == v) {
                    ++c.second;
                    found = true;
                    break;
                }
            }
            if (!found)
                counts.push_back({v, 1});
        }
        uint8_t best = counts.front().first;
        int best_count = counts.front().second;
        for (auto& c : counts) {
            if (c.second > best_count) {
                best = c.first;
                best_count = c.second;
            }
        }
        return static_cast<int>(best);
    }
}
