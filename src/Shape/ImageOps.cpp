#include "Shape/ImageOps.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace sc2fla {
    RgbaImage RotateImage(RgbaImage src, double degrees_ccw_negated) {
        double angle = degrees_ccw_negated;

        long norm = ((static_cast<long>(std::llround(angle)) % 360) + 360) % 360;

        if (norm == 0)
            return src;

        RgbaImage out;
        if (norm == 180) {
            out.width = src.width;
            out.height = src.height;
            out.pixels.resize(src.pixels.size());
            for (int y = 0; y < src.height; ++y)
                for (int x = 0; x < src.width; ++x) {
                    size_t s = (static_cast<size_t>(y) * src.width + x) * 4;
                    size_t d = (static_cast<size_t>(src.height - 1 - y) * src.width + (src.width - 1 - x)) * 4;
                    for (int k = 0; k < 4; ++k)
                        out.pixels[d + k] = src.pixels[s + k];
                }
            return out;
        }

        out.width = src.height;
        out.height = src.width;
        out.pixels.assign(static_cast<size_t>(out.width) * out.height * 4, 0);
        for (int y = 0; y < src.height; ++y) {
            for (int x = 0; x < src.width; ++x) {
                size_t s = (static_cast<size_t>(y) * src.width + x) * 4;
                int dx, dy;
                if (norm == 90) {
                    dx = y;
                    dy = src.width - 1 - x;
                } else {
                    dx = src.height - 1 - y;
                    dy = x;
                }
                size_t d = (static_cast<size_t>(dy) * out.width + dx) * 4;
                for (int k = 0; k < 4; ++k)
                    out.pixels[d + k] = src.pixels[s + k];
            }
        }
        return out;
    }

    void FlipHorizontal(RgbaImage& image) {
        const size_t width = static_cast<size_t>(image.width);
        for (int y = 0; y < image.height; ++y) {
            uint8_t* row = image.pixels.data() + static_cast<size_t>(y) * width * 4;
            for (size_t x = 0; x < width / 2; ++x) {
                uint8_t* left = row + x * 4;
                uint8_t* right = row + (width - 1 - x) * 4;
                for (int k = 0; k < 4; ++k)
                    std::swap(left[k], right[k]);
            }
        }
    }

    bool NormalizeSpriteOrientation(RgbaImage& sprite, ShapeBitmapCommand::MatrixResult& mres) {
        auto& p = mres.transform.params;
        const double L00 = p[0], L01 = p[1], L10 = p[2], L11 = p[3];
        const double scale = std::max({std::abs(L00), std::abs(L01), std::abs(L10), std::abs(L11)});
        if (scale < 1e-12)
            return false;
        const double tol = 1e-3 * scale;
        if (std::abs(L01) < tol && std::abs(L10) < tol && L00 > 0.0 && L11 > 0.0)
            return false;

        static const int kQ[8][4] = {{1, 0, 0, 1}, {-1, 0, 0, 1}, {1, 0, 0, -1}, {-1, 0, 0, -1}, {0, 1, 1, 0}, {0, -1, 1, 0}, {0, 1, -1, 0}, {0, -1, -1, 0}};
        for (const auto& q : kQ) {
            const double n00 = L00 * q[0] + L01 * q[1];
            const double n01 = L00 * q[2] + L01 * q[3];
            const double n10 = L10 * q[0] + L11 * q[1];
            const double n11 = L10 * q[2] + L11 * q[3];
            if (!(std::abs(n01) < tol && std::abs(n10) < tol && n00 > 0.0 && n11 > 0.0))
                continue;

            const int w = sprite.width, h = sprite.height;
            const bool swap_axes = (q[0] == 0);
            const int w2 = swap_axes ? h : w, h2 = swap_axes ? w : h;

            const int ox = (q[0] < 0 ? w - 1 : 0) + (q[1] < 0 ? h - 1 : 0);
            const int oy = (q[2] < 0 ? w - 1 : 0) + (q[3] < 0 ? h - 1 : 0);
            const double ocx = (q[0] < 0 ? w : 0) + (q[1] < 0 ? h : 0);
            const double ocy = (q[2] < 0 ? w : 0) + (q[3] < 0 ? h : 0);

            RgbaImage out;
            out.width = w2;
            out.height = h2;
            out.pixels.assign(static_cast<size_t>(w2) * static_cast<size_t>(h2) * 4, 0);
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    const int nx = q[0] * x + q[1] * y + ox;
                    const int ny = q[2] * x + q[3] * y + oy;
                    const size_t src = (static_cast<size_t>(y) * w + x) * 4;
                    const size_t dst = (static_cast<size_t>(ny) * w2 + nx) * 4;
                    for (int k = 0; k < 4; ++k)
                        out.pixels[dst + k] = sprite.pixels[src + k];
                }
            }
            sprite = std::move(out);

            for (auto& pt : mres.sprite_box) {
                const double bx = pt.x, by = pt.y;
                pt.x = q[0] * bx + q[1] * by + ocx;
                pt.y = q[2] * bx + q[3] * by + ocy;
            }
            const double t0 = p[4] - (n00 * ocx + n01 * ocy);
            const double t1 = p[5] - (n10 * ocx + n11 * ocy);
            p[0] = n00;
            p[1] = 0.0;
            p[2] = 0.0;
            p[3] = n11;
            p[4] = t0;
            p[5] = t1;
            mres.rotation = 0.0;
            mres.mirroring = false;
            return true;
        }
        return false;
    }
}
