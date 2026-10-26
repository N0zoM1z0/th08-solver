#include "EnemyManager.hpp"
#include "Player.hpp"
#include "Spellcard.hpp"
#include "modern/headless/score_audit.hpp"
#include "modern/headless/session.hpp"
#include "th08/shadow_native.hpp"
#include "th08/shadow_rollout.hpp"
#include <filesystem>
#include <iostream>
#include <unistd.h>

namespace h = th08::headless;
namespace s = th08::shadow;
void check(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
int main(int argc, char **argv) try {
    check(argc == 2 || argc == 3, "expected private DAT and optional selected story replay");
    const auto dat = std::filesystem::absolute(argv[1]).string();
    const auto replay = argc == 3 ? std::filesystem::absolute(argv[2]).string() : std::string{};
    const auto previous = std::filesystem::current_path();
    char scratch[] = "/tmp/th08-capture-goal-test-XXXXXX";
    check(mkdtemp(scratch), "goal scratch failed");
    std::filesystem::current_path(scratch);
    h::score_audit_enable();
    h::Config config;
    config.dat_path = dat.c_str();
    config.difficulty = 3;
    config.shot = 1;
    config.input_profile = h::InputProfile::SampledGameplay;
    if (!replay.empty()) {
        config.replay = replay.c_str();
        config.input_profile = h::InputProfile::Controller;
    }
    auto &session = *new h::Session(config);
    if (!replay.empty()) {
        bool clear = false;
        for (unsigned i = 0; i < 40000; ++i) {
            const auto state = session.step(0);
            h::score_audit_observe();
            if (state.stage == 0 && state.stage_complete) {
                clear = true;
                break;
            }
        }
        check(clear && h::score_capture_coverage(h::score_audit(), true, 0).all,
              "selected story did not reach its captured first-stage clear");
        bool handoff = false;
        for (unsigned i = 0; i < 256; ++i) {
            const auto before = session.state();
            const auto after = session.step(0);
            h::score_audit_observe();
            check(!s::capture_goal_failed(true, before.stage, after),
                  "new-stage clear-flag carry falsely failed capture coverage");
            if (before.stage == 0 && after.stage == 1) {
                handoff = true;
                check(!h::score_capture_coverage(h::score_audit(), true).all &&
                          h::score_capture_coverage(h::score_audit(), true, 0).all,
                      "handoff did not expose the new incomplete catalogue");
                // Playback can batch the control step. Project the recording
                // owner's observed carried flag without changing the world.
                auto carried = after;
                carried.stage_complete = true;
                check(!s::capture_goal_failed(true, before.stage, carried),
                      "recording clear-flag carry invalidated an unfinished new stage");
            }
        }
        check(handoff, "selected replay missed the control-only handoff");
        std::filesystem::current_path(previous);
        std::filesystem::remove_all(scratch);
        std::cout << "selected-story lifecycle and projected recording clear flag pass\n";
        return 0;
    }
    bool reached = false;
    for (unsigned i = 0; i < 40000; ++i) {
        // Explicit synthetic invulnerability checkpoint: isolate the capture
        // goal from dodging. This is not evidence of a playable NMNB route.
        th08::g_Player.playerState = th08::PLAYER_STATE_INVULNERABLE;
        th08::g_Player.timer = 100000;
        session.step(0x1000);
        h::score_audit_observe();
        if (th08::g_Spellcard.IsActive() && th08::g_Spellcard.activeEnemy &&
            !(th08::g_Spellcard.activeEnemy->flags1 & th08::ENEMY_FLAG_TIMEOUT_SPELL) &&
            th08::g_Spellcard.timeRemaining.current > 0 &&
            th08::g_Spellcard.timeRemaining.current <= 32) {
            reached = true;
            break;
        }
    }
    check(reached && !h::score_capture_failed(h::score_audit()),
          "ordinary source timeout checkpoint was not reached");
    s::Packet origin;
    origin.state = s::capture(session);
    const std::vector<std::uint16_t> inputs(256, 0x1000);
    const auto safety = s::predict(session, origin, inputs, {false, -1}, 10000);
    const auto goal = s::predict(session, origin, inputs, {false, -1, true}, 10000);
    check(safety.certifies() && goal.status == s::BranchStatus::CaptureFailure &&
              !goal.certifies() && goal.completed > 0 && goal.completed < safety.completed &&
              goal.charged_updates == goal.completed,
          "ordinary timeout was accepted as an all-capture future or its work was hidden");
    for (std::size_t i = 0; i < goal.frames.size(); ++i)
        check(!s::difference(goal.frames[i], safety.frames[i]),
              "capture goal changed Native gameplay before stopping");
    check(s::capture(session).values == origin.state.values &&
              !h::score_capture_failed(h::score_audit()),
          "capture failure in a child changed the actual checkpoint/audit");
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(scratch);
    std::cout << "synthetic-invulnerability checkpoint: ordinary timeout rejected after "
              << goal.completed << " Native updates\n";
    return 0;
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
