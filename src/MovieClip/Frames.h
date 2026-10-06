#pragma once

#include "Animate/Library/LibraryItem/DocumentPage.h"
#include "Animate/Pic/PicLayer.h"
#include "Animate/Pic/PicPage.h"
#include "Model/ResourceTable.h"
#include "MovieClip/BuildState.h"
#include "MovieClip/VirtualLayout.h"

#include <vector>

namespace sc2fla {
    void CreateVirtualLayers(Animate::Pic::Page& page, const VirtualLayout& vl, std::vector<Animate::Pic::Layer*>& layers_instance);

    void BuildVirtualLayerFrames(const MovieClipRecord& movieclip,
                                 const ResourceTable& resources,
                                 const MovieClipBuildState& state,
                                 const VirtualLayout& vl,
                                 const std::vector<Animate::Pic::Layer*>& layers_instance);

    void ApplyNineSliceScaleGrid(Animate::Library::DocumentPage& movie, const MovieClipRecord& movieclip);

    void InsertFrameLabelsLayer(Animate::Pic::Page& page, const MovieClipRecord& movieclip);
}
