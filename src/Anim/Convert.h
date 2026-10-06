#pragma once

#include "Animate/Library/Properties/GraphicEffectInstance.h"
#include "Animate/Math/ColorTransform.h"
#include "Animate/XFL/DOM/Math/Matrix.h"
#include "Model/BlendMode.h"
#include "Model/MatrixBank.h"

namespace sc2fla::anim {
    inline Animate::Matrix ToMatrix(double a, double b, double c, double d, double tx, double ty) {
        Animate::Matrix m;
        m.a = static_cast<float>(a);
        m.b = static_cast<float>(b);
        m.c = static_cast<float>(c);
        m.d = static_cast<float>(d);
        m.tx = static_cast<float>(tx);
        m.ty = static_cast<float>(ty);
        return m;
    }

    inline Animate::Library::GraphicEffectInstance::BlendMode ToBlendMode(BlendMode blend) {
        using Target = Animate::Library::GraphicEffectInstance::BlendMode;
        switch (blend) {
            case BlendMode::Layer:
                return Target::Layer;
            case BlendMode::Multiply:
                return Target::Multiply;
            case BlendMode::Screen:
                return Target::Screen;
            case BlendMode::Lighten:
                return Target::Lighten;
            case BlendMode::Darken:
                return Target::Darken;
            case BlendMode::Difference:
                return Target::Difference;
            case BlendMode::Add:
                return Target::Add;
            case BlendMode::Subtract:
                return Target::Subtract;
            case BlendMode::Invert:
                return Target::Invert;
            case BlendMode::Alpha:
                return Target::Alpha;
            case BlendMode::Erase:
                return Target::Erase;
            case BlendMode::Overlay:
                return Target::Overlay;
            case BlendMode::HardLight:
                return Target::HardLight;
            case BlendMode::None:
                break;
        }
        return Target::Normal;
    }

    Animate::ColorTransform ToColorTransform(const ColorTransformEntry& c);
}
