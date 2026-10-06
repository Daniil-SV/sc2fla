#include "MovieClip/VirtualLayout.h"

#include "MovieClip/LayerOrder.h"

#include <algorithm>
#include <map>
#include <unordered_map>

namespace sc2fla {
    namespace {
        using Role = VirtualLayer::Role;

        uint64_t LayerKey(int bind, Role role, int parent) {
            return (static_cast<uint64_t>(static_cast<uint32_t>(bind)) << 36) | (static_cast<uint64_t>(role) << 32) | static_cast<uint32_t>(parent + 1);
        }

        struct Item {
            int bind;
            Role role;
            const MovieClipFrameElementRecord* fe;
        };

        struct Candidates {
            std::vector<int> begin;
            std::vector<int> pos;
            std::vector<int> id;
            std::vector<int> item;
            std::vector<int> length;
            std::vector<int> prev;
        };

        void ChooseChain(Candidates& cand, int n, std::vector<int>& chosen) {
            const int total = static_cast<int>(cand.pos.size());
            cand.length.assign(static_cast<size_t>(total), 1);
            cand.prev.assign(static_cast<size_t>(total), -1);

            for (int q = 0; q < total; ++q) {
                const int c = cand.pos[static_cast<size_t>(q)];
                const int limit = cand.begin[static_cast<size_t>(cand.item[static_cast<size_t>(q)])];
                int best_len = 1, best_prev = -1;
                for (int r = 0; r < limit; ++r) {
                    const int c2 = cand.pos[static_cast<size_t>(r)];
                    if (c2 >= c)
                        continue;
                    const int l = cand.length[static_cast<size_t>(r)] + 1;
                    if (l > best_len || (l == best_len && best_prev != -1 && c2 < cand.pos[static_cast<size_t>(best_prev)])) {
                        best_len = l;
                        best_prev = r;
                    }
                }
                cand.length[static_cast<size_t>(q)] = best_len;
                cand.prev[static_cast<size_t>(q)] = best_prev;
            }

            chosen.assign(static_cast<size_t>(n), -1);
            int best = -1;
            for (int q = 0; q < total; ++q) {
                if (best < 0) {
                    best = q;
                    continue;
                }
                const int l1 = cand.length[static_cast<size_t>(q)], l0 = cand.length[static_cast<size_t>(best)];
                const int p1 = cand.pos[static_cast<size_t>(q)], p0 = cand.pos[static_cast<size_t>(best)];
                const bool better = (l1 != l0)   ? (l1 > l0)
                                    : (p1 != p0) ? (p1 < p0)
                                                 : (cand.item[static_cast<size_t>(q)] > cand.item[static_cast<size_t>(best)]);
                if (better)
                    best = q;
            }
            for (int cq = best; cq >= 0; cq = cand.prev[static_cast<size_t>(cq)]) {
                chosen[static_cast<size_t>(cand.item[static_cast<size_t>(cq)])] = cand.id[static_cast<size_t>(cq)];
            }
        }
    }

    VirtualLayout BuildVirtualLayout(const MovieClipRecord& movieclip, const std::vector<BindUse>& uses) {
        VirtualLayout vl;
        const size_t frame_count = movieclip.frames.size();
        vl.frames.resize(frame_count);

        std::unordered_map<uint64_t, std::vector<int>> layers_by_key;
        std::vector<int> stack;
        std::map<int, std::vector<int>> children;
        std::vector<std::vector<int>> frame_main_layers(frame_count);
        int unnamed_counter = 0;

        auto make_layer = [&](int bind, Role role, int parent) {
            VirtualLayer layer;
            layer.bind = bind;
            layer.role = role;
            layer.parent = parent;
            const std::string& bind_name = movieclip.binds[static_cast<size_t>(bind)].name;
            layer.name = bind_name.empty() ? ("Layer " + std::to_string(unnamed_counter++)) : bind_name;
            int id = static_cast<int>(vl.layers.size());
            vl.layers.push_back(std::move(layer));
            layers_by_key[LayerKey(bind, role, parent)].push_back(id);
            return id;
        };
        auto stack_index_of = [&](int id) { return static_cast<int>(std::find(stack.begin(), stack.end(), id) - stack.begin()); };

        std::vector<Item> items;
        std::vector<int> main_items;
        std::vector<int> pos_of;
        std::vector<int> chosen;
        std::vector<int> item_layer;
        Candidates cand;

        for (size_t f = 0; f < frame_count; ++f) {
            items.clear();
            enum class Mode {
                Normal,
                MaskContent,
                MaskedContent
            } mode = Mode::Normal;
            bool have_mask = false;
            for (const MovieClipFrameElementRecord& fe : movieclip.frames[f].elements) {
                size_t b = fe.instance_index;
                if (b >= uses.size())
                    continue;
                const BindUse use = uses[b];
                if (use != BindUse::Symbol) {
                    if (use == BindUse::MaskModifier) {
                        mode = Mode::MaskContent;
                    } else if (use == BindUse::MaskedModifier) {
                        mode = Mode::MaskedContent;
                    } else if (use == BindUse::UnmaskedModifier) {
                        mode = Mode::Normal;
                        have_mask = false;
                    }
                    continue;
                }
                Role role = Role::Normal;
                if (mode == Mode::MaskContent) {
                    role = Role::Mask;
                    have_mask = true;
                } else if (mode == Mode::MaskedContent && have_mask) {
                    role = Role::Masked;
                }
                items.push_back({static_cast<int>(b), role, &fe});
            }

            main_items.clear();
            for (size_t k = 0; k < items.size(); ++k) {
                if (items[k].role != Role::Masked)
                    main_items.push_back(static_cast<int>(k));
            }
            const int n = static_cast<int>(main_items.size());

            pos_of.assign(vl.layers.size(), -1);
            for (size_t p = 0; p < stack.size(); ++p)
                pos_of[static_cast<size_t>(stack[p])] = static_cast<int>(p);

            cand.begin.assign(static_cast<size_t>(n) + 1, 0);
            cand.pos.clear();
            cand.id.clear();
            cand.item.clear();
            for (int i = 0; i < n; ++i) {
                cand.begin[static_cast<size_t>(i)] = static_cast<int>(cand.pos.size());
                const Item& it = items[static_cast<size_t>(main_items[static_cast<size_t>(i)])];
                auto found = layers_by_key.find(LayerKey(it.bind, it.role, -1));
                if (found == layers_by_key.end())
                    continue;
                for (int id : found->second) {
                    cand.pos.push_back(pos_of[static_cast<size_t>(id)]);
                    cand.id.push_back(id);
                    cand.item.push_back(i);
                }
            }
            cand.begin[static_cast<size_t>(n)] = static_cast<int>(cand.pos.size());

            ChooseChain(cand, n, chosen);

            item_layer.assign(items.size(), -1);
            int prev_main_layer = -1;
            bool prev_main_new = false;
            int current_mask = -1;
            int mi = 0;
            for (size_t k = 0; k < items.size(); ++k) {
                const Item& it = items[k];
                if (it.role != Role::Masked) {
                    const int i = mi++;
                    int id;
                    bool is_new = false;
                    if (chosen[static_cast<size_t>(i)] >= 0) {
                        id = chosen[static_cast<size_t>(i)];
                    } else {
                        id = make_layer(it.bind, it.role, -1);
                        int ins = static_cast<int>(stack.size());
                        for (int j = i + 1; j < n; ++j) {
                            if (chosen[static_cast<size_t>(j)] >= 0) {
                                ins = stack_index_of(chosen[static_cast<size_t>(j)]);
                                break;
                            }
                        }
                        if (i > 0 && prev_main_new)
                            ins = stack_index_of(prev_main_layer) + 1;
                        stack.insert(stack.begin() + ins, id);
                        is_new = true;
                    }
                    prev_main_layer = id;
                    prev_main_new = is_new;
                    if (it.role == Role::Mask)
                        current_mask = id;
                    item_layer[k] = id;
                    frame_main_layers[f].push_back(id);
                } else {
                    auto found = layers_by_key.find(LayerKey(it.bind, Role::Masked, current_mask));
                    int id;
                    if (found != layers_by_key.end() && !found->second.empty()) {
                        id = found->second.front();
                    } else {
                        id = make_layer(it.bind, Role::Masked, current_mask);
                        auto& kids = children[current_mask];
                        kids.insert(kids.begin(), id);
                    }
                    item_layer[k] = id;
                }
                vl.frames[f].push_back({item_layer[k], it.fe->matrix_index, it.fe->color_index});
            }
        }

        std::vector<int> main_ids(stack.begin(), stack.end());
        std::sort(main_ids.begin(), main_ids.end());
        const std::vector<LayerEdge> edges = DecideLayerOrderPairs(frame_count, frame_main_layers);
        const std::vector<int> order = TopologicalSortLayerOrder(main_ids, edges);
        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            vl.final_order.push_back(*it);
            if (vl.layers[static_cast<size_t>(*it)].role == Role::Mask) {
                auto kids = children.find(*it);
                if (kids != children.end()) {
                    for (int child : kids->second)
                        vl.final_order.push_back(child);
                }
            }
        }
        return vl;
    }
}
