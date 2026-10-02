#include "test_support.hpp"
#include <iostream>
#include <th08/scenario.hpp>

namespace scene = th08::scenario;
using th08::test::check;
namespace {
bool same_rng(const th08::random::Rng &a, const th08::random::Rng &b) {
    const auto x = a.state(), y = b.state();
    return x.seed == y.seed && x.generation_count == y.generation_count &&
           x.saved_seed == y.saved_seed && x.saved_seed_valid == y.saved_seed_valid;
}
void continuity_and_rng() {
    auto initial = scene::checkpoint(scene::profile("relay", 900), 17);
    auto &world = initial.world;
    for (unsigned frame = 0; frame < 300; ++frame)
        scene::step(world);
    check(!world.bullets.empty() && world.phase == 0, "curtain ended without carried bullets");
    const auto before = world;
    const auto carried = world.bullets.front();
    const auto model = scene::forecast(world, 120);
    check(world.frame == before.frame && world.bullets.size() == before.bullets.size() &&
              world.digests.world == before.digests.world &&
              same_rng(world.gameplay_rng, before.gameplay_rng) &&
              same_rng(world.visual_rng, before.visual_rng),
          "lookahead mutated the committed checkpoint");
    scene::step(world);
    check(world.phase == 1 && world.transitions.size() == 1 &&
              world.transitions[0].carried_bullets == before.bullets.size(),
          "phase transition reset or lost live bullets");
    const auto found =
        std::find_if(world.bullets.begin(), world.bullets.end(),
                     [&](const scene::Bullet &bullet) { return bullet.id == carried.id; });
    check(found != world.bullets.end() &&
              found->position.x == carried.position.x + carried.velocity.x &&
              found->position.y == carried.position.y + carried.velocity.y,
          "carried bullet failed to continue its pre-boundary motion");
    auto expected_rng = before.gameplay_rng;
    for (unsigned draw = 0; draw < 4; ++draw)
        expected_rng.next_u16();
    check(same_rng(world.gameplay_rng, expected_rng) &&
              world.visual_rng.generation_count() == before.visual_rng.generation_count() + 2 &&
              world.transitions[0].gameplay_draws == before.gameplay_rng.generation_count() &&
              world.transitions[0].visual_draws == before.visual_rng.generation_count(),
          "phase transition reset or reordered RNG");
    auto actual = before;
    for (std::size_t frame = 0; frame < model.frames.size(); ++frame) {
        scene::step(actual);
        for (const auto player :
             {scene::Vec2{32, 352}, scene::Vec2{192, 352}, scene::Vec2{304, 96}})
            check(model.frames[frame].scan(player) == scene::collision(actual, player),
                  "forecast collision phase differs from freshly advanced world");
    }
    auto visual_zero = scene::checkpoint(scene::profile("relay", 900, 0), 17).world;
    auto visual_seven = scene::checkpoint(scene::profile("relay", 900, 7), 17).world;
    for (unsigned frame = 0; frame < 900; ++frame) {
        scene::step(visual_zero);
        scene::step(visual_seven);
    }
    check(visual_zero.digests.world == visual_seven.digests.world &&
              visual_zero.digests.events == visual_seven.digests.events &&
              visual_zero.digests.gameplay_rng == visual_seven.digests.gameplay_rng &&
              visual_zero.digests.visual_rng != visual_seven.digests.visual_rng &&
              visual_seven.visual_rng.generation_count() == 6300,
          "controlled visual hook contaminated gameplay or failed to record draws");
}
void full_duration_and_replay() {
    const auto initial = scene::checkpoint(scene::profile("relay"), 1);
    const auto result = scene::run(initial);
    check(result.outcome == scene::Outcome::survived && result.completed_frames == 7200 &&
              result.actions.size() == 7200 && result.death_frame == 0 && result.replay_verified,
          "complete 7200-frame rolling route did not survive fresh replay");
    check(result.peak_model_frames == 120 && result.forecast_frames > 7200 &&
              result.peak_live_bullets > 100 && result.transitions.size() == 2 &&
              result.transitions[0].carried_bullets > 0 &&
              result.transitions[1].carried_bullets > 0,
          "long scenario lost rolling memory bound or phase continuity");
    const auto repeated = scene::run(initial);
    check(result.actions == repeated.actions && result.expansions == repeated.expansions &&
              result.digests.world == repeated.digests.world &&
              result.route_digest == repeated.route_digest &&
              result.digests.events == repeated.digests.events &&
              result.digests.gameplay_rng == repeated.digests.gameplay_rng &&
              result.digests.visual_rng == repeated.digests.visual_rng,
          "repeated seeded scenario changed deterministic evidence");
    const auto stationary = scene::replay(initial, std::vector<std::uint8_t>(7200, 4));
    check(stationary.death_frame > 0 && stationary.death_frame < 7200,
          "fresh unindexed replay failed to reject a colliding action tape");
    bool rejected = false;
    try {
        scene::replay(initial, {9});
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    check(rejected, "replay accepted a tenth player action");
    std::cout << "{\"duration\":7200,\"rolling_expansions\":" << result.expansions
              << ",\"peak_live_bullets\":" << result.peak_live_bullets
              << ",\"peak_model_frames\":" << result.peak_model_frames << "}\n";
}
void bounded_search_counterexample() {
    const auto initial = scene::checkpoint(scene::profile("late-gate", 900), 1);
    const auto short_route =
        th08::solver::plan(scene::forecast(initial.world, 120), initial.player, {});
    check(short_route.status == th08::solver::Status::found && short_route.actions.size() == 120,
          "counterexample lacks its initial short safe horizon");
    const auto failed = scene::run(initial);
    check(failed.outcome == scene::Outcome::search_limit &&
              failed.search_status == th08::solver::Status::search_exhausted &&
              failed.completed_frames < 300 && failed.death_frame == 0 && failed.replay_verified,
          "short safe horizon or search exhaustion was mislabeled full survival/collision");
    scene::RunOptions options;
    options.goal = {24, 352};
    const auto escaped = scene::run(initial, options);
    check(escaped.outcome == scene::Outcome::survived && escaped.completed_frames == 900 &&
              escaped.replay_verified,
          "explicit early escape heuristic did not resolve the counterexample");
    options.expansions_per_plan = 1;
    const auto limited = scene::run(initial, options);
    check(limited.outcome == scene::Outcome::search_limit &&
              limited.search_status == th08::solver::Status::expansion_limit &&
              limited.completed_frames == 0 && limited.actions.empty() && limited.replay_verified,
          "expansion budget failure produced executed actions or a collision claim");
    options.strategy = scene::Strategy::stationary;
    const auto collision = scene::run(initial, options);
    check(collision.outcome == scene::Outcome::collision && collision.death_frame == 301 &&
              collision.completed_frames == 301 && collision.replay_verified,
          "gate activation collision frame is off by one");
}
void invalid_checkpoints() {
    const auto initial = scene::checkpoint(scene::profile("relay", 12), 1);
    for (unsigned fault = 0; fault < 3; ++fault) {
        auto broken = initial;
        if (fault == 0)
            broken.world.phase = 3;
        else if (fault == 1)
            broken.world.frame = 13;
        else
            broken.world.definition = std::make_shared<scene::Definition>();
        unsigned rejected = 0;
        try {
            scene::step(broken.world);
        } catch (const std::invalid_argument &) {
            ++rejected;
        }
        try {
            scene::forecast(broken.world, 1);
        } catch (const std::invalid_argument &) {
            ++rejected;
        }
        try {
            scene::replay(broken, {});
        } catch (const std::invalid_argument &) {
            ++rejected;
        }
        try {
            scene::run(broken);
        } catch (const std::invalid_argument &) {
            ++rejected;
        }
        check(rejected == 4,
              "malformed checkpoint reached simulation or unsigned duration arithmetic");
    }
    auto finished = initial;
    while (finished.world.frame < 12)
        scene::step(finished.world);
    check(scene::forecast(finished.world, 120).frames.empty() &&
              scene::replay(finished, {}).completed_frames == 0,
          "completed checkpoint did not allow empty forecast/replay");
}
} // namespace
int main() {
    continuity_and_rng();
    full_duration_and_replay();
    bounded_search_counterexample();
    invalid_checkpoints();
}
