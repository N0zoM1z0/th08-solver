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
} // namespace
int main(int argc, char **argv) {
    try {
        check(argc == 2, "test needs an isolated scratch directory");
        risk_rows(argv[1]);
        observation_history();
        pruning_and_isolation();
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
