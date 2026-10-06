#include "Anim/Text.h"

#include "Animate/Effects/DropShadowEffect.h"
#include "Animate/Effects/GlowEffect.h"
#include "Animate/Library/Properties/AdvancedEffectInstance.h"
#include "Animate/Pic/PicText.h"

namespace sc2fla::anim {
    using namespace Animate;

    TextStyle BuildTextStyle(const TextFieldRecord& tf) {
        TextStyle style;

        std::string face = tf.font_name;
        if (tf.bold || tf.italic)
            face += "-";
        if (tf.bold)
            face += "-Bold";
        if (tf.italic)
            face += "-Italic";
        style.fontName = face;

        style.fontSize = tf.font_size;

        style.fontColor = ColorAlpha {
            static_cast<uint8_t>((tf.font_color >> 24) & 0xFF),
            static_cast<uint8_t>((tf.font_color >> 16) & 0xFF),
            static_cast<uint8_t>((tf.font_color >> 8) & 0xFF),
            static_cast<uint8_t>(tf.font_color & 0xFF),
        };

        switch (tf.alignment) {
            case TextAlign::Right:
                style.aligment = TextStyle::Aligment::Right;
                break;
            case TextAlign::Center:
                style.aligment = TextStyle::Aligment::Center;
                break;
            case TextAlign::Justify:
                style.aligment = TextStyle::Aligment::Justify;
                break;
            case TextAlign::Left:
                style.aligment = TextStyle::Aligment::Left;
                break;
        }

        style.autoKern = tf.auto_kern;
        style.lineSpacing = 4.f;

        return style;
    }

    void ApplyTextFilters(Animate::Pic::Text& text, const TextFieldRecord& tf) {
        auto& effects = static_cast<Library::AdvancedEffectInstance&>(*text.GetGraphicEffectInstance());

        if (tf.has_outline) {
            auto& shadow = static_cast<Effect::DropShadowEffect&>(effects.AddGraphicEffect(Effect::GraphicEffect::Type::DropShadow));

            shadow.SetColor(ColorAlpha {0, 0, 0, 255});
            shadow.SetQuality(Effect::GraphicEffect::Quality::High);
            shadow.SetAngle(45.f);
            shadow.SetBlurX(0);
            shadow.SetBlurY(0);
            shadow.SetDistance(1);
            shadow.SetStrength(0);
        }

        if (tf.bend_angle != 0) {
            auto& shadow = static_cast<Effect::DropShadowEffect&>(effects.AddGraphicEffect(Effect::GraphicEffect::Type::DropShadow));

            shadow.SetColor(ColorAlpha {0, 0, 0, 255});
            shadow.SetAngle(static_cast<float>((tf.bend_angle / 65535.0) * 360.0));
            shadow.SetBlurX(4);
            shadow.SetBlurY(4);
            shadow.SetDistance(4);
            shadow.SetStrength(0);
        }
    }
}
