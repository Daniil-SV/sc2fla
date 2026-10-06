#include "Core/Math.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace sc2fla {
    long long Round0(double x) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.0f", x);
        return std::atoll(buf);
    }

    double Round(double x, int digits) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.*f", digits, x);
        return std::strtod(buf, nullptr);
    }

    long long Ceil(double x) {
        return static_cast<long long>(std::ceil(x));
    }

    double Fmod(double a, double b) {
        double r = std::fmod(a, b);
        if (r != 0.0 && ((r < 0.0) != (b < 0.0)))
            r += b;
        return r;
    }
}
