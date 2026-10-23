#pragma once
#include "th08/online.hpp"

namespace th08::online::detail {
struct MovementDelta {
    float x, y;
};
// Only input and the immutable movement snapshot determine this displacement.
// Preserve each float operation and sign of zero before adding player position.
inline MovementDelta movement_delta(const Movement &s, std::uint16_t in) {
    int dx = 0, dy = 0;
    if ((in & 0x50) == 0x50)
        dx = -1, dy = -1;
    else if ((in & 0x60) == 0x60)
        dx = -1, dy = 1;
    else if ((in & 0x90) == 0x90)
        dx = 1, dy = -1;
    else if ((in & 0xA0) == 0xA0)
        dx = 1, dy = 1;
    else if (in & 0x20)
        dy = 1;
    else if (in & 0x10)
        dy = -1;
    else if (in & 0x40)
        dx = -1;
    else if (in & 0x80)
        dx = 1;
    const int f = (in & 4) ? 1 : 0;
    float h = 0, v = 0;
    if (dx && dy)
        h = dx * s.diag[f], v = dy * s.diag[f];
    else
        h = dx * s.axis[f], v = dy * s.axis[f];
    h *= s.hmul;
    v *= s.vmul;
    return {h * s.frm, v * s.frm};
}
inline void apply_movement(const Movement &s, float &x, float &y, MovementDelta delta) {
    x += delta.x;
    y += delta.y;
    if (x < s.minx)
        x = s.minx;
    else if (x > s.maxx)
        x = s.maxx;
    if (y < s.miny)
        y = s.miny;
    else if (y > s.maxy)
        y = s.maxy;
}
} // namespace th08::online::detail
