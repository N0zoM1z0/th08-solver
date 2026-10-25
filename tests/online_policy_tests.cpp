#include "test_support.hpp"
#include "th08/ecl_turn_hint.hpp"
#include "th08/geometry.hpp"
#include "th08/online.hpp"
#include "th08/online_child_birth.hpp"
#include "th08/online_collision.hpp"
#include "th08/online_dedup.hpp"
#include "th08/online_lifetime.hpp"
#include "th08/online_motion_envelope.hpp"
#include "th08/online_movement.hpp"
#include "th08/online_plan.hpp"
#include "th08/online_rotation.hpp"

#include <cerrno>
#include <cfenv>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <unordered_set>

namespace fs = std::filesystem;
namespace on = th08::online;
using th08::test::check;
namespace {
template <class F> void rejects(F action, const char *message) {
    bool rejected = false;
    try {
        action();
    } catch (const std::runtime_error &) {
        rejected = true;
    }
    check(rejected, message);
}
void write(const fs::path &file, const std::string &contents) {
    std::ofstream output(file);
    output.exceptions(std::ios::badbit | std::ios::failbit);
    output << contents;
}
void risk_rows(const fs::path &root) {
    fs::remove_all(root);
    fs::create_directories(root);
    write(root / "risk_table_merged.tsv", "seg\tsprite\tsize10\tsb\tab\th\tn\tq99\tq999\tp_gt2\n"
                                          "88\t4\t60\t1\t2\t4\t200\t3\t5\t0.2\n");
    write(root / "risk_backoff_seg.tsv", "seg\tsb\tab\th\tn\tq99\n"
                                         "88\t1\t2\t4\t200\t1\n88\t1\t2\t8\t200\t2\n"
                                         "88\t1\t2\t16\t200\t4\n88\t1\t2\t32\t200\t8\n");
    const std::string global = "sb\tab\th\tn\tq99\n"
                               "1\t2\t4\t1\t1\n1\t2\t8\t1\t2\n"
                               "1\t2\t16\t1\t4\n1\t2\t32\t1\t8\n";
    write(root / "risk_backoff_global.tsv", global);
    write(root / "nowarn_merged.tsv", "seg\tcx\tcy\tcount\trate\n88\t2\t3\t10\t0.25\n");
    const auto risk = on::load_risk_tables(root.string());
    const std::array<float, 4> expected{1, 2, 4, 8};
    check(risk.loaded && risk.segment.at(on::risk_key(88, 0, 0, 1, 2)) == expected &&
              risk.global.at(on::risk_key(0, 0, 0, 1, 2)) == expected,
          "backoff parser swallowed alternate horizon rows");
    check(risk.fine.at(on::risk_key(88, 4, 60, 1, 2))[0] == 3 &&
              risk.spawn.at(88)[3 * 24 + 2] == .25f,
          "fine or spawn columns shifted");
    write(root / "risk_backoff_global.tsv", "sb\tab\th\tn\tq99\n1\t2\t4\t1\n1\t2\t8\t1\t2\n");
    rejects([&] { on::load_risk_tables(root.string()); }, "truncated row consumed the next row");
    write(root / "risk_backoff_global.tsv", global + "1\t2\t4\t1\t9\n");
    rejects([&] { on::load_risk_tables(root.string()); }, "duplicate model key/horizon accepted");
    fs::remove_all(root);
}
void spawn_neighborhood_contract() {
    on::SpawnGrid source{};
    source[0] = 1;
    source[1] = 2;
    source[24] = 4;
    source[25] = 8;
    source.back() = 16;
    for (int i = 0; i < 9; ++i)
        source[(9 + i / 3) * 24 + 9 + i % 3] = 1;
    source[9 * 24 + 9] = 100000000.f;
    const auto sums = on::spawn_neighborhood_sums(source);
    check(sums[0] == 15 && sums[2] == 10 && sums[48] == 12 && sums.back() == 16 && sums[23] == 0 &&
              sums[27 * 24] == 0,
          "spawn neighborhoods duplicate clipped edges or wrap between rows");
    // Eight unit terms round away when added individually after the large term.
    // A reassociated/running sum can incorrectly retain them as one extra ULP.
    check(sums[10 * 24 + 10] == 100000000.f,
          "spawn neighborhood changed the original row-major float sum");
    on::SpawnGrid zero{};
    zero[0] = -0.f;
    const auto empty = on::spawn_neighborhood_sums(zero);
    check(empty[0] == 0 && !std::signbit(empty[0]),
          "spawn neighborhood changed the original positive-zero accumulator");
}
void saved_risk_tables(const fs::path &root) {
    for (const auto *name : {"teacher-near-v1", "teacher-all-v1", "teacher-plus-student-v1"}) {
        const auto model = on::load_risk_tables((root / name).string());
        check(model.loaded && !model.fine.empty() && !model.segment.empty() &&
                  !model.global.empty() && !model.spawn.empty(),
              "a saved risk snapshot no longer loads through the production parser");
    }
}
on::Observation scene() {
    on::Observation o;
    o.x = 192;
    o.y = 400;
    o.movement.axis[0] = 4;
    o.movement.axis[1] = 2;
    o.movement.diag[0] = 2.828427f;
    o.movement.diag[1] = 1.4142135f;
    o.movement.minx = 8;
    o.movement.miny = 16;
    o.movement.maxx = 376;
    o.movement.maxy = 432;
    o.movement.half = {1, 1, 0};
    return o;
}
on::Bullet bullet(int slot, float x, float y, float vx, float vy) {
    on::Bullet b;
    b.slot = slot;
    b.state = 1;
    b.x = x;
    b.y = y;
    b.vx = vx;
    b.vy = vy;
    b.full_width = b.full_height = 2;
    return b;
}
void policy_style_contract() {
    on::Configuration base;
    base.horizon = 24;
    base.beam = 73;
    base.reach_filter = true;
    const auto sniper = on::with_style("sniper", base);
    check(sniper.horizon == 24 && sniper.beam == 73 && sniper.reach_filter,
          "style changed search dimensions or collision configuration");
    auto empty = scene();
    on::Policy offense(sniper);
    check(offense.decide(empty).action & 16,
          "sniper preference did not move toward its higher anchor");
    rejects([&] { on::with_style("snipre"); }, "unknown style silently selected a default");
    auto invalid = base;
    invalid.style.graze_reward = std::numeric_limits<float>::quiet_NaN();
    rejects([&] { on::Policy bad(invalid); }, "nonfinite style cost accepted");
    // Contact during the immutable first movement belongs to every path. A
    // bullet may not pay repeatedly for staying in, leaving or reentering its
    // graze box later in this forecast. This was absent from the reviewed proxy.
    auto contact = scene();
    contact.bullets.push_back(bullet(0, 212, 400, 0, 0));
    auto greedy = base;
    greedy.style.graze_reward = 100;
    for (bool already_grazed : {false, true}) {
        contact.bullets.front().grazed = already_grazed;
        on::Policy plain(base), rewarded(greedy);
        std::vector<on::ValueProposal> a, b;
        const auto da = plain.decide(contact, nullptr, &a);
        const auto db = rewarded.decide(contact, nullptr, &b);
        check(da.action == db.action && da.depth == db.depth && a.size() == b.size(),
              "graze proxy rewarded an already consumed contact");
        for (std::size_t i = 0; i < a.size(); ++i)
            check(a[i].inputs == b[i].inputs,
                  "repeated graze proxy changed retained proposal ranking");
    }
}
void legacy_style_signature() {
    // Pinned against the unchanged 13778ba policy, including all exported
    // proposals and tie draws over a moving synthetic current-memory corpus.
    on::Configuration c;
    c.horizon = 24;
    c.beam = 192;
    c.diversity_cell = 16;
    c.spawn_guard = 16;
    c.hard_phantoms = c.motion_envelope = c.reach_filter = true;
    on::Policy policy(c);
    auto v = scene();
    std::uint64_t signature = 1469598103934665603ull;
    const auto mix = [&](std::uint64_t a) {
        signature ^= a;
        signature *= 1099511628211ull;
    };
    for (unsigned update = 0; update < 96; ++update) {
        v.update = update;
        v.bullets.clear();
        v.lethal_regions = {{float(60 + update % 180), 60, float(92 + update % 180), 92}};
        for (int slot = 0; slot < 20; ++slot) {
            auto b =
                bullet(slot, float(22 + slot * 17), float(180 + (slot * 13 + update * 2) % 240),
                       float(slot % 3 - 1) * .2f, 2);
            b.full_width = b.full_height = 4;
            v.bullets.push_back(b);
        }
        std::vector<on::ValueProposal> proposals;
        const auto d = policy.decide(v, nullptr, &proposals);
        mix(d.action);
        mix(d.depth);
        mix(proposals.size());
        for (const auto &a : proposals)
            for (auto input : a.inputs)
                mix(input);
        on::detail::apply_movement(v.movement, v.x, v.y,
                                   on::detail::movement_delta(v.movement, v.latched_input));
        v.latched_input = d.action;
    }
    check(signature == 0xeca066c1adb5a367ull,
          "default style changed legacy proposals or tie sequence");
}
void score_goal_contract() {
    auto config = on::with_style("score");
    config.horizon = 24;
    auto o = scene();
    on::Policy missing(config);
    rejects([&] { missing.decide(o); }, "score control accepted absent current-memory inputs");
    o.scoring.available = o.scoring.auto_collect_eligible = true;
    o.scoring.collection_line = 128;
    o.scoring.pickups.assign(4, {192, 250, on::PickupKind::Point});
    on::Policy policy(config);
    check(policy.needs_scoring_memory() && (policy.decide(o).action & 16),
          "visible item batch did not generate a collection trip");
    ++o.update;
    o.scoring.pickups.pop_back();
    check(policy.decide(o).action & 16, "collection trip ended before its final items arrived");
    ++o.update;
    o.scoring.has_boss = true;
    o.scoring.boss_x = 300;
    check(policy.decide(o).action & 128, "boss arrival did not end collection and restore aiming");
    config.planner_lag = 1;
    rejects([&] { on::Policy bad(config); },
            "score goal silently used a fixed-height delayed plan");
}
void score_form_contract() {
    auto config = on::with_style("score-survival");
    auto o = scene();
    o.scoring.available = o.scoring.can_switch_forms = o.scoring.has_boss = true;
    o.scoring.boss_x = o.x;
    o.spell_active = o.scoring.survival_spell = true;
    on::Policy spell(config);
    check(spell.decide(o).action & 4, "spell economy did not prefer the youkai form");
    o.scoring.survival_spell = false;
    on::Policy nonspell(config);
    on::Policy baseline(on::with_style("score"));
    check(nonspell.decide(o).action == baseline.decide(o).action,
          "survival preference changed an ordinary attack");
    config.planner_lag = 1;
    rejects([&] { on::Policy bad(config); }, "form economy accepted the delayed executor");
}
void committed_prefix_contract() {
    auto o = scene();
    o.x = o.y = 100;
    o.latched_input = 128;
    const std::vector<std::uint16_t> committed{32, 64};
    const auto zero = on::detail::prefix_arrival(o.movement, {o.x, o.y, 0}, o.latched_input, {});
    const auto one = on::detail::prefix_arrival(o.movement, {o.x, o.y, 0}, o.latched_input, {32});
    const auto two =
        on::detail::prefix_arrival(o.movement, {o.x, o.y, 0}, o.latched_input, committed);
    check(zero.x == 100 && zero.y == 100 && one.x == 104 && one.y == 100 && two.x == 104 &&
              two.y == 104,
          "committed prefix moved its final input before adoption or skipped the old latch");
}
void planner_lag_contract() {
    on::Configuration config;
    config.horizon = 24;
    config.beam = 64;
    config.grow = 0;
    config.planner_lag = 2;
    config.executor_check = 12;
    config.emergency_horizon = 12;
    config.emergency_beam = 16;
    auto advance = [](on::Observation &o, std::uint16_t action) {
        on::detail::apply_movement(o.movement, o.x, o.y,
                                   on::detail::movement_delta(o.movement, o.latched_input));
        o.latched_input = action;
        ++o.update;
        for (auto &b : o.bullets)
            b.x += b.vx, b.y += b.vy;
    };
    on::Policy policy(config);
    auto o = scene();
    const auto d0 = policy.decide(o);
    check(d0.search.emergency_searches == 1 && d0.search.planner_searches == 1 &&
              d0.search.plan_adoptions == 0 && d0.depth == config.emergency_horizon,
          "lag execution did not bootstrap an emergency plan and delayed search");
    auto shorter_check = config;
    shorter_check.executor_check = 4;
    const auto capped = on::Policy(shorter_check).decide(scene());
    check(capped.depth == 4 && capped.search.emergency_searches == 1,
          "a longer emergency search overstated the executor's configured check depth");
    auto longer_check = config;
    longer_check.executor_check = longer_check.horizon;
    longer_check.queued_shots = true;
    auto normal_check = longer_check;
    normal_check.executor_check = normal_check.emergency_horizon;
    on::Policy longer(longer_check), unchanged(normal_check);
    auto boundary = scene();
    const auto previous = longer.decide(boundary), retained = unchanged.decide(boundary);
    check(previous.action == retained.action, "deeper-check fixture changed its initial plan");
    advance(boundary, previous.action);
    boundary.queued_shots.push_back({20, 1, boundary.x, boundary.y, 0, 0, 10000, 10000});
    const auto deeper = longer.decide(boundary), old = unchanged.decide(boundary);
    check(deeper.depth == 19 && deeper.action == old.action &&
              deeper.search.emergency_searches == 1 && deeper.search.pending_dropped == 1,
          "a shorter emergency replaced a deeper already checked plan");
    advance(o, d0.action);
    const auto d1 = policy.decide(o);
    check(d1.search.executor_checks == 1 && d1.search.emergency_searches == 0 &&
              d1.search.planner_searches == 0 && d1.search.plan_adoptions == 0,
          "a pending plan was adopted early or a valid executor searched unnecessarily");
    advance(o, d1.action);
    const auto d2 = policy.decide(o);
    check(d2.search.plan_adoptions == 1 && d2.search.planner_searches == 1 &&
              d2.search.emergency_searches == 0 && d2.search.executor_invalidations == 0,
          "a plan was not adopted at its exact committed-prefix boundary");
    advance(o, d2.action);
    auto incoming = bullet(5, o.x + 30, o.y, -6, 0);
    incoming.full_width = incoming.full_height = 6;
    o.bullets.push_back(incoming);
    const auto d3 = policy.decide(o);
    check(d3.search.emergency_searches == 1 && d3.search.pending_dropped == 1 &&
              d3.depth == config.emergency_horizon && (d3.action & 0x30) != 0,
          "a newly unsafe plan retained its pending result or lost the emergency escape");
    for (int change = 0; change < 7; ++change) {
        on::Policy isolated(config);
        auto current = scene();
        const auto first = isolated.decide(current);
        advance(current, first.action);
        if (change == 0)
            current.latched_input = (first.action & 0xf4) == 128 ? 64 : 128;
        if (change == 1)
            current.x += 1;
        if (change == 2)
            current.movement.axis[0] += .25f;
        if (change == 3) {
            current.stage = 3;
            current.spell_active = true;
            current.spell = 22;
        }
        if (change == 4) {
            // Observer warming may be consecutive even when decisions skip.
            isolated.observe(current);
            advance(current, first.action);
        }
        if (change == 5)
            current.latched_input ^= 1;
        if (change == 6)
            current.latched_input |= 2;
        const auto invalidated = isolated.decide(current);
        check(invalidated.search.executor_invalidations == 1 &&
                  invalidated.search.pending_dropped == 1 &&
                  invalidated.search.plan_adoptions == 0 &&
                  invalidated.search.emergency_searches == 1 &&
                  invalidated.search.planner_searches == 1,
              "stale prefix, position, movement, lifecycle or decision gap kept a plan");
    }
    auto blocked = scene();
    blocked.bullets = {bullet(0, blocked.x, blocked.y, 0, 0)};
    const auto doomed = on::Policy(config).decide(blocked);
    check(doomed.depth == 0 && doomed.search.emergency_searches == 1 &&
              doomed.search.planner_searches == 0 && doomed.search.plan_adoptions == 0,
          "an unavoidable latched hit launched or certified a new delayed plan");
    for (const auto &bad : {std::array<int, 3>{23, 12, 12}, std::array<int, 3>{2, 1, 12},
                            std::array<int, 3>{2, 25, 12}, std::array<int, 3>{2, 12, 25},
                            std::array<int, 3>{-1, 12, 12}}) {
        auto invalid = config;
        invalid.planner_lag = bad[0];
        invalid.executor_check = bad[1];
        invalid.emergency_horizon = bad[2];
        rejects([&] { on::Policy p(invalid); }, "invalid lag budget was accepted");
    }
    for (int fallback = 0; fallback < 3; ++fallback) {
        auto invalid = config;
        invalid.held_fallback = fallback == 0;
        invalid.warm_fallback = fallback == 1;
        invalid.tail_check = fallback == 2;
        rejects([&] { on::Policy p(invalid); }, "lag silently ignored a synchronous fallback");
    }
    auto sync = config;
    sync.planner_lag = 0;
    auto unused = sync;
    unused.executor_check = unused.emergency_horizon = unused.emergency_beam = -1;
    on::Policy original(sync), with_unused(unused);
    auto current = scene();
    for (int step = 0; step < 8; ++step) {
        const auto a = original.decide(current), b = with_unused.decide(current);
        check(a.action == b.action && a.depth == b.depth && a.search.planner_searches == 0 &&
                  b.search.planner_searches == 0 && b.search.emergency_searches == 0,
              "unused executor settings changed the synchronous policy");
        advance(current, a.action);
    }
}
void lazy_index_contract() {
    on::Configuration config;
    config.horizon = 24;
    config.beam = 64;
    config.grow = 0;
    config.planner_lag = 2;
    config.executor_check = config.emergency_horizon = 12;
    config.emergency_beam = 16;
    auto lazy_config = config;
    lazy_config.lazy_index = true;
    on::Policy eager(config), lazy(lazy_config);
    auto o = scene();
    bool skipped = false, extended_after_invalidation = false;
    for (int step = 0; step < 16; ++step) {
        if (step == 3) {
            auto incoming = bullet(5, o.x + 30, o.y, -6, 0);
            incoming.full_width = incoming.full_height = 6;
            o.bullets.push_back(incoming);
        }
        if (step == 7)
            o.stage = 3;
        const auto a = eager.decide(o);
        on::DecisionTiming timing;
        const auto begin = std::chrono::steady_clock::now();
        const auto b = lazy.decide(o, &timing);
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                 std::chrono::steady_clock::now() - begin)
                                 .count();
        check(a.action == b.action && a.depth == b.depth &&
                  a.search.selections == b.search.selections &&
                  a.search.candidates == b.search.candidates && a.search.bins == b.search.bins &&
                  a.search.retained_bins == b.search.retained_bins &&
                  a.search.planner_searches == b.search.planner_searches &&
                  a.search.plan_adoptions == b.search.plan_adoptions &&
                  a.search.executor_checks == b.search.executor_checks &&
                  a.search.emergency_searches == b.search.emergency_searches &&
                  a.search.pending_dropped == b.search.pending_dropped &&
                  a.search.executor_invalidations == b.search.executor_invalidations,
              "lazy frame indices changed a proposal, latch, event or tie order");
        check(a.search.indexed_forecast_frames == unsigned(config.horizon) &&
                  b.search.indexed_forecast_frames <= a.search.indexed_forecast_frames,
              "lazy indexing rebuilt or omitted requested frame ownership");
        skipped = skipped || b.search.indexed_forecast_frames < a.search.indexed_forecast_frames;
        extended_after_invalidation =
            extended_after_invalidation ||
            (step == 3 && b.search.emergency_searches &&
             b.search.indexed_forecast_frames == unsigned(config.horizon));
        const auto total = timing.history_ns + timing.forecast_ns + timing.index_ns +
                           timing.search_ns + timing.fallback_ns;
        check(timing.index_ns <= std::uint64_t(elapsed) &&
                  timing.search_ns <= std::uint64_t(elapsed) && total <= std::uint64_t(elapsed),
              "extended indexing was subtracted twice or left inside search time");
        on::detail::apply_movement(o.movement, o.x, o.y,
                                   on::detail::movement_delta(o.movement, o.latched_input));
        o.latched_input = a.action;
        ++o.update;
        for (auto &bullet : o.bullets)
            bullet.x += bullet.vx, bullet.y += bullet.vy;
    }
    check(skipped && extended_after_invalidation,
          "lazy index fixture missed either a skipped frame or an emergency extension");
}
void transform_motion_history() {
    on::Configuration config;
    config.horizon = 12;
    config.beam = 192;
    config.grow = 0;
    auto previous = scene();
    previous.bullets = {bullet(0, 180, 400, 0, 0)};
    previous.bullets[0].full_height = 200;
    previous.bullets[0].active_transforms = 0x40u;
    auto current = previous;
    current.update = 1;
    current.bullets[0].x = 184;
    current.bullets[0].vx = 4;
    current.bullets[0].active_transforms = 0;
    on::Policy policy(config);
    policy.observe(previous);
    const auto decision = policy.decide(current);
    check(decision.depth == 12 && (decision.action & 0xf0),
          "a completed turn's speed jump became continuing acceleration");
    config.reach_filter = true;
    on::Policy pruned(config);
    pruned.observe(previous);
    const auto filtered = pruned.decide(current);
    check(filtered.depth == decision.depth && filtered.action == decision.action,
          "transform-history invalidation differs under reach pruning");

    // A source freeze/restore callback may change velocity without changing the
    // active transform mask. Its ghost-to-real boundary is also discontinuous.
    previous.bullets[0].active_transforms = current.bullets[0].active_transforms = 0x400000u;
    previous.bullets[0].collision_disabled = true;
    on::Policy restored(config);
    restored.observe(previous);
    const auto real = restored.decide(current);
    check(real.depth == 12 && (real.action & 0xf0),
          "a collision-mode velocity jump became continuing acceleration");
    config.reach_filter = false;
    on::Policy unpruned_restore(config);
    unpruned_restore.observe(previous);
    const auto unfiltered = unpruned_restore.decide(current);
    check(unfiltered.depth == real.depth && unfiltered.action == real.action,
          "collision-mode history invalidation differs under reach pruning");

    // A vector transform that remains active can really accelerate from rest.
    // Keep that measured rate rather than treating every zero-speed jump alike.
    previous.bullets[0].collision_disabled = false;
    previous.bullets[0].active_transforms = current.bullets[0].active_transforms = 0x10u;
    for (auto *o : {&previous, &current}) {
        o->bullets[0].vector_acceleration_x = 4;
        o->bullets[0].vector_acceleration_duration = 100;
    }
    current.bullets[0].vector_acceleration_timer = 1;
    on::Policy accelerating(config);
    accelerating.observe(previous);
    check(accelerating.decide(current).depth == 0,
          "an unchanged active acceleration lost its unavoidable latched hit");
}
void hard_phantom_contract() {
    on::Configuration config;
    config.horizon = 12;
    config.beam = 64;
    config.grow = 0;
    config.risk_phantoms = true;
    for (bool pruned : {false, true}) {
        config.reach_filter = pruned;
        auto hard = config;
        hard.hard_phantoms = true;
        auto ghost = scene();
        ghost.bullets = {bullet(0, 184, 400, 4, 0)};
        ghost.bullets[0].full_height = 200;
        ghost.bullets[0].collision_disabled = true;
        auto real = ghost;
        real.bullets[0].collision_disabled = false;
        auto empty = ghost;
        empty.bullets.clear();
        const auto ignored = on::Policy(config).decide(ghost);
        const auto absent = on::Policy(config).decide(empty);
        const auto ordinary = on::Policy(config).decide(real);
        const auto conservative = on::Policy(hard).decide(ghost);
        check(ignored.action == absent.action && ignored.depth == absent.depth &&
                  ignored.search.candidates == absent.search.candidates &&
                  ignored.search.hard_phantom_frames == 0,
              "default no-model policy changed its collision-disabled contract");
        check(conservative.action == ordinary.action && conservative.depth == ordinary.depth &&
                  conservative.depth == hard.horizon && (conservative.action & 0xf0) &&
                  conservative.action != ignored.action &&
                  conservative.search.candidates == ordinary.search.candidates &&
                  conservative.search.hard_phantom_frames == unsigned(hard.horizon),
              "hard phantom did not use the ordinary fired-bullet forecast");
        for (std::uint16_t state : {2, 3, 4}) {
            ghost.bullets[0].state = state;
            const auto spawning = on::Policy(hard).decide(ghost);
            check(spawning.action == absent.action && spawning.depth == absent.depth &&
                      spawning.search.hard_phantom_frames == 0,
                  "fired-phantom prior promoted a disabled spawning bullet");
        }
        ghost.bullets[0] = bullet(0, ghost.x, ghost.y, 0, 0);
        ghost.bullets[0].collision_disabled = true;
        check(on::Policy(hard).decide(ghost).depth == 0 &&
                  on::Policy(config).decide(ghost).depth == config.horizon,
              "conservative root overlap incorrectly certified a safe prefix");

        // Hard promotion replaces phantom-only soft risk. Ordinary model margins
        // still apply exactly as they do to a real fired bullet.
        on::RiskTables model;
        model.loaded = true;
        const auto with_model = on::Policy(hard, model).decide(real);
        auto restored = real;
        restored.bullets[0].collision_disabled = true;
        const auto hard_model = on::Policy(hard, model).decide(restored);
        check(hard_model.action == with_model.action && hard_model.depth == with_model.depth &&
                  hard_model.search.candidates == with_model.search.candidates,
              "hard phantom retained an extra phantom-only soft obstacle");

        auto previous = real;
        previous.bullets[0].x = 180;
        previous.bullets[0].vx = 0;
        previous.bullets[0].collision_disabled = true;
        auto current = real;
        current.update = 1;
        on::Policy transition(hard), reference(config);
        transition.observe(previous);
        reference.observe(previous);
        const auto a = transition.decide(current), b = reference.decide(current);
        check(a.action == b.action && a.depth == b.depth && a.depth == hard.horizon,
              "hard prior erased the collision-mode motion-history boundary");
    }
}
void motion_envelope_contract() {
    std::fenv_t environment;
    std::fegetenv(&environment);
    for (const int rounding : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
        check(std::fesetround(rounding) == 0, "cannot set envelope test rounding");
        std::uint32_t random = 0x902ac57du;
        const auto draw = [&] {
            random = random * 1664525u + 1013904223u;
            return (float(int(random & 65535u) - 32768)) / 1024.f;
        };
        for (int sample = 0; sample < 64; ++sample) {
            float x = draw(), y = draw();
            const float vx = draw() / 8, vy = draw() / 8;
            on::detail::MotionEnvelope envelope(x, y);
            std::vector<std::array<float, 2>> switched{{x, y}};
            for (int step = 0; step < 48; ++step) {
                x += draw() / 16;
                y += draw() / 16;
                for (auto &point : switched)
                    point[0] += vx, point[1] += vy;
                switched.push_back({x, y});
                envelope.advance(x, y, vx, vy);
                for (const auto &point : switched)
                    check(point[0] >= envelope.min_x && point[0] <= envelope.max_x &&
                              point[1] >= envelope.min_y && point[1] <= envelope.max_y,
                          "motion envelope omitted a modeled float switch time");
            }
        }
    }
    std::fesetenv(&environment);

    on::Configuration config;
    config.horizon = 24;
    config.beam = 64;
    config.hard_phantoms = true;
    auto current = scene();
    current.bullets = {bullet(0, 192.f, 380.f, -.6f, 0)};
    auto &ghost = current.bullets[0];
    ghost.full_width = ghost.full_height = 4;
    ghost.collision_disabled = true;
    ghost.alternate_motion = true;
    ghost.alternate_vy = 10.f;
    const auto original = on::Policy(config).decide(current);
    config.motion_envelope = true;
    const auto guarded = on::Policy(config).decide(current);
    check(original.depth == 24 && guarded.depth == 24 && guarded.action != original.action &&
              (guarded.action & 0xf0) && original.search.motion_envelope_frames == 0 &&
              guarded.search.motion_envelope_frames == 24,
          "an owned alternative flight did not expose the mode-restoration risk");
    config.reach_filter = true;
    const auto pruned = on::Policy(config).decide(current);
    check(pruned.action == guarded.action && pruned.depth == guarded.depth,
          "reach pruning removed an alternative-flight escape");
    ghost.alternate_motion = false;
    const auto absent = on::Policy(config).decide(current);
    check(absent.action == original.action && absent.depth == original.depth &&
              absent.search.motion_envelope_frames == 0,
          "missing alternative motion changed the old forecast");
    ghost.alternate_motion = true;
    ghost.alternate_vx = std::numeric_limits<float>::quiet_NaN();
    rejects([&] { on::Policy(config).decide(current); },
            "non-finite source alternative entered the forecast");
    config.hard_phantoms = false;
    rejects([&] { on::Policy invalid(config); },
            "a motion envelope silently ignored disabled fired bullets");
}
on::Observation gate_scene() {
    on::Observation o;
    o.x = 104;
    o.y = 400;
    o.movement.axis[0] = o.movement.axis[1] = 4;
    o.movement.diag[0] = o.movement.diag[1] = 4;
    o.movement.minx = 100;
    o.movement.maxx = 108;
    o.movement.miny = o.movement.maxy = 400;
    o.movement.half = {.1f, .1f, 0};
    o.queued_shots = {{13, 1, -396.2f, 400, 0, 0, 1000, 10},
                      {13, 1, 604.2f, 400, 0, 0, 1000, 10},
                      {14, 1, 104, 400, 0, 0, .5f, 10}};
    return o;
}
void retain_heading_contract() {
    on::detail::ProposalSet keys;
    keys.reset({100, 400, 100, 400}, true);
    check(keys.first(100, 400, false, 0) && keys.first(100, 400, false, 4) &&
              !keys.first(100, 400, false, 4) && keys.first(100, 400, true, 4),
          "heading membership lost direction/focus separation");
    check(keys.first(500, 400, false, 7) && keys.expansions() == 1 &&
              !keys.first(100, 400, false, 0) && !keys.first(100, 400, false, 4) &&
              !keys.first(100, 400, true, 4),
          "heading expansion forgot previously occupied channels");
    keys.reset({100, 400, 100, 400});
    check(keys.first(100, 400, false, 0) && !keys.first(100, 400, false, 4),
          "legacy membership unexpectedly retains headings");
    const auto o = gate_scene();
    // Independently verify one path against literal one-update rectangles and
    // the same public inclusive collision kernel. The nonbranch layer repeats
    // right at movement 14 rather than inventing an unavailable late turn.
    float x = o.x, y = o.y;
    std::uint16_t previous = 0;
    for (int step = 1; step <= 14; ++step) {
        const std::uint16_t a = step == 12 ? 64 : step >= 13 ? 128 : 0;
        check(!(step > 1 && step - 1 >= 12 && ((step - 1) % 2) != 0 && a != previous),
              "oracle changed a forced repeat");
        on::detail::apply_movement(o.movement, x, y, on::detail::movement_delta(o.movement, a));
        for (const auto &shot : o.queued_shots) {
            if (step < int(shot.update) || step >= int(shot.update + shot.linear_updates))
                continue;
            const float sx = shot.x + (step - shot.update) * shot.vx;
            const float sy = shot.y + (step - shot.update) * shot.vy;
            const on::Box box{sx - shot.width / 2, sy - shot.height / 2, sx + shot.width / 2,
                              sy + shot.height / 2};
            check(!on::detail::box_hit(box, {x - .1f, y - .1f, 0}, {x + .1f, y + .1f, 0}),
                  "literal path collided");
        }
        previous = a;
    }
    check(x == 108 && y == 400, "oracle endpoint changed");
    for (int beam : {64, 192, 8192}) {
        on::Configuration cfg;
        cfg.horizon = 14;
        cfg.beam = beam;
        cfg.grow = 0;
        cfg.risk_weight = 0;
        cfg.queued_shots = true;
        cfg.diversity_cell = 1;
        const auto before = on::Policy(cfg).decide(o);
        cfg.retain_headings = true;
        const auto after = on::Policy(cfg).decide(o);
        check(before.depth == 13 && after.depth == 14, "forced heading witness disappeared");
        check(before.search.selections == 0 && after.search.selections == 0,
              "witness depends on beam truncation");
        check(before.search.heading_layers == 0 && before.search.heading_keys == 0,
              "default adds hidden repeat work");
        check(after.search.heading_layers == 2 && after.search.heading_keys > 0,
              "extra repeat work is uncharged");
        auto impossible = o;
        impossible.queued_shots.push_back({14, 1, 104, 400, 0, 0, 1000, 1000});
        const auto partial = on::Policy(cfg).decide(impossible);
        cfg.retain_headings = false;
        const auto old_partial = on::Policy(cfg).decide(impossible);
        check(partial.depth == 13 && old_partial.depth == 13,
              "all dead compulsory repeats hid the longest safe partial layer");
        cfg.retain_headings = true;
        auto blocked = o;
        blocked.queued_shots.push_back({1, 1, blocked.x, blocked.y, 0, 0, 10, 10});
        const auto doomed = on::Policy(cfg).decide(blocked);
        check(doomed.depth == 0 && doomed.search.heading_layers == 0,
              "repeat check bypassed an already latched hit");
    }
    on::Configuration traced_cfg;
    traced_cfg.horizon = 24;
    traced_cfg.beam = 64;
    traced_cfg.retain_headings = true;
    auto empty = o;
    empty.queued_shots.clear();
    on::Policy plain(traced_cfg), traced(traced_cfg);
    std::vector<on::ValueProposal> proposals;
    for (unsigned step = 0; step < 8; ++step) {
        const auto a = plain.decide(empty), b = traced.decide(empty, nullptr, &proposals);
        check(a.depth == 24 && b.depth == 24 && a.action == b.action &&
                  a.search.heading_layers == 7 &&
                  a.search.heading_layers == b.search.heading_layers &&
                  a.search.heading_keys == b.search.heading_keys && proposals.size() == 4,
              "terminal heading retention depends on tracing or lost H24 target context");
        on::detail::apply_movement(empty.movement, empty.x, empty.y,
                                   on::detail::movement_delta(empty.movement, empty.latched_input));
        empty.latched_input = a.action;
        ++empty.update;
    }
}
void child_cursor_contract() {
    std::array<on::detail::ChildCursorRecord, 18> records{};
    records[0] = {on::detail::kChildSpawnKind, false};
    auto r = on::detail::child_birth_cursor(records, 0, on::detail::kChildSpawnKind, 0, 0, 0, 24);
    check(r.index == 0 && r.update == 1 && !r.unsupported, "active-zero birth lost");
    records[0] = {};
    records[1] = {on::detail::kChildSpawnKind, true};
    r = on::detail::child_birth_cursor(records, 0, on::detail::kChildSpawnKind, 0x10, -1, 1, 24);
    check(r.index < 0 && !r.unsupported, "NONE did not end narrow preview");
    records[0] = {on::detail::kChildWaitKind, false};
    records[1] = {0x10, false};
    records[2] = {on::detail::kChildSpawnKind, true};
    r = on::detail::child_birth_cursor(records, 1,
                                       on::detail::kChildSpawnKind | on::detail::kChildWaitKind,
                                       on::detail::kChildWaitKind, 3, 0, 24);
    check(r.index == 2 && r.update == 5 && !r.unsupported, "mask skip bypassed WAIT gate");
    r = on::detail::child_birth_cursor(records, 1,
                                       on::detail::kChildSpawnKind | on::detail::kChildWaitKind,
                                       on::detail::kChildWaitKind, 0, 0, 24);
    check(r.index == 2 && r.update == 2 && !r.unsupported, "zero WAIT omitted clear callback");
    r = on::detail::child_birth_cursor(records, 1,
                                       on::detail::kChildSpawnKind | on::detail::kChildWaitKind,
                                       on::detail::kChildWaitKind, 3, 0, 4);
    check(r.index < 0 && !r.unsupported, "birth beyond horizon entered forecast");
    r = on::detail::child_birth_cursor(records, 1,
                                       on::detail::kChildSpawnKind | on::detail::kChildWaitKind,
                                       on::detail::kChildWaitKind, 3, 0, 5);
    check(r.index == 2 && r.update == 5, "birth at horizon was omitted");
    r = on::detail::child_birth_cursor(
        records, 1, on::detail::kChildSpawnKind | on::detail::kChildWaitKind,
        on::detail::kChildWaitKind, std::numeric_limits<int>::max(), 0, 200);
    check(r.index < 0 && !r.unsupported, "WAIT clock overflowed");
    r = on::detail::child_birth_cursor(
        records, 1, on::detail::kChildSpawnKind | on::detail::kChildWaitKind,
        on::detail::kChildWaitKind, 3, std::numeric_limits<float>::quiet_NaN(), 24);
    check(r.unsupported, "nonfinite WAIT subframe accepted");
    r = on::detail::child_birth_cursor(
        records, 1, on::detail::kChildSpawnKind | on::detail::kChildWaitKind, 0x10, 3, 0, 24);
    check(r.unsupported, "unmodeled blocking active transform accepted");
    records[2] = {};
    r = on::detail::child_birth_cursor(
        records, 1, on::detail::kChildSpawnKind | on::detail::kChildWaitKind, 0x10, 3, 0, 24);
    check(r.index < 0 && !r.unsupported, "unrelated cursor with no child caused false unsupported");
    records = {};
    records[17] = {on::detail::kChildSpawnKind, true};
    r = on::detail::child_birth_cursor(records, 17, on::detail::kChildSpawnKind, 0, 0, 0, 24);
    check(r.unsupported, "child record17 silently lost missing secondary");
    r = on::detail::child_birth_cursor(records, 17, 0, 0, 0, 0, 24);
    check(r.index < 0 && !r.unsupported, "disabled record17 incorrectly requires secondary");
    r = on::detail::child_birth_cursor(records, 18, on::detail::kChildSpawnKind, 0, 0, 0, 24);
    check(r.index < 0 && !r.unsupported, "terminal cursor unsupported");
    r = on::detail::child_birth_cursor(records, -1, on::detail::kChildSpawnKind, 0, 0, 0, 24);
    check(r.unsupported, "negative cursor accepted");
    r = on::detail::child_birth_cursor(records, 17, on::detail::kChildSpawnKind, 0, 0, 0, 0);
    check(r.index < 0 && !r.unsupported, "zero horizon decoded a future");
    std::cout << "child cursor contract passed\n";
}
void child_birth_contract() {
    on::Configuration config;
    config.horizon = 24;
    auto observation = scene();
    observation.child_births.push_back({1, observation.x, observation.y, 0, 4, 4, 0});
    on::Policy ordinary(config);
    config.child_births = true;
    on::Policy guarded(config);
    check(ordinary.decide(observation).depth == 24 && guarded.decide(observation).depth == 0,
          "birth coverage changed the disabled path or missed the forced first movement");
    observation.unsupported_child_births = 1;
    on::Policy unknown(config);
    rejects([&] { unknown.decide(observation); }, "unsupported child birth became empty space");
    const on::ChildBirth birth{3, 193.5076599f, 397.5036316f, 3.3f, 4, 4, 358};
    const auto boxes = on::detail::child_birth_boxes(birth, 0);
    rejects([&] { on::detail::child_birth_boxes(birth, std::numeric_limits<float>::max()); },
            "non-finite derived child boxes reached indexing");
    for (float vx : {-3.3f, 0.f, 3.3f})
        for (float vy : {-3.3f, 0.f, 3.3f})
            for (unsigned movements = 0; movements <= 2; ++movements) {
                float x = birth.x, y = birth.y;
                for (unsigned step = 0; step < movements; ++step)
                    x += vx, y += vy;
                const auto &box = boxes[movements < 2 ? 0 : 1];
                check(box.x0 <= x - 2 && box.y0 <= y - 2 && box.x1 >= x + 2 && box.y1 >= y + 2,
                      "child birth bounds omitted a pool-order/movement extreme");
            }
}
void scheduled_expiry_contract() {
    namespace detail = on::detail;
    std::array<detail::LifetimeRecord, 18> records{};
    records[0] = {detail::kLifetimeDespawnKind, false};
    auto decode = [&](std::uint32_t active, int timer, float fraction, unsigned horizon,
                      bool clock = true) {
        return detail::scheduled_collision_lifetime(
            records, 0, detail::kLifetimeWaitKind | detail::kLifetimeDespawnKind, active, timer,
            fraction, horizon, clock);
    };
    check(decode(0, 0, 0, 24).collision_until == 1 &&
              decode(detail::kLifetimeWaitKind, 0, 0, 24).collision_until == 2 &&
              decode(detail::kLifetimeWaitKind, 1, 0, 24).collision_until == 3,
          "DESPAWN lost the active-gate/WAIT-clear callback order");
    // Independent source-order replica: the switch enters Fired before Advance
    // changes its state; collision still runs in that callback, not the next.
    for (int initial_wait : {-1, 0, 1, 3}) {
        bool fired = true, waiting = initial_wait >= 0;
        int timer = std::max(0, initial_wait);
        const unsigned last =
            decode(waiting ? detail::kLifetimeWaitKind : 0, timer, 0, 24).collision_until;
        for (unsigned update = 1; update <= last + 1; ++update) {
            const bool entered_fired = fired;
            if (fired && !waiting)
                fired = false;
            if (waiting) {
                if (timer <= 0)
                    waiting = false;
                else
                    --timer;
            }
            check(entered_fired == (update <= last),
                  "lifetime excluded the activation collision or kept the next callback");
        }
    }
    records[0] = {0x10u, false}; // Masked, but its active gate is still tested.
    records[1] = {detail::kLifetimeDespawnKind, true};
    check(decode(detail::kLifetimeWaitKind, 3, 0, 24).collision_until == 5,
          "masked blocking record bypassed a WAIT before allowed DESPAWN");
    check(decode(detail::kLifetimeWaitKind, 3, 0, 4).collision_until == 0 &&
              decode(detail::kLifetimeWaitKind, 3, 0, 5).collision_until == 5 &&
              decode(detail::kLifetimeWaitKind, std::numeric_limits<int>::max(), 0, 200)
                      .collision_until == 0,
          "lifetime crossed its horizon or overflowed a WAIT clock");
    check(decode(0x10u, 0, 0, 24).unsupported &&
              decode(detail::kLifetimeWaitKind, -1, 0, 24).unsupported &&
              decode(detail::kLifetimeWaitKind, 0, std::numeric_limits<float>::quiet_NaN(), 24)
                  .unsupported &&
              decode(0, 0, 0, 24, false).unsupported,
          "unsupported lifetime clock became an expiry promise");
    records[0] = {};
    check(!decode(0x10u, -1, 1, 24).unsupported && decode(0x10u, -1, 1, 24).collision_until == 0,
          "NONE did not end the narrow lifetime preview");
    records = {};
    records[17] = {detail::kLifetimeDespawnKind, false};
    check(detail::scheduled_collision_lifetime(records, 17, detail::kLifetimeDespawnKind, 0, 0, 0,
                                               24, true)
                  .collision_until == 1,
          "one-record DESPAWN incorrectly required a secondary payload");

    on::Configuration config;
    config.horizon = 3;
    config.beam = 64;
    config.grow = config.risk_weight = 0;
    config.queued_shots = true;
    auto observation = scene();
    observation.x = 188;
    observation.latched_input = 128;
    observation.movement.minx = 188;
    observation.movement.maxx = 200;
    observation.movement.miny = observation.movement.maxy = 400;
    observation.movement.axis[0] = observation.movement.axis[1] = 4;
    observation.movement.diag[0] = observation.movement.diag[1] = 4;
    observation.movement.half = {.1f, .1f, 0};
    observation.bullets = {bullet(0, 196, 400, 0, 0)};
    observation.bullets[0].collision_until = 1;
    // Movement two must enter x196, after the old incarnation stops colliding.
    observation.queued_shots = {{2, 1, -305.2f, 400, 0, 0, 1000, 10},
                                {2, 1, 697.2f, 400, 0, 0, 1000, 10}};
    const auto legacy = on::Policy(config).decide(observation);
    config.scheduled_expiry = true;
    const auto expired = on::Policy(config).decide(observation);
    check(legacy.depth == 1 && expired.depth == 3 && legacy.search.lifetime_records == 0 &&
              expired.search.lifetime_records == 1 &&
              expired.search.lifetime_projected_frames == 1 &&
              expired.search.lifetime_skipped_frames == 2,
          "scheduled expiry missed the corridor or hid projection work");
    observation.bullets[0].collision_until = 2;
    check(on::Policy(config).decide(observation).depth == 1,
          "inclusive expiry removed a lethal update and overstated partial depth");
    observation.bullets[0].collision_until = 0;
    observation.unsupported_lifetimes = 1;
    const auto unknown = on::Policy(config).decide(observation);
    check(unknown.depth == legacy.depth && unknown.action == legacy.action &&
              unknown.search.unsupported_lifetimes == 1 && unknown.search.lifetime_records == 0,
          "unsupported lifetime removed an unbounded hazard or lost coverage accounting");
    observation.unsupported_lifetimes = 0;
    observation.bullets[0].collision_until = 1;
    observation.x = 192;
    const auto latched = on::Policy(config).decide(observation);
    check(latched.depth == 0 && latched.search.lifetime_projected_frames == 1,
          "expiry hid an already-latched first-update collision");
    observation.bullets[0].collision_until = 201;
    rejects([&] { on::Policy(config).decide(observation); },
            "out-of-profile positive lifetime metadata was accepted");

    // Disabled metadata is inert across consecutive observations, including tie RNG.
    on::Configuration ordinary;
    ordinary.horizon = 12;
    ordinary.beam = 64;
    on::Policy plain(ordinary), ignored(ordinary);
    auto current = scene();
    current.bullets = {bullet(0, 110, 100, 1, 0)};
    for (unsigned update = 0; update < 8; ++update) {
        current.bullets[0].collision_until = 0;
        const auto a = plain.decide(current);
        current.bullets[0].collision_until = 1;
        const auto b = ignored.decide(current);
        check(a.action == b.action && a.depth == b.depth && b.search.lifetime_records == 0,
              "disabled lifetime metadata changed ordinary actions or tie progression");
        on::detail::apply_movement(
            current.movement, current.x, current.y,
            on::detail::movement_delta(current.movement, current.latched_input));
        current.latched_input = a.action;
        ++current.update;
        current.bullets[0].x += current.bullets[0].vx;
    }
}
void branch_value_contract() {
    on::Configuration config;
    config.horizon = on::kValueHorizon;
    config.beam = 64;
    config.hard_phantoms = config.motion_envelope = true;
    on::BranchValueModel model;
    model.loaded = true;
    model.scale.fill(1);
    on::Policy plain(config), traced(config), constant(config, {}, model);
    config.value_weight = 0;
    model.weight[0] = 100;
    on::Policy disabled(config, {}, model);
    auto current = scene();
    for (int step = 0; step < 12; ++step) {
        if (step == 7)
            current.bullets = {bullet(0, current.x, current.y, 0, 0)};
        if (step == 8)
            current.bullets.clear();
        std::vector<on::ValueProposal> proposals;
        const auto a = plain.decide(current), b = traced.decide(current, nullptr, &proposals),
                   c = constant.decide(current), d = disabled.decide(current);
        check(a.action == b.action && a.action == c.action && a.action == d.action &&
                  a.depth == b.depth && a.depth == c.depth && a.depth == d.depth &&
                  a.search.candidates == b.search.candidates,
              "proposal extraction, constant value or zero weight changed ordinary search");
        if (a.depth == config.horizon) {
            check(proposals.size() == 4, "complete beam lost its four sampled endpoints");
            for (const auto &proposal : proposals) {
                check(proposal.inputs.size() == on::kValueHorizon - 1,
                      "branch prefix disagrees with the movement latch");
                for (const auto input : proposal.inputs)
                    check((input & 1) && !(input & ~0xf5u),
                          "branch proposal changed student action space");
                for (const auto value : proposal.features)
                    check(std::isfinite(value), "non-finite student feature exported");
            }
            check(c.search.value_evaluations > 0 && d.search.value_evaluations == 0,
                  "learned ranking work is missing or zero weight still evaluates models");
        } else {
            check(proposals.empty(),
                  "partial or failed search emitted a complete branch label request");
        }
        on::detail::apply_movement(
            current.movement, current.x, current.y,
            on::detail::movement_delta(current.movement, current.latched_input));
        current.latched_input = a.action;
        ++current.update;
    }
}
void observation_history() {
    on::Configuration config;
    config.horizon = 8;
    config.beam = 24;
    auto o = scene();
    o.x = o.y = 100;
    o.latched_input = 128;
    o.bullets = {bullet(0, 104, 100, 0, 0)};
    on::Policy delayed(config);
    const auto unavoidable = delayed.decide(o);
    check(unavoidable.depth == 0 && unavoidable.action == 129,
          "new input incorrectly avoided the already latched update");
    rejects([&] { delayed.decide(o); }, "duplicate decision advanced observer/tie state");

    on::Policy warmed(config), repeated(config), cold(config);
    o.latched_input = 0;
    o.y = 97;
    o.movement.half = {.25f, .25f, 0};
    o.bullets = {bullet(0, 100, 102, 0, -1)};
    o.bullets[0].full_width = o.bullets[0].full_height = .5f;
    warmed.observe(o);
    repeated.observe(o);
    repeated.observe(o);
    o.update = 1;
    o.bullets[0].y = 100;
    o.bullets[0].vy = -2;
    const auto a = warmed.decide(o), b = repeated.decide(o), c = cold.decide(o);
    check(a.depth == 0 && b.depth == 0 && a.action == b.action && c.depth > 0,
          "prefix observation failed to warm velocity history without a search");
    o.update = 3;
    rejects([&] { warmed.observe(o); }, "gapped history was interpreted as one update");
    auto invalid = scene();
    invalid.x = std::numeric_limits<float>::quiet_NaN();
    rejects([&] { on::Policy(config).decide(invalid); }, "non-finite memory was accepted");
}
void pruning_and_isolation() {
    on::Configuration config;
    config.horizon = 12;
    config.beam = 48;
    on::Policy reference(config), interleaved(config);
    config.reach_filter = true;
    on::Policy pruned(config), unrelated(config);
    auto o = scene();
    // Diagonal movement can exceed axis movement in a typed snapshot. The
    // reach bound must cover both rather than assume a particular shot file.
    o.movement.diag[0] = 6;
    for (std::uint64_t i = 0; i < 20; ++i) {
        o.update = i;
        o.bullets = {bullet(0, 180 + i, 390, 1, 0), bullet(1, 220, 360 + i, 0, 1),
                     bullet(2, -10000, -10000, 0, 0)};
        o.laser_hitboxes = {{192, 380, 240, 4, 192, 380, .003f * float(i), -1},
                            {-10000, -10000, 100, 2, -10000, -10000, 1, -1}};
        auto other = o;
        other.x = 20;
        unrelated.decide(other);
        const auto a = reference.decide(o), b = interleaved.decide(o), c = pruned.decide(o);
        check(a.action == b.action && a.depth == b.depth,
              "another policy instance changed observer/search state");
        check(a.action == c.action && a.depth == c.depth,
              "reach filtering changed an achievable action/depth");
        o.latched_input = a.action;
    }
}
void search_scratch_contract() {
    on::Configuration config;
    config.horizon = 12;
    config.beam = 48;
    config.diversity_cell = 16;
    on::Policy failed(config), skipped(config);
    auto o = scene();
    const auto first = failed.decide(o), reference = skipped.decide(o);
    check(first.action == reference.action && first.depth == config.horizon,
          "scratch fixture did not warm a complete search");
    ++o.update;
    o.latched_input = 0;
    o.bullets = {bullet(0, o.x, o.y, 0, 0)};
    check(failed.decide(o).depth == 0, "scratch fixture did not take the latched failure return");
    // The failed root consumes no search tie RNG. Warming without a decision
    // supplies identical observer history while leaving a different scratch use.
    skipped.observe(o);
    ++o.update;
    o.bullets.clear();
    const auto a = failed.decide(o), b = skipped.decide(o);
    check(a.depth == config.horizon && a.action == b.action && a.depth == b.depth &&
              a.search.selections == b.search.selections &&
              a.search.candidates == b.search.candidates && a.search.bins == b.search.bins &&
              a.search.retained_bins == b.search.retained_bins,
          "an early-return search leaked scratch nodes, marks or parent chains");
}
void proposal_set_contract() {
    std::fenv_t environment;
    std::fegetenv(&environment);
    const int saved_errno = errno;
    on::detail::ProposalSet actual, independent;
    std::uint32_t random = 0x735acde1u;
    const auto draw = [&] { return random = random * 1664525u + 1013904223u; };
    const auto old_key = [](float x, float y, bool focus) {
        return ((std::uint32_t(std::lround(x * 2)) & 2047u) << 12 |
                (std::uint32_t(std::lround(y * 2)) & 2047u))
                   << 1 |
               std::uint32_t(focus);
    };
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const std::array<on::Box, 7> domains{{{8, 16, 376, 432},
                                          {1022, -2, 1026, 2},
                                          {-0.f, -0.f, 0.f, 0.f},
                                          {-1048576, -1048576, 1048576, 1048576},
                                          {0, 0, 1, 1},
                                          {1, 1, 0, 0},
                                          {nan, 0, 1, 1}}};
    for (const int rounding : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
        check(std::fesetround(rounding) == 0, "cannot set proposal rounding mode");
        for (const auto bounds : domains) {
            actual.reset(bounds);
            independent.reset({0, 0, 0, 0});
            std::unordered_set<std::uint32_t> expected;
            const auto compare = [&](float x, float y, bool focus) {
                const bool first = expected.insert(old_key(x, y, focus)).second;
                independent.first(x, y, focus);
                check(actual.first(x, y, focus) == first,
                      "local proposal grid changed the first key owner or retained stale marks");
            };
            const bool valid = bounds.x0 <= bounds.x1 && bounds.y0 <= bounds.y1;
            const auto range = valid ? bounds : domains.front();
            for (int i = 0; i < 1024; ++i) {
                const float fx = float(draw() % 4097) / 4096.f;
                const float fy = float(draw() % 4097) / 4096.f;
                const float x =
                    std::clamp(range.x0 + (range.x1 - range.x0) * fx, range.x0, range.x1);
                const float y =
                    std::clamp(range.y0 + (range.y1 - range.y0) * fy, range.y0, range.y1);
                const bool focus = (draw() & 1) != 0;
                compare(x, y, focus);
                compare(x, y, focus); // includes keys owned by a discarded/lethal proposal
            }
            // Force expansion after populated cyclic bins, then revisit their
            // owners and aliases. Bounds only accelerate key membership.
            for (int repeat = 0; repeat < 2; ++repeat)
                for (const auto point : std::array<on::Vector, 6>{
                         {{1023, -1, 0},
                          {1025, 1, 0},
                          {999, 100, 0},
                          {-1, -1, 0},
                          {-0.f, 0.f, 0},
                          {std::numeric_limits<float>::denorm_min(), -0.f, 0}}})
                    for (const bool focus : {false, true})
                        compare(point.x, point.y, focus);
        }
    }
    actual.reset({1022, -2, 1026, 2});
    check(actual.first(1023, -1, false) && actual.first(1025, 1, true) &&
              actual.expansions() == 0 && actual.first(999, 100, false) &&
              actual.expansions() == 1 && !actual.first(1023, -1, false) &&
              !actual.first(-1, -1, false) && !actual.first(1025, 1, true),
          "proposal fallback lost cyclic/focus owners or its expansion count");
    actual.reset({1022, -2, 1026, 2});
    check(actual.expansions() == 0 && actual.first(1023, -1, false),
          "a new layer retained the fallback state or marked bits");
    std::fesetenv(&environment);
    errno = saved_errno;
}
void actor_risk() {
    auto o = scene();
    o.actors = {{0, o.x, o.y, 0, 0}, {1, -10000, -10000, 0, 0}};
    on::Configuration config;
    config.horizon = 12;
    config.beam = 48;
    const auto baseline = on::Policy(config).decide(o);
    config.spawn_guard = 16;
    config.risk_hard = true;
    const auto guarded = on::Policy(config).decide(o);
    config.reach_filter = true;
    const auto pruned = on::Policy(config).decide(o);
    check((baseline.action & 0xf0) == 0 && (guarded.action & 0xf0) != 0,
          "current actor risk was ignored without a learned table");
    check(guarded.depth == config.horizon && guarded.action == pruned.action &&
              guarded.depth == pruned.depth,
          "actor proximity became a hard collision or pruning changed the decision");
}
void spatial_retention() {
    const auto o = scene();
    on::Configuration config;
    config.horizon = 16;
    config.beam = 48;
    config.diversity_cell = 16;
    const auto a = on::Policy(config).decide(o), b = on::Policy(config).decide(o);
    check(a.depth == config.horizon && a.action == b.action && a.depth == b.depth &&
              a.search.selections > 0 && a.search.selections == b.search.selections &&
              a.search.candidates == b.search.candidates && a.search.bins == b.search.bins &&
              a.search.retained_bins == b.search.retained_bins &&
              a.search.candidates >= a.search.bins && a.search.bins >= a.search.retained_bins,
          "spatial pruning changed determinism or omitted selection costs");
}
void queued_birth() {
    auto o = scene();
    o.queued_shots = {{3, 2, o.x, o.y, 0, 0, 8, 8}};
    on::Configuration config;
    config.horizon = 12;
    config.beam = 48;
    const auto baseline = on::Policy(config).decide(o);
    config.queued_shots = true;
    const auto warned = on::Policy(config).decide(o);
    check((baseline.action & 0xf0) == 0 && (warned.action & 0xf0) != 0 &&
              warned.depth == config.horizon,
          "known queued birth was ignored before it became an observable bullet");
    o.queued_shots[0].update = 1;
    check(on::Policy(config).decide(o).depth == 0,
          "queued birth incorrectly bypassed the already latched movement");
}
void queued_direct_laser() {
    auto o = scene();
    o.queued_lasers = {{3, 6, {487, 100, 590, 8, 192, 100, 1.57079632679f, -1}}};
    on::Configuration config;
    config.horizon = 12;
    config.beam = 48;
    const auto baseline = on::Policy(config).decide(o);
    config.queued_lasers = true;
    const auto warned = on::Policy(config).decide(o);
    check((baseline.action & 0xf0) == 0 && (warned.action & 0xf0) != 0 &&
              warned.depth == config.horizon,
          "queued rotated direct laser did not induce an early escape");
    o.queued_lasers[0].first_update = 1;
    check(on::Policy(config).decide(o).depth == 0,
          "queued direct laser incorrectly bypassed latched movement");
    o.queued_lasers[0].last_update = 0;
    rejects([&] { on::Policy(config).decide(o); }, "invalid queued laser interval was accepted");
}
void queued_pooled_laser() {
    auto o = scene();
    on::LaserState laser;
    laser.position = {o.x, 448, 0};
    laser.angle = -1.57079632679f;
    laser.speed = 30;
    laser.width = 16;
    laser.startLength = 2400;
    laser.startTime = laser.hitboxStartTime = 1;
    laser.duration = laser.despawnDuration = 40;
    laser.hitboxEndDelay = 20;
    laser.flags = 6;
    o.queued_pooled_lasers = {laser};
    on::Configuration config;
    config.horizon = 12;
    config.beam = 48;
    const auto baseline = on::Policy(config).decide(o);
    config.queued_lasers = true;
    const auto warned = on::Policy(config).decide(o);
    check((baseline.action & 0xf0) == 0 && (warned.action & 0xf0) != 0 &&
              warned.depth == config.horizon,
          "current-cursor pooled laser was not forecast before its birth");
    config.reach_filter = true;
    const auto pruned = on::Policy(config).decide(o);
    check(pruned.action == warned.action && pruned.depth == warned.depth,
          "reach pruning lost a queued pooled laser");
    // At birth this source is still harmless. Starting the copy one callback
    // late loses the only extra controllable update before the growing beam.
    o.queued_pooled_lasers.clear();
    laser.endOffset = 30;
    laser.timer = 1;
    o.lasers = {laser};
    check(on::Policy(config).decide(o).depth < config.horizon,
          "late observation unexpectedly bypassed the input latch");
    o.queued_pooled_lasers = {laser};
    rejects([&] { on::Policy(config).decide(o); }, "noninitial queued pooled laser was accepted");
}
void current_relative_turn() {
    auto o = scene();
    o.bullets = {bullet(0, o.x, o.y - 17, 0, 0)};
    o.bullets[0].full_width = o.bullets[0].full_height = 4;
    on::Configuration config;
    config.horizon = 12;
    config.beam = 48;
    on::Policy baseline(config);
    config.relative_turns = true;
    on::Policy projected(config);
    baseline.observe(o);
    projected.observe(o);
    o.update = 1;
    auto &b = o.bullets[0];
    b.active_transforms = 0x40;
    b.relative_turn.updates = 1;
    b.relative_turn.state = {
        th08::bullet::TurnMode::relative, true, 0, 1, 0, 0, 1.57079632679f, 5, 0};
    const auto a = baseline.decide(o), p = projected.decide(o);
    check((a.action & 0xf0) == 0 && (p.action & 0xf0) != 0 && p.depth == config.horizon &&
              p.search.relative_turn_frames == 1,
          "current timed turn was hidden by stationary velocity history");
    config.reach_filter = true;
    const auto pruned = on::Policy(config).decide(o);
    check(pruned.action == p.action && pruned.depth == p.depth &&
              pruned.search.relative_turn_frames == 1,
          "reach pruning changed the bounded relative-turn proposal");
    b.y = o.y - 5;
    check(on::Policy(config).decide(o).depth == 0,
          "relative turn bypassed the first latched movement");
    b.relative_turn.state.mode = th08::bullet::TurnMode::aimed;
    rejects([&] { on::Policy(config).decide(o); },
            "target-dependent turn was accepted as an independent forecast");
    b.relative_turn.state.mode = th08::bullet::TurnMode::relative;
    b.relative_turn.state.active = false;
    rejects([&] { on::Policy(config).decide(o); },
            "inactive transform was accepted as a bounded relative turn");
}
void held_escape() {
    auto o = scene();
    o.x = 316.374908f;
    o.y = 405.144501f;
    o.latched_input = 145;
    o.movement.half = {.825f, .825f, 0};
    o.queued_lasers = {
        {9, 64, {531.404175f, 70.4011536f, 590, 160, 236.404205f, 70.4011536f, 1.50952911f, -1}},
        {9, 64, {531.404175f, 70.4011536f, 590, 160, 236.404205f, 70.4011536f, 1.77132845f, -1}}};
    on::Configuration config;
    config.horizon = 64;
    config.beam = 192;
    config.diversity_cell = 16;
    config.queued_lasers = true;
    const auto baseline = on::Policy(config).decide(o);
    config.held_fallback = true;
    const auto escaped = on::Policy(config).decide(o);
    config.reach_filter = true;
    const auto pruned = on::Policy(config).decide(o);
    check(baseline.depth == 8 && baseline.search.held_paths == 0 &&
              escaped.depth == config.horizon && escaped.action == 129 &&
              escaped.action == pruned.action && escaped.depth == pruned.depth &&
              escaped.search.held_paths == 18 && escaped.search.held_selected == 1 &&
              escaped.search.held_updates <= 18 * unsigned(config.horizon),
          "pruning discarded a full held-direction escape or fallback work was uncounted");
    o.queued_lasers[0].first_update = o.queued_lasers[1].first_update = 1;
    const auto latched = on::Policy(config).decide(o);
    check(latched.depth == 0 && latched.search.held_paths == 0,
          "held fallback claimed to avoid an already latched collision");
}
void current_spawn_clock() {
    auto o = scene();
    auto b = bullet(0, 177.5f, 400, 5, 0);
    b.state = 2;
    b.full_width = b.full_height = 4;
    b.activation_update = 3;
    o.bullets = {b};
    on::Configuration config;
    config.horizon = 12;
    config.beam = 192;
    config.grow = 0;
    const auto legacy = on::Policy(config).decide(o);
    config.spawn_clock = true;
    const auto early = on::Policy(config).decide(o);
    config.reach_filter = true;
    const auto pruned = on::Policy(config).decide(o);
    check((legacy.action & 0xf0) == 0 && (early.action & 0xf0) != 0 && early.depth == 12 &&
              early.action == pruned.action && early.search.spawn_clock_frames == 3,
          "source clock did not preserve an escape before animation completion");
    // A late first snapshot has one call left. The final fractional plus full
    // displacement is already lethal under the actual latched idle input.
    o.bullets[0].x = 182.5f;
    o.bullets[0].activation_update = 1;
    const auto late = on::Policy(config).decide(o);
    check(late.depth == 0 && late.search.spawn_clock_frames == 1,
          "activation lost fractional movement or bypassed the input latch");
    o.bullets[0].state = 1;
    rejects([&] { on::Policy(config).decide(o); }, "fired bullet claimed a spawn clock");
    o.bullets[0].state = 2;
    o.movement.frm = .5f;
    rejects([&] { on::Policy(config).decide(o); }, "unit spawn clock accepted a fractional update");
    o.movement.frm = 1;
    o.bullets[0].activation_update = 0;
    o.bullets[0].x = 177.5f;
    check(on::Policy(config).decide(o).action == legacy.action,
          "unsupported spawn clock changed the named legacy hypothesis");
}
void warm_escape() {
    // Two crossing linear bullets expose a full path discarded by a one-node
    // beam after the observation boundary moves. No native future or route tape.
    for (int variant = 0; variant < 7; ++variant) {
        auto o = scene();
        o.bullets = {bullet(1, 203, 396, -3, 0), bullet(10, 192, 370, 2.5f, 4)};
        for (auto &b : o.bullets)
            b.full_width = b.full_height = 4;
        on::Configuration config;
        config.horizon = 24;
        config.beam = 1;
        config.grow = 0;
        config.queued_shots = true;
        config.reach_filter = variant == 6;
        on::Policy baseline(config);
        config.warm_fallback = true;
        on::Policy warmed(config);
        const auto first = baseline.decide(o), first_warm = warmed.decide(o);
        check(first.action == first_warm.action && first.depth == 24 &&
                  first_warm.search.warm_paths == 0,
              "warm fallback changed a full initial beam or reused another instance's plan");
        ++o.update;
        o.latched_input = first.action;
        for (auto &b : o.bullets)
            b.x += b.vx, b.y += b.vy;
        if (variant == 1) {
            baseline.observe(o);
            warmed.observe(o);
            ++o.update;
            for (auto &b : o.bullets)
                b.x += b.vx, b.y += b.vy;
            o.queued_shots.push_back({20, 1, 192, 400, 0, 0, 1000, 1000});
        }
        if (variant == 2)
            o.latched_input = 4;
        if (variant == 3) {
            o.queued_shots.push_back({2, 1, 193.4142135f, 401.4142135f, 0, 0, .1f, .1f});
            o.queued_shots.push_back({13, 1, 192, 400, 0, 0, 1000, 1000});
        }
        if (variant == 4)
            o.lethal_regions.push_back({0, 0, 384, 448});
        if (variant == 5)
            o.queued_shots.push_back({13, 1, 192, 400, 0, 0, 1000, 1000});
        const auto before = baseline.decide(o), after = warmed.decide(o);
        const auto &s = after.search;
        if (variant == 0 || variant == 6)
            check(before.depth == 13 && after.depth == 24 && before.action != after.action &&
                      s.warm_paths == 1 && s.warm_selected == 1 && s.warm_updates == 24,
                  "previous-plan suffix did not restore the full path or charge its checks");
        else {
            check(before.action == after.action && before.depth == after.depth &&
                      s.warm_selected == 0,
                  "unusable warm proposal changed the beam or its depth tie");
            if (variant == 1 || variant == 2)
                check(s.warm_paths == 0 && s.warm_invalidated == 1,
                      "warm proposal survived a skipped decision or changed latched input");
            if (variant == 3)
                check(s.warm_paths == 1 && s.warm_updates == 2,
                      "warm proposal bypassed a new collision in the current forecast");
            if (variant == 4)
                check(after.depth == 0 && s.warm_paths == 0,
                      "warm fallback bypassed an already latched collision");
            if (variant == 5)
                check(s.warm_paths == 1 && s.warm_updates == 13,
                      "warm depth tie omitted the terminating collision check");
        }
    }
}
void reversing_direct_laser() {
    auto previous = scene();
    previous.movement.axis[0] = previous.movement.axis[1] = 0;
    previous.movement.diag[0] = previous.movement.diag[1] = 0;
    const float ox = previous.x - 300 * std::cos(1.3f);
    const float oy = previous.y - 300 * std::sin(1.3f);
    previous.laser_hitboxes = {{ox + 295, oy, 590, 8, ox, oy, 1.22f, -1}};
    auto current = previous;
    current.update = 1;
    current.laser_hitboxes[0].angle = 1.2f;
    on::Configuration config;
    config.horizon = 12;
    config.beam = 48;
    on::Policy baseline(config);
    baseline.observe(previous);
    const auto fixed_turn = baseline.decide(current);
    config.direct_turn_envelope = true;
    on::Policy guarded(config);
    guarded.observe(previous);
    const auto both_turns = guarded.decide(current);
    config.reach_filter = true;
    on::Policy pruned(config);
    pruned.observe(previous);
    const auto filtered = pruned.decide(current);
    check(fixed_turn.depth == config.horizon && both_turns.depth < config.horizon &&
              both_turns.depth > 0 && both_turns.depth == filtered.depth &&
              both_turns.action == filtered.action &&
              both_turns.search.turn_envelope_frames == unsigned(config.horizon) &&
              fixed_turn.search.turn_envelope_frames == 0,
          "an observed reverse turn was omitted, pruned or uncounted");
    // Rotate the entire construction around the origin. A sampled opposite turn
    // reaches this stationary hurtbox at update three in every orientation;
    // signed extrapolation moves away. Include the +/-pi observation boundary.
    for (int k = 0; k < 16; ++k) {
        const float angle = float(-std::acos(-1.0) + k * (2 * std::acos(-1.0) / 16));
        auto before = scene();
        before.x = 192 + 60 * std::cos(angle + .08f);
        before.y = 200 + 60 * std::sin(angle + .08f);
        before.movement.axis[0] = before.movement.axis[1] = 0;
        before.movement.diag[0] = before.movement.diag[1] = 0;
        before.laser_hitboxes = {{487, 200, 590, 2, 192, 200, angle + .02f, -1}};
        auto after = before;
        after.update = 1;
        after.laser_hitboxes[0].angle = angle;
        on::Policy oriented(config);
        oriented.observe(before);
        const auto result = oriented.decide(after);
        check(result.depth == 2, "turn envelope missed a sampled native rectangle pose");
    }
}
void current_program_turn_cues() {
    std::vector<std::uint8_t> bytes;
    const auto word = [](float value) {
        std::uint32_t bits;
        std::memcpy(&bits, &value, 4);
        return bits;
    };
    const auto append = [&](std::int32_t time, std::int16_t opcode, std::uint16_t flags,
                            std::uint8_t mask, std::initializer_list<std::uint32_t> operands) {
        const auto start = bytes.size();
        const auto length = std::int16_t(12 + 4 * operands.size());
        bytes.resize(start + length);
        std::memcpy(bytes.data() + start, &time, 4);
        std::memcpy(bytes.data() + start + 4, &opcode, 2);
        std::memcpy(bytes.data() + start + 6, &length, 2);
        bytes[start + 9] = mask;
        std::memcpy(bytes.data() + start + 10, &flags, 2);
        std::size_t offset = start + 12;
        for (auto value : operands) {
            std::memcpy(bytes.data() + offset, &value, 4);
            offset += 4;
        }
    };
    append(120, 137, 0, 0xff, {11});
    append(120, 51, 1, 0xff, {word(10018.f), word(0), 120, 48});
    append(120, 15, 1, 8, {word(10094.f), word(.02f)});
    append(120, 16, 1, 8, {word(10094.f), word(.03f)});
    append(120, 17, 3, 1, {word(10094.f), word(10020.f)}); // Disabled rank.
    append(121, 0, 0, 0xff, {});
    const auto hint = on::current_batch_turn_hint(bytes.data(), bytes.size(), 8);
    check(hint.supported && hint.records == 5 && std::abs(hint.radians - .05f) < 1e-7f,
          "literal branch union or difficulty filtering lost the turn cue");
    check(!on::current_batch_turn_hint(bytes.data(), bytes.size() - 1, 8).supported &&
              !on::current_batch_turn_hint(bytes.data(), bytes.size(), 1).supported,
          "truncated or variable turn writes supplied a supported cue");
    const std::int32_t backward = -20;
    std::memcpy(bytes.data() + 16 + 12 + 12, &backward, 4);
    check(!on::current_batch_turn_hint(bytes.data(), bytes.size(), 8).supported,
          "a backward current-batch loop supplied an unbounded cue");

    auto o = scene();
    o.movement.axis[0] = o.movement.axis[1] = 0;
    o.movement.diag[0] = o.movement.diag[1] = 0;
    const float ox = o.x - 300 * std::cos(1.3f), oy = o.y - 300 * std::sin(1.3f);
    o.queued_lasers = {{3, 12, {ox + 295, oy, 590, 8, ox, oy, 1.2f, -1}, .1f}};
    on::Configuration config;
    config.horizon = 12;
    config.beam = 48;
    config.queued_lasers = true;
    const auto unguarded = on::Policy(config).decide(o);
    config.direct_turn_envelope = true;
    const auto guarded = on::Policy(config).decide(o);
    check(unguarded.depth == 12 && guarded.depth == 2 && guarded.search.turn_envelope_frames == 10,
          "a pre-birth turn cue was ignored or applied before its lethal interval");
    o.queued_lasers[0].turn_hint = -1;
    rejects([&] { on::Policy(config).decide(o); }, "a negative turn cue was accepted");
}
void terminal_tail_contract() {
    auto o = scene();
    o.queued_shots = {{9, 8, 192, 400, 0, 0, 40, 200}};
    on::Configuration c;
    c.horizon = 8;
    c.beam = 192;
    c.diversity_cell = 16;
    c.grow = 0;
    c.queued_shots = true;
    auto parent = o;
    parent.queued_shots.clear();
    const auto base = on::Policy(c).decide(parent);
    c.tail_check = 8;
    const auto tail = on::Policy(c).decide(o);
    c.reach_filter = true;
    const auto pruned = on::Policy(c).decide(o);
    check((base.action & 0xf0) == 0 && (tail.action & 0xf0) && base.depth == 8 && tail.depth == 8 &&
              tail.search.tail_best_depth == 8 && tail.search.tail_selected == 1 &&
              tail.action == pruned.action && tail.search.tail_updates >= tail.search.tail_paths,
          "terminal checks failed to retain or count an early escape");
    o = scene();
    const auto clear = on::Policy(c).decide(o);
    check(clear.search.tail_nodes == 1 && clear.search.tail_paths == 1 &&
              clear.search.tail_updates == 8 && clear.search.tail_selected == 0,
          "a complete first tail failed to stop endpoint enumeration");
    o.lethal_regions = {{0, 0, 384, 448}};
    const auto latched = on::Policy(c).decide(o);
    check(latched.depth == 0 && latched.search.tail_paths == 0,
          "tail checks bypassed a lethal latched movement");
    c.horizon = 198;
    c.tail_check = 3;
    rejects([&] { on::Policy invalid(c); }, "an oversized forecast horizon was accepted");
}

void reference_move(const on::Movement &s, float &x, float &y, std::uint16_t in) {
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
void cached_move(const on::Movement &s, float &x, float &y, std::uint16_t input) {
    on::detail::apply_movement(s, x, y, on::detail::movement_delta(s, input));
}
void movement_kernel_contract() {
    using Move = void (*)(const on::Movement &, float &, float &, std::uint16_t);
    Move volatile reference = reference_move, cached = cached_move;
    std::fenv_t saved;
    check(std::fegetenv(&saved) == 0, "cannot capture movement floating environment");
    const auto bits = [](float f) {
        std::uint32_t result;
        std::memcpy(&result, &f, sizeof(result));
        return result;
    };
    for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
        check(std::fesetround(mode) == 0, "cannot set movement rounding mode");
        for (int profile = 0; profile < 20; ++profile) {
            on::Movement s;
            const float small = std::numeric_limits<float>::denorm_min();
            s.axis[0] = profile == 0 ? small : profile == 1 ? -0.f : 4.125f;
            s.axis[1] = profile == 2 ? small : profile == 3 ? 0.f : 2.25f;
            s.diag[0] = 2.828427f;
            s.diag[1] = 1.4142135f;
            s.hmul = profile % 4 == 0   ? -0.f
                     : profile % 4 == 1 ? .3f
                     : profile % 4 == 2 ? -.75f
                                        : 1.25f;
            s.vmul = profile % 3 == 0 ? small : profile % 3 == 1 ? .7f : 1.f;
            s.frm = profile % 5 == 0 ? .625f : profile % 5 == 1 ? -1.f : 1.f;
            s.minx = -384;
            s.maxx = 384;
            s.miny = -448;
            s.maxy = 448;
            const std::array<float, 10> positions{0.f,       -0.f,  small,  -small, 192.125f,
                                                  -192.125f, 384.f, -384.f, 1000.f, -1000.f};
            for (unsigned input = 0; input < 256; ++input)
                for (float position : positions) {
                    float ax = position, ay = position, bx = position, by = position;
                    std::feclearexcept(FE_ALL_EXCEPT);
                    reference(s, ax, ay, std::uint16_t(input | 0x1100));
                    const auto flags = std::fetestexcept(FE_ALL_EXCEPT);
                    std::feclearexcept(FE_ALL_EXCEPT);
                    cached(s, bx, by, std::uint16_t(input | 0x1100));
                    check(bits(ax) == bits(bx) && bits(ay) == bits(by) &&
                              flags == std::fetestexcept(FE_ALL_EXCEPT),
                          "movement decomposition changed float bits or exception flags");
                }
        }
    }
    check(std::fesetenv(&saved) == 0, "cannot restore movement floating environment");
}
void reference_rotate(on::Vector *out, const on::Vector *point, float angle) {
    const float sine = std::sin(angle), cosine = std::cos(angle);
    out->x = cosine * point->x - sine * point->y;
    out->y = cosine * point->y + sine * point->x;
}
void rotation_contract() {
    using Rotate = void (*)(on::Vector *, const on::Vector *, float);
    Rotate volatile reference = reference_rotate;
    std::array<float, 50> angles{};
    std::array<on::Vector, 1000> points{}, expected{};
    std::uint32_t random = 0x7a31f092;
    const auto draw = [&] { return random = random * 1664525u + 1013904223u; };
    for (auto &angle : angles)
        angle = float(int(draw() % 2097153) - 1048576) * .125f;
    const float small = std::numeric_limits<float>::denorm_min();
    const std::array<float, 13> special{
        0,           -0.f,       small,       -small,  1.5707963f, -1.5707963f, 3.1415927f,
        -3.1415927f, 6.2831855f, -6.2831855f, 1048576, -1048576,   .000001f};
    std::copy(special.begin(), special.end(), angles.begin());
    for (auto &point : points)
        point = {float(int(draw() % 2097153) - 1048576) * .5f,
                 float(int(draw() % 2097153) - 1048576) * .5f, 0};
    points[0] = {0, -0.f, 0};
    points[1] = {-0.f, 0, 0};
    points[2] = {small, -small, 0};
    points[3] = {1048576, -1048576, 0};
    const auto bits = [](float value) {
        std::uint32_t result;
        std::memcpy(&result, &value, sizeof(result));
        return result;
    };
    std::fenv_t saved;
    check(std::fegetenv(&saved) == 0, "cannot capture rotation floating environment");
    const int saved_errno = errno;
    for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
        check(std::fesetround(mode) == 0, "cannot set rotation rounding mode");
        for (float angle : angles) {
            std::feclearexcept(FE_ALL_EXCEPT);
            errno = E2BIG;
            for (std::size_t i = 0; i < points.size(); ++i)
                reference(&expected[i], &points[i], -angle);
            const int flags = std::fetestexcept(FE_ALL_EXCEPT), error = errno;
            std::feclearexcept(FE_ALL_EXCEPT);
            errno = E2BIG;
            const auto rotation = on::detail::prepare_rotation(-angle);
            for (std::size_t i = 0; i < points.size(); ++i) {
                const auto actual = rotation.apply(points[i]);
                check(bits(actual.x) == bits(expected[i].x) &&
                          bits(actual.y) == bits(expected[i].y) && actual.z == 0,
                      "prepared rotation changed float results");
            }
            check(std::fetestexcept(FE_ALL_EXCEPT) == flags && errno == error &&
                      std::fegetround() == mode,
                  "rotation batch changed floating environment");
        }
    }
    check(std::fesetenv(&saved) == 0, "cannot restore rotation floating environment");
    errno = saved_errno;
}
bool reference_box_hit(const on::Box &box, const on::Vector &minimum, const on::Vector &maximum) {
    return !(minimum.x > box.x1 || minimum.y > box.y1 || maximum.x < box.x0 || maximum.y < box.y0);
}
void box_predicate_contract() {
    using Predicate = bool (*)(const on::Box &, const on::Vector &, const on::Vector &);
    Predicate volatile reference = reference_box_hit, current = on::detail::box_hit;
    std::fenv_t saved;
    check(std::fegetenv(&saved) == 0, "cannot capture box floating environment");
    const int saved_errno = errno;
    const auto agrees = [&](const on::Box &box, const on::Vector &minimum,
                            const on::Vector &maximum) {
        std::feclearexcept(FE_ALL_EXCEPT);
        std::feraiseexcept(FE_DIVBYZERO);
        errno = E2BIG;
        const bool expected = reference(box, minimum, maximum);
        const int flags = std::fetestexcept(FE_ALL_EXCEPT), error = errno;
        std::feclearexcept(FE_ALL_EXCEPT);
        std::feraiseexcept(FE_DIVBYZERO);
        errno = E2BIG;
        const bool actual = current(box, minimum, maximum);
        check(actual == expected && std::fetestexcept(FE_ALL_EXCEPT) == flags && errno == error,
              "box predicate changed contact or the floating environment");
    };
    for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
        check(std::fesetround(mode) == 0, "cannot set box rounding mode");
        const float small = std::numeric_limits<float>::denorm_min();
        const std::array<float, 13> edges{
            -2, -1,      -small,  -0.f, 0.f, small, 1, 15.999999f, 16, std::nextafter(16.f, 17.f),
            17, 1048576, -1048576};
        for (float x : edges)
            for (float y : edges)
                for (float half : {0.f, .5f, 2.f})
                    agrees({0, 0, 16, 16}, {x - half, y - half, 0}, {x + half, y + half, 0});
        for (unsigned mask = 0; mask < 16; ++mask)
            agrees({0, 0, 1, 1}, {mask & 1 ? 2.f : -1.f, mask & 2 ? 2.f : -1.f, 0},
                   {mask & 4 ? -1.f : 2.f, mask & 8 ? -1.f : 2.f, 0});
        for (unsigned mask = 0; mask < 256; ++mask) {
            const auto zero = [&](unsigned bit) { return mask & (1u << bit) ? -0.f : 0.f; };
            agrees({zero(0), zero(1), zero(2), zero(3)}, {zero(4), zero(5), 0},
                   {zero(6), zero(7), 0});
        }
        std::uint32_t bits = 0x9e3779b9;
        const auto draw = [&] {
            bits ^= bits << 13;
            bits ^= bits >> 17;
            bits ^= bits << 5;
            const std::uint32_t finite = (bits & 0x807fffffu) | (((bits >> 23) % 148) << 23);
            float result;
            std::memcpy(&result, &finite, sizeof(result));
            return result;
        };
        for (unsigned i = 0; i < 100000; ++i) {
            const on::Box box{draw(), draw(), draw(), draw()};
            const on::Vector minimum{draw(), draw(), 0}, maximum{draw(), draw(), 0};
            agrees(box, minimum, maximum);
        }
        check(std::fegetround() == mode, "box predicate changed rounding mode");
    }
    check(std::fesetenv(&saved) == 0, "cannot restore box floating environment");
    errno = saved_errno;
}
void phase_timing_contract() {
    on::Configuration config;
    config.horizon = 12;
    config.beam = 48;
    config.diversity_cell = 16;
    config.reach_filter = config.held_fallback = config.warm_fallback = true;
    config.tail_check = 8;
    on::Policy reference(config), profiled(config);
    on::DecisionTiming timing;
    auto o = scene();
    for (std::uint64_t i = 0; i < 20; ++i) {
        o.update = i;
        o.bullets = {bullet(0, 180 + i, 390, 1, 0), bullet(1, 220, 360 + i, 0, 1)};
        if (i == 10)
            o.bullets.push_back(bullet(2, o.x, o.y, 0, 0));
        const auto a = reference.decide(o), b = profiled.decide(o, &timing);
        check(a.action == b.action && a.depth == b.depth &&
                  a.search.selections == b.search.selections &&
                  a.search.candidates == b.search.candidates && a.search.bins == b.search.bins &&
                  a.search.retained_bins == b.search.retained_bins &&
                  a.search.held_updates == b.search.held_updates &&
                  a.search.warm_updates == b.search.warm_updates &&
                  a.search.tail_updates == b.search.tail_updates,
              "phase clocks changed decisions, search order or reused policy state");
        check(timing.history_ns + timing.forecast_ns + timing.index_ns + timing.search_ns > 0,
              "enabled phase clocks did not record work");
        rejects([&] { profiled.decide(o, &timing); }, "profiled duplicate decision was accepted");
        check(timing.history_ns == 0 && timing.forecast_ns == 0 && timing.index_ns == 0 &&
                  timing.search_ns == 0 && timing.fallback_ns == 0,
              "a rejected decision retained an earlier timing sample");
        o.latched_input = a.action;
    }
}
void narrow_arc_contract() {
    on::Observation o;
    o.x = 10.343133f;
    o.y = 271.997406f;
    o.movement.minx = o.movement.miny = 0;
    o.movement.maxx = 384;
    o.movement.maxy = 448;
    o.movement.half = {1, 1, 0};
    constexpr float origin_x = 209.311523f, origin_y = 59.083252f;
    constexpr float previous_angle = 1.88237405f, current_angle = 1.88802648f;
    o.laser_hitboxes = {
        {origin_x + 295, origin_y, 590, 240, origin_x, origin_y, previous_angle, -1}};
    on::Configuration config;
    config.horizon = 2;
    config.beam = 8;
    config.direct_turn_envelope = true;
    on::Policy policy(config);
    policy.observe(o);
    o.update = 1;
    o.laser_hitboxes.front().angle = current_angle;
    const auto decision = policy.decide(o);
    // Independently sample the static source predicate across the first-update
    // arc. The second arc reaches the box, so a stationary policy stops at one.
    unsigned hits = 0;
    const auto delta = current_angle - previous_angle;
    for (unsigned i = 0; i <= 10000; ++i) {
        const float angle = current_angle - delta + 2 * delta * (float(i) / 10000);
        th08::geometry::Laser laser{
            {{origin_x + 295, origin_y}, {590, 240}}, {origin_x, origin_y}, angle};
        hits += th08::geometry::laser_hit({o.x, o.y}, {1, 1}, laser);
    }
    check(decision.depth == 1 && hits == 0,
          "narrow-arc rounding widened a clear first update into a collision");
    // Wide arcs need a union of endpoint half-planes; narrow arcs need their
    // intersection and a center-hemisphere check, including near-zero angles.
    o.laser_hitboxes.clear();
    o.movement.minx = o.movement.miny = 0;
    o.movement.maxx = 384;
    o.movement.maxy = 448;
    config.queued_lasers = true;
    auto depth = [&](float x, float y, float reach) {
        o.x = x;
        o.y = y;
        o.queued_lasers = {{1, 1, {292, 200, 10, 10, 192, 200, 0, -1}, reach}};
        return on::Policy(config).decide(o).depth;
    };
    check(depth(192, 300, 2) == 0 && depth(92, 200, 2) == 2,
          "a wide arc lost its endpoint half-plane union");
    check(depth(92, 200, 3.14159274f) == 0 && depth(92, 200, 1e-6f) == 2,
          "full-circle or near-zero arc membership changed");
}

} // namespace
int main(int argc, char **argv) {
    try {
        policy_style_contract();
        legacy_style_signature();
        score_goal_contract();
        score_form_contract();
        check(argc == 2 || argc == 3, "test needs scratch and optional saved-model directories");
        risk_rows(argv[1]);
        if (argc == 3)
            saved_risk_tables(argv[2]);
        spawn_neighborhood_contract();
        committed_prefix_contract();
        planner_lag_contract();
        lazy_index_contract();
        hard_phantom_contract();
        motion_envelope_contract();
        branch_value_contract();
        child_birth_contract();
        scheduled_expiry_contract();
        child_cursor_contract();
        retain_heading_contract();
        observation_history();
        transform_motion_history();
        pruning_and_isolation();
        search_scratch_contract();
        proposal_set_contract();
        actor_risk();
        spatial_retention();
        queued_birth();
        queued_direct_laser();
        queued_pooled_laser();
        current_relative_turn();
        held_escape();
        current_spawn_clock();
        warm_escape();
        reversing_direct_laser();
        current_program_turn_cues();
        terminal_tail_contract();
        phase_timing_contract();
        movement_kernel_contract();
        rotation_contract();
        box_predicate_contract();
        narrow_arc_contract();
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
