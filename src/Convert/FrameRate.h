#pragma once

#include "Convert/GroupSelection.h"
#include "Model/ResourceTable.h"

namespace sc2fla {
    int DetectFrameRate(const ResourceTable& resources, bool group_active, const GroupSelection& group_sel);
}
