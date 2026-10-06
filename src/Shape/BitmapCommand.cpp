#include "Core/Math.h"
#include "Model/Shape.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace sc2fla {
    namespace {
        constexpr double PI = 3.14159265358979323846;

        double DegToRad(double d) {
            return d * PI / 180.0;
        }

        double RadToDeg(double r) {
            return r * 180.0 / PI;
        }

        Point2 GetSize(const std::vector<Point2>& coords) {
            double left = coords[0].x, top = coords[0].y, right = coords[0].x, bottom = coords[0].y;
            for (auto& c : coords) {
                left = std::min(left, c.x);
                top = std::min(top, c.y);
                right = std::max(right, c.x);
                bottom = std::max(bottom, c.y);
            }
            return {right - left, bottom - top};
        }

        Point2 MoveByAngle(const Point2& point, double angle_rad, double distance) {
            double x = point.x + distance * std::sin(angle_rad);
            double y = point.y + distance * std::cos(angle_rad);
            return {static_cast<double>(Round0(x)), static_cast<double>(Round0(y))};
        }

        double FindAngle(const Point2& p1, const Point2& p2) {
            double dX = p2.x - p1.x;
            double dY = p2.y - p1.y;
            double rads = std::atan2(-dY, dX);
            return RadToDeg(rads);
        }

        std::vector<Point2> GetRightUv(bool inside, const std::vector<Point2>& coords) {
            std::vector<Point2> res = coords;
            Point2 size = GetSize(coords);
            double w = size.x, h = size.y;

            bool cond = (!inside && w == 0.0) || (h == 0.0);

            if (cond) {
                std::vector<Point2> unique_points;
                for (auto& c : coords) {
                    bool found = false;
                    for (auto& u : unique_points)
                        if (u.x == c.x && u.y == c.y) {
                            found = true;
                            break;
                        }
                    if (!found)
                        unique_points.push_back(c);
                }

                bool first_two_equal = (coords.size() >= 2 && coords[0].x == coords[1].x && coords[0].y == coords[1].y);

                for (size_t index = 0; index < unique_points.size(); ++index) {
                    const Point2& point = unique_points[index];
                    long point_idx;
                    if (first_two_equal) {
                        point_idx = static_cast<long>(index) + 1;
                    } else {
                        long found_from_end = -1;
                        for (long ri = 0; ri < static_cast<long>(res.size()); ++ri) {
                            const Point2& rp = res[res.size() - 1 - ri];
                            if (rp.x == point.x && rp.y == point.y) {
                                found_from_end = ri;
                                break;
                            }
                        }
                        point_idx = 3 - found_from_end;
                    }
                    if (point_idx < 0 || point_idx >= static_cast<long>(res.size()))
                        continue;
                    res[point_idx].x += (w == 0.0 ? 1.0 : 0.0);
                    res[point_idx].y += (h == 0.0 ? 1.0 : 0.0);
                }
                return res;
            }

            if (coords.size() <= 4) {
                double w_m = inside ? (w + 2.0) : (w > 2.0 ? (w - 2.0) : 0.0);
                double h_m = inside ? (h + 2.0) : (h > 2.0 ? (h - 2.0) : 0.0);

                double sx = 0, sy = 0;
                for (auto& c : coords) {
                    sx += c.x;
                    sy += c.y;
                }
                double c_x = sx / static_cast<double>(coords.size());
                double c_y = sy / static_cast<double>(coords.size());

                std::vector<Point2> out;
                out.reserve(coords.size());
                for (auto& c : coords) {
                    out.push_back({static_cast<double>(Round0((w_m / w) * (c.x - c_x) + c_x)), static_cast<double>(Round0((h_m / h) * (c.y - c_y) + c_y))});
                }
                return out;
            }

            for (size_t i = 0; i < coords.size(); ++i) {
                size_t last = (i == 0) ? coords.size() - 1 : i - 1;
                double angle = DegToRad(FindAngle(coords[last], coords[i]) - 45.0);
                res[i] = MoveByAngle(coords[i], angle, inside ? -1.0 : 1.0);
            }
            return res;
        }
    }

    std::pair<double, bool> ShapeBitmapCommand::GetRotation(bool nearest) const {
        auto is_clockwise = [](const std::vector<Point2>& points) {
            double sum = 0;
            for (size_t x = 0; x < points.size(); ++x) {
                const Point2& p1 = points[(x + 1) % points.size()];
                const Point2& p2 = points[x];
                sum += (p1.x - p2.x) * (p1.y + p2.y);
            }
            return sum < 0.0;
        };

        bool uv_cw = is_clockwise(uv_coords);
        bool xy_cw = is_clockwise(xy_coords);
        bool mirroring = !(uv_cw == xy_cw);

        double dx = xy_coords[1].x - xy_coords[0].x;
        double dy = xy_coords[1].y - xy_coords[0].y;
        double du = uv_coords[1].x - uv_coords[0].x;
        double dv = uv_coords[1].y - uv_coords[0].y;

        double angle_xy = Fmod(RadToDeg(std::atan2(dy, dx) + 360.0), 360.0);
        double angle_uv = Fmod(RadToDeg(std::atan2(dv, du) + 360.0), 360.0);

        double angle = Fmod(angle_xy - angle_uv + 360.0, 360.0);

        if (mirroring)
            angle -= 180.0;
        if (nearest)
            angle = Round0(angle / 90.0) * 90.0;

        return {angle, mirroring};
    }

    ShapeBitmapCommand::MatrixResult ShapeBitmapCommand::GetMatrix(const std::vector<Point2>* custom_uv_coords, bool use_nearest) const {
        std::vector<Point2> uv = (custom_uv_coords && !custom_uv_coords->empty()) ? *custom_uv_coords : uv_coords;

        double rotation = 0.0;
        bool mirroring = false;

        if (use_nearest) {
            auto rr = GetRotation(use_nearest);
            rotation = rr.first;
            mirroring = rr.second;

            double rad = DegToRad(rotation);
            double cs = std::cos(rad), sn = std::sin(rad);

            std::vector<Point2> rotated;
            rotated.reserve(uv_coords.size());
            for (auto& p : uv_coords) {
                double rx = cs * p.x - sn * p.y;
                double ry = sn * p.x + cs * p.y;
                rotated.push_back({static_cast<double>(Round0(rx)), static_cast<double>(Round0(ry))});
            }
            if (mirroring) {
                for (auto& p : rotated)
                    p.x = -p.x;
            }
            uv = rotated;
        }

        std::vector<Point2> sprite_box;
        sprite_box.reserve(uv.size());
        for (size_t idx = 0; idx < uv.size(); ++idx) {
            if (idx == 0) {
                sprite_box.push_back({0.0, 0.0});
            } else {
                double x_distance = uv[idx].x - uv[idx - 1].x;
                double y_distance = uv[idx].y - uv[idx - 1].y;
                sprite_box.push_back({Round(sprite_box[idx - 1].x + x_distance, 3), Round(sprite_box[idx - 1].y + y_distance, 3)});
            }
            if (sprite_box[idx].x < 0.0) {
                double off = sprite_box[idx].x;
                for (auto& p : sprite_box)
                    p.x -= off;
            }
            if (sprite_box[idx].y < 0.0) {
                double off = sprite_box[idx].y;
                for (auto& p : sprite_box)
                    p.y -= off;
            }
        }

        Point2 size = GetSize(uv);
        if (size.x == 0.0 || size.y == 0.0) {
            sprite_box = GetRightUv(false, sprite_box);
        }

        MatrixResult result;
        result.sprite_box = sprite_box;
        result.rotation = rotation;
        result.mirroring = mirroring;
        try {
            result.transform = AffineEstimate(sprite_box, xy_coords);
        } catch (const std::domain_error&) {
            result.transform = AffineTransform {};
        }
        return result;
    }

    RgbaImage ShapeBitmapCommand::GetImage(const RgbaImage& texture_image) const {
        Point2 size = GetSize(uv_coords);
        long w = static_cast<long>(size.x);
        long h = static_cast<long>(size.y);
        if (w == 0)
            w = 1;
        if (h == 0)
            h = 1;

        auto sample = [&](long x, long y) -> std::array<uint8_t, 4> {
            if (x < 0 || y < 0 || x >= texture_image.width || y >= texture_image.height)
                return {0, 0, 0, 0};
            size_t off = (static_cast<size_t>(y) * texture_image.width + x) * 4;
            return {texture_image.pixels[off], texture_image.pixels[off + 1], texture_image.pixels[off + 2], texture_image.pixels[off + 3]};
        };

        if (w + h == 2) {
            auto px = sample(static_cast<long>(uv_coords[0].x), static_cast<long>(uv_coords[0].y));
            RgbaImage img;
            img.width = 1;
            img.height = 1;
            img.pixels = {px[0], px[1], px[2], px[3]};
            return img;
        }

        double left = uv_coords[0].x, top = uv_coords[0].y, right = uv_coords[0].x, bottom = uv_coords[0].y;
        for (auto& c : uv_coords) {
            left = std::min(left, c.x);
            top = std::min(top, c.y);
            right = std::max(right, c.x);
            bottom = std::max(bottom, c.y);
        }
        if (w == 1)
            right += 1;
        if (h == 1)
            bottom += 1;

        long bl = static_cast<long>(left), bt = static_cast<long>(top);
        long br = static_cast<long>(right), bb = static_cast<long>(bottom);
        long bw = std::max<long>(1, br - bl);
        long bh = std::max<long>(1, bb - bt);

        std::vector<uint8_t> mask(static_cast<size_t>(bw) * bh, 0);
        std::vector<double> xs;
        const size_t n = uv_coords.size();
        for (long y = 0; y < bh; ++y) {
            double py_center = (bt + y) + 0.5;
            xs.clear();
            for (size_t i = 0; i < n; ++i) {
                const Point2& a = uv_coords[i];
                const Point2& b = uv_coords[(i + 1) % n];
                if (a.y == b.y)
                    continue;
                double y0 = a.y, y1 = b.y, x0 = a.x, x1 = b.x;
                if ((py_center >= std::min(y0, y1)) && (py_center < std::max(y0, y1))) {
                    double t = (py_center - y0) / (y1 - y0);
                    xs.push_back(x0 + t * (x1 - x0));
                }
            }
            std::sort(xs.begin(), xs.end());
            for (size_t i = 0; i + 1 < xs.size(); i += 2) {
                long x_start = static_cast<long>(std::ceil(xs[i] - bl - 0.5));
                long x_end = static_cast<long>(std::ceil(xs[i + 1] - bl - 0.5));
                x_start = std::max<long>(0, x_start);
                x_end = std::min<long>(bw, x_end);
                for (long x = x_start; x < x_end; ++x)
                    mask[static_cast<size_t>(y) * bw + x] = 255;
            }
        }

        RgbaImage sprite;
        sprite.width = static_cast<int>(w);
        sprite.height = static_cast<int>(h);
        sprite.pixels.assign(static_cast<size_t>(w) * h * 4, 0);

        for (long y = 0; y < h && y < bh; ++y) {
            for (long x = 0; x < w && x < bw; ++x) {
                if (!mask[static_cast<size_t>(y) * bw + x])
                    continue;
                auto px = sample(bl + x, bt + y);
                size_t off = (static_cast<size_t>(y) * w + x) * 4;
                sprite.pixels[off] = px[0];
                sprite.pixels[off + 1] = px[1];
                sprite.pixels[off + 2] = px[2];
                sprite.pixels[off + 3] = px[3];
            }
        }
        return sprite;
    }
}
