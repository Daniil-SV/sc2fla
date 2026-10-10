#include "MovieClip/MaskBinds.h"

namespace sc2fla {
    std::vector<char> FindMaskContentBinds(const ResourceTable& resources, const MovieClipRecord& movieclip) {
        std::vector<char> in_mask(movieclip.binds.size(), 0);
        for (const MovieClipFrameRecord& frame : movieclip.frames) {
            bool mask_content = false;
            for (const MovieClipFrameElementRecord& fe : frame.elements) {
                if (fe.instance_index >= movieclip.binds.size())
                    continue;
                const uint32_t id = movieclip.binds[fe.instance_index].id;
                if (resources.KindOf(id) == ResourceKind::Modifier) {
                    mask_content = resources.modifiers.at(id).type == MovieClipModifierRecord::Type::Mask;
                    continue;
                }
                if (mask_content)
                    in_mask[fe.instance_index] = 1;
            }
        }
        return in_mask;
    }
}
