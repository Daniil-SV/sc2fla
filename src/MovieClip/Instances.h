#pragma once

#include "Anim/Project.h"
#include "Animate/Pic/PicFrame.h"
#include "Convert/Context.h"
#include "Model/ResourceTable.h"
#include "MovieClip/BuildState.h"

namespace sc2fla {
    void PrepareLayersAndInstances(anim::Project& project,
                                   ConversionContext& ctx,
                                   const ResourceTable& resources,
                                   const MovieClipRecord& movieclip,
                                   Animate::Library::DocumentPage& movie_page,
                                   MovieClipBuildState& state);

    Animate::Pic::Object& PlaceInstance(Animate::Pic::Frame& frame, const BindInstanceSpec& spec);
}
