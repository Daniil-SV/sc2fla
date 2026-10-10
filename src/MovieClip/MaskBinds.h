#pragma once

#include "Model/ResourceTable.h"

#include <vector>

namespace sc2fla {
    std::vector<char> FindMaskContentBinds(const ResourceTable& resources, const MovieClipRecord& movieclip);
}
