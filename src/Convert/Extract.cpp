#include "Convert/Extract.h"

#include "Core/Math.h"
#include "Core/Recover.h"

#include <algorithm>
#include <cmath>

namespace sc2fla {
    namespace {
        void NormalizeVertexOrder(sc::flash::Shape& shape) {
            for (auto& cmd : shape.commands) {
                if (cmd.type == sc::flash::TriangulatorFunction::Strip) {
                    cmd.sort_advanced_vertices(false);
                }
            }
        }
    }

    ShapeRecord ExtractShape(sc::flash::Shape& shape, const std::vector<RgbaImage>& textures) {
        NormalizeVertexOrder(shape);

        ShapeRecord rec;
        rec.id = shape.id;
        rec.bitmaps.reserve(shape.commands.size());

        for (auto& cmd : shape.commands) {
            ShapeBitmapCommand bc;
            bc.texture_index = static_cast<int>(cmd.texture_index);

            int tw = 1, th = 1;
            if (bc.texture_index >= 0 && static_cast<size_t>(bc.texture_index) < textures.size()) {
                tw = std::max(1, textures[static_cast<size_t>(bc.texture_index)].width);
                th = std::max(1, textures[static_cast<size_t>(bc.texture_index)].height);
            }

            size_t vcount = cmd.vertices.size();
            bc.uv_coords.reserve(vcount);
            bc.xy_coords.reserve(vcount);

            for (size_t vi = 0; vi < vcount; ++vi) {
                const auto& v = cmd.vertices[vi];

                double px = RecoverTwip(v.x);
                double py = RecoverTwip(v.y);
                bc.xy_coords.push_back({px, py});

                uint32_t raw_u = RecoverU16FromNormalized(v.u);
                uint32_t raw_v = RecoverU16FromNormalized(v.v);
                double pu = (raw_u / 65535.0) * tw;
                double pv = (raw_v / 65535.0) * th;
                bc.uv_coords.push_back({static_cast<double>(Ceil(pu)), static_cast<double>(Ceil(pv))});
            }
            rec.bitmaps.push_back(std::move(bc));
        }
        return rec;
    }

    TextFieldRecord ExtractTextField(const sc::flash::TextField& tf) {
        TextFieldRecord rec;
        rec.id = tf.id;
        rec.left = tf.left;
        rec.top = tf.top;
        rec.right = tf.right;
        rec.bottom = tf.bottom;
        switch (tf.font_horizontal_align) {
            case sc::flash::TextField::Align::Right:
                rec.alignment = TextAlign::Right;
                break;
            case sc::flash::TextField::Align::Center:
                rec.alignment = TextAlign::Center;
                break;
            case sc::flash::TextField::Align::Justify:
                rec.alignment = TextAlign::Justify;
                break;
            default:
                rec.alignment = TextAlign::Left;
                break;
        }
        rec.text = tf.text.string();
        rec.font_name = tf.font_name.string();
        rec.font_size = tf.font_size;
        rec.bold = tf.style.is_bold();
        rec.italic = tf.style.is_italic();
        rec.font_color = tf.font_color.as_value();
        rec.has_outline = tf.style.is_outlined();
        rec.auto_kern = tf.style.auto_kern();

        long long raw_bend = std::llround(static_cast<double>(tf.bend_angle) * 360.0 / 32767.0);
        rec.bend_angle = static_cast<int16_t>(raw_bend);

        return rec;
    }

    MovieClipModifierRecord ExtractModifier(const sc::flash::MovieClipModifier& mod) {
        MovieClipModifierRecord rec;
        rec.id = mod.id;
        switch (mod.type) {
            case sc::flash::MovieClipModifier::Type::Mask:
                rec.type = MovieClipModifierRecord::Type::Mask;
                break;
            case sc::flash::MovieClipModifier::Type::Masked:
                rec.type = MovieClipModifierRecord::Type::Masked;
                break;
            case sc::flash::MovieClipModifier::Type::Unmasked:
                rec.type = MovieClipModifierRecord::Type::Unmasked;
                break;
        }
        return rec;
    }

    MovieClipRecord ExtractMovieClip(const sc::flash::MovieClip& mc) {
        MovieClipRecord rec;
        rec.id = mc.id;
        rec.matrix_bank = mc.bank_index;
        rec.frame_rate = mc.frame_rate;

        rec.binds.reserve(mc.childrens.size());
        for (auto& child : mc.childrens) {
            MovieClipBind b;
            b.id = child.id;
            b.name = child.name.string();
            b.blend = ResolveBlendMode(static_cast<uint8_t>(child.blend_mode));
            rec.binds.push_back(std::move(b));
        }

        if (mc.scaling_grid) {
            rec.has_nine_slice = true;
            rec.nine_slice_xy = {RecoverTwip(mc.scaling_grid->x), RecoverTwip(mc.scaling_grid->y)};

            rec.nine_slice_wh = {RecoverTwip(mc.scaling_grid->right) - rec.nine_slice_xy.x, RecoverTwip(mc.scaling_grid->bottom) - rec.nine_slice_xy.y};
        }

        size_t cursor = 0;
        rec.frames.reserve(mc.frames.size());
        for (auto& f : mc.frames) {
            MovieClipFrameRecord fr;
            fr.label = f.label.string();
            for (uint32_t k = 0; k < f.elements_count && cursor < mc.frame_elements.size(); ++k, ++cursor) {
                const auto& fe = mc.frame_elements[cursor];
                MovieClipFrameElementRecord e;
                e.instance_index = fe.instance_index;
                e.matrix_index = fe.matrix_index;
                e.color_index = fe.colorTransform_index;
                fr.elements.push_back(e);
            }
            rec.frames.push_back(std::move(fr));
        }

        return rec;
    }

    MatrixBankRecord ExtractMatrixBank(const sc::flash::MatrixBank& bank) {
        MatrixBankRecord rec;
        rec.matrices.reserve(bank.matrices.size());
        for (auto& m : bank.matrices) {
            MatrixBankEntry e;
            e.a = RecoverMatrixComponentAuto(m.a);
            e.b = RecoverMatrixComponentAuto(m.b);
            e.c = RecoverMatrixComponentAuto(m.c);
            e.d = RecoverMatrixComponentAuto(m.d);
            e.tx = RecoverTwip(m.tx);
            e.ty = RecoverTwip(m.ty);
            rec.matrices.push_back(e);
        }
        rec.color_transforms.reserve(bank.color_transforms.size());
        for (auto& c : bank.color_transforms) {
            ColorTransformEntry e;
            e.r_add = c.add.r;
            e.g_add = c.add.g;
            e.b_add = c.add.b;
            e.r_mul = c.multiply.r / 255.0;
            e.g_mul = c.multiply.g / 255.0;
            e.b_mul = c.multiply.b / 255.0;
            e.a_mul = c.alpha / 255.0;
            rec.color_transforms.push_back(e);
        }
        return rec;
    }
}
