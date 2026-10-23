#include "test_support.hpp"
#include "th08/plan_beam.hpp"
#include "th08/plan_contracts.hpp"
#include "th08/plan_matrix.hpp"
#include "th08/plan_observation.hpp"
#include "th08/plan_prefix.hpp"
#include "th08/plan_route.hpp"

#include <cerrno>
#include <cfenv>
#include <csignal>
#include <cstring>
#include <limits>
#include <new>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <unordered_set>
#include <vector>

namespace {
using th08::plan::RolloutStatus;
using th08::plan::RolloutSummary;
using th08::test::check;

th08::plan::Options options(std::initializer_list<const char *> extra) {
    std::vector<const char *> args{"th08_plan", "--dat", "private.dat", "--out", "output"};
    args.insert(args.end(), extra);
    return th08::plan::parse_options(int(args.size()), args.data());
}

void rejected(std::initializer_list<const char *> args) {
    try {
        options(args);
    } catch (const std::invalid_argument &) {
        return;
    }
    check(false, "invalid planner option was accepted");
}

void cli_contract() {
    const auto large = options({"--rollout", "240", "--commit", "240", "--cegar", "0"});
    check(large.rollout == 240 && large.commit == 240 && large.cegar == 0,
          "explicit large horizon or zero retry budget changed");
    check(options({"--stage", "extra", "--difficulty", "4", "--seed", "65535"}).stage == 8,
          "valid native checkpoint rejected");
    check(options({"--stage", "extra", "--difficulty", "4", "--story", "1"}).story,
          "a full Extra run was rejected");
    check(options({"--story", "1", "--final-route", "b"}).final_route == th08::plan::FinalRoute::B,
          "explicit Final B target was lost");
    rejected({"--final-route", "b"});
    rejected({"--story", "1", "--final-route", "unknown"});
    rejected({"--story", "1", "--stage", "extra", "--difficulty", "3"});
    rejected({"--story", "1", "--difficulty", "4"});
    rejected({"--story", "1", "--stage", "extra", "--difficulty", "4", "--final-route", "a"});
    rejected({"--rollout", "241"});
    rejected({"--rollout", "0"});
    rejected({"--beam", "-1"});
    rejected({"--commit", "65"});
    rejected({"--frames", "100junk"});
    rejected({"--seed", "65536"});
    rejected({"--seed", "999999999999999999999999"});
    rejected({"--seconds", "nan"});
    rejected({"--seconds", "inf"});
    rejected({"--seconds", "0"});
    rejected({"--boss-follow", "-0.1"});
    rejected({"--story", "2"});
    check(options({}).quiet_retries &&
              !options({"--quiet-retries", "0", "--shoot", "1"}).quiet_retries &&
              options({"--quiet-retries", "1"}).quiet_retries,
          "teacher shooting constraints changed the legacy default or lost the opt-in");
    rejected({"--quiet-retries", "2"});
    rejected({"--story", "1", "--spell", "92", "--stage", "4b"});
    rejected({"--spell", "92"});
    const auto spell = options({"--stage", "4b", "--spell", "92"});
    check(spell.stage == 4 && spell.spell == 92,
          "explicit spell stage was not preserved in the checkpoint");
    rejected({"--stage", "bogus"});
    rejected({"--unknown", "1"});
    rejected({"--unknown"});
    rejected({"--seconds"});
    rejected({"--prefix", ""});
    rejected({"--prefix", "prefix.actions"});
    rejected({"--prefix", "prefix.actions", "--frames", "100"});
    rejected({"--prefix", "prefix.actions", "--seconds", "10"});
    rejected({"--prefix", "prefix.actions", "--frames", "100", "--seconds", "10", "--verify",
              "verify.actions"});
    check(options({"--prefix", "prefix.actions", "--frames", "100", "--seconds", "10"}).prefix ==
              "prefix.actions",
          "explicit bounded prefix option was rejected");
    check(options({}).diversity_cell == 0 &&
              options({"--diversity-cell", "48"}).diversity_cell == 48,
          "spatial diversity is not an explicit opt-in");
    rejected({"--diversity-cell", "-1"});
    rejected({"--diversity-cell", "1025"});
    check(th08::plan::nmnb_input("4101") == 4101, "valid movement/focus/dialogue input rejected");
    try {
        th08::plan::nmnb_input("2");
        check(false, "a bomb action could verify an NMNB route");
    } catch (const std::invalid_argument &) {
    }
}

void tape_contract() {
    check(th08::plan::fits_tape(2048, 640, 512, 2048, 640, 512), "exact tape capacity rejected");
    check(!th08::plan::fits_tape(2049, 640, 512, 2048, 640, 512) &&
              !th08::plan::fits_tape(2048, 641, 512, 2048, 640, 512) &&
              !th08::plan::fits_tape(2048, 640, 513, 2048, 640, 512),
          "a hazard family could be silently truncated");
    check(th08::plan::verified_window({240, 0, RolloutStatus::Horizon}, 240),
          "full native horizon rejected");
    check(th08::plan::verified_window({3, 0, RolloutStatus::Terminal}, 240),
          "actual early terminal rejected");
    check(!th08::plan::verified_window({239, 0, RolloutStatus::Horizon}, 240) &&
              !th08::plan::verified_window({239, 239, RolloutStatus::Death}, 240),
          "a short or late-death window was accepted");
    check(th08::plan::failed_rollout({239, 0, RolloutStatus::Running}, 240) &&
              !th08::plan::failed_rollout({239, 239, RolloutStatus::Death}, 240),
          "runtime failure was confused with an observed native death");
}

void child_contract() {
    void *storage = mmap(nullptr, sizeof(RolloutSummary), PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    check(storage != MAP_FAILED, "test shared allocation failed");
    auto *result = new (storage) RolloutSummary{};
    const pid_t incomplete = fork();
    check(incomplete >= 0, "test fork failed");
    if (incomplete == 0) {
        // This reproduces the former catch-and-break path: death stays zero,
        // but only part of the requested native horizon has been stepped.
        *result = {3, 0, RolloutStatus::StepError};
        _exit(1);
    }
    const auto incomplete_child = th08::plan::wait_for_child(incomplete);
    check(!incomplete_child.exited() && th08::plan::failed_rollout(*result, 64) &&
              !th08::plan::verified_window(*result, 64),
          "an aborted native update became a death-free success");

    const pid_t signaled = fork();
    check(signaled >= 0, "test fork failed");
    if (signaled == 0) {
        // Even a completely published tape cannot hide abnormal child exit.
        *result = {64, 0, RolloutStatus::Horizon};
        kill(getpid(), SIGKILL);
        _exit(0);
    }
    const auto signaled_child = th08::plan::wait_for_child(signaled);
    check(!signaled_child.exited() && WIFSIGNALED(signaled_child.status),
          "a signaled rollout child was reported successful");

    const pid_t unrelated = fork();
    check(unrelated >= 0, "test fork failed");
    if (unrelated == 0)
        _exit(0);
    check(!th08::plan::wait_for_child(-1).waited, "failed fork could wait for an unrelated child");
    check(th08::plan::wait_for_child(unrelated).exited(),
          "invalid child wait consumed another child's result");
    munmap(storage, sizeof(RolloutSummary));
}

void observation_contract() {
    th08::headless::State expected;
    expected.frame = 64;
    expected.rng_seed = 123;
    expected.rng_draws = 40;
    expected.graze = 10;
    auto committed = expected;
    check(th08::plan::same_observation(expected, committed), "identical observation rejected");
    ++committed.rng_draws;
    check(!th08::plan::same_observation(expected, committed),
          "same-position commit hid a gameplay RNG consumption change");
    committed = expected;
    committed.stage_complete = true;
    check(!th08::plan::same_observation(expected, committed),
          "same-position commit hid a lifecycle transition");
    committed = expected;
    ++committed.graze;
    check(!th08::plan::same_observation(expected, committed),
          "same-position commit hid feedback divergence");
    committed = expected;
    committed.youkai = !expected.youkai;
    check(!th08::plan::same_observation(expected, committed),
          "same-input commit hid a source-owned player form change");
    committed = expected;
    ++committed.calculation_updates;
    check(!th08::plan::same_observation(expected, committed),
          "same-position commit hid a native calculation boundary change");
    committed = expected;
    committed.final_route_selection = true;
    check(!th08::plan::same_observation(expected, committed),
          "a dialogue controller boundary was omitted from commit agreement");
    check(th08::plan::supported_recording_step(1, false) &&
              th08::plan::supported_recording_step(0, true) &&
              !th08::plan::supported_recording_step(0, false) &&
              !th08::plan::supported_recording_step(2, true),
          "unexplained skipped or batched recording calculations were accepted");
}

void route_contract() {
    using th08::plan::FinalRoute;
    th08::headless::State state;
    check(!th08::plan::final_route_input(FinalRoute::B, state, 1),
          "route control replaced gameplay input outside the choice");
    state.final_route_selection = true;
    check(!th08::plan::final_route_input(FinalRoute::Natural, state, 1),
          "legacy automatic route behavior changed");
    check(th08::plan::final_route_input(FinalRoute::A, state, 1) == 20 &&
              th08::plan::final_route_input(FinalRoute::B, state, 1) == 36 &&
              th08::plan::final_route_input(FinalRoute::B, state, 2) == 5,
          "route choice must alternate direction and shoot edges in the recorded tape");
    check(th08::plan::story_terminal(8, true, 8) && !th08::plan::story_terminal(8, false, 8) &&
              !th08::plan::story_terminal(5, true, 8) && th08::plan::story_terminal(7, true, 8),
          "Extra or regular full-run terminal boundary is incorrect");
}

void matrix_contract() {
    int counts[12][5]{};
    int final_counts[12][2]{};
    int fourth_counts[2]{};
    for (const auto &entry : th08::plan::full_run_matrix()) {
        ++counts[entry.shot][entry.difficulty];
        if (entry.difficulty == 4) {
            check(entry.stage() == 8 && entry.final_stage() == 8 &&
                      entry.route == th08::plan::FinalRoute::Natural,
                  "Extra must be one independent full run per shot");
        } else {
            ++final_counts[entry.shot][entry.final_stage() - 6];
            ++fourth_counts[entry.fourth_stage() - 3];
        }
    }
    for (int shot = 0; shot < 12; ++shot) {
        for (int difficulty = 0; difficulty < 5; ++difficulty)
            check(counts[shot][difficulty] == (difficulty == 4 ? 1 : 2),
                  "full-run matrix omitted or duplicated a shot/difficulty/route");
        check(final_counts[shot][0] == 4 && final_counts[shot][1] == 4,
              "both final routes must exist at all four standard difficulties");
    }
    check(fourth_counts[0] == 48 && fourth_counts[1] == 48,
          "native character-dependent Stage 4 partition changed");
}

void spatial_beam_contract() {
    using th08::plan::BeamNode;
    const std::vector<BeamNode> input{
        {192.125f, 400.25f, 11, 4, 0, 0},   {193.125f, 400.25f, 12, 20, 0, 1},
        {194.125f, 400.25f, 13, 36, 0, 2},  {195.125f, 400.25f, 14, 68, 0, 3},
        {32.875f, 400.25f, 77, 68, 2, 100},
    };
    auto global = input;
    std::sort(global.begin(), global.end(),
              [](const BeamNode &a, const BeamNode &b) { return a.score < b.score; });
    global.resize(3);
    // A later gate leaves only x < 64 open. Center score alone removes every
    // already reachable exit before that gate enters the hazard horizon.
    const auto reaches_exit = [](const BeamNode &node) { return node.x < 64; };
    check(std::none_of(global.begin(), global.end(), reaches_exit),
          "fixture does not reproduce center pruning");
    auto diverse = input;
    const auto selected = th08::plan::retain_spatial_beam(diverse, 3, 48);
    const auto exit = std::find_if(diverse.begin(), diverse.end(), reaches_exit);
    check(exit != diverse.end() && selected.candidates == 5 && selected.bins == 2 &&
              selected.retained_bins == 2 && diverse.size() == 3,
          "spatial pruning discarded a reachable exit or misstated its work");
    check(th08::plan::same_float(exit->x, input.back().x) &&
              th08::plan::same_float(exit->y, input.back().y) && exit->parent == 77 &&
              exit->act == 68 && exit->viol == 2 && exit->score == 100,
          "spatial pruning invented a state or changed its parent chain");
    check(diverse[0].parent == 11 && diverse[1].parent == 12,
          "remaining spatial beam slots do not follow the existing score rank");

    std::vector<BeamNode> many_bins{
        {160, 100, 3, 4, 0, 3}, {80, 100, 2, 4, 0, 2}, {16, 100, 1, 4, 0, 1}};
    const auto bounded = th08::plan::retain_spatial_beam(many_bins, 2, 48);
    check(bounded.bins == 3 && bounded.retained_bins == 2 && many_bins[0].parent == 1 &&
              many_bins[1].parent == 2,
          "too many spatial bins bypassed the beam budget or score ordering");
    std::vector<BeamNode> focus_bins{{100, 100, 1, 0, 0, 1}, {100, 100, 2, 4, 0, 1}};
    const auto focus = th08::plan::retain_spatial_beam(focus_bins, 2, 48);
    check(focus.bins == 2 && focus_bins[0].parent == 1 && focus_bins[1].parent == 2,
          "focus diversity or stable equal-score ordering was lost");
}

void half_pixel_quantization_contract() {
    const auto reference = [](float x) { return std::uint32_t(std::lround(x * 2)) & 2047u; };
    const auto agrees = [&](float x) {
        check(th08::plan::detail::half_pixel_bin(x) == reference(x),
              "half-pixel key differs from the original lround/mask expression");
    };
    // Every rounding boundary in the binary32 fast range, plus the adjacent
    // representable values on both sides. Include sign changes and key wrapping.
    for (int i = -4194304; i < 4194304; ++i) {
        const float x = (float(i) + .5f) * .5f;
        agrees(std::nextafter(x, -std::numeric_limits<float>::infinity()));
        agrees(x);
        agrees(std::nextafter(x, std::numeric_limits<float>::infinity()));
    }
    std::uint32_t random = 0x76543210;
    const auto draw = [&] { return random = random * 1664525u + 1013904223u; };
    if constexpr (sizeof(float) == sizeof(std::uint32_t) && std::numeric_limits<float>::is_iec559) {
        for (int i = 0; i < 1000000; ++i) {
            const auto bits = (draw() & 0x807fffffu) | (((draw() >> 16) % 148u) << 23);
            float x;
            std::memcpy(&x, &bits, sizeof(x));
            agrees(x);
        }
    }
    std::fenv_t environment;
    check(std::fegetenv(&environment) == 0, "cannot capture the floating environment");
    const int saved_errno = errno;
    for (const int mode : {FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO}) {
        check(std::fesetround(mode) == 0, "cannot select the rounding-mode fixture");
        for (const float value :
             {0.f, -0.f, .24999998f, .25f, .25000003f, -.24999998f, -.25f, -.25000003f, 1048576.f,
              -1048576.f, 2097152.f, -2097152.f, std::numeric_limits<float>::denorm_min(),
              std::numeric_limits<float>::max(), std::numeric_limits<float>::infinity(),
              std::numeric_limits<float>::quiet_NaN()}) {
            volatile float input = value;
            std::feclearexcept(FE_ALL_EXCEPT);
            errno = E2BIG;
            const volatile auto expected = reference(input);
            const auto expected_flags = std::fetestexcept(FE_ALL_EXCEPT);
            const auto expected_errno = errno;
            std::feclearexcept(FE_ALL_EXCEPT);
            errno = E2BIG;
            const volatile auto actual = th08::plan::detail::half_pixel_bin(input);
            const bool same = actual == expected &&
                              std::fetestexcept(FE_ALL_EXCEPT) == expected_flags &&
                              errno == expected_errno && std::fegetround() == mode;
            if (!same) {
                std::fesetenv(&environment);
                errno = saved_errno;
                check(false, "half-pixel key changed rounding mode, exception flags or errno");
            }
        }
    }
    std::fesetenv(&environment);
    errno = saved_errno;
}

// Frozen full-sort selector: the independent oracle for partial rank selection.
th08::plan::BeamSelection reference_spatial_beam(std::vector<th08::plan::BeamNode> &nodes,
                                                 std::size_t width, int cell_size) {
    std::stable_sort(nodes.begin(), nodes.end(),
                     [](const auto &a, const auto &b) { return a.score < b.score; });
    const auto limit = std::min(width, nodes.size());
    std::vector<unsigned char> selected(nodes.size(), 0);
    std::unordered_set<th08::plan::detail::SpatialBin, th08::plan::detail::SpatialHash> bins;
    std::size_t count = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        const auto &node = nodes[i];
        const th08::plan::detail::SpatialBin bin{
            th08::plan::detail::bin_coordinate(node.x, cell_size),
            th08::plan::detail::bin_coordinate(node.y, cell_size), (node.act & 4) != 0};
        if (bins.insert(bin).second && count < limit) {
            selected[i] = 1;
            ++count;
        }
    }
    const th08::plan::BeamSelection result{nodes.size(), bins.size(), count};
    for (std::size_t i = 0; i < nodes.size() && count < limit; ++i)
        if (!selected[i]) {
            selected[i] = 1;
            ++count;
        }
    std::size_t output = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i)
        if (selected[i])
            nodes[output++] = nodes[i];
    nodes.resize(output);
    return result;
}

void spatial_beam_rank_agreement() {
    using th08::plan::BeamNode;
    std::uint32_t random = 0x89abcdef;
    const auto draw = [&] { return random = random * 1664525u + 1013904223u; };
    for (const int size : {0, 1, 2, 37, 193, 1024, 3500})
        for (const int width : {1, 2, 17, 192, 4096})
            for (const int cell : {1, 16, 48})
                for (int mode = 0; mode < 4; ++mode) {
                    std::vector<BeamNode> actual;
                    for (int i = 0; i < size; ++i) {
                        const auto coordinate = [&] {
                            return (int(draw() % (mode & 1 ? 32 : 2048)) - 16) * 0.5f;
                        };
                        const float x = coordinate(), y = coordinate();
                        const auto action = std::uint16_t(draw() & 0xf4);
                        const auto violations = std::uint16_t(draw() & 15);
                        const auto score = float(int(draw() % (mode & 2 ? 7 : 100000)) - 3);
                        actual.push_back({x, y, i, action, violations, score});
                    }
                    if (size > 1) {
                        actual[0].score = -0.f;
                        actual[1].score = 0.f;
                    }
                    if (size > 3 && (mode & 2)) {
                        actual[2].score = -std::numeric_limits<float>::infinity();
                        actual[3].score = std::numeric_limits<float>::infinity();
                    }
                    auto expected = actual;
                    const auto reference = reference_spatial_beam(expected, width, cell);
                    const auto selected = th08::plan::retain_spatial_beam(actual, width, cell);
                    check(selected.candidates == reference.candidates &&
                              selected.bins == reference.bins &&
                              selected.retained_bins == reference.retained_bins &&
                              actual.size() == expected.size(),
                          "partial spatial rank changed membership counts");
                    for (std::size_t i = 0; i < actual.size(); ++i) {
                        const auto &a = actual[i], &e = expected[i];
                        check(th08::plan::same_float(a.x, e.x) &&
                                  th08::plan::same_float(a.y, e.y) &&
                                  th08::plan::same_float(a.score, e.score) &&
                                  a.parent == e.parent && a.act == e.act && a.viol == e.viol,
                              "partial spatial rank changed a node, stable tie or parent chain");
                    }
                }
    for (const int cell : {0, 16}) {
        std::vector<BeamNode> invalid{{std::numeric_limits<float>::infinity(), 0, 0, 0, 0, 0}};
        try {
            th08::plan::retain_spatial_beam(invalid, 1, cell);
        } catch (const std::invalid_argument &) {
            continue;
        }
        check(false, "invalid spatial cell or coordinate stopped failing");
    }
}

void prefix_contract() {
    th08::plan::PrefixParser parser(3);
    // Simulate the file/hash buffer ending in the middle of a decimal input.
    parser.append("5\r\n41", 5);
    parser.append("01 000000128", 12);
    const auto actions = parser.finish();
    check(actions == std::vector<std::uint16_t>({5, 4101, 128}),
          "streamed prefix parsing changed an action at a buffer boundary");
    auto invalid = [](const char *text, std::size_t maximum) {
        try {
            th08::plan::PrefixParser reader(maximum);
            reader.append(text, std::char_traits<char>::length(text));
            reader.finish();
        } catch (const std::invalid_argument &) {
            return;
        }
        check(false, "invalid or over-budget prefix was accepted");
    };
    invalid("5 2", 3);
    invalid("5 65536", 3);
    invalid("5 -1", 3);
    invalid("5 1junk", 3);
    invalid("5 5 5", 2);
    using th08::plan::PrefixBoundary;
    check(th08::plan::prefix_boundary(3, 2, true) == PrefixBoundary::TrailingTerminal &&
              th08::plan::prefix_boundary(3, 2, false) == PrefixBoundary::Incomplete,
          "a truncated or terminal-with-trailing-input prefix could resume planning");
    check(th08::plan::prefix_boundary(3, 3, true) == PrefixBoundary::Complete &&
              th08::plan::prefix_boundary(3, 3, false) == PrefixBoundary::Resume,
          "an exact prefix boundary was misclassified");
}
} // namespace

int main() {
    cli_contract();
    tape_contract();
    child_contract();
    observation_contract();
    route_contract();
    matrix_contract();
    spatial_beam_contract();
    half_pixel_quantization_contract();
    spatial_beam_rank_agreement();
    prefix_contract();
}
