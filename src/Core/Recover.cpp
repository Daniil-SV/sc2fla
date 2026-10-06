#include "Core/Recover.h"

#include <cmath>
#include <initializer_list>

namespace sc2fla {
    double RecoverTwip(float value) {
        double d = static_cast<double>(value);
        double raw = std::llround(d / 0.05);
        return raw / 20.0;
    }

    double RecoverMatrixComponentAuto(float value) {
        for (double divider : {65535.0, 1024.0}) {
            long long raw = std::llround(static_cast<double>(value) * divider);
            float recon = static_cast<float>(static_cast<float>(raw) / static_cast<float>(divider));
            if (recon == value)
                return raw / divider;
        }
        long long raw = std::llround(static_cast<double>(value) * 65535.0);
        return raw / 65535.0;
    }

    uint32_t RecoverU16FromNormalized(float value) {
        double d = static_cast<double>(value);
        long long raw = std::llround(d * 65535.0);
        if (raw < 0)
            raw = 0;
        if (raw > 65535)
            raw = 65535;
        return static_cast<uint32_t>(raw);
    }
}
