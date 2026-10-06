#pragma once

#include <cstdint>

namespace sc2fla {
    double RecoverTwip(float value);
    double RecoverMatrixComponentAuto(float value);
    uint32_t RecoverU16FromNormalized(float value);
}
