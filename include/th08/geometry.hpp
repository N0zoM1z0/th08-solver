#pragma once

// Lethal geometry only; the world adapter resolves collision gates beforehand.
// Source: th08 a45e99f, Player.cpp CheckBulletCollision / CalcLaserHitbox.
// Full box sizes, player half sizes, inclusive contact. No x87 equivalence claim.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace th08::geometry {
struct Vec2 {
    float x = 0, y = 0;
};
struct Box {
    Vec2 center, size;
};
struct Laser {
    Box box;
    Vec2 origin;
    float angle = 0;
};
inline bool finite(Vec2 v) {
    return std::isfinite(v.x) && std::isfinite(v.y);
}
// Bounds keep float arithmetic and grid conversion away from overflow. These
// limits are a solver API contract, not a claim about the engine's full domain.
inline void coordinate(Vec2 v) {
    if (!finite(v) || std::abs(v.x) > 1e6f || std::abs(v.y) > 1e6f)
        throw std::invalid_argument("coordinate outside supported finite domain");
}
inline void size(Vec2 v) {
    coordinate(v);
    if (v.x < 0 || v.y < 0)
        throw std::invalid_argument("negative size");
}
inline Vec2 rotate(Vec2 p, float angle) {
    const float s = std::sin(angle), c = std::cos(angle);
    return {c * p.x - s * p.y, s * p.x + c * p.y};
}
// Unchecked narrow predicates: use validated snapshots at public query boundaries.
inline bool box_hit(Vec2 p, Vec2 half, const Box &box) {
    return !(p.x - half.x > box.size.x / 2 + box.center.x ||
             p.y - half.y > box.size.y / 2 + box.center.y ||
             p.x + half.x < box.center.x - box.size.x / 2 ||
             p.y + half.y < box.center.y - box.size.y / 2);
}
inline bool laser_hit(Vec2 p, Vec2 half, const Laser &laser) {
    Vec2 q = rotate({p.x - laser.origin.x, p.y - laser.origin.y}, -laser.angle);
    q = {q.x + laser.origin.x, q.y + laser.origin.y};
    return box_hit(q, half, laser.box); // Rotate center only, not player extents.
}
struct Hazard {
    enum class Kind { bullet, laser } kind = Kind::bullet;
    Box box{};
    Laser beam{};
    static Hazard bullet(Box b) {
        Hazard h;
        h.box = b;
        return h;
    }
    static Hazard laser(Laser b) {
        Hazard h;
        h.kind = Kind::laser;
        h.beam = b;
        return h;
    }
};
inline void validate(const Hazard &h) {
    if (h.kind != Hazard::Kind::bullet && h.kind != Hazard::Kind::laser)
        throw std::invalid_argument("unknown hazard kind");
    const Box &b = h.kind == Hazard::Kind::bullet ? h.box : h.beam.box;
    coordinate(b.center);
    if (h.kind == Hazard::Kind::bullet)
        size(b.size);
    else
        coordinate(b.size); // Source laser ramps can produce signed terminal dimensions.
    if (h.kind == Hazard::Kind::laser) {
        coordinate(h.beam.origin);
        if (!std::isfinite(h.beam.angle) || std::abs(h.beam.angle) > 16)
            throw std::invalid_argument("laser angle outside supported domain");
    }
}
inline bool hit(Vec2 p, Vec2 half, const Hazard &h) {
    return h.kind == Hazard::Kind::bullet ? box_hit(p, half, h.box) : laser_hit(p, half, h.beam);
}
struct Bounds {
    Vec2 lo, hi;
};
inline Bounds expanded(const Hazard &h, Vec2 half) {
    const Box &b = h.kind == Hazard::Kind::bullet ? h.box : h.beam.box;
    float scale = 1 + half.x + half.y + std::abs(b.center.x) + std::abs(b.center.y) +
                  std::abs(b.size.x) + std::abs(b.size.y);
    if (h.kind == Hazard::Kind::laser)
        scale += std::abs(h.beam.origin.x) + std::abs(h.beam.origin.y);
    // Broad-phase allowance only. Narrow-phase predicate is unchanged.
    const float pad = .002f + 32 * std::numeric_limits<float>::epsilon() * scale;
    Vec2 r{std::max(0.0f, b.size.x / 2 + half.x) + pad,
           std::max(0.0f, b.size.y / 2 + half.y) + pad},
        c = b.center;
    if (h.kind == Hazard::Kind::laser) {
        c = rotate({c.x - h.beam.origin.x, c.y - h.beam.origin.y}, h.beam.angle);
        c = {c.x + h.beam.origin.x, c.y + h.beam.origin.y};
        const float s = std::abs(std::sin(h.beam.angle)), co = std::abs(std::cos(h.beam.angle));
        r = {co * r.x + s * r.y + pad, s * r.x + co * r.y + pad};
    }
    return {{c.x - r.x, c.y - r.y}, {c.x + r.x, c.y + r.y}};
}

// Immutable owned snapshot. Copy/move, temporary inputs and caller mutation do
// not invalidate buckets. Construct once per shared collision phase.
class Snapshot {
    static constexpr int columns = 24, rows = 28;
    std::vector<Hazard> hazards_;
    Vec2 half_;
    // CSR buckets: one contiguous reference array instead of 672 small vectors.
    std::array<std::size_t, columns * rows + 1> offsets_{};
    std::vector<std::size_t> references_;
    std::vector<std::size_t> large_;
    bool scan_unchecked(Vec2 p, std::uint64_t *checks) const {
        for (const auto &h : hazards_) {
            if (checks)
                ++*checks;
            if (hit(p, half_, h))
                return true;
        }
        return false;
    }

  public:
    explicit Snapshot(std::vector<Hazard> hazards, Vec2 half)
        : hazards_(std::move(hazards)), half_(half) {
        size(half_); // Validate even an empty world.
        struct CellRange {
            std::size_t hazard;
            int x0, x1, y0, y1;
        };
        std::vector<CellRange> ranges;
        ranges.reserve(hazards_.size());
        for (std::size_t i = 0; i < hazards_.size(); ++i) {
            validate(hazards_[i]);
            const auto b = expanded(hazards_[i], half_);
            if (b.hi.x < 0 || b.hi.y < 0 || b.lo.x > 384 || b.lo.y > 448)
                continue;
            const auto cell = [](float x, int n) {
                return int(std::clamp(std::floor(x / 16), 0.f, float(n - 1)));
            };
            const int x0 = cell(b.lo.x, columns), x1 = cell(b.hi.x, columns);
            const int y0 = cell(b.lo.y, rows), y1 = cell(b.hi.y, rows);
            if ((x1 - x0 + 1) * (y1 - y0 + 1) > 64) {
                large_.push_back(i);
                continue;
            }
            ranges.push_back({i, x0, x1, y0, y1});
            for (int y = y0; y <= y1; ++y)
                for (int x = x0; x <= x1; ++x)
                    ++offsets_[y * columns + x + 1];
        }
        for (std::size_t i = 1; i < offsets_.size(); ++i)
            offsets_[i] += offsets_[i - 1];
        references_.resize(offsets_.back());
        auto cursor = offsets_;
        for (const auto &range : ranges) {
            for (int y = range.y0; y <= range.y1; ++y)
                for (int x = range.x0; x <= range.x1; ++x)
                    references_[cursor[y * columns + x]++] = range.hazard;
        }
    }
    bool scan(Vec2 p, std::uint64_t *checks = nullptr) const {
        coordinate(p);
        return scan_unchecked(p, checks);
    }
    bool query(Vec2 p, std::uint64_t *checks = nullptr) const {
        coordinate(p); // Reject NaN before conversion to a bucket index.
        if (p.x < 0 || p.y < 0 || p.x > 384 || p.y > 448)
            return scan_unchecked(p, checks);
        const int x = std::min(int(p.x / 16), columns - 1), y = std::min(int(p.y / 16), rows - 1);
        const auto begin = offsets_[y * columns + x], end = offsets_[y * columns + x + 1];
        if (end - begin + large_.size() > hazards_.size() * 3 / 4)
            return scan_unchecked(p, checks);
        for (auto i : large_) {
            if (checks)
                ++*checks;
            if (hit(p, half_, hazards_[i]))
                return true;
        }
        for (auto slot = begin; slot < end; ++slot) {
            if (checks)
                ++*checks;
            if (hit(p, half_, hazards_[references_[slot]]))
                return true;
        }
        return false;
    }
};
} // namespace th08::geometry
