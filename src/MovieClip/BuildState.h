#pragma once

#include "Animate/Library/LibraryItem/DocumentPage.h"
#include "Animate/Pic/PicGroup.h"
#include "Animate/Pic/PicLayer.h"
#include "Model/BlendMode.h"
#include "Model/TextField.h"
#include "MovieClip/VirtualLayout.h"

#include <optional>
#include <string>
#include <vector>

namespace sc2fla {
    struct BindInstanceSpec {
        enum class Kind {
            ShapeSymbol,
            MovieClipSymbol,
            NineSliceGroup,
            TextField
        };

        Kind kind = Kind::ShapeSymbol;
        Animate::Library::DocumentPage* page = nullptr;
        bool force_graphic = false;
        std::optional<std::string> instance_name;
        BlendMode blend = BlendMode::None;
        std::vector<Animate::Pic::Group*> nine_slice_template;
        const TextFieldRecord* textfield = nullptr;
        std::optional<std::string> text_instance_name;
    };

    struct MovieClipBuildState {
        std::vector<Animate::Pic::Layer*> layers_instance;
        std::vector<std::optional<BindInstanceSpec>> symbols_instance;
        std::vector<BindUse> uses;
    };
}
