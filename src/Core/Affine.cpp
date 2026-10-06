#include "Core/Affine.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace sc2fla {

    namespace {

        AffineTransform EstimateFull(const std::vector<Point2>& origin, const std::vector<Point2>& convert) {
            size_t N = std::min(origin.size(), convert.size());

            double mat00 = 0, mat11 = 0, mat22 = 0, mat01 = 0, mat10 = 0, mat02 = 0, mat20 = 0, mat12 = 0, mat21 = 0;
            double vec0 = 0, vec1 = 0, vec2 = 0, vec3 = 0, vec4 = 0, vec5 = 0;

            for (size_t i = 0; i < N; ++i) {
                double ox = origin[i].x, oy = origin[i].y;
                double cx = convert[i].x, cy = convert[i].y;

                mat00 += ox * ox;
                mat01 += ox * oy;
                mat02 += ox;
                mat10 += ox * oy;
                mat11 += oy * oy;
                mat12 += oy;
                mat20 += ox;
                mat21 += oy;
                mat22 += 1;

                vec0 += ox * cx;
                vec1 += oy * cx;
                vec2 += cx;
                vec3 += ox * cy;
                vec4 += oy * cy;
                vec5 += cy;
            }

            double det = 0.0;
            det += (mat00 * mat11 * mat22 + mat10 * mat21 * mat02 + mat20 * mat01 * mat12);
            det -= (mat00 * mat21 * mat12 + mat20 * mat11 * mat02 + mat10 * mat01 * mat22);

            if (std::fabs(det) < 1e-8) {
                throw std::domain_error("Value of determinant is almost zero.");
            }

            double inv_det = 1.0 / det;

            double inv_mat00 = inv_det * (mat11 * mat22 - mat12 * mat21);
            double inv_mat01 = inv_det * (mat12 * mat20 - mat10 * mat22);
            double inv_mat02 = inv_det * (mat10 * mat21 - mat11 * mat20);
            double inv_mat10 = inv_mat01;
            double inv_mat11 = inv_det * (mat22 * mat00 - mat20 * mat02);
            double inv_mat12 = inv_det * (mat20 * mat01 - mat21 * mat00);
            double inv_mat20 = inv_mat02;
            double inv_mat21 = inv_mat12;
            double inv_mat22 = inv_det * (mat00 * mat11 - mat01 * mat10);

            AffineTransform t;
            t.params[0] = inv_mat00 * vec0 + inv_mat01 * vec1 + inv_mat02 * vec2;
            t.params[1] = inv_mat10 * vec0 + inv_mat11 * vec1 + inv_mat12 * vec2;
            t.params[2] = inv_mat00 * vec3 + inv_mat01 * vec4 + inv_mat02 * vec5;
            t.params[3] = inv_mat10 * vec3 + inv_mat11 * vec4 + inv_mat12 * vec5;
            t.params[4] = inv_mat20 * vec0 + inv_mat21 * vec1 + inv_mat22 * vec2;
            t.params[5] = inv_mat20 * vec3 + inv_mat21 * vec4 + inv_mat22 * vec5;
            return t;
        }

        AffineTransform EstimateHelmert(const std::vector<Point2>& origin, const std::vector<Point2>& convert) {
            size_t N = std::min(origin.size(), convert.size());

            double x0 = 0, x1 = 0, y0 = 0, y1 = 0, x02 = 0, y02 = 0, x0x1 = 0, y0y1 = 0, y0x1 = 0, x0y1 = 0;

            for (size_t i = 0; i < N; ++i) {
                double ox = origin[i].x, oy = origin[i].y;
                double cx = convert[i].x, cy = convert[i].y;

                x0 += ox;
                y0 += oy;
                x1 += cx;
                y1 += cy;

                x02 += ox * ox;
                y02 += oy * oy;
                x0x1 += ox * cx;
                x0y1 += ox * cy;
                y0x1 += oy * cx;
                y0y1 += oy * cy;
            }

            double det = x0 * x0 + y0 * y0 - static_cast<double>(N) * (x02 + y02);

            if (std::fabs(det) < 1e-8) {
                throw std::domain_error("Value of determinant is almost zero.");
            }

            double inv_det = 1.0 / det;

            AffineTransform t;
            t.params[0] = (x0 * x1 + y0 * y1 - N * (x0x1 + y0y1)) * inv_det;
            t.params[1] = (y0 * x1 - x0 * y1 - N * (y0x1 - x0y1)) * inv_det;
            t.params[2] = -t.params[1];
            t.params[3] = t.params[0];
            t.params[4] = (x1 - t.params[0] * x0 - t.params[1] * y0) / N;
            t.params[5] = (y1 - t.params[0] * y0 + t.params[1] * x0) / N;
            return t;
        }

    }

    AffineTransform AffineEstimate(const std::vector<Point2>& origin, const std::vector<Point2>& convert) {
        size_t N = std::min(origin.size(), convert.size());

        if (N >= 3) {
            return EstimateFull(origin, convert);
        } else if (N == 2) {
            return EstimateHelmert(origin, convert);
        } else if (N == 1) {
            AffineTransform t;
            t.params = {1.0, 0.0, 0.0, 1.0, convert[0].x - origin[0].x, convert[0].y - origin[0].y};
            return t;
        }
        throw std::invalid_argument("AffineEstimate: need at least 1 point pair");
    }

}
