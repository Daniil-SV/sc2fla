#include "MovieClip/Playback.h"

#include "Animate/Pic/PicFrame.h"
#include "Animate/Pic/PicLayer.h"

#include <algorithm>
#include <vector>

namespace sc2fla {
    namespace {
        bool PlaysOnItsOwn(const ResourceTable& resources, ConversionContext& ctx, uint32_t id, std::vector<uint32_t>& in_progress) {
            auto cached = ctx.plays_on_its_own_cache.find(id);
            if (cached != ctx.plays_on_its_own_cache.end())
                return cached->second;
            auto it = resources.movieclips.find(id);
            if (it == resources.movieclips.end())
                return false;
            if (std::find(in_progress.begin(), in_progress.end(), id) != in_progress.end())
                return false;
            const MovieClipRecord& mc = it->second;

            bool result = false;
            if (mc.frames.size() > 1) {
                result = std::none_of(mc.frames.begin(), mc.frames.end(), [](const MovieClipFrameRecord& f) { return !f.label.empty(); });
            } else if (mc.frames.size() == 1) {
                in_progress.push_back(id);
                for (const MovieClipFrameElementRecord& element : mc.frames[0].elements) {
                    if (element.instance_index >= mc.binds.size())
                        continue;
                    const uint32_t child = mc.binds[element.instance_index].id;
                    if (resources.KindOf(child) != ResourceKind::MovieClip)
                        continue;
                    if (PlaysOnItsOwn(resources, ctx, child, in_progress)) {
                        result = true;
                        break;
                    }
                }
                in_progress.pop_back();
            }
            ctx.plays_on_its_own_cache[id] = result;
            return result;
        }

        uint32_t PlayableLength(const ResourceTable& resources, ConversionContext& ctx, uint32_t id, std::vector<uint32_t>& in_progress) {
            auto cached = ctx.playable_length_cache.find(id);
            if (cached != ctx.playable_length_cache.end())
                return cached->second;
            auto it = resources.movieclips.find(id);
            if (it == resources.movieclips.end())
                return 1;
            if (std::find(in_progress.begin(), in_progress.end(), id) != in_progress.end())
                return 1;
            const MovieClipRecord& mc = it->second;

            uint32_t length = mc.frames.size() > 1 ? static_cast<uint32_t>(mc.frames.size()) : 1;
            if (mc.frames.size() == 1) {
                in_progress.push_back(id);
                for (const MovieClipFrameElementRecord& element : mc.frames[0].elements) {
                    if (element.instance_index >= mc.binds.size())
                        continue;
                    const uint32_t child = mc.binds[element.instance_index].id;
                    if (resources.KindOf(child) != ResourceKind::MovieClip)
                        continue;
                    length = std::max(length, PlayableLength(resources, ctx, child, in_progress));
                }
                in_progress.pop_back();
            }
            ctx.playable_length_cache[id] = length;
            return length;
        }
    }

    bool PlaysOnItsOwn(const ResourceTable& resources, ConversionContext& ctx, uint32_t id) {
        std::vector<uint32_t> in_progress;
        return ctx.animated_clips_play && PlaysOnItsOwn(resources, ctx, id, in_progress);
    }

    uint32_t PlayableLength(const ResourceTable& resources, ConversionContext& ctx, uint32_t id) {
        std::vector<uint32_t> in_progress;
        return PlayableLength(resources, ctx, id, in_progress);
    }

    void HoldLayersUntil(Animate::Pic::Page& page, uint32_t target) {
        for (Animate::Pic::Layer& layer : page) {
            uint32_t total = 0;
            Animate::Pic::Frame* last = nullptr;
            for (Animate::Pic::Frame& frame : layer) {
                total += frame.GetDuration();
                last = &frame;
            }
            if (last != nullptr && total < target)
                last->SetDuration(last->GetDuration() + (target - total));
        }
    }
}
