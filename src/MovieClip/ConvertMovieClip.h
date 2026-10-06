#pragma once

#include "Anim/Project.h"
#include "Convert/Context.h"
#include "Model/ResourceTable.h"

#include <cstdint>
#include <string>
#include <vector>

namespace sc2fla {
    void ConvertMovieClip(anim::Project& project,
                          ConversionContext& ctx,
                          const ResourceTable& resources,
                          uint32_t id,
                          const MovieClipRecord& movieclip,
                          const std::vector<std::string>* export_names);
}
