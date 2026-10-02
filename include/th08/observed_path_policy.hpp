#pragma once
#include "native_policy.hpp"
#include <stdexcept>

namespace th08::policy {
// This bounded proposal tree owns only player coordinates. Immutable observed
// bullet projections are shared; no candidate owns or predicts a native world.
// Stable sorting retains the existing comparator and generation-order full ties.
// Seven prefixes per first action preserve all nine families, at most 63 nodes.
// The diagnostic continuation_action is the LAST action of a retained prefix,
// not a two-leg command or a tape intended for open-loop native execution.
template <class Bullets, class Lasers>
std::uint16_t observed_path_beam(float px, float py, float hx, float hy, float axis, float diagonal,
                                 std::uint16_t pending, const Bullets &bullets,
                                 const Lasers &lasers, HazardReactiveStats &stats,
                                 HazardReactiveOptions options = {},
                                 HazardReactiveDecision *decision = nullptr) {
    if (!lasers.empty())
        throw std::runtime_error("observed path beam does not support lasers");
    if (options.bullet_horizon != 32 || options.candidate_mask != 0x1ff)
        throw std::runtime_error("observed path beam requires H32 and all nine first actions");
    ++stats.decisions;
    if (decision)
        *decision = {};
    struct Hazard {
        float x, y, w, h;
        bool hard;
    };
    std::vector<std::vector<Hazard>> cached(options.bullet_horizon + 1);
    for (unsigned step = 2; step <= options.bullet_horizon; ++step)
        for (const auto &b : bullets) {
            if (b.state == 5 || b.state == 6)
                continue;
            const auto p = detail::project_bullet(
                b, step, options.vector_acceleration, options.wait_linear_projection,
                options.relative_direction_projection, options.boundary_bounce_projection,
                options.wait_vector_projection);
            ++stats.bullet_projections;
            if (p.kind == detail::BulletProjectionKind::vector_acceleration)
                ++stats.vector_acceleration_checks;
            else if (p.kind == detail::BulletProjectionKind::wait_vector)
                ++stats.wait_vector_checks;
            else if (p.kind == detail::BulletProjectionKind::unsupported)
                ++stats.unsupported_transform_checks;
            cached[step].push_back(
                {p.x, p.y, b.full_width, b.full_height,
                 b.state == 1 && p.kind != detail::BulletProjectionKind::unsupported});
        }
    struct Node {
        float x, y;
        detail::CandidateScore score;
    };
    detail::advance(px, py, detail::direction(pending), axis, diagonal);
    std::vector<Node> beam{{px,
                            py,
                            {std::max(options.bullet_horizon, options.laser_horizon) + 1,
                             std::numeric_limits<float>::infinity(), 0, 0, 4, 4}}};
    for (unsigned step = 2; step <= options.bullet_horizon; ++step) {
        std::vector<Node> next;
        next.reserve(beam.size() * 9);
        for (const auto &parent : beam)
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                    if (step == 2 && !(options.candidate_mask & (1u << ((dy + 1) * 3 + dx + 1))))
                        continue;
                    ++stats.candidates;
                    Node n = parent;
                    const detail::Direction d{dx, dy};
                    detail::advance(n.x, n.y, d, axis, diagonal);
                    const auto action = detail::input(d);
                    if (step == 2) {
                        n.score.action = action;
                        const double cx = n.x - 192, cy = n.y - 380;
                        n.score.center_distance = cx * cx + cy * cy;
                    }
                    n.score.continuation_action = action;
                    for (const auto &b : cached[step]) {
                        ++stats.bullet_checks;
                        float c = detail::box_clearance(n.x, n.y, hx, hy, b.x, b.y, b.w, b.h);
                        n.score.minimum_clearance = std::min(n.score.minimum_clearance, c);
                        const double pos = std::max(0.f, c);
                        n.score.danger += 1 / ((pos + 1) * (pos + 1) * step);
                        if (c <= 0 && b.hard) {
                            n.score.first_overlap = std::min(n.score.first_overlap, step);
                            ++stats.predicted_overlaps;
                        }
                    }
                    next.push_back(n);
                }
        std::stable_sort(next.begin(), next.end(), [](const Node &a, const Node &b) {
            return detail::better(a.score, b.score);
        });
        std::array<unsigned, 9> kept{};
        std::vector<Node> diverse;
        diverse.reserve(63);
        for (const auto &n : next) {
            const auto d = detail::direction(n.score.action);
            const unsigned family = (d.y + 1) * 3 + d.x + 1;
            if (kept[family] < 7) {
                diverse.push_back(n);
                ++kept[family];
            }
        }
        beam = std::move(diverse);
    }
    const auto &best = beam.front().score;
    if (decision) {
        decision->selected_action = best.action;
        for (const auto &n : beam) {
            const auto d = detail::direction(n.score.action);
            auto &c = decision->candidates[(d.y + 1) * 3 + d.x + 1];
            if (!c.enabled)
                c = {true,
                     n.score.action,
                     n.score.continuation_action,
                     n.score.first_overlap,
                     n.score.minimum_clearance,
                     n.score.danger,
                     n.score.center_distance};
        }
    }
    return best.action;
}
} // namespace th08::policy
