#include "test_support.hpp"
#include "th08/ecl_turn_hint.hpp"
#include "th08/online.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>

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

    // A vector transform that remains active can really accelerate from rest.
    // Keep that measured rate rather than treating every zero-speed jump alike.
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
} // namespace
int main(int argc, char **argv) {
    try {
        check(argc == 2, "test needs an isolated scratch directory");
        risk_rows(argv[1]);
        observation_history();
        transform_motion_history();
        pruning_and_isolation();
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
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
