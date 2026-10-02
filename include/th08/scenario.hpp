#pragma once
#include "planner.hpp"
#include "rng.hpp"
#include <memory>

namespace th08::scenario {
using geometry::Vec2;
inline constexpr std::uint64_t digest_basis = 1469598103934665603ULL;

enum class Pattern { curtain, rings, lane_shift, quiet, closing_gate };
struct Phase {
    std::uint32_t end_frame = 0;
    Pattern pattern = Pattern::quiet;
};
struct Definition {
    std::string name;
    std::vector<Phase> phases;
    solver::Movement movement{};
    Vec2 player_start{192, 352}, player_half{.825f, .825f};
    // An explicitly controlled visual channel, not retail shared-RNG ordering.
    std::uint32_t visual_draws_per_frame = 2;
    std::uint32_t duration() const;
};
std::shared_ptr<const Definition> profile(const std::string &name, std::uint32_t duration = 7200,
                                          std::uint32_t visual_draws_per_frame = 2);
std::vector<std::string> profile_names();

struct Bullet {
    std::uint64_t id = 0;
    Vec2 position{}, velocity{}, size{};
    std::uint32_t expires_after_frame = 0;
};
struct Digests {
    std::uint64_t world = digest_basis, events = digest_basis;
    std::uint64_t gameplay_rng = digest_basis, visual_rng = digest_basis;
};
struct Transition {
    std::uint32_t frame = 0;
    std::size_t phase = 0, carried_bullets = 0;
    std::uint32_t gameplay_draws = 0, visual_draws = 0;
};
struct World {
    std::shared_ptr<const Definition> definition;
    random::Rng gameplay_rng{0}, visual_rng{0};
    std::vector<Bullet> bullets;
    std::vector<Transition> transitions;
    std::uint32_t frame = 0;
    std::size_t phase = 0, peak_live_bullets = 0;
    std::uint64_t next_bullet_id = 1, emission_events = 0;
    Digests digests{};
};
struct Checkpoint {
    World world;
    Vec2 player{};
    std::uint16_t seed = 0;
};
Checkpoint checkpoint(std::shared_ptr<const Definition> definition, std::uint16_t seed);
// Emission, movement, lifetime/cull, then lethal collision. Phase changes never
// reset live bullets or either RNG. No player-dependent emitter is supported.
void step(World &world);
bool collision(const World &world, Vec2 player);
struct ForecastStats {
    std::uint64_t bullet_references = 0;
    std::size_t peak_live_bullets = 0;
};
solver::Model forecast(const World &world, std::uint32_t horizon, ForecastStats *stats = nullptr);

enum class Strategy { stationary, greedy, rolling_beam };
enum class Outcome { survived, collision, search_limit };
struct RunOptions {
    Strategy strategy = Strategy::rolling_beam;
    std::uint32_t horizon = 120, commit_frames = 30;
    std::size_t beam = 128;
    // With goal recovery enabled, this is shared by both searches in a decision.
    std::uint64_t expansions_per_plan = 200000;
    Vec2 goal{192, 352};
    bool recover_goal = false;
};
struct ReplayResult {
    std::uint32_t completed_frames = 0, death_frame = 0;
    Vec2 player{};
    Digests digests{};
    std::uint64_t route_digest = digest_basis;
};
struct RunResult {
    Outcome outcome = Outcome::search_limit;
    solver::Status search_status = solver::Status::found;
    std::uint32_t completed_frames = 0, death_frame = 0, search_stop_frame = 0;
    std::uint32_t decisions = 0, peak_model_frames = 0;
    std::uint64_t expansions = 0, forecast_frames = 0, peak_model_bullet_references = 0;
    std::uint64_t collision_queries = 0, duplicate_successors = 0;
    std::uint64_t peak_decision_expansions = 0;
    // An attempt includes refuge selection even when no target permits a retry.
    std::uint32_t recovery_attempts = 0, recovery_successes = 0;
    // Target probes are separate from action-expansion collision queries.
    std::uint64_t recovery_target_queries = 0, recovery_initial_expansions = 0;
    std::uint64_t recovery_retry_expansions = 0;
    std::size_t peak_live_bullets = 0;
    std::uint64_t emitted_bullets = 0, emission_events = 0;
    std::uint32_t gameplay_draws = 0, visual_draws = 0;
    Vec2 player{};
    Digests digests{};
    std::uint64_t route_digest = digest_basis;
    std::vector<Transition> transitions;
    // One byte per executed frame; geometry memory is bounded by the horizon.
    std::vector<std::uint8_t> actions;
    ReplayResult replay{};
    bool replay_verified = false;
    double generation_ms = 0, search_ms = 0, execution_ms = 0, replay_ms = 0, total_ms = 0;
    // Recovery timings are subsets of search_ms, not additional total costs.
    double recovery_initial_ms = 0, recovery_target_ms = 0, recovery_retry_ms = 0;
};
RunResult run(const Checkpoint &initial, const RunOptions &options = {});
ReplayResult replay(const Checkpoint &initial, const std::vector<std::uint8_t> &actions);
const char *name(Strategy value);
const char *name(Outcome value);
const char *name(solver::Status value);
const char *name(Pattern value);
} // namespace th08::scenario
