#include "test_support.hpp"
#include "th08/online.hpp"

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
} // namespace
int main(int argc, char **argv) {
    try {
        check(argc == 2, "test needs an isolated scratch directory");
        risk_rows(argv[1]);
        observation_history();
        pruning_and_isolation();
        actor_risk();
        spatial_retention();
        queued_birth();
        queued_direct_laser();
        held_escape();
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
