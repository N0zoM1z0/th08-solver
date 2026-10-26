// Adapted from the reviewed October 22 online-controller prototype. Native
// current-memory fields are copied by the adapter; this policy owns only its
// observation history, immutable risk model and heuristic proposal search.
#include "th08/online.hpp"
#include "th08/online_child_birth.hpp"
#include "th08/online_collision.hpp"
#include "th08/online_dedup.hpp"
#include "th08/online_lifetime.hpp"
#include "th08/online_motion_envelope.hpp"
#include "th08/online_movement.hpp"
#include "th08/online_plan.hpp"
#include "th08/online_rotation.hpp"
#include "th08/plan_beam.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace th08::online {
Configuration with_style(const std::string &name, Configuration configuration) {
    // The owner's October 25 review supplies the initial proposal preferences.
    // Contact rewards below additionally track unique bullets along each path.
    auto &w = configuration.style;
    w = {};
    configuration.scoring_collection = configuration.scoring_aim = false;
    configuration.scoring_survival_focus = false;
    configuration.scoring_survival_flow = false;
    configuration.scoring_attack_pacing = false;
    configuration.scoring_hit_income = false;
    configuration.scoring_boss_collection = false;
    configuration.scoring_precollect = false;
    configuration.scoring_priced_collection = configuration.scoring_upper_stage = false;
    configuration.collection_focus_bias = configuration.economic_human_bias = 0;
    configuration.boss_follow = 0.25f;
    if (name == "baseline")
        return configuration;
    if (name == "zen") {
        w.anchor_weight = 0.005f;
        w.change_cost = 0.15f;
        w.move_cost = 0.05f;
        w.focused_move_cost = 0.025f;
        w.reversal_cost = 0.5f;
    } else if (name == "expert") {
        w.anchor_weight = 0.006f;
        w.anchor_y = 390;
        w.change_cost = 0.08f;
        w.reversal_cost = 0.6f;
        w.move_cost = 0.01f;
        w.edge_cost = 2;
        w.edge_margin = 40;
        configuration.boss_follow = 0.6f;
    } else if (name == "sniper") {
        w.anchor_weight = 0.05f;
        w.anchor_y = 360;
        configuration.boss_follow = 1;
    } else if (name == "score" || name == "score-survival" || name == "score-harvest" ||
               name == "score-precollect" || name == "score-flow") {
        configuration.scoring_collection = configuration.scoring_aim = true;
        configuration.scoring_survival_focus = name != "score";
        configuration.scoring_boss_collection =
            name == "score-harvest" || name == "score-precollect" || name == "score-flow";
        configuration.scoring_survival_flow = name == "score-flow";
        if (configuration.scoring_survival_flow)
            configuration.survival_focus_bias = 5.01f;
        configuration.scoring_precollect = name == "score-precollect";
        w.anchor_weight = 0.05f;
        if (configuration.scoring_boss_collection) {
            w.edge_cost = 2;
            w.edge_margin = 40;
        }
        configuration.boss_follow = 1;
    } else if (name == "graze") {
        w.anchor_weight = 0.005f;
        w.change_cost = 0.05f;
        w.reversal_cost = 0.3f;
        w.graze_reward = 0.15f;
    } else
        throw std::runtime_error("unknown_online_style");
    return configuration;
}
const char *build_architecture() {
#ifdef TH08_ONLINE_ARCH_NAME
    return TH08_ONLINE_ARCH_NAME;
#else
    return "portable";
#endif
}
namespace {
using Float3 = Vector;
using Speeds = Movement;
using detail::box_hit;
struct Laser {
    float cx, cy, w, h, ox, oy, angle;
    // Proposal uncertainty only: all turn signs up to the last observed speed.
    // This does not certify that future scripts preserve that speed or origin.
    double angle_reach = 0, reach_cos = 1, reach_sin = 0;
    detail::Rotation2 inverse;
};
// Player::UpdateMovementAndOptions direction priority and float operation order.
inline void move(const Speeds &s, float &x, float &y, std::uint16_t in) {
    detail::apply_movement(s, x, y, detail::movement_delta(s, in));
}

struct HFrame {
    std::vector<Box> b; // bullets and lethal regions
    std::vector<Laser> l;
    std::vector<int> start, items; // CSR grid over b
    std::vector<Box> risk;         // learned uncertainty regions (soft cost)
    std::vector<int> rstart, ritems;
    std::vector<Box> laabb; // world-space AABB of each laser (index parallel to l)
    std::vector<int> lstart, litems;
    std::vector<Box> graze_boxes;
    std::vector<std::uint64_t> graze_bits;
    std::vector<int> gstart, gitems;
};
constexpr float kCell = 16.f, kGx0 = -64.f, kGy0 = -64.f;
constexpr int kGW = 34, kGH = 38;
constexpr float kMargin = 1.5f;
constexpr float kGrazeExpansion = 20.f; // Source Player::CheckGrazeCollision ordinary bullets.
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
    if (!f.graze_boxes.empty())
        build_grid(f.graze_boxes, {}, f.gstart, f.gitems);
    else
        f.gstart.clear(), f.gitems.clear();
    f.laabb.resize(f.l.size());
    for (std::size_t i = 0; i < f.l.size(); ++i) {
        f.l[i].inverse = detail::prepare_rotation(-f.l[i].angle);
        f.laabb[i] = laser_aabb(f.l[i], std::max(half.x, half.y) + kMargin);
    }
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
inline bool laser_hit(const Laser &l, const Float3 &p, const Float3 &half, float margin) {
    Float3 origin, position, size, in_min, in_max, pmin, pmax;
    origin.x = l.ox, origin.y = l.oy, origin.z = 0;
    position.x = l.cx, position.y = l.cy, position.z = 0;
    size.x = l.w, size.y = l.h, size.z = 0;
    in_min = p - origin;
    in_max = l.inverse.apply(in_min);
    if (l.angle_reach > 0) {
        // A circle arc intersects a rectangle either at an arc endpoint or a
        // rectangle edge. Endpoint half-planes test membership without atan2.
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
            const double dot = u * x + v * y, cross = u * y - v * x;
            // Oriented endpoint half-planes stay well-conditioned for narrow
            // arcs. A cosine/dot tolerance otherwise expands a tiny angle by
            // much more than the intended coordinate rounding allowance.
            const double from_begin = l.reach_cos * cross + l.reach_sin * dot;
            const double to_end = l.reach_sin * dot - l.reach_cos * cross;
            if (l.reach_cos >= 0)
                return dot >= -arc_round && from_begin >= -arc_round && to_end >= -arc_round;
            return from_begin >= -arc_round || to_end >= -arc_round;
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
    StyleWeights proposal_style;
    const RiskTables risk;
    const BranchValueModel value_model;
    Movement current_movement;
    std::vector<HFrame> frames;
    int indexed_horizon = 0;
    bool time_index_extensions = false;
    std::uint64_t index_extension_ns = 0;
    std::uint64_t last_observed = UINT64_MAX;
    std::uint64_t last_decided = UINT64_MAX;
    bool collecting = false;
    float proposal_focus_bias = 0;
    float collection_x = 192;
    bool preparing_collection = false;
    struct AttackPacing {
        enum class Phase { Probe, Sample, Farm, Capture };
        Phase phase = Phase::Probe;
        int spell = -1, initial_hp = 0, previous_hp = 0, income_orbs = 0;
        std::uint64_t samples = 0, sample_begin = 0, income_begin = 0;
        std::array<int, 120> recent_damage{};
        std::int64_t damage = 0;
        bool farming = false;
    } attack;
    static constexpr unsigned collection_batch = 4, collection_start_bullet_limit = 64;
    explicit Impl(Configuration configuration, RiskTables tables, BranchValueModel value)
        : opt(configuration), proposal_style(configuration.style), risk(std::move(tables)),
          value_model(std::move(value)) {
        const auto &w = opt.style;
        for (float cost : {w.margin_early, w.margin_late, w.anchor_weight, w.change_cost,
                           w.move_cost, w.focused_move_cost, w.reversal_cost, w.edge_cost,
                           w.graze_reward, opt.survival_focus_bias, opt.attack_focus_bias,
                           opt.collection_focus_bias, opt.economic_human_bias})
            if (!std::isfinite(cost) || cost < 0 || cost > 10000)
                throw std::runtime_error("invalid_online_style_cost");
        for (float coordinate : {w.anchor_x, w.anchor_y, w.anchor_x_min, w.anchor_x_max})
            if (!std::isfinite(coordinate) || std::abs(coordinate) > 1048576)
                throw std::runtime_error("invalid_online_style_anchor");
        if (w.anchor_x_min > w.anchor_x_max || w.margin_split < 1 || w.margin_split > 200 ||
            !std::isfinite(w.edge_margin) || w.edge_margin < 0 || w.edge_margin > 1024 ||
            (w.edge_cost > 0 && w.edge_margin == 0))
            throw std::runtime_error("invalid_online_style_bounds");
        if ((opt.scoring_boss_collection || opt.scoring_precollect ||
             opt.scoring_priced_collection || opt.scoring_upper_stage ||
             opt.collection_focus_bias > 0) &&
            !opt.scoring_collection)
            throw std::runtime_error("boss_collection_requires_item_goals");
        if (!std::isfinite(opt.attack_reserve_factor) || opt.attack_reserve_factor < 1 ||
            opt.attack_reserve_factor > 10 || opt.attack_reserve_updates > 10000)
            throw std::runtime_error("invalid_online_capture_reserve");
        if (!std::isfinite(opt.value_weight) || opt.value_weight < 0 ||
            (value_model.loaded &&
             (opt.horizon != int(kValueHorizon) || opt.planner_lag || opt.tail_check ||
              opt.warm_fallback || opt.held_fallback || risk.loaded || !opt.hard_phantoms ||
              !opt.motion_envelope || opt.scoring_collection || opt.scoring_aim ||
              opt.scoring_survival_focus || opt.scoring_survival_flow ||
              opt.scoring_attack_pacing || opt.scoring_hit_income || opt.economic_human_bias > 0)))
            throw std::runtime_error("unsupported_branch_value_configuration");
        if (opt.motion_envelope && !opt.hard_phantoms)
            throw std::runtime_error("motion_envelope_requires_hard_phantoms");
        if (opt.horizon < 2 || opt.horizon > 200 || opt.beam < 1 || opt.beam > 8192 ||
            opt.tail_check < 0 || opt.tail_check > 200 - opt.horizon || !std::isfinite(opt.grow) ||
            opt.grow < 0 || !std::isfinite(opt.boss_follow) || !std::isfinite(opt.risk_weight) ||
            opt.risk_weight < 0 || !std::isfinite(opt.nowarn_weight) || opt.nowarn_weight < 0 ||
            !std::isfinite(opt.focus_bias) || !std::isfinite(opt.spawn_guard) ||
            opt.spawn_guard < 0 || opt.spawn_guard > 128 || opt.diversity_cell < 0 ||
            opt.diversity_cell > 128 || opt.planner_lag < 0 ||
            (opt.planner_lag > 0 &&
             (opt.planner_lag >= opt.horizon - 1 || opt.executor_check < 2 ||
              opt.executor_check > opt.horizon || opt.emergency_horizon < 2 ||
              opt.emergency_horizon > opt.horizon || opt.emergency_beam < 1 ||
              opt.emergency_beam > 8192)))
            throw std::runtime_error("invalid_online_configuration");
        if (opt.planner_lag > 0 && (opt.held_fallback || opt.warm_fallback || opt.tail_check))
            throw std::runtime_error("unsupported_online_lag_fallback");
        if (opt.planner_lag > 0 &&
            (opt.scoring_collection || opt.scoring_aim || opt.scoring_survival_focus ||
             opt.scoring_survival_flow || opt.scoring_attack_pacing || opt.scoring_hit_income ||
             opt.economic_human_bias > 0))
            throw std::runtime_error("unsupported_scoring_lag_configuration");
        frames.resize(opt.horizon + opt.tail_check + 1);
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
        if (opt.scoring_collection || opt.scoring_aim || opt.scoring_survival_focus ||
            opt.scoring_survival_flow || opt.scoring_attack_pacing || opt.scoring_hit_income ||
            opt.economic_human_bias > 0) {
            if (!o.scoring.available)
                throw std::runtime_error("missing_online_score_memory");
            for (float v : {o.scoring.boss_x, o.scoring.collection_line, o.scoring.power,
                            o.scoring.impending_x, o.scoring.orb_value, o.scoring.bonus_decay,
                            o.scoring.pickup_reach_y})
                finite(v);
            if (o.scoring.pickup_reach_y < 0)
                throw std::runtime_error("invalid_online_pickup_box");
            if (opt.scoring_attack_pacing &&
                (o.scoring.capture_hp < 0 || o.scoring.collected_orbs < 0 ||
                 o.scoring.orb_value < 0 || o.scoring.bonus_decay < 0))
                throw std::runtime_error("invalid_online_attack_memory");
            for (const auto &item : o.scoring.pickups) {
                finite(item.x);
                finite(item.y);
            }
            for (const auto &target : o.scoring.income_targets) {
                finite(target.x);
                finite(target.y);
                if (target.reduction_updates < 0)
                    throw std::runtime_error("invalid_online_income_target");
            }
        }
        if (opt.style.graze_reward > 0 || opt.scoring_survival_flow || opt.scoring_attack_pacing) {
            finite(o.graze_half.x);
            finite(o.graze_half.y);
            if (o.graze_half.x < 0 || o.graze_half.y < 0)
                throw std::runtime_error("invalid_online_graze_box");
        }
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
            if (b.alternate_motion) {
                finite(b.alternate_vx);
                finite(b.alternate_vy);
            }
            if (b.full_width < 0 || b.full_height < 0)
                throw std::runtime_error("unsupported_online_observation");
            if (opt.spawn_clock && b.activation_update &&
                (b.state < 2 || b.state > 4 || o.movement.frm != 1 || b.activation_update > 32768))
                throw std::runtime_error("unsupported_online_spawn_clock");
            if (opt.scheduled_expiry && b.collision_until &&
                (b.state != 1 || b.collision_until > 200 || o.movement.frm != 1 ||
                 (b.active_transforms && b.active_transforms != detail::kLifetimeWaitKind)))
                throw std::runtime_error("invalid_online_collision_lifetime");
            const auto &r = b.relative_turn;
            if (opt.relative_turns && r.updates) {
                const auto &turn = r.state;
                for (float v : {r.angle, r.speed, turn.angle, turn.speed})
                    finite(v);
                if (b.state != 1 || b.active_transforms != 0x40u || o.movement.frm != 1 ||
                    !turn.active || turn.mode != bullet::TurnMode::relative || turn.interval < 0 ||
                    turn.timer < 0 || turn.timer > std::numeric_limits<int>::max() - 201 ||
                    turn.completed < 0 || turn.completed > std::numeric_limits<int>::max() - 201 ||
                    turn.subframe != 0)
                    throw std::runtime_error("unsupported_online_relative_turn");
            }
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
            if (shot.update == 0 || shot.update > unsigned(opt.horizon + opt.tail_check) ||
                shot.linear_updates == 0 || shot.width < 0 || shot.height < 0)
                throw std::runtime_error("unsupported_online_observation");
            for (float v : {shot.x, shot.y, shot.vx, shot.vy, shot.width, shot.height})
                finite(v);
        }
        if (opt.child_births && o.unsupported_child_births)
            throw std::runtime_error("unsupported_online_child_birth");
        for (const auto &birth : o.child_births) {
            finite(birth.x);
            finite(birth.y);
            if (birth.update == 0 || birth.update > 200 || !std::isfinite(birth.max_speed) ||
                birth.max_speed < 0 || birth.max_speed > 1048576 || !std::isfinite(birth.width) ||
                !std::isfinite(birth.height) || birth.width < 0 || birth.height < 0 ||
                birth.width > 1048576 || birth.height > 1048576)
                throw std::runtime_error("invalid_online_child_birth");
        }
        for (const auto &laser : o.queued_lasers) {
            const auto &l = laser.hitbox;
            if (laser.first_update == 0 || laser.last_update < laser.first_update ||
                laser.last_update > unsigned(opt.horizon + opt.tail_check) || l.w < 0 || l.h < 0 ||
                laser.turn_hint < 0)
                throw std::runtime_error("unsupported_online_observation");
            finite(laser.turn_hint);
            for (float v : {l.cx, l.cy, l.w, l.h, l.ox, l.oy, l.angle})
                finite(v);
        }
    }

    bool flow_active = false;
    float flow_x = 192, flow_y = 360, flow_contacts = 0;
    std::uint64_t macro_due = 0;
    int macro_spell = -1;
    void update_flow_goal(const Observation &o) {
        // Rank ordinary-bullet streams using a bounded linear hypothesis.
        // The cached stationary targets reduce chatter/work; neither their
        // contacts nor danger proxy certify future scripts or collision safety.
        if (macro_spell == o.spell && o.update < macro_due)
            return;
        macro_spell = o.spell;
        macro_due = o.update + 64;
        constexpr int nx = 10, ny = 7, cells = nx * ny;
        std::array<int, cells> contacts{}, danger{};
        for (const auto &b : o.bullets) {
            if (b.state != 1 || b.collision_disabled || b.grazed || b.vx * b.vx + b.vy * b.vy > 9)
                continue;
            std::array<bool, cells> seen{}, hit{};
            const float ex = b.full_width / 2 + kGrazeExpansion + o.graze_half.x;
            const float ey = b.full_height / 2 + kGrazeExpansion + o.graze_half.y;
            const float hx = o.movement.half.x + b.full_width / 2 + 1.5f;
            const float hy = o.movement.half.y + b.full_height / 2 + 1.5f;
            for (int dt = 0; dt <= 192; dt += 16) {
                if (unsigned(dt) < b.graze_ready_in)
                    continue;
                const float bx = b.x + b.vx * dt, by = b.y + b.vy * dt;
                const int x0 = std::max(0, int(std::ceil((bx - ex - 48) / 32)));
                const int x1 = std::min(nx - 1, int(std::floor((bx + ex - 48) / 32)));
                const int y0 = std::max(0, int(std::ceil((by - ey - 208) / 32)));
                const int y1 = std::min(ny - 1, int(std::floor((by + ey - 208) / 32)));
                for (int y = y0; y <= y1; ++y)
                    for (int x = x0; x <= x1; ++x) {
                        const int cell = y * nx + x;
                        seen[cell] = true;
                        hit[cell] = hit[cell] || (std::abs(48 + x * 32 - bx) < hx &&
                                                  std::abs(208 + y * 32 - by) < hy);
                    }
            }
            for (int i = 0; i < cells; ++i) {
                contacts[i] += seen[i];
                danger[i] += hit[i];
            }
        }
        float best = -1e30f;
        for (int y = 0; y < ny; ++y)
            for (int x = 0; x < nx; ++x) {
                const float px = 48 + x * 32, py = 208 + y * 32;
                if (px < o.movement.minx || px > o.movement.maxx || py < o.movement.miny ||
                    py > o.movement.maxy)
                    continue;
                bool blocked = false;
                for (const auto &r : o.lethal_regions)
                    blocked = blocked || (px >= r.x0 - 8 && px <= r.x1 + 8 && py >= r.y0 - 8 &&
                                          py <= r.y1 + 8);
                if (blocked)
                    continue;
                const int cell = y * nx + x;
                const float dx = px - o.x, dy = py - o.y;
                const float value =
                    contacts[cell] - 3.f * danger[cell] - .02f * std::sqrt(dx * dx + dy * dy);
                if (value > best) {
                    best = value;
                    flow_x = px;
                    flow_y = py;
                    flow_contacts = float(std::max(0, contacts[cell] - danger[cell]));
                }
            }
    }

    void update_attack_pacing(const Observation &o) {
        auto &a = attack;
        using Phase = AttackPacing::Phase;
        a.farming = false;
        const auto &s = o.scoring;
        if (!opt.scoring_attack_pacing || !s.can_switch_forms || !o.spell_active || !s.has_boss) {
            a.spell = -1;
            return;
        }
        update_flow_goal(o);
        bool graze_goal = s.survival_spell && opt.scoring_survival_focus;
        if (!s.survival_spell && s.spell_damageable && s.capture_hp > 0 && s.spell_remaining > 0) {
            if (a.spell != o.spell || !a.initial_hp) {
                a.spell = o.spell;
                a.phase = Phase::Probe;
                a.initial_hp = a.previous_hp = s.capture_hp;
                a.samples = 0;
                a.damage = 0;
                a.recent_damage.fill(0);
            }
            if (a.phase == Phase::Probe && s.reduction_remaining <= 0) {
                auto &old = a.recent_damage[a.samples % a.recent_damage.size()];
                a.damage -= old;
                old = std::max(0, a.previous_hp - s.capture_hp);
                a.damage += old;
                ++a.samples;
            }
            a.previous_hp = s.capture_hp;
            if (a.samples >= 60 && a.damage > 0) {
                // Exclude initial projectile flight/damage reduction. Keep a
                // conservative capture reserve; Native rollouts still decide
                // actual HP, shot carry-over, safety and timeout outcomes.
                const float dps = float(a.damage) /
                                  float(std::min<std::uint64_t>(a.samples, a.recent_damage.size()));
                const float reserve = opt.attack_reserve_factor * float(s.capture_hp) / dps +
                                      opt.attack_reserve_updates;
                if ((a.phase != Phase::Probe && float(s.spell_remaining) <= reserve) ||
                    s.capture_hp < a.initial_hp / 5)
                    a.phase = Phase::Capture;
                else if (a.phase == Phase::Probe && float(s.spell_remaining) > reserve + 700) {
                    unsigned slow_nearby = 0;
                    for (const auto &b : o.bullets) {
                        const float dx = b.x - o.x, dy = b.y - o.y;
                        slow_nearby += b.state == 1 && !b.collision_disabled && !b.grazed &&
                                       dx * dx + dy * dy < 160 * 160 &&
                                       b.vx * b.vx + b.vy * b.vy <= 9;
                    }
                    if (slow_nearby >= 16 && flow_contacts >= 8) {
                        a.phase = Phase::Sample;
                        a.sample_begin = o.update;
                        a.income_begin = 0;
                    }
                }
            }
            if (a.phase == Phase::Sample) {
                // Releasing fire lets the gauge decay. Measure realized income
                // after source extreme-youkai activation, with a bounded wait.
                if (!a.income_begin && s.extreme_youkai) {
                    a.income_begin = o.update;
                    a.income_orbs = s.collected_orbs;
                }
                if (a.income_begin && o.update - a.income_begin >= 240) {
                    const float rate = float(std::max(0, s.collected_orbs - a.income_orbs)) /
                                       float(o.update - a.income_begin);
                    a.phase =
                        rate * (s.orb_value + 8000) > s.bonus_decay ? Phase::Farm : Phase::Capture;
                } else if (o.update - a.sample_begin >= 1200)
                    a.phase = Phase::Capture;
            }
            a.farming = a.phase == Phase::Sample || a.phase == Phase::Farm;
            graze_goal = a.farming;
        }
        if (graze_goal) {
            flow_active = true;
            proposal_focus_bias += opt.attack_focus_bias;
        }
    }

    float collection_goal_y(const Observation &o) const {
        const float clearance = opt.scoring_priced_collection ? o.scoring.pickup_reach_y + 2 : 12;
        return std::clamp(o.scoring.collection_line - clearance, o.movement.miny, o.movement.maxy);
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
        if (opt.scoring_collection) {
            const auto &score = observation.scoring;
            const bool impending = opt.scoring_precollect && !score.has_boss &&
                                   score.impending_points >= collection_batch;
            const bool boss_blocks =
                score.has_boss && (!opt.scoring_boss_collection || observation.spell_active ||
                                   observation.bullets.size() >= 128);
            if (boss_blocks || !score.auto_collect_eligible ||
                (score.pickups.empty() && !impending))
                collecting = false;
            else if (!collecting && (score.pickups.size() >= collection_batch || impending) &&
                     observation.bullets.size() < collection_start_bullet_limit) {
                // Keep the trip until its drops/items disappear or the boss
                // gate closes. Recomputing a four-item threshold each frame
                // would turn away while the final items still approach.
                collecting = true;
                float sum = 0;
                for (const auto &item : score.pickups)
                    sum += item.x;
                collection_x =
                    score.pickups.empty() ? score.impending_x : sum / float(score.pickups.size());
            }
            // Current unconditional drop counts justify positioning before the
            // kill. Actual native drops, RNG scatter and pickup prices remain
            // action-dependent. Finish the climb before launching new shots.
            preparing_collection =
                impending && collecting && observation.y > collection_goal_y(observation);
        }
        proposal_focus_bias = opt.focus_bias;
        if (collecting && observation.scoring.can_switch_forms)
            proposal_focus_bias -= opt.collection_focus_bias;
        const bool survival_focus =
            opt.scoring_survival_focus && observation.scoring.can_switch_forms &&
            observation.scoring.survival_spell && observation.scoring.has_boss &&
            observation.spell_active && !collecting;
        if (survival_focus) {
            // Source-marked survival attacks have different opportunity costs
            // from HP races. The default prior uses the action-change scale;
            // stronger recorded profiles can give sustained form more weight.
            proposal_focus_bias += opt.survival_focus_bias;
        }
        proposal_style = opt.style;
        flow_active = opt.scoring_survival_flow && observation.scoring.can_switch_forms &&
                      observation.scoring.survival_spell && observation.scoring.has_boss &&
                      observation.spell_active;
        update_attack_pacing(observation);
        if (!flow_active && !survival_focus && observation.scoring.can_switch_forms)
            proposal_focus_bias -= opt.economic_human_bias;
        if (flow_active) {
            update_flow_goal(observation);
            proposal_style.anchor_weight = 0.03f;
            proposal_style.graze_reward = 3;
        }
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
            current_vel[b.slot] = {true,    b.collision_disabled, b.vx, b.vy,
                                   b.state, b.active_transforms};
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
        bool valid = false, collision_disabled = false;
        float vx = 0, vy = 0;
        std::uint16_t state = 0;
        std::uint32_t active_transforms = 0;
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

    int collision_horizon(const Bullet &bullet, int horizon) {
        if (!opt.scheduled_expiry || !bullet.collision_until)
            return horizon;
        const int projected = std::min(horizon, int(bullet.collision_until));
        ++search_stats.lifetime_records;
        search_stats.lifetime_projected_frames += projected;
        search_stats.lifetime_skipped_frames += horizon - projected;
        return projected;
    }
    void kinematic_forecast(const Observation &observation, int horizon,
                            std::vector<HFrame> &frames) {
        for (int j = 1; j <= horizon; ++j)
            frames[j].b.clear(), frames[j].l.clear(), frames[j].risk.clear(),
                frames[j].graze_boxes.clear(), frames[j].graze_bits.clear();
        std::array<std::uint64_t, kSlots> graze_slots{};
        if (proposal_style.graze_reward > 0) {
            std::vector<std::pair<float, int>> eligible;
            for (const auto &b : observation.bullets)
                if (b.slot >= 0 && b.slot < kSlots && b.state == 1 && !b.grazed &&
                    !b.collision_disabled) {
                    const float dx = b.x - observation.x, dy = b.y - observation.y;
                    eligible.emplace_back(dx * dx + dy * dy, b.slot);
                }
            std::sort(eligible.begin(), eligible.end());
            for (std::size_t i = 0; i < std::min<std::size_t>(64, eligible.size()); ++i)
                graze_slots[eligible[i].second] = std::uint64_t(1) << i;
        }
        const auto &bs = observation.bullets;
        for (const auto &b : bs) {
            if (b.slot < 0 || b.slot >= kSlots)
                continue;
            const auto &nb = b;
            const float w = b.full_width, h = b.full_height;
            const auto &m = mem.slots[b.slot];
            if (nb.collision_disabled && !(opt.hard_phantoms && b.state == 1)) {
                // A visible phantom is harmless now but may become real (e.g. illusion
                // patterns): no hard hazard, only a learned soft-risk region.
                if (!(opt.risk_phantoms && risk.loaded && b.state == 1))
                    continue;
                const int projected = collision_horizon(b, horizon);
                const float s1 = std::hypot(b.vx, b.vy);
                const auto *rq =
                    lookup_risk(current_segment, int(nb.sprite), int(std::lround(w * 10)),
                                speed_bucket(s1) + 4, age_bucket(m.born >= 0 ? now - m.born : 0));
                float x = b.x, y = b.y;
                for (int j = 1; j <= projected; ++j) {
                    x += b.vx, y += b.vy;
                    const float rm = std::max(2.f, rq ? std::min(12.f, risk_margin(*rq, j)) : 4.f);
                    frames[j].risk.push_back({x - w / 2.0f - rm, y - h / 2.0f - rm,
                                              w / 2.0f + x + rm, h / 2.0f + y + rm});
                }
                continue;
            }
            if (b.state == 1) {
                const int projected = collision_horizon(b, horizon);
                // This optional prior changes only proposal obstacles. The native
                // collision flag and the observer's mode-change history stay intact.
                if (b.collision_disabled)
                    search_stats.hard_phantom_frames += projected;
                detail::MotionEnvelope envelope(b.x, b.y);
                const bool alternate = opt.motion_envelope && b.alternate_motion;
                if (alternate)
                    search_stats.motion_envelope_frames += projected;
                const Prev &p = prev_vel[b.slot];
                float x = b.x, y = b.y;
                float s1 = std::hypot(b.vx, b.vy), a1 = std::atan2(b.vy, b.vx), ds = 0, da = 0;
                // Transform or collision-mode changes can reset velocity discretely.
                // That difference is not a measured acceleration/rotation rate.
                const bool fd = p.valid && p.state == 1 &&
                                p.collision_disabled == b.collision_disabled &&
                                p.active_transforms == b.active_transforms;
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
                const auto &relative = b.relative_turn;
                bullet::Flight flight{x, y, b.vx, b.vy, relative.angle, relative.speed};
                auto turn = relative.state;
                const std::array<float, 4> *rq = nullptr;
                if (risk.loaded)
                    rq = lookup_risk(current_segment, int(nb.sprite), int(std::lround(w * 10)),
                                     speed_bucket(s1),
                                     age_bucket((opt.risk_stop_age && s1 < 0.05f && m.stopped >= 0)
                                                    ? now - m.stopped
                                                    : (m.born >= 0 ? now - m.born : 0)));
                for (int j = 1; j <= projected; ++j) {
                    if (opt.relative_turns && relative.updates) {
                        if (unsigned(j) <= relative.updates) {
                            if (bullet::advance_direction(flight, turn, current_movement.frm) !=
                                bullet::Status::advanced)
                                throw std::runtime_error("unsupported_online_relative_turn");
                            ++search_stats.relative_turn_frames;
                        }
                        // Past the certified transform boundary, constant velocity
                        // resumes the proposal hypothesis, not a native guarantee.
                        x += flight.velocity_x;
                        y += flight.velocity_y;
                    } else if (fd) {
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
                    if (graze_slots[b.slot] && unsigned(j) >= b.graze_ready_in) {
                        // Source CheckGrazeCollision expands each bullet by 20px
                        // against the current player graze box. Motion remains a
                        // conditional proposal hypothesis; native awards decide.
                        const float gx = w / 2 + kGrazeExpansion + observation.graze_half.x;
                        const float gy = h / 2 + kGrazeExpansion + observation.graze_half.y;
                        frames[j].graze_boxes.push_back({x - gx, y - gy, x + gx, y + gy});
                        frames[j].graze_bits.push_back(graze_slots[b.slot]);
                    }
                    if (alternate)
                        envelope.advance(x, y, b.alternate_vx, b.alternate_vy);
                    else
                        envelope = detail::MotionEnvelope(x, y);
                    float rm = rq ? std::min(12.f, risk_margin(*rq, j)) : 0.f;
                    if (opt.risk_hard && rm > 1.5f) {
                        frames[j].b.push_back(
                            {envelope.min_x - w / 2.0f - rm, envelope.min_y - h / 2.0f - rm,
                             w / 2.0f + envelope.max_x + rm, h / 2.0f + envelope.max_y + rm});
                        continue;
                    }
                    frames[j].b.push_back(
                        {envelope.min_x - w / 2.0f - g, envelope.min_y - h / 2.0f - g,
                         w / 2.0f + envelope.max_x + g, h / 2.0f + envelope.max_y + g});
                    if (rm > 1.5f)
                        frames[j].risk.push_back(
                            {envelope.min_x - w / 2.0f - rm, envelope.min_y - h / 2.0f - rm,
                             w / 2.0f + envelope.max_x + rm, h / 2.0f + envelope.max_y + rm});
                }
            } else if (b.state >= 2 && b.state <= 4) {
                const float div = b.state == 2 ? 2.0f : b.state == 3 ? 2.5f : 3.0f;
                const long age = m.spawn_seen >= 0 ? now - m.spawn_seen : 0;
                // Prefer the observed ANM clock even in a first/late snapshot.
                // Unsupported clocks retain the explicit empirical hypothesis.
                const bool clock = opt.spawn_clock && b.activation_update;
                const float fraction_x = clock ? b.vx * (1.0f / div) : b.vx / div;
                const float fraction_y = clock ? b.vy * (1.0f / div) : b.vy / div;
                const int remaining = clock
                                          ? int(b.activation_update - 1)
                                          : int(std::max(0L, spawn_duration(b.state, w) - age - 1));
                float x = b.x, y = b.y;
                for (int j = 1; j <= horizon; ++j) {
                    if (clock && j <= remaining + 1)
                        ++search_stats.spawn_clock_frames;
                    if (j <= remaining) {
                        x += fraction_x, y += fraction_y;
                        continue; // not collidable while spawning
                    }
                    if (clock && j == remaining + 1)
                        // Source still performs the fractional spawning step on
                        // its final animation call, then enters the FIRED branch.
                        x += fraction_x, y += fraction_y;
                    x += b.vx, y += b.vy;
                    const float g = opt.grow * j + 1.0f; // unknown transform at activation
                    frames[j].b.push_back(
                        {x - w / 2.0f - g, y - h / 2.0f - g, w / 2.0f + x + g, h / 2.0f + y + g});
                }
            }
        }
        if (opt.child_births)
            for (const auto &birth : observation.child_births) {
                const auto boxes = detail::child_birth_boxes(birth, opt.grow);
                for (unsigned age = 0; age < boxes.size(); ++age)
                    if (birth.update + age <= unsigned(horizon)) {
                        frames[birth.update + age].b.push_back(boxes[age]);
                        ++search_stats.child_birth_frames;
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
        std::uint64_t graze_mask = 0;
    };
    static std::uint64_t graze_contacts(const HFrame &frame, float x, float y) {
        if (frame.gstart.empty())
            return 0;
        const int cell = cell_of(y, kGy0, kGH) * kGW + cell_of(x, kGx0, kGW);
        std::uint64_t mask = 0;
        for (int i = frame.gstart[cell]; i < frame.gstart[cell + 1]; ++i) {
            const int index = frame.gitems[i];
            const auto &box = frame.graze_boxes[index];
            if (x >= box.x0 && x <= box.x1 && y >= box.y0 && y <= box.y1)
                mask |= frame.graze_bits[index];
        }
        return mask;
    }
    float style_cost(const Node &parent, Node &node, const Speeds &movement,
                     const HFrame &frame) const {
        const auto &w = proposal_style;
        float cost = 0;
        if (node.act & 0xf0)
            cost += (node.act & 4) ? w.focused_move_cost : w.move_cost;
        const auto sign = [](std::uint16_t a, unsigned negative, unsigned positive) {
            return int(bool(a & positive)) - int(bool(a & negative));
        };
        if (w.reversal_cost > 0 && (sign(node.act, 64, 128) * sign(parent.act, 64, 128) < 0 ||
                                    sign(node.act, 16, 32) * sign(parent.act, 16, 32) < 0))
            cost += w.reversal_cost;
        if (w.edge_cost > 0) {
            const float gap = std::min({node.x - movement.minx, movement.maxx - node.x,
                                        node.y - movement.miny, movement.maxy - node.y});
            const float amount = std::max(0.f, (w.edge_margin - gap) / w.edge_margin);
            cost += w.edge_cost * amount * amount;
        }
        if (w.graze_reward > 0) {
            const auto contacts = graze_contacts(frame, node.x, node.y);
            auto fresh = contacts & ~parent.graze_mask;
            unsigned count = 0;
            while (fresh) {
                fresh &= fresh - 1;
                ++count;
            }
            node.graze_mask |= contacts;
            cost -= w.graze_reward * count;
        }
        return cost;
    }
    struct ValueContext {
        using Grid = std::array<float, 24 * 28>;
        Grid real{}, ghost{}, spawning{}, actors{}, canonical_gap{};
        float x = 0, y = 0, youkai = 0, bullets = 0, ghosts = 0, lasers = 0;
        static int bin(float x, float y) {
            return std::clamp(int(y / 16), 0, 27) * 24 + std::clamp(int(x / 16), 0, 23);
        }
        static bool inside(float x, float y) {
            return x >= 0 && x <= 384 && y >= 0 && y <= 448;
        }
        static float local(const Grid &grid, float x, float y) {
            const int cx = std::clamp(int(x / 16), 0, 23), cy = std::clamp(int(y / 16), 0, 27);
            float sum = 0;
            for (int yy = std::max(0, cy - 1); yy <= std::min(27, cy + 1); ++yy)
                for (int xx = std::max(0, cx - 1); xx <= std::min(23, cx + 1); ++xx)
                    sum += grid[yy * 24 + xx];
            return sum;
        }
        void prepare(const Observation &observation) {
            *this = {};
            x = observation.x;
            y = observation.y;
            youkai = observation.youkai;
            bullets = float(observation.bullets.size());
            lasers = float(observation.laser_hitboxes.size());
            for (const auto &b : observation.bullets) {
                ghosts += b.state == 1 && b.collision_disabled;
                if (!inside(b.x, b.y))
                    continue;
                const int cell = bin(b.x, b.y);
                if (b.state >= 2 && b.state <= 4)
                    ++spawning[cell];
                else if (b.state == 1) {
                    if (b.collision_disabled)
                        ++ghost[cell];
                    else
                        ++real[cell];
                    if (b.alternate_motion)
                        canonical_gap[cell] +=
                            std::hypot(b.vx - b.alternate_vx, b.vy - b.alternate_vy) / 8;
                }
            }
            for (const auto &actor : observation.actors)
                if (inside(actor.origin_x, actor.origin_y))
                    ++actors[bin(actor.origin_x, actor.origin_y)];
        }
    } value_context;
    ValueFeatures value_features(const Node &node, const Speeds &sp) const {
        const auto &context = value_context;
        const auto &frame = frames[opt.horizon];
        const int cell = cell_of(node.y, kGy0, kGH) * kGW + cell_of(node.x, kGx0, kGW);
        const auto delta = detail::movement_delta(sp, node.act);
        ValueFeatures features{node.x / 384,
                               node.y / 448,
                               (node.x - context.x) / 96,
                               (node.y - context.y) / 96,
                               (node.x - sp.minx) / 96,
                               (sp.maxx - node.x) / 96,
                               (node.y - sp.miny) / 96,
                               (sp.maxy - node.y) / 96,
                               delta.x / 8,
                               delta.y / 8,
                               float((node.act & 4) != 0),
                               node.score / opt.horizon,
                               ValueContext::local(context.real, node.x, node.y) / 64,
                               ValueContext::local(context.ghost, node.x, node.y) / 64,
                               ValueContext::local(context.spawning, node.x, node.y) / 64,
                               ValueContext::local(context.actors, node.x, node.y) / 16,
                               ValueContext::local(context.canonical_gap, node.x, node.y) / 32,
                               float(frame.start[cell + 1] - frame.start[cell]) / 64,
                               float(frame.lstart[cell + 1] - frame.lstart[cell]) / 8,
                               float(frame.rstart[cell + 1] - frame.rstart[cell]) / 64,
                               context.youkai,
                               context.bullets / 1536,
                               context.ghosts / 1536,
                               context.lasers / 64};
        for (auto &feature : features)
            feature = std::clamp(feature, -32.f, 32.f);
        return features;
    }
    // Storage belongs to this policy. Each search clears nodes and generation
    // marks before use; retained capacity carries no previous proposal state.
    struct SearchScratch {
        std::vector<std::vector<Node>> layers;
        detail::ProposalSet seen;
        plan::SpatialBeamScratch<Node> selection;
    } search_scratch;
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
    SearchResult previous_plan;
    std::uint64_t previous_plan_update = UINT64_MAX;
    // These continuations rank retained endpoints only; the normal H-update
    // branch budget and later partial-beam fallbacks keep their own contracts.
    int check_terminal_tail(const Node &node, const Speeds &sp) {
        ++search_stats.tail_nodes;
        int longest = 0;
        for (int focus = 0; focus < 2; ++focus)
            for (const auto direction : kDirs) {
                const auto action = std::uint16_t(direction | (focus ? 4 : 0));
                float x = node.x, y = node.y;
                ++search_stats.tail_paths;
                for (int step = 1; step <= opt.tail_check; ++step) {
                    move(sp, x, y, action);
                    ++search_stats.tail_updates;
                    if (probe(frames[opt.horizon + step], sp.half, x, y, false) == 2)
                        break;
                    longest = std::max(longest, step);
                }
                if (longest == opt.tail_check)
                    return longest;
            }
        return longest;
    }
    void ensure_index(int horizon) {
        if (horizon <= indexed_horizon)
            return;
        const bool timed = time_index_extensions && indexed_horizon > 0;
        const auto begin =
            timed ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point{};
        search_stats.indexed_forecast_frames += horizon - indexed_horizon;
        for (int j = indexed_horizon + 1; j <= horizon; ++j)
            build_index(frames[j], current_movement.half);
        indexed_horizon = horizon;
        if (timed)
            index_extension_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(
                                      std::chrono::steady_clock::now() - begin)
                                      .count();
    }
    SearchResult search(const std::vector<HFrame> &frames, int horizon, const Speeds &sp,
                        std::uint16_t latched, float x0, float y0, float pref_x, float pref_y,
                        const std::vector<std::uint16_t> *forced = nullptr, int beam = 0,
                        std::vector<ValueProposal> *proposals = nullptr) {
        if (beam == 0)
            beam = opt.beam;
        if (horizon + (horizon == opt.horizon ? opt.tail_check : 0) > indexed_horizon)
            throw std::runtime_error("unprepared_online_index");
        SpawnGrid summed_spawns;
        const SpawnGrid *nw = nullptr;
        if (risk.loaded && opt.nowarn_weight > 0) {
            auto it = risk.spawn.find(current_segment);
            if (it != risk.spawn.end()) {
                // This immutable grid is shared by every node in this search.
                // Rebuild at each decision so no phase/model cache can go stale.
                summed_spawns = spawn_neighborhood_sums(it->second);
                nw = &summed_spawns;
            }
        }
        auto &layers = search_scratch.layers;
        layers.resize(horizon + 1);
        for (auto &layer : layers)
            layer.clear();
        float x1 = x0, y1 = y0;
        move(sp, x1, y1, latched);
        layers[1].push_back({x1, y1, -1, std::uint16_t(latched & 0xF4), 0.f});
        if (proposal_style.graze_reward > 0)
            layers[1].back().graze_mask = graze_contacts(frames[1], x1, y1);
        if (probe(frames[1], sp.half, x1, y1, false) == 2)
            return {{std::uint16_t(latched & 0xF4)}, 0};
        // Every successor shares this immutable movement snapshot. Keep the
        // original float operations in the 18 deltas; clamp each node afterward.
        std::array<detail::MovementDelta, 18> deltas;
        for (int f = 0; f < 2; ++f)
            for (int d = 0; d < 9; ++d)
                deltas[f * 9 + d] =
                    detail::movement_delta(sp, std::uint16_t(kDirs[d] | (f ? 4 : 0)));
        detail::MovementDelta low{}, high{};
        for (const auto &delta : deltas) {
            low.x = std::min(low.x, delta.x);
            low.y = std::min(low.y, delta.y);
            high.x = std::max(high.x, delta.x);
            high.y = std::max(high.y, delta.y);
        }
        int deepest = 1;
        for (int j = 1; j < horizon && !layers[j].empty(); ++j) {
            auto &cur = layers[j];
            auto &next = layers[j + 1];
            // j=1 is movement two: the first input submitted at this snapshot.
            // Forced layers have one parent and cannot branch or draw tie RNG.
            if (forced && j <= int(forced->size())) {
                const auto &parent = cur.front();
                float x = parent.x, y = parent.y;
                const auto action = std::uint16_t((*forced)[j - 1] & 0xf4);
                move(sp, x, y, action);
                if (probe(frames[j + 1], sp.half, x, y, false) == 2)
                    break;
                next.push_back({x, y, 0, action, parent.score, parent.graze_mask});
                if (proposal_style.graze_reward > 0)
                    next.back().graze_mask |= graze_contacts(frames[j + 1], x, y);
                deepest = j + 1;
                continue;
            }
            Box bounds{cur.front().x, cur.front().y, cur.front().x, cur.front().y};
            for (const auto &node : cur) {
                bounds.x0 = std::min(bounds.x0, node.x);
                bounds.x1 = std::max(bounds.x1, node.x);
                bounds.y0 = std::min(bounds.y0, node.y);
                bounds.y1 = std::max(bounds.y1, node.y);
            }
            detail::apply_movement(sp, bounds.x0, bounds.y0, low);
            detail::apply_movement(sp, bounds.x1, bounds.y1, high);
            const bool next_repeats = j + 2 <= horizon && j + 1 >= 12 && ((j + 1) % 2) != 0;
            const bool retain_heading = opt.retain_headings && (next_repeats || j + 1 == horizon);
            search_scratch.seen.reset(bounds, retain_heading);
            search_stats.heading_layers += retain_heading;
            const bool branch = j < 12 || (j % 2) == 0;
            for (int pi = 0; pi < int(cur.size()); ++pi) {
                const Node &p = cur[pi];
                for (int f = 0; f < 2; ++f)
                    for (int d = 0; d < 9; ++d) {
                        const std::uint16_t a = std::uint16_t(kDirs[d] | (f ? 4 : 0));
                        if (!branch && a != p.act)
                            continue;
                        float x = p.x, y = p.y;
                        detail::apply_movement(sp, x, y, deltas[f * 9 + d]);
                        if (!search_scratch.seen.first(x, y, f != 0, unsigned(d)))
                            continue;
                        search_stats.heading_keys += retain_heading;
                        const int hit = probe(frames[j + 1], sp.half, x, y, true);
                        if (hit == 2)
                            continue;
                        Node n{x, y, pi, a, p.score, p.graze_mask};
                        const float dx = (x - pref_x) / 96.f, dy = (y - pref_y) / 96.f;
                        if ((risk.loaded || opt.spawn_guard > 0 || opt.queued_shots) &&
                            probe_risk(frames[j + 1], sp.half, x, y))
                            n.score += opt.risk_weight * (j < 16 ? 1.f : 0.5f);
                        if (nw) {
                            // expected no-warning spawns in the 3x3 cells around the node
                            const int cx = std::clamp(int(x / 16.f), 0, 23),
                                      cy = std::clamp(int(y / 16.f), 0, 27);
                            const float r = (*nw)[cy * 24 + cx];
                            n.score += opt.nowarn_weight * r * (j < 16 ? 1.f : 0.25f);
                        }
                        if (!(a & 4))
                            n.score += proposal_focus_bias;
                        const auto &w = proposal_style;
                        n.score += (hit == 1 ? (j < w.margin_split ? w.margin_early : w.margin_late)
                                             : 0.f) +
                                   w.anchor_weight * (dx * dx + dy * dy) +
                                   (a != p.act ? w.change_cost : 0.f) +
                                   float(xrand() % 1000) * 1e-6f;
                        if (w.move_cost || w.focused_move_cost || w.reversal_cost || w.edge_cost ||
                            w.graze_reward)
                            n.score += style_cost(p, n, sp, frames[j + 1]);
                        next.push_back(n);
                    }
            }
            search_stats.proposal_grid_expansions += search_scratch.seen.expansions();
            if (int(next.size()) > beam) {
                if (opt.diversity_cell > 0) {
                    const auto begin = std::chrono::steady_clock::now();
                    const auto stats = plan::retain_spatial_beam_bounded(
                        next, beam, opt.diversity_cell, {sp.minx, sp.miny, sp.maxx, sp.maxy},
                        search_scratch.selection);
                    search_stats.selection_ms += std::chrono::duration<double, std::milli>(
                                                     std::chrono::steady_clock::now() - begin)
                                                     .count();
                    ++search_stats.selections;
                    search_stats.candidates += stats.candidates;
                    search_stats.bins += stats.bins;
                    search_stats.retained_bins += stats.retained_bins;
                } else {
                    std::nth_element(
                        next.begin(), next.begin() + beam, next.end(),
                        [](const Node &a, const Node &b) { return a.score < b.score; });
                    next.resize(beam);
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
        if (value_model.loaded && opt.value_weight > 0 && deepest == horizon) {
            const auto begin = std::chrono::steady_clock::now();
            double minimum = std::numeric_limits<double>::infinity();
            for (int i = 0; i < int(last.size()); ++i) {
                const auto probability = value_model.probability(value_features(last[i], sp));
                const double score =
                    last[i].score - opt.value_weight * std::log(std::max(probability, 1e-4));
                ++search_stats.value_evaluations;
                if (score < minimum)
                    minimum = score, best = i;
            }
            search_stats.value_ms +=
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin)
                    .count();
        }
        if (proposals && deepest == horizon) {
            std::vector<int> order(last.size());
            for (int i = 0; i < int(last.size()); ++i)
                order[i] = i;
            std::stable_sort(order.begin(), order.end(),
                             [&](int a, int b) { return last[a].score < last[b].score; });
            const auto count = std::min<std::size_t>(4, last.size());
            for (std::size_t sample = 0; sample < count; ++sample) {
                const auto index =
                    order[count == 1 ? 0 : sample * (order.size() - 1) / (count - 1)];
                ValueProposal proposal;
                proposal.features = value_features(last[index], sp);
                int cursor = index;
                for (int j = deepest; j > 1; --j) {
                    proposal.inputs.push_back(
                        std::uint16_t(layers[j][cursor].act |
                                      ((preparing_collection || attack.farming) ? 0 : 1)));
                    cursor = layers[j][cursor].parent;
                }
                std::reverse(proposal.inputs.begin(), proposal.inputs.end());
                proposals->push_back(std::move(proposal));
            }
        }
        if (opt.tail_check && deepest == horizon && horizon == opt.horizon) {
            const auto begin = std::chrono::steady_clock::now();
            std::vector<int> order;
            for (int i = 0; i < int(last.size()); ++i)
                order.push_back(i);
            std::stable_sort(order.begin(), order.end(),
                             [&](int a, int b) { return last[a].score < last[b].score; });
            int tail_depth = -1, selected = best;
            for (const int index : order) {
                const auto depth = check_terminal_tail(last[index], sp);
                if (depth > tail_depth)
                    tail_depth = depth, selected = index;
                if (depth == opt.tail_check)
                    break;
            }
            search_stats.tail_best_depth = unsigned(tail_depth);
            search_stats.tail_selected = selected != best;
            best = selected;
            search_stats.tail_selection_ms +=
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin)
                    .count();
        }
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

    struct FollowedPlan {
        std::vector<std::uint16_t> moves;
        std::size_t offset = 0;
        bool present() const {
            return offset < moves.size();
        }
        std::uint16_t input(std::size_t ahead = 0) const {
            return moves[std::min(offset + ahead, moves.size() - 1)];
        }
    };
    struct PendingPlan {
        std::vector<std::uint16_t> moves, forced;
        std::uint64_t origin = 0, due = 0, epoch = 0;
        Vector arrival;
        bool present = false;
    };
    struct LagExecutor {
        FollowedPlan active;
        PendingPlan pending;
        std::uint64_t epoch = 0, free_at = 0, last_update = 0;
        bool anchored = false;
        int stage = 0, spell = -1;
        Movement movement;
        Vector expected;
        std::uint16_t last_input = 0;
    } executor;
    static bool same_bits(float a, float b) {
        return std::memcmp(&a, &b, sizeof(float)) == 0;
    }
    static bool same_movement(const Movement &a, const Movement &b) {
        const float av[] = {a.axis[0], a.axis[1], a.diag[0], a.diag[1], a.hmul,   a.vmul,  a.frm,
                            a.minx,    a.miny,    a.maxx,    a.maxy,    a.half.x, a.half.y};
        const float bv[] = {b.axis[0], b.axis[1], b.diag[0], b.diag[1], b.hmul,   b.vmul,  b.frm,
                            b.minx,    b.miny,    b.maxx,    b.maxy,    b.half.x, b.half.y};
        for (std::size_t i = 0; i < sizeof(av) / sizeof(av[0]); ++i)
            if (!same_bits(av[i], bv[i]))
                return false;
        return true;
    }
    void drop_pending() {
        if (executor.pending.present)
            ++search_stats.pending_dropped;
        executor.pending.present = false;
        executor.pending.moves.clear();
        executor.pending.forced.clear();
    }
    // Revalidate the latch and every followed movement using this observation's
    // freshly built forecast. Beyond a finite proposal, hold its final input.
    int checked_depth(const std::vector<std::uint16_t> &moves, std::size_t offset,
                      const Observation &o) {
        ensure_index(opt.executor_check);
        ++search_stats.executor_checks;
        float x = o.x, y = o.y;
        int depth = 0;
        for (int step = 1; step <= opt.executor_check; ++step) {
            const auto input =
                step == 1 ? o.latched_input
                          : moves[std::min(offset + std::size_t(step - 2), moves.size() - 1)];
            move(o.movement, x, y, input);
            if (probe(frames[step], o.movement.half, x, y, false) == 2)
                break;
            depth = step;
        }
        return depth;
    }
    struct ExecutedInput {
        std::uint16_t action;
        int depth;
    };
    ExecutedInput lag_decide(const Observation &o, float pref_x) {
        auto &a = executor;
        auto &p = a.pending;
        const auto u = o.update;
        const int spell = o.spell_active ? o.spell : -1;
        if (a.anchored) {
            const bool matches = u == a.last_update + 1 && o.stage == a.stage && spell == a.spell &&
                                 same_movement(o.movement, a.movement) &&
                                 (o.latched_input & 0xf7) == std::uint16_t(a.last_input | 1) &&
                                 same_bits(o.x, a.expected.x) && same_bits(o.y, a.expected.y);
            if (!matches) {
                ++search_stats.executor_invalidations;
                ++a.epoch;
                drop_pending();
                a.active.moves.clear();
                a.active.offset = 0;
                a.free_at = u;
            } else if (a.active.present()) {
                ++a.active.offset;
            }
        }
        if (p.present) {
            const auto elapsed = u - p.origin;
            // Check the inputs actually latched, not merely those we proposed.
            // Emergency/lifecycle replacement also changes the owning epoch.
            const bool prefix_matches =
                p.epoch == a.epoch && elapsed > 0 && elapsed <= p.forced.size() &&
                (o.latched_input & 0xf7) == std::uint16_t(p.forced[elapsed - 1] | 1);
            if (!prefix_matches) {
                drop_pending();
                a.free_at = u;
            }
        }
        int checked = a.active.present() ? checked_depth(a.active.moves, a.active.offset, o) : -1;
        if (p.present && u >= p.due) {
            const bool arrived =
                u == p.due && same_bits(o.x, p.arrival.x) && same_bits(o.y, p.arrival.y);
            const int candidate = arrived ? checked_depth(p.moves, 0, o) : -1;
            if (candidate >= opt.executor_check) {
                a.active.moves = std::move(p.moves);
                a.active.offset = 0;
                p.present = false;
                p.forced.clear();
                checked = candidate;
                ++search_stats.plan_adoptions;
            } else {
                drop_pending();
            }
        }
        if (!a.active.present() || checked < opt.executor_check) {
            ++search_stats.emergency_searches;
            ++a.epoch;
            drop_pending();
            a.free_at = u;
            auto rescue = search(frames, opt.emergency_horizon, o.movement, o.latched_input, o.x,
                                 o.y, pref_x, opt.style.anchor_y, nullptr, opt.emergency_beam);
            // A bounded rescue may be shallower than the plan just checked.
            if (!a.active.present() || rescue.depth >= checked) {
                a.active.moves = std::move(rescue.moves);
                a.active.offset = 0;
                checked = rescue.depth;
            }
        }
        if (!p.present && u >= a.free_at && checked >= 2) {
            std::vector<std::uint16_t> forced;
            forced.reserve(opt.planner_lag);
            for (int i = 0; i < opt.planner_lag; ++i)
                forced.push_back(a.active.input(std::size_t(i)));
            // Shared indexing is executor work; only the following full search
            // is subtracted by the deterministic latency model.
            ensure_index(opt.horizon);
            const auto begin = std::chrono::steady_clock::now();
            auto full = search(frames, opt.horizon, o.movement, o.latched_input, o.x, o.y, pref_x,
                               opt.style.anchor_y, &forced);
            search_stats.planner_ms =
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin)
                    .count();
            ++search_stats.planner_searches;
            a.free_at = u + std::uint64_t(opt.planner_lag);
            if (full.moves.size() > forced.size()) {
                p.moves.assign(full.moves.begin() + std::ptrdiff_t(forced.size()),
                               full.moves.end());
                p.arrival =
                    detail::prefix_arrival(o.movement, {o.x, o.y, 0}, o.latched_input, forced);
                p.forced = std::move(forced);
                p.origin = u;
                p.due = a.free_at;
                p.epoch = a.epoch;
                p.present = true;
            }
        }
        const auto action = a.active.input();
        a.anchored = true;
        a.stage = o.stage;
        a.spell = spell;
        a.movement = o.movement;
        a.last_update = u;
        a.last_input = action;
        a.expected = {o.x, o.y, 0};
        move(o.movement, a.expected.x, a.expected.y, o.latched_input);
        return {action, std::min(checked, opt.executor_check)};
    }

    void check_warm_path(SearchResult &result, const Observation &observation) {
        if (!opt.warm_fallback || result.depth == 0 || result.depth == opt.horizon ||
            previous_plan.moves.empty())
            return;
        if (previous_plan_update == UINT64_MAX || observation.update != previous_plan_update + 1 ||
            (observation.latched_input & 0xf4) != previous_plan.moves.front()) {
            ++search_stats.warm_invalidated;
            return;
        }
        ++search_stats.warm_paths;
        float x = observation.x, y = observation.y;
        SearchResult candidate;
        // moves[0] was sampled last time and is now latched: it owns update one.
        // Shift the remaining proposal and repeat its last input to fill the new
        // horizon. Every position is checked again; the old forecast is not reused.
        for (int update = 1; update <= opt.horizon; ++update) {
            const auto action =
                update == 1
                    ? observation.latched_input
                    : previous_plan
                          .moves[std::min(std::size_t(update - 1), previous_plan.moves.size() - 1)];
            move(observation.movement, x, y, action);
            ++search_stats.warm_updates;
            if (probe(frames[update], observation.movement.half, x, y, false) == 2)
                break;
            candidate.depth = update;
            if (update > 1)
                candidate.moves.push_back(action);
        }
        if (candidate.depth > result.depth) {
            // Keep the beam on depth ties, without consuming another tie RNG draw.
            result = std::move(candidate);
            ++search_stats.warm_selected;
        }
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

    Decision choose(const Observation &observation, DecisionTiming *timing,
                    std::vector<ValueProposal> *proposals) {
        if (proposals) {
            proposals->clear();
            if (opt.horizon != int(kValueHorizon) || opt.planner_lag || opt.tail_check ||
                opt.warm_fallback || opt.held_fallback)
                throw std::runtime_error("unsupported_branch_proposal_configuration");
        }
        using Timer = std::chrono::steady_clock;
        if (timing)
            *timing = {};
        auto stamp = timing ? Timer::now() : Timer::time_point{};
        const auto mark = [&](std::uint64_t DecisionTiming::*field) {
            if (timing) {
                const auto end = Timer::now();
                timing->*field =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(end - stamp).count();
                stamp = end;
            }
        };
        if (last_decided == observation.update)
            throw std::runtime_error("duplicate_online_decision");
        observe(observation);
        last_decided = observation.update;
        search_stats = {};
        if (opt.scheduled_expiry)
            search_stats.unsupported_lifetimes = observation.unsupported_lifetimes;
        indexed_horizon = 0;
        index_extension_ns = 0;
        time_index_extensions = timing != nullptr;
        current_movement = observation.movement;
        current_segment = observation.spell_active ? observation.spell : 300 + observation.stage;
        if ((value_model.loaded && opt.value_weight > 0) || proposals) {
            const auto begin = std::chrono::steady_clock::now();
            value_context.prepare(observation);
            search_stats.value_ms +=
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin)
                    .count();
        }
        mark(&DecisionTiming::history_ns);
        const int forecast_horizon = opt.horizon + opt.tail_check;
        kinematic_forecast(observation, forecast_horizon, frames);
        mark(&DecisionTiming::forecast_ns);
        const auto &sp = observation.movement;
        if (opt.reach_filter)
            prune_unreachable(frames, forecast_horizon, sp, observation.x, observation.y);
        const bool pending_only = opt.planner_lag > 0 && opt.lazy_index &&
                                  executor.pending.present &&
                                  observation.update < executor.pending.due;
        ensure_index(pending_only ? std::max(opt.executor_check, opt.emergency_horizon)
                                  : forecast_horizon);
        mark(&DecisionTiming::index_ns);
        float pref_x = opt.style.anchor_x, area = 0;
        for (const auto &r : observation.lethal_regions) {
            const float a = (r.x1 - r.x0) * (r.y1 - r.y0);
            if (a > area && r.y1 < 300)
                area = a, pref_x = opt.style.anchor_x +
                                   opt.boss_follow * ((r.x0 + r.x1) / 2 - opt.style.anchor_x);
        }
        pref_x = std::clamp(pref_x, opt.style.anchor_x_min, opt.style.anchor_x_max);
        float pref_y = opt.style.anchor_y;
        if (opt.scoring_upper_stage && !observation.scoring.has_boss)
            pref_y = collection_goal_y(observation);
        if (opt.scoring_aim && observation.scoring.has_boss)
            pref_x = std::clamp(observation.scoring.boss_x, sp.minx, sp.maxx);
        if (opt.scoring_hit_income && !observation.scoring.has_boss) {
            float best = 0;
            for (const auto &target : observation.scoring.income_targets) {
                const auto y = target.y + 64;
                if (target.reduction_updates < 24 || target.x < sp.minx || target.x > sp.maxx ||
                    target.y < 0 || y > sp.maxy)
                    continue;
                const auto distance = std::hypot(target.x - observation.x, y - observation.y);
                const float value = std::min(120, target.reduction_updates) / (64.f + distance);
                if (value > best) {
                    best = value;
                    pref_x = target.x;
                    pref_y = std::max(collection_goal_y(observation), y);
                }
            }
        }
        if (collecting) {
            pref_x = std::clamp(collection_x, sp.minx, sp.maxx);
            pref_y = collection_goal_y(observation);
        }
        if (flow_active) {
            pref_x = std::clamp(flow_x, sp.minx, sp.maxx);
            pref_y = std::clamp(flow_y, sp.miny, sp.maxy);
        }
        if (opt.planner_lag > 0) {
            const auto executed = lag_decide(observation, pref_x);
            mark(&DecisionTiming::search_ns);
            if (timing) {
                timing->index_ns += index_extension_ns;
                timing->search_ns -= index_extension_ns;
            }
            mark(&DecisionTiming::fallback_ns);
            return {std::uint16_t(executed.action | 1), executed.depth, search_stats};
        }
        auto result = search(frames, opt.horizon, sp, observation.latched_input, observation.x,
                             observation.y, pref_x, pref_y, nullptr, 0, proposals);
        mark(&DecisionTiming::search_ns);
        check_warm_path(result, observation);
        check_held_directions(result, observation);
        if (opt.warm_fallback) {
            previous_plan = result;
            previous_plan_update = observation.update;
        }
        mark(&DecisionTiming::fallback_ns);
        return {std::uint16_t(result.moves.front() |
                              ((preparing_collection || attack.farming) ? 0 : 1)),
                result.depth, search_stats};
    }
};
Policy::Policy(Configuration configuration, RiskTables risk, BranchValueModel value)
    : impl_(new Impl(configuration, std::move(risk), std::move(value))) {}
Policy::~Policy() = default;
void Policy::observe(const Observation &observation) {
    impl_->observe(observation);
}
bool Policy::wants_fire() const {
    return !impl_->preparing_collection && !impl_->attack.farming;
}
bool Policy::needs_scoring_memory() const {
    return impl_->opt.scoring_collection || impl_->opt.scoring_aim ||
           impl_->opt.scoring_survival_focus || impl_->opt.scoring_survival_flow ||
           impl_->opt.scoring_attack_pacing || impl_->opt.scoring_hit_income ||
           impl_->opt.economic_human_bias > 0;
}
bool Policy::needs_hit_income_memory() const {
    return impl_->opt.scoring_hit_income;
}
Decision Policy::decide(const Observation &observation, DecisionTiming *timing,
                        std::vector<ValueProposal> *proposals) {
    return impl_->choose(observation, timing, proposals);
}
} // namespace th08::online
