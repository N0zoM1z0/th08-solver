#pragma once
#include "th08/online.hpp"

#include <cmath>

namespace th08::online::detail {
struct Rotation2 {
    float sine = 0, cosine = 1;
    Vector apply(const Vector &point) const {
        return {cosine * point.x - sine * point.y, cosine * point.y + sine * point.x, 0};
    }
};
// The angle is in radians. Call after a forecast laser's geometry is final;
// immutable coefficients can then be shared by all queries of that frame.
inline Rotation2 prepare_rotation(float angle) {
    return {std::sin(angle), std::cos(angle)};
}
} // namespace th08::online::detail
