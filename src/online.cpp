// Adapted from the reviewed October 22 online-controller prototype. Native
// current-memory fields are copied by the adapter; this policy owns only its
// observation history, immutable risk model and heuristic proposal search.
#include "th08/online.hpp"
#include "th08/plan_beam.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace th08::online {
namespace {
using Float3 = Vector;
using Speeds = Movement;
struct Laser {
    float cx, cy, w, h, ox, oy, angle;
    // Proposal uncertainty only: all turn signs up to the last observed speed.
    // This does not certify that future scripts preserve that speed or origin.
    double angle_reach = 0, reach_cos = 1, reach_sin = 0;
};
void rotate(Float3 *out, const Float3 *point, float angle) {
    const float sine = std::sin(angle), cosine = std::cos(angle);
    out->x = cosine * point->x - sine * point->y;
    out->y = cosine * point->y + sine * point->x;
}
// Player::UpdateMovementAndOptions direction priority and float operation order.
inline void move(const Speeds &s, float &x, float &y, std::uint16_t in) {
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
    x += h * s.frm;
    y += v * s.frm;
    if (x < s.minx)
        x = s.minx;
    else if (x > s.maxx)
        x = s.maxx;
    if (y < s.miny)
        y = s.miny;
    else if (y > s.maxy)
        y = s.maxy;
}

struct HFrame {
    std::vector<Box> b; // bullets and lethal regions
    std::vector<Laser> l;
    std::vector<int> start, items; // CSR grid over b
    std::vector<Box> risk;         // learned uncertainty regions (soft cost)
    std::vector<int> rstart, ritems;
    std::vector<Box> laabb; // world-space AABB of each laser (index parallel to l)
    std::vector<int> lstart, litems;
};
constexpr float kCell = 16.f, kGx0 = -64.f, kGy0 = -64.f;
constexpr int kGW = 34, kGH = 38;
constexpr float kMargin = 1.5f;
inline int cell_of(float v, float origin, int n) {
    const float c = std::floor((v - origin) / kCell);
    return c <= 0 ? 0 : c >= n - 1 ? n - 1 : int(c);
}
void build_grid(const std::vector<Box> &boxes, const Float3 &half, std::vector<int> &start,
                std::vector<int> &items) {
    const float ex = half.x + kMargin + 0.01f, ey = half.y + kMargin + 0.01f;
    std::vector<int> cnt(kGW * kGH + 1, 0);
    auto each = [&](const Box &b, auto &&fn) {
        const int cx0 = cell_of(b.x0 - ex, kGx0, kGW), cx1 = cell_of(b.x1 + ex, kGx0, kGW);
        const int cy0 = cell_of(b.y0 - ey, kGy0, kGH), cy1 = cell_of(b.y1 + ey, kGy0, kGH);
        for (int cy = cy0; cy <= cy1; ++cy)
            for (int cx = cx0; cx <= cx1; ++cx)
                fn(cy * kGW + cx);
    };
    for (const auto &b : boxes)
        each(b, [&](int c) { ++cnt[c + 1]; });
    for (int c = 0; c < kGW * kGH; ++c)
        cnt[c + 1] += cnt[c];
    start = cnt;
    items.assign(cnt[kGW * kGH], 0);
    std::vector<int> pos(cnt.begin(), cnt.end() - 1);
    for (int i = 0; i < int(boxes.size()); ++i)
        each(boxes[i], [&](int c) { items[pos[c]++] = i; });
}
Box laser_aabb(const Laser &l, float padding = 0) {
    if (l.angle_reach > 0) {
        // The query rotates the player's center, then expands in local axes.
        // A full radial bound includes that rotated hurtbox and every turn sign.
        const double round = 16 * std::numeric_limits<float>::epsilon() *
                             (1 + 2 * 1048576.0 + std::abs(double(l.ox)) + std::abs(double(l.oy)) +
                              std::abs(double(l.cx)) + std::abs(double(l.cy)));
        const double x = std::abs(double(l.cx) - l.ox) + l.w / 2 + padding + round;
        const double y = std::abs(double(l.cy) - l.oy) + l.h / 2 + padding + round;
        const float radius =
            std::nextafter(float(std::hypot(x, y) + 1), std::numeric_limits<float>::infinity());
        return {l.ox - radius, l.oy - radius, l.ox + radius, l.oy + radius};
    }
    const float c = std::cos(l.angle), s = std::sin(l.angle);
    const float xs[2] = {l.cx - l.w / 2, l.cx + l.w / 2}, ys[2] = {l.cy - l.h / 2, l.cy + l.h / 2};
    Box b{1e9f, 1e9f, -1e9f, -1e9f};
    for (float qx : xs)
        for (float qy : ys) {
            const float rx = qx - l.ox, ry = qy - l.oy;
            const float wx = rx * c - ry * s + l.ox, wy = rx * s + ry * c + l.oy;
            b.x0 = std::min(b.x0, wx), b.x1 = std::max(b.x1, wx);
            b.y0 = std::min(b.y0, wy), b.y1 = std::max(b.y1, wy);
        }
    // Conservative padding for the game's own Rotate rounding.
    b.x0 -= 1, b.y0 -= 1, b.x1 += 1, b.y1 += 1;
    return b;
}
void build_index(HFrame &f, const Float3 &half) {
    build_grid(f.risk, half, f.rstart, f.ritems);
    f.laabb.resize(f.l.size());
    for (std::size_t i = 0; i < f.l.size(); ++i)
        f.laabb[i] = laser_aabb(f.l[i], std::max(half.x, half.y) + kMargin);
    build_grid(f.laabb, half, f.lstart, f.litems);
    const float ex = half.x + kMargin + 0.01f, ey = half.y + kMargin + 0.01f;
    std::vector<int> cnt(kGW * kGH + 1, 0);
    auto each = [&](const Box &b, auto &&fn) {
        const int cx0 = cell_of(b.x0 - ex, kGx0, kGW), cx1 = cell_of(b.x1 + ex, kGx0, kGW);
        const int cy0 = cell_of(b.y0 - ey, kGy0, kGH), cy1 = cell_of(b.y1 + ey, kGy0, kGH);
        for (int cy = cy0; cy <= cy1; ++cy)
            for (int cx = cx0; cx <= cx1; ++cx)
                fn(cy * kGW + cx);
    };
    for (const auto &b : f.b)
        each(b, [&](int c) { ++cnt[c + 1]; });
    for (int c = 0; c < kGW * kGH; ++c)
        cnt[c + 1] += cnt[c];
    f.start = cnt;
    f.items.assign(cnt[kGW * kGH], 0);
    std::vector<int> pos(cnt.begin(), cnt.end() - 1);
    for (int i = 0; i < int(f.b.size()); ++i)
        each(f.b[i], [&](int c) { f.items[pos[c]++] = i; });
}
inline bool box_hit(const Box &b, const Float3 &mn, const Float3 &mx) {
    return !(mn.x > b.x1 || mn.y > b.y1 || mx.x < b.x0 || mx.y < b.y0);
}
inline bool laser_hit(const Laser &l, const Float3 &p, const Float3 &half, float margin) {
    Float3 origin, position, size, in_min, in_max, pmin, pmax;
    origin.x = l.ox, origin.y = l.oy, origin.z = 0;
    position.x = l.cx, position.y = l.cy, position.z = 0;
    size.x = l.w, size.y = l.h, size.z = 0;
    in_min = p - origin;
    rotate(&in_max, &in_min, -l.angle);
    if (l.angle_reach > 0) {
        // A circle arc intersects a rectangle either at an arc endpoint or a
        // rectangle edge. Dot products test arc membership without atan2.
        // Outward padding covers float Rotate/store rounding between poses.
        const double scale = 1 + std::abs(double(p.x)) + std::abs(double(p.y)) +
                             std::abs(double(l.ox)) + std::abs(double(l.oy)) +
                             std::abs(double(l.cx)) + std::abs(double(l.cy));
        const double round = 16 * std::numeric_limits<float>::epsilon() * scale;
        const double ex = half.x + margin + round, ey = half.y + margin + round;
        const double x0 = double(l.cx) - l.w / 2 - l.ox - ex;
        const double x1 = double(l.cx) + l.w / 2 - l.ox + ex;
        const double y0 = double(l.cy) - l.h / 2 - l.oy - ey;
        const double y1 = double(l.cy) + l.h / 2 - l.oy + ey;
        const double u = in_max.x, v = in_max.y, radius2 = u * u + v * v;
        const double arc_round = round * (1 + std::sqrt(radius2));
        const auto inside = [&](double x, double y) {
            return x >= x0 && x <= x1 && y >= y0 && y <= y1;
        };
        if (inside(u * l.reach_cos - v * l.reach_sin, v * l.reach_cos + u * l.reach_sin) ||
            inside(u * l.reach_cos + v * l.reach_sin, v * l.reach_cos - u * l.reach_sin))
            return true;
        const auto in_arc = [&](double x, double y) {
            return u * x + v * y >= radius2 * l.reach_cos - arc_round;
        };
        for (double x : {x0, x1})
            if (x * x <= radius2) {
                const double y = std::sqrt(std::max(0.0, radius2 - x * x));
                if ((y >= y0 && y <= y1 && in_arc(x, y)) || (-y >= y0 && -y <= y1 && in_arc(x, -y)))
                    return true;
            }
        for (double y : {y0, y1})
            if (y * y <= radius2) {
                const double x = std::sqrt(std::max(0.0, radius2 - y * y));
                if ((x >= x0 && x <= x1 && in_arc(x, y)) || (-x >= x0 && -x <= x1 && in_arc(-x, y)))
                    return true;
            }
        return false;
    }
    in_max.z = 0.0f;
    in_min = in_max + origin;
    pmin = in_min - half;
    pmax = in_min + half;
    in_min = position - size / 2.0f;
    in_max = position + size / 2.0f;
    return !(pmin.x - margin > in_max.x) && !(pmax.x + margin < in_min.x) &&
           !(pmin.y - margin > in_max.y) && !(pmax.y + margin < in_min.y);
}
inline bool probe_risk(const HFrame &f, const Float3 &half, float x, float y) {
    if (f.risk.empty())
        return false;
    Float3 p;
    p.x = x, p.y = y, p.z = 0;
    const Float3 mn = p - half, mx = p + half;
    const int c = cell_of(y, kGy0, kGH) * kGW + cell_of(x, kGx0, kGW);
    for (int k = f.rstart[c]; k < f.rstart[c + 1]; ++k)
        if (box_hit(f.risk[f.ritems[k]], mn, mx))
            return true;
    return false;
}
// 2 lethal, 1 margin-only near miss, 0 clear.
inline int probe(const HFrame &f, const Float3 &half, float x, float y, bool want_margin) {
    Float3 p;
    p.x = x, p.y = y, p.z = 0;
    const Float3 mn = p - half, mx = p + half;
    Float3 mmn = mn, mmx = mx;
    mmn.x -= kMargin, mmn.y -= kMargin, mmx.x += kMargin, mmx.y += kMargin;
    const int c = cell_of(y, kGy0, kGH) * kGW + cell_of(x, kGx0, kGW);
    int result = 0;
    for (int k = f.start[c]; k < f.start[c + 1]; ++k) {
        const Box &b = f.b[f.items[k]];
        if (box_hit(b, mn, mx))
            return 2;
        if (want_margin && !result && box_hit(b, mmn, mmx))
            result = 1;
    }
    for (int k = f.lstart[c]; k < f.lstart[c + 1]; ++k) {
        const Laser &l = f.l[f.litems[k]];
        if (laser_hit(l, p, half, 0))
            return 2;
        if (want_margin && !result && laser_hit(l, p, half, kMargin))
            result = 1;
    }
    return result;
}

} // namespace
struct Policy::Impl {
    Configuration opt;
    const RiskTables risk;
    Movement current_movement;
    std::vector<HFrame> frames;
    std::uint64_t last_observed = UINT64_MAX;
    std::uint64_t last_decided = UINT64_MAX;
    explicit Impl(Configuration configuration, RiskTables tables)
        : opt(configuration), risk(std::move(tables)) {
        if (opt.horizon < 2 || opt.horizon > 200 || opt.beam < 1 || opt.beam > 8192 ||
            !std::isfinite(opt.grow) || opt.grow < 0 || !std::isfinite(opt.boss_follow) ||
            !std::isfinite(opt.risk_weight) || opt.risk_weight < 0 ||
            !std::isfinite(opt.nowarn_weight) || opt.nowarn_weight < 0 ||
            !std::isfinite(opt.focus_bias) || !std::isfinite(opt.spawn_guard) ||
            opt.spawn_guard < 0 || opt.spawn_guard > 128 || opt.diversity_cell < 0 ||
            opt.diversity_cell > 128)
            throw std::runtime_error("invalid_online_configuration");
        frames.resize(opt.horizon + 1);
    }
    static constexpr int kSlots = 0x601;
    struct SlotMemory {
        bool active = false;
        long born = -1, spawn_seen = -1, stopped = -1;
        float x = 0, y = 0, vx = 0, vy = 0;
        std::uint16_t state = 0;
    };
    struct DirectCall {
        float cx, cy, w, h, ox, oy, angle;
    };
    struct Memory {
        std::array<SlotMemory, kSlots> slots{};
        std::vector<DirectCall> direct_prev;
        std::vector<Box> regions_prev;
    };
    Memory mem;
    std::vector<DirectCall> direct_current;
    std::vector<Box> regions_current;
    long now = 0; // committed native updates

    // Spawn-flash durations measured from native observations: (state, collision size)
    // determine the frames spent in the spawning state almost deterministically.
    int spawn_duration(int state, float size) {
        if (state == 2)
            return size <= 6.5f ? 9 : size >= 20.f ? 23 : 29;
        if (state == 3)
            return size <= 6.5f ? 14 : size >= 20.f ? 23 : 29;
        return 29;
    }

    void validate(const Observation &o) {
        // Explicit finite-coordinate profile keeps grid/quantization conversions
        // defined. Out-of-profile memory stops the case instead of losing hazards.
        auto finite = [](float v) {
            if (!std::isfinite(v) || std::fabs(v) > 1048576.f)
                throw std::runtime_error("unsupported_online_observation");
        };
        finite(o.x);
        finite(o.y);
        const auto &m = o.movement;
        for (float v : {m.axis[0], m.axis[1], m.diag[0], m.diag[1], m.hmul, m.vmul, m.frm, m.minx,
                        m.maxx, m.miny, m.maxy, m.half.x, m.half.y})
            finite(v);
        if (m.minx > m.maxx || m.miny > m.maxy || m.half.x < 0 || m.half.y < 0 || o.x < m.minx ||
            o.x > m.maxx || o.y < m.miny || o.y > m.maxy ||
            o.update > std::uint64_t(std::numeric_limits<long>::max() - 201))
            throw std::runtime_error("unsupported_online_observation");
        std::array<bool, kSlots> slots{};
        for (const auto &b : o.bullets) {
            if (b.slot < 0 || b.slot >= kSlots || slots[b.slot] || b.state == 0 || b.state > 5)
                throw std::runtime_error("unsupported_online_observation");
            slots[b.slot] = true;
            for (float v : {b.x, b.y, b.vx, b.vy, b.full_width, b.full_height,
                            b.vector_acceleration_x, b.vector_acceleration_y})
                finite(v);
            if (b.full_width < 0 || b.full_height < 0)
                throw std::runtime_error("unsupported_online_observation");
        }
        const auto validate_laser = [&](const LaserState &l) {
            for (float v :
                 {l.endOffset, l.startOffset, l.startLength, l.width, l.position.x, l.position.y,
                  l.angle, l.speed, l.origin_delta_x, l.origin_delta_y, l.angle_delta})
                finite(v);
            if (l.state < 0 || l.state > 2 || l.timer > std::numeric_limits<int>::max() - 201)
                throw std::runtime_error("unsupported_online_observation");
        };
        for (const auto &l : o.lasers)
            validate_laser(l);
        for (const auto &l : o.queued_pooled_lasers) {
            validate_laser(l);
            if (l.timer != 0 || l.state != (l.startTime == 0 ? 1 : 0) || l.startOffset < 0 ||
                l.endOffset < l.startOffset || l.startLength < 0 || l.width < 0 || l.speed < 0 ||
                l.startTime < 0 || l.hitboxStartTime < 0 || l.duration < 0 ||
                l.despawnDuration < 0 || l.hitboxEndDelay < 0 || l.motion_observed)
                throw std::runtime_error("unsupported_online_observation");
        }
        for (const auto &l : o.laser_hitboxes)
            for (float v : {l.cx, l.cy, l.w, l.h, l.ox, l.oy, l.angle})
                finite(v);
        for (const auto &b : o.lethal_regions) {
            for (float v : {b.x0, b.y0, b.x1, b.y1})
                finite(v);
            if (b.x0 > b.x1 || b.y0 > b.y1)
                throw std::runtime_error("unsupported_online_observation");
        }
        std::array<bool, 480> actors{};
        for (const auto &actor : o.actors) {
            if (actor.slot < 0 || actor.slot >= int(actors.size()) || actors[actor.slot])
                throw std::runtime_error("unsupported_online_observation");
            actors[actor.slot] = true;
            for (float v : {actor.origin_x, actor.origin_y, actor.delta_x, actor.delta_y})
                finite(v);
        }
        for (const auto &shot : o.queued_shots) {
            if (shot.update == 0 || shot.update > unsigned(opt.horizon) ||
                shot.linear_updates == 0 || shot.width < 0 || shot.height < 0)
                throw std::runtime_error("unsupported_online_observation");
            for (float v : {shot.x, shot.y, shot.vx, shot.vy, shot.width, shot.height})
                finite(v);
        }
        for (const auto &laser : o.queued_lasers) {
            const auto &l = laser.hitbox;
            if (laser.first_update == 0 || laser.last_update < laser.first_update ||
                laser.last_update > unsigned(opt.horizon) || l.w < 0 || l.h < 0 ||
                laser.turn_hint < 0)
                throw std::runtime_error("unsupported_online_observation");
            finite(laser.turn_hint);
            for (float v : {l.cx, l.cy, l.w, l.h, l.ox, l.oy, l.angle})
                finite(v);
        }
    }

    void observe(const Observation &observation) {
        validate(observation);
        if (last_observed == observation.update)
            return;
        if (last_observed != UINT64_MAX && observation.update != last_observed + 1)
            throw std::runtime_error("nonconsecutive_online_observation");
        prev_vel = current_vel;
        current_vel = {};
        mem.direct_prev = direct_current;
        direct_current.clear();
        for (const auto &c : observation.laser_hitboxes)
            if (c.pooled_slot < 0)
                direct_current.push_back({c.cx, c.cy, c.w, c.h, c.ox, c.oy, c.angle});
        mem.regions_prev = regions_current;
        regions_current = observation.lethal_regions;
        last_observed = observation.update;
        now = long(observation.update);
        const auto &bs = observation.bullets;
        std::array<bool, kSlots> seen{};
        for (const auto &b : bs) {
            if (b.slot < 0 || b.slot >= kSlots)
                continue;
            auto &m = mem.slots[b.slot];
            seen[b.slot] = true;
            bool fresh = !m.active;
            if (!fresh &&
                (std::fabs(b.x - (m.x + m.vx)) > 48.f || std::fabs(b.y - (m.y + m.vy)) > 48.f))
                fresh = true;
            if (fresh) {
                m = SlotMemory{};
                m.active = true;
                m.born = now;
                prev_vel[b.slot] = {};
            }
            if (b.state >= 2 && b.state <= 4 && m.spawn_seen < 0)
                m.spawn_seen = now;
            if (std::hypot(b.vx, b.vy) < 0.05f) {
                if (m.stopped < 0)
                    m.stopped = now;
            } else
                m.stopped = -1;
            m.x = b.x, m.y = b.y, m.vx = b.vx, m.vy = b.vy, m.state = b.state;
            current_vel[b.slot] = {true, b.vx, b.vy, b.state};
        }
        for (int s = 0; s < kSlots; ++s)
            if (!seen[s])
                mem.slots[s] = SlotMemory{};
    }

    inline int speed_bucket(float s) {
        return s < 0.05f ? 0 : s < 1.0f ? 1 : s < 3.0f ? 2 : 3;
    }
    inline int age_bucket(long a) {
        return a < 30 ? 0 : a < 60 ? 1 : a < 120 ? 2 : 3;
    }
    // Interpolated q99 margin for a bullet h updates ahead.
    inline float risk_margin(const std::array<float, 4> &q, int h) {
        if (h <= 4)
            return q[0] * float(h) / 4.f;
        if (h <= 8)
            return q[0] + (q[1] - q[0]) * float(h - 4) / 4.f;
        if (h <= 16)
            return q[1] + (q[2] - q[1]) * float(h - 8) / 8.f;
        if (h <= 32)
            return q[2] + (q[3] - q[2]) * float(h - 16) / 16.f;
        return q[3];
    }
    const std::array<float, 4> *lookup_risk(int seg, int sprite, int size10, int sb, int ab) {
        auto it = risk.fine.find(risk_key(seg, sprite, size10, sb, ab));
        if (it != risk.fine.end())
            return &it->second;
        it = risk.segment.find(risk_key(seg, 0, 0, sb, ab));
        if (it != risk.segment.end())
            return &it->second;
        it = risk.global.find(risk_key(0, 0, 0, sb, ab));
        if (it != risk.global.end())
            return &it->second;
        return nullptr;
    }
    int current_segment = 0;

    struct Prev {
        bool valid = false;
        float vx = 0, vy = 0;
        std::uint16_t state = 0;
    };
    std::array<Prev, kSlots> prev_vel{}; // previous-update velocity per slot for finite differences
    std::array<Prev, kSlots> current_vel{};

    void laser_forecast(const LaserState &src, float dox, float doy, float dang, int horizon,
                        std::vector<HFrame> &frames) {
        // Mirror BulletManager's laser update on a scalar copy (no native mutation).
        float endOffset = src.endOffset, startOffset = src.startOffset;
        float px = src.position.x, py = src.position.y, angle = src.angle;
        int timer = src.timer, state = src.state;
        for (int j = 1; j <= horizon; ++j) {
            px += dox, py += doy, angle += dang;
            endOffset += current_movement.frm * src.speed;
            if (endOffset - startOffset > src.startLength)
                startOffset = endOffset - src.startLength;
            if (startOffset < 0.0f)
                startOffset = 0.0f;
            float sx, sy = src.width / 2.0f;
            if (startOffset <= 0.0f)
                sx = endOffset - startOffset;
            else
                sx = (endOffset - startOffset) * 0.7f;
            const float cx = (endOffset - startOffset) / 2.0f + startOffset + px, cy = py;
            bool lethal = false, dead = false;
            float lw = sx;
            if (state == 0) { // STARTING
                if ((src.flags & 1) == 0) {
                    const int ramp = src.startTime > 30 ? 30 : src.startTime;
                    float cw = (src.startTime - ramp < timer)
                                   ? float(timer) * src.width / src.startTime
                                   : 1.2f;
                    lw = cw / 2.0f;
                }
                if (timer >= src.hitboxStartTime)
                    lethal = true;
                if (timer >= src.startTime) {
                    timer = 0;
                    state = 1;
                    lethal = true; // falls through into ACTIVE with the startup size
                }
            } else if (state == 1) {
                lethal = true;
                if (timer >= src.duration) {
                    timer = 0;
                    state = 2;
                    if (src.despawnDuration == 0)
                        dead = true;
                }
            } else if (state == 2) {
                if ((src.flags & 1) == 0 && src.despawnDuration > 0)
                    lw = (src.width - float(timer) * src.width / src.despawnDuration) / 2.0f;
                lethal = timer < src.hitboxEndDelay;
                if (timer >= src.despawnDuration)
                    dead = true;
            }
            if (lethal) // the native call precedes the lifetime check on the final update
                frames[j].l.push_back({cx, cy, lw, sy, px, py, angle});
            if (dead || startOffset >= 640.0f)
                break;
            ++timer;
        }
    }

    void kinematic_forecast(const Observation &observation, int horizon,
                            std::vector<HFrame> &frames) {
        for (int j = 1; j <= horizon; ++j)
            frames[j].b.clear(), frames[j].l.clear(), frames[j].risk.clear();
        const auto &bs = observation.bullets;
        for (const auto &b : bs) {
            if (b.slot < 0 || b.slot >= kSlots)
                continue;
            const auto &nb = b;
            const float w = b.full_width, h = b.full_height;
            const auto &m = mem.slots[b.slot];
            if (nb.collision_disabled) {
                // A visible phantom is harmless now but may become real (e.g. illusion
                // patterns): no hard hazard, only a learned soft-risk region.
                if (!(opt.risk_phantoms && risk.loaded && b.state == 1))
                    continue;
                const float s1 = std::hypot(b.vx, b.vy);
                const auto *rq =
                    lookup_risk(current_segment, int(nb.sprite), int(std::lround(w * 10)),
                                speed_bucket(s1) + 4, age_bucket(m.born >= 0 ? now - m.born : 0));
                float x = b.x, y = b.y;
                for (int j = 1; j <= horizon; ++j) {
                    x += b.vx, y += b.vy;
                    const float rm = std::max(2.f, rq ? std::min(12.f, risk_margin(*rq, j)) : 4.f);
                    frames[j].risk.push_back({x - w / 2.0f - rm, y - h / 2.0f - rm,
                                              w / 2.0f + x + rm, h / 2.0f + y + rm});
                }
                continue;
            }
            if (b.state == 1) {
                const Prev &p = prev_vel[b.slot];
                float x = b.x, y = b.y;
                float s1 = std::hypot(b.vx, b.vy), a1 = std::atan2(b.vy, b.vx), ds = 0, da = 0;
                const bool fd = p.valid && p.state == 1;
                if (fd) {
                    const float s0 = std::hypot(p.vx, p.vy), a0 = std::atan2(p.vy, p.vx);
                    ds = s1 - s0;
                    da = a1 - a0;
                    while (da > 3.14159265f)
                        da -= 6.2831853f;
                    while (da < -3.14159265f)
                        da += 6.2831853f;
                    if (s0 < 1e-4f || s1 < 1e-4f)
                        da = 0;
                    // A large turn between two updates is a discrete transform event, not
                    // a steady rate: do not extrapolate it.
                    if (std::fabs(da) > 0.2f)
                        da = 0, ds = 0;
                }
                float vx = b.vx, vy = b.vy;
                const bool acc = (b.active_transforms & 0x10u) && !fd;
                int acc_left = acc ? int(std::clamp(std::int64_t(b.vector_acceleration_duration) -
                                                        b.vector_acceleration_timer,
                                                    std::int64_t(0), std::int64_t(horizon)))
                                   : 0;
                float s = s1, a = a1;
                const std::array<float, 4> *rq = nullptr;
                if (risk.loaded)
                    rq = lookup_risk(current_segment, int(nb.sprite), int(std::lround(w * 10)),
                                     speed_bucket(s1),
                                     age_bucket((opt.risk_stop_age && s1 < 0.05f && m.stopped >= 0)
                                                    ? now - m.stopped
                                                    : (m.born >= 0 ? now - m.born : 0)));
                for (int j = 1; j <= horizon; ++j) {
                    if (fd) {
                        s += ds, a += da;
                        if (s < 0)
                            s = 0;
                        x += s * std::cos(a), y += s * std::sin(a);
                    } else {
                        if (acc_left > 0)
                            vx += b.vector_acceleration_x, vy += b.vector_acceleration_y,
                                --acc_left;
                        x += vx, y += vy;
                    }
                    const float g = opt.grow * j;
                    float rm = rq ? std::min(12.f, risk_margin(*rq, j)) : 0.f;
                    if (opt.risk_hard && rm > 1.5f) {
                        frames[j].b.push_back({x - w / 2.0f - rm, y - h / 2.0f - rm,
                                               w / 2.0f + x + rm, h / 2.0f + y + rm});
                        continue;
                    }
                    frames[j].b.push_back(
                        {x - w / 2.0f - g, y - h / 2.0f - g, w / 2.0f + x + g, h / 2.0f + y + g});
                    if (rm > 1.5f)
                        frames[j].risk.push_back({x - w / 2.0f - rm, y - h / 2.0f - rm,
                                                  w / 2.0f + x + rm, h / 2.0f + y + rm});
                }
            } else if (b.state >= 2 && b.state <= 4) {
                const float div = b.state == 2 ? 2.0f : b.state == 3 ? 2.5f : 3.0f;
                const long age = m.spawn_seen >= 0 ? now - m.spawn_seen : 0;
                const int remaining = int(std::max(0L, spawn_duration(b.state, w) - age - 1));
                float x = b.x, y = b.y;
                for (int j = 1; j <= horizon; ++j) {
                    if (j <= remaining) {
                        x += b.vx / div, y += b.vy / div;
                        continue; // not collidable while spawning
                    }
                    x += b.vx, y += b.vy;
                    const float g = opt.grow * j + 1.0f; // unknown transform at activation
                    frames[j].b.push_back(
                        {x - w / 2.0f - g, y - h / 2.0f - g, w / 2.0f + x + g, h / 2.0f + y + g});
                }
            }
        }
        // Pooled lasers: deterministic geometry/timer state machine from the visible laser.
        for (const auto &src : observation.lasers) {
            laser_forecast(src, src.motion_observed ? src.origin_delta_x : 0.f,
                           src.motion_observed ? src.origin_delta_y : 0.f,
                           src.motion_observed ? src.angle_delta : 0.f, horizon, frames);
        }
        // This copied spawn has not received its first laser callback yet.
        // Project from timer zero, with update one equal to the birth update.
        if (opt.queued_lasers)
            for (const auto &src : observation.queued_pooled_lasers)
                laser_forecast(src, 0, 0, 0, horizon, frames);
        // Direct (ECL) laser calls drawn this update persist with their observed rotation.
        std::vector<DirectCall> direct;
        for (const auto &c : observation.laser_hitboxes)
            if (c.pooled_slot < 0)
                direct.push_back({c.cx, c.cy, c.w, c.h, c.ox, c.oy, c.angle});
        for (const auto &c : direct) {
            float dang = 0, dox = 0, doy = 0;
            for (const auto &p : mem.direct_prev)
                if (p.w == c.w && p.h == c.h && std::fabs(p.ox - c.ox) < 16 &&
                    std::fabs(p.oy - c.oy) < 16) {
                    dang = c.angle - p.angle;
                    while (dang > 3.14159265f)
                        dang -= 6.2831853f;
                    while (dang < -3.14159265f)
                        dang += 6.2831853f;
                    if (std::fabs(dang) > 0.1f)
                        dang = 0;
                    dox = c.ox - p.ox, doy = c.oy - p.oy;
                    break;
                }
            for (int j = 1; j <= horizon; ++j) {
                Laser laser{c.cx + dox * j, c.cy + doy * j,    c.w, c.h, c.ox + dox * j,
                            c.oy + doy * j, c.angle + dang * j};
                if (opt.direct_turn_envelope && dang != 0) {
                    laser.angle = c.angle;
                    laser.angle_reach = std::min(std::abs(double(dang)) * j, std::acos(-1.0));
                    laser.reach_cos = std::cos(laser.angle_reach);
                    laser.reach_sin = std::sin(laser.angle_reach);
                    ++search_stats.turn_envelope_frames;
                }
                frames[j].l.push_back(laser);
            }
        }
        // Lethal bodies drawn this update, extrapolated with their matched displacement.
        std::vector<Box> regions;
        for (const auto &r : observation.lethal_regions)
            regions.push_back(r);
        for (const auto &r : regions) {
            float dx = 0, dy = 0;
            for (const auto &p : mem.regions_prev)
                if (std::fabs((p.x1 - p.x0) - (r.x1 - r.x0)) < 0.01f &&
                    std::fabs((p.y1 - p.y0) - (r.y1 - r.y0)) < 0.01f &&
                    std::fabs(p.x0 - r.x0) < 8 && std::fabs(p.y0 - r.y0) < 8) {
                    dx = r.x0 - p.x0, dy = r.y0 - p.y0;
                    break;
                }
            for (int j = 1; j <= horizon; ++j) {
                const float g = 1.0f + opt.grow * j;
                frames[j].b.push_back(
                    {r.x0 + dx * j - g, r.y0 + dy * j - g, r.x1 + dx * j + g, r.y1 + dy * j + g});
            }
        }
        // Unobserved births can be lethal before a newly sampled input moves the
        // player. Use current actor origins as an optional soft positioning cost;
        // this neither predicts a shot schedule nor certifies a safe future.
        if (opt.spawn_guard > 0)
            for (const auto &actor : observation.actors)
                for (int j = 1; j <= horizon; ++j) {
                    const float x = actor.origin_x + actor.delta_x * j;
                    const float y = actor.origin_y + actor.delta_y * j;
                    const float radius = opt.spawn_guard + opt.grow * j;
                    frames[j].risk.push_back({x - radius, y - radius, x + radius, y + radius});
                }
        if (opt.queued_shots)
            for (const auto &shot : observation.queued_shots) {
                float x = shot.x, y = shot.y;
                for (unsigned j = shot.update; j <= unsigned(horizon); ++j) {
                    const float grow = opt.grow * j;
                    const Box box{x - shot.width / 2 - grow, y - shot.height / 2 - grow,
                                  x + shot.width / 2 + grow, y + shot.height / 2 + grow};
                    if (j - shot.update < shot.linear_updates)
                        frames[j].b.push_back(box);
                    else
                        // Past the bounded linear prefix, keep a soft constant-
                        // velocity hypothesis rather than treating the tail as empty.
                        frames[j].risk.push_back(box);
                    x += shot.vx;
                    y += shot.vy;
                }
            }
        if (opt.queued_lasers)
            for (const auto &laser : observation.queued_lasers) {
                const auto &l = laser.hitbox;
                for (unsigned j = laser.first_update; j <= laser.last_update; ++j) {
                    Laser projected{l.cx, l.cy, l.w, l.h, l.ox, l.oy, l.angle};
                    if (opt.direct_turn_envelope && laser.turn_hint > 0) {
                        projected.angle_reach =
                            std::min(double(laser.turn_hint) * (j - laser.first_update + 1),
                                     std::acos(-1.0));
                        projected.reach_cos = std::cos(projected.angle_reach);
                        projected.reach_sin = std::sin(projected.angle_reach);
                        ++search_stats.turn_envelope_frames;
                    }
                    frames[j].l.push_back(projected);
                }
            }
    }

    // Exact pruning: a box farther (Chebyshev) than the player's maximum displacement
    // after j updates (+ the latched update) can never be touched in frame j.
    void prune_unreachable(std::vector<HFrame> &frames, int horizon, const Speeds &sp, float px,
                           float py) {
        const float vmax = std::max({std::fabs(sp.axis[0]), std::fabs(sp.axis[1]),
                                     std::fabs(sp.diag[0]), std::fabs(sp.diag[1])}) *
                           std::max(std::fabs(sp.hmul), std::fabs(sp.vmul)) * std::fabs(sp.frm);
        auto keep = [&](std::vector<Box> &v, float reach) {
            std::size_t o = 0;
            for (std::size_t i = 0; i < v.size(); ++i) {
                const Box &b = v[i];
                const float dx = std::max({b.x0 - px, px - b.x1, 0.f});
                const float dy = std::max({b.y0 - py, py - b.y1, 0.f});
                if (dx <= reach && dy <= reach)
                    v[o++] = b;
            }
            v.resize(o);
        };
        for (int j = 1; j <= horizon; ++j) {
            const float reach =
                vmax * float(j + 1) + std::max(sp.half.x, sp.half.y) + kMargin + 2.f;
            keep(frames[j].b, reach);
            keep(frames[j].risk, reach);
            auto &L = frames[j].l;
            std::size_t o = 0;
            for (std::size_t i = 0; i < L.size(); ++i) {
                const Box b = laser_aabb(L[i], std::max(sp.half.x, sp.half.y) + kMargin);
                const float dx = std::max({b.x0 - px, px - b.x1, 0.f});
                const float dy = std::max({b.y0 - py, py - b.y1, 0.f});
                if (dx <= reach && dy <= reach)
                    L[o++] = L[i];
            }
            L.resize(o);
        }
    }

    // ------------------------------------------------------------------ beam search
    struct Node {
        float x, y;
        int parent;
        std::uint16_t act;
        float score;
    };
    const std::uint16_t kDirs[9] = {0, 16, 32, 64, 128, 16 | 64, 16 | 128, 32 | 64, 32 | 128};
    std::uint64_t tie = 88172645463325252ULL;
    SearchStatistics search_stats;
    inline std::uint64_t xrand() {
        tie ^= tie << 13, tie ^= tie >> 7, tie ^= tie << 17;
        return tie;
    }
    struct SearchResult {
        std::vector<std::uint16_t> moves;
        int depth = 0;
    };
    SearchResult search(const std::vector<HFrame> &frames, int horizon, const Speeds &sp,
                        std::uint16_t latched, float x0, float y0, float pref_x, float pref_y) {
        const std::array<float, 24 * 28> *nw = nullptr;
        if (risk.loaded && opt.nowarn_weight > 0) {
            auto it = risk.spawn.find(current_segment);
            if (it != risk.spawn.end())
                nw = &it->second;
        }
        std::vector<std::vector<Node>> layers(horizon + 1);
        float x1 = x0, y1 = y0;
        move(sp, x1, y1, latched);
        layers[1].push_back({x1, y1, -1, std::uint16_t(latched & 0xF4), 0.f});
        if (probe(frames[1], sp.half, x1, y1, false) == 2)
            return {{std::uint16_t(latched & 0xF4)}, 0};
        // At most 18 successors per retained node. Keep an empty hash slot even
        // at the maximum supported beam, so linear probing always terminates.
        std::size_t table_size = 1 << 16;
        while (table_size <= std::size_t(opt.beam) * 18)
            table_size *= 2;
        std::vector<std::uint32_t> seen_key(table_size), seen_gen(table_size, 0);
        std::uint32_t gen = 0;
        int deepest = 1;
        for (int j = 1; j < horizon && !layers[j].empty(); ++j) {
            auto &cur = layers[j];
            auto &next = layers[j + 1];
            ++gen;
            const bool branch = j < 12 || (j % 2) == 0;
            for (int pi = 0; pi < int(cur.size()); ++pi) {
                const Node &p = cur[pi];
                for (int f = 0; f < 2; ++f)
                    for (int d = 0; d < 9; ++d) {
                        const std::uint16_t a = std::uint16_t(kDirs[d] | (f ? 4 : 0));
                        if (!branch && a != p.act)
                            continue;
                        float x = p.x, y = p.y;
                        move(sp, x, y, a);
                        const std::uint32_t key =
                            ((std::uint32_t(std::lround(x * 2)) & 2047u) << 12 |
                             (std::uint32_t(std::lround(y * 2)) & 2047u))
                                << 1 |
                            std::uint32_t(f);
                        std::uint32_t h = (key * 2654435761u) >> 16;
                        bool dup = false;
                        while (seen_gen[h] == gen) {
                            if (seen_key[h] == key) {
                                dup = true;
                                break;
                            }
                            h = (h + 1) & (table_size - 1);
                        }
                        if (dup)
                            continue;
                        seen_gen[h] = gen, seen_key[h] = key;
                        const int hit = probe(frames[j + 1], sp.half, x, y, true);
                        if (hit == 2)
                            continue;
                        Node n{x, y, pi, a, p.score};
                        const float dx = (x - pref_x) / 96.f, dy = (y - pref_y) / 96.f;
                        if ((risk.loaded || opt.spawn_guard > 0 || opt.queued_shots) &&
                            probe_risk(frames[j + 1], sp.half, x, y))
                            n.score += opt.risk_weight * (j < 16 ? 1.f : 0.5f);
                        if (nw) {
                            // expected no-warning spawns in the 3x3 cells around the node
                            const int cx = std::clamp(int(x / 16.f), 0, 23),
                                      cy = std::clamp(int(y / 16.f), 0, 27);
                            float r = 0;
                            for (int yy = std::max(0, cy - 1); yy <= std::min(27, cy + 1); ++yy)
                                for (int xx = std::max(0, cx - 1); xx <= std::min(23, cx + 1); ++xx)
                                    r += (*nw)[yy * 24 + xx];
                            n.score += opt.nowarn_weight * r * (j < 16 ? 1.f : 0.25f);
                        }
                        if (!(a & 4))
                            n.score += opt.focus_bias;
                        n.score += (hit == 1 ? (j < 12 ? 40.f : 10.f) : 0.f) +
                                   0.02f * (dx * dx + dy * dy) + (a != p.act ? 0.01f : 0.f) +
                                   float(xrand() % 1000) * 1e-6f;
                        next.push_back(n);
                    }
            }
            if (int(next.size()) > opt.beam) {
                if (opt.diversity_cell > 0) {
                    const auto begin = std::chrono::steady_clock::now();
                    const auto stats =
                        plan::retain_spatial_beam(next, opt.beam, opt.diversity_cell);
                    search_stats.selection_ms += std::chrono::duration<double, std::milli>(
                                                     std::chrono::steady_clock::now() - begin)
                                                     .count();
                    ++search_stats.selections;
                    search_stats.candidates += stats.candidates;
                    search_stats.bins += stats.bins;
                    search_stats.retained_bins += stats.retained_bins;
                } else {
                    std::nth_element(
                        next.begin(), next.begin() + opt.beam, next.end(),
                        [](const Node &a, const Node &b) { return a.score < b.score; });
                    next.resize(opt.beam);
                }
            }
            if (!next.empty())
                deepest = j + 1;
        }
        const auto &last = layers[deepest];
        int best = 0;
        for (int i = 1; i < int(last.size()); ++i)
            if (last[i].score < last[best].score)
                best = i;
        std::vector<std::uint16_t> rev;
        int i = best;
        for (int j = deepest; j > 1; --j) {
            rev.push_back(layers[j][i].act);
            i = layers[j][i].parent;
        }
        SearchResult r;
        r.moves.assign(rev.rbegin(), rev.rend());
        r.depth = deepest;
        if (r.moves.empty())
            r.moves.push_back(std::uint16_t(latched & 0xF4));
        return r;
    }

    void check_held_directions(SearchResult &result, const Observation &observation) {
        // A finite beam can discard an escape even in a static forecast. Keep
        // this cheap independent proposal family when it extends modeled life.
        // Equal depths retain the beam; held ties use the existing action order.
        if (!opt.held_fallback || result.depth == 0 || result.depth == opt.horizon)
            return;
        bool selected = false;
        for (int focus = 0; focus < 2; ++focus)
            for (const auto direction : kDirs) {
                const auto action = std::uint16_t(direction | (focus ? 4 : 0));
                float x = observation.x, y = observation.y;
                int depth = 0;
                ++search_stats.held_paths;
                for (int j = 1; j <= opt.horizon; ++j) {
                    move(observation.movement, x, y, j == 1 ? observation.latched_input : action);
                    ++search_stats.held_updates;
                    if (probe(frames[j], observation.movement.half, x, y, false) == 2)
                        break;
                    depth = j;
                }
                if (depth > result.depth) {
                    result.depth = depth;
                    result.moves.assign(std::max(1, depth - 1), action);
                    selected = true;
                }
            }
        search_stats.held_selected = selected;
    }

    Decision choose(const Observation &observation) {
        if (last_decided == observation.update)
            throw std::runtime_error("duplicate_online_decision");
        observe(observation);
        last_decided = observation.update;
        search_stats = {};
        current_movement = observation.movement;
        current_segment = observation.spell_active ? observation.spell : 300 + observation.stage;
        kinematic_forecast(observation, opt.horizon, frames);
        const auto &sp = observation.movement;
        if (opt.reach_filter)
            prune_unreachable(frames, opt.horizon, sp, observation.x, observation.y);
        for (int j = 1; j <= opt.horizon; ++j)
            build_index(frames[j], sp.half);
        float pref_x = 192.f, area = 0;
        for (const auto &r : observation.lethal_regions) {
            const float a = (r.x1 - r.x0) * (r.y1 - r.y0);
            if (a > area && r.y1 < 300)
                area = a, pref_x = 192.f + opt.boss_follow * ((r.x0 + r.x1) / 2 - 192.f);
        }
        pref_x = std::clamp(pref_x, 48.f, 336.f);
        auto result = search(frames, opt.horizon, sp, observation.latched_input, observation.x,
                             observation.y, pref_x, 400.f);
        check_held_directions(result, observation);
        return {std::uint16_t(result.moves.front() | 1), result.depth, search_stats};
    }
};
Policy::Policy(Configuration configuration, RiskTables risk)
    : impl_(new Impl(configuration, std::move(risk))) {}
Policy::~Policy() = default;
void Policy::observe(const Observation &observation) {
    impl_->observe(observation);
}
Decision Policy::decide(const Observation &observation) {
    return impl_->choose(observation);
}
} // namespace th08::online
