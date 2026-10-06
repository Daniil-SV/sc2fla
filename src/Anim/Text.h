#pragma once

#include "Animate/Text/Style.h"
#include "Model/TextField.h"

namespace Animate::Pic {
    class Text;
}

namespace sc2fla::anim {
    Animate::TextStyle BuildTextStyle(const TextFieldRecord& tf);

    void ApplyTextFilters(Animate::Pic::Text& text, const TextFieldRecord& tf);
}
