#include "Convert/GroupSelection.h"

#include "Core/GroupPattern.h"

namespace sc2fla {
    GroupSelection ComputeGroupSelection(const ResourceTable& resources, const std::vector<std::string>& patterns) {
        GroupSelection sel;
        std::vector<uint32_t> queue;

        for (auto& kv : resources.exports) {
            uint32_t id = kv.first;
            std::vector<std::string> matched;
            for (auto& name : kv.second) {
                for (auto& pattern : patterns) {
                    if (NameMatchesGroupPattern(name, pattern)) {
                        matched.push_back(name);
                        break;
                    }
                }
            }
            if (matched.empty())
                continue;

            sel.matched_root_names[id] = std::move(matched);
            if (resources.KindOf(id) == ResourceKind::MovieClip) {
                if (sel.keep_movieclips.insert(id).second)
                    queue.push_back(id);
            } else if (resources.KindOf(id) == ResourceKind::Shape) {
                sel.keep_shapes.insert(id);
            }
        }

        while (!queue.empty()) {
            uint32_t id = queue.back();
            queue.pop_back();
            auto it = resources.movieclips.find(id);
            if (it == resources.movieclips.end())
                continue;
            for (auto& bind : it->second.binds) {
                ResourceKind kind = resources.KindOf(bind.id);
                if (kind == ResourceKind::MovieClip) {
                    if (sel.keep_movieclips.insert(bind.id).second)
                        queue.push_back(bind.id);
                } else if (kind == ResourceKind::Shape) {
                    sel.keep_shapes.insert(bind.id);
                }
            }
        }

        return sel;
    }
}
