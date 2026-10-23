#pragma once
#include "th08/online.hpp"

namespace th08::online::detail {
// Forecast queries use validated coordinates. Evaluate all four original
// rejection comparisons to avoid data-dependent branches; contact stays inclusive.
inline bool box_hit(const Box &box, const Vector &minimum, const Vector &maximum) {
    return !((minimum.x > box.x1) | (minimum.y > box.y1) | (maximum.x < box.x0) |
             (maximum.y < box.y0));
}
} // namespace th08::online::detail
