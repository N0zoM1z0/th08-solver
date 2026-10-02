#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace th08::policy {
// A proposal heuristic for a native-runtime baseline, not a collision oracle.
// It extrapolates observed bullet velocities over 12 frames; the actual game
// executes every action and establishes its outcome, including new emissions,
// transforms, lasers, enemies, graze feedback and RNG changes.
template <class Bullets>
std::uint16_t reactive(float px, float py, float axis, float diagonal, const Bullets &bullets) {
    double best = 1e100;
    std::uint16_t chosen = 4;
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x) {
            const float speed = x && y ? diagonal : axis;
            const float nx = std::clamp(px + x * speed, 8.f, 376.f);
            const float ny = std::clamp(py + y * speed, 16.f, 432.f);
            const float vx = nx - px, vy = ny - py;
            double danger = ((nx - 192) * (nx - 192) + (ny - 380) * (ny - 380)) * 1e-9;
            for (const auto &b : bullets) {
                if (b.state == 5 || b.state == 6)
                    continue;
                const double rx = b.x - px, ry = b.y - py;
                if (rx * rx + ry * ry > 180 * 180)
                    continue;
                const double rvx = b.vx - vx, rvy = b.vy - vy;
                const double vv = rvx * rvx + rvy * rvy;
                const double t = vv > 0 ? std::clamp(-(rx * rvx + ry * rvy) / vv, 1., 12.) : 1;
                const double dx = rx + t * rvx, dy = ry + t * rvy;
                const double d = dx * dx + dy * dy + 1;
                danger += 1 / (d * d);
            }
            if (danger < best) {
                best = danger;
                chosen = 4 | (x < 0 ? 64 : x > 0 ? 128 : 0) | (y < 0 ? 16 : y > 0 ? 32 : 0);
            }
        }
    return chosen;
}
} // namespace th08::policy
