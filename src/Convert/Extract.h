#pragma once

#include "Media/RgbaImage.h"
#include "Model/MatrixBank.h"
#include "Model/MovieClip.h"
#include "Model/Shape.h"
#include "Model/TextField.h"

#include <flash/display_object/MovieClip.h>
#include <flash/display_object/MovieClipModifier.h>
#include <flash/display_object/Shape.h>
#include <flash/display_object/TextField.h>
#include <flash/transform/MatrixBank.h>
#include <vector>

namespace sc2fla {
    ShapeRecord ExtractShape(sc::flash::Shape& shape, const std::vector<RgbaImage>& textures);

    TextFieldRecord ExtractTextField(const sc::flash::TextField& tf);

    MovieClipModifierRecord ExtractModifier(const sc::flash::MovieClipModifier& mod);

    MovieClipRecord ExtractMovieClip(const sc::flash::MovieClip& mc);

    MatrixBankRecord ExtractMatrixBank(const sc::flash::MatrixBank& bank);
}
