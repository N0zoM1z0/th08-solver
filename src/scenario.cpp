#include <chrono>
#include <cstring>
#include <th08/kinematics.hpp>
#include <th08/scenario.hpp>

namespace th08::scenario {
namespace {
using Clock = std::chrono::steady_clock;
double elapsed(Clock::time_point start) {
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}
void hash(std::uint64_t &digest, std::uint64_t value) {
    for (unsigned byte = 0; byte < 8; ++byte) {
        digest ^= value & 255;
        digest *= 1099511628211ULL;
        value >>= 8;
    }
}
void hash_float(std::uint64_t &digest, float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    hash(digest, bits);
}
void hash_rng(std::uint64_t &digest, const random::Rng &rng) {
    const auto state = rng.state();
    hash(digest, state.seed);
    hash(digest, state.generation_count);
    hash(digest, state.saved_seed);
    hash(digest, state.saved_seed_valid);
}
void require(bool valid, const char *message) {
    if (!valid)
        throw std::invalid_argument(message);
}
bool player_valid(Vec2 player) {
    return geometry::finite(player) && player.x >= 8 && player.x <= 376 && player.y >= 16 &&
           player.y <= 432;
}
void validate(const Definition &definition) {
    require(!definition.phases.empty() && definition.phases.size() <= 64,
            "scenario needs 1..64 phases");
    std::uint32_t previous = 0;
    for (const auto &phase : definition.phases) {
        require(phase.end_frame > previous && phase.end_frame <= 10000000,
                "scenario phase endpoints must increase within 10000000 frames");
        require(unsigned(phase.pattern) <= unsigned(Pattern::closing_gate), "unknown pattern");
        previous = phase.end_frame;
    }
    require(player_valid(definition.player_start), "invalid scenario player start");
    geometry::size(definition.player_half);
    const auto speed = definition.movement;
    require(std::isfinite(speed.axis) && std::isfinite(speed.diagonal) && speed.axis > 0 &&
                speed.axis <= 32 && speed.diagonal > 0 && speed.diagonal <= 32,
            "invalid scenario movement");
    require(definition.visual_draws_per_frame <= 32, "visual hook exceeds 32 draws per frame");
}
void validate_cursor(const World &world, bool allow_completed) {
    require(bool(world.definition) && !world.definition->phases.empty(),
            "missing scenario definition or phases");
    const auto &phases = world.definition->phases;
    require(world.phase < phases.size(), "scenario checkpoint phase is out of range");
    require(world.frame <= world.definition->duration() &&
                (allow_completed || world.frame < world.definition->duration()),
            "scenario checkpoint is finished or past its duration");
    const auto begin = world.phase ? phases[world.phase - 1].end_frame : 0;
    require(world.frame >= begin && world.frame <= phases[world.phase].end_frame,
            "scenario checkpoint frame and phase disagree");
}
void validate_checkpoint_world(const World &world) {
    validate_cursor(world, true);
    validate(*world.definition);
    for (const auto &bullet : world.bullets) {
        geometry::coordinate(bullet.position);
        geometry::coordinate(bullet.velocity);
        geometry::size(bullet.size);
    }
}
void spawn(World &world, Vec2 position, Vec2 velocity, Vec2 size, std::uint32_t lifetime) {
    Bullet bullet{world.next_bullet_id++, position, velocity, size, world.frame + lifetime};
    world.bullets.push_back(bullet);
    hash(world.digests.events, bullet.id);
    hash_float(world.digests.events, position.x);
    hash_float(world.digests.events, position.y);
    hash_float(world.digests.events, velocity.x);
    hash_float(world.digests.events, velocity.y);
    hash_float(world.digests.events, size.x);
    hash_float(world.digests.events, size.y);
    hash(world.digests.events, bullet.expires_after_frame);
}
void event(World &world, Pattern pattern) {
    ++world.emission_events;
    hash(world.digests.events, world.frame);
    hash(world.digests.events, unsigned(pattern));
}
void emit(World &world, std::uint32_t local) {
    const auto pattern = world.definition->phases[world.phase].pattern;
    if (pattern == Pattern::curtain || pattern == Pattern::lane_shift) {
        const auto interval = pattern == Pattern::curtain ? 40U : 28U;
        if (local % interval)
            return;
        event(world, pattern);
        // Slow lane motion is a world-time function; it never reads a candidate.
        const float sweep = std::sin(float(world.frame) * (kinematics::pi / 600.f));
        const float gap = 192.f + (pattern == Pattern::curtain ? 70.f : 108.f) * sweep;
        const float jitter = world.gameplay_rng.range_signed_float(2.f);
        for (unsigned column = 0; column < 25; ++column) {
            const float x = float(column) * 16.f + jitter;
            if (std::abs(x - gap) > 46.f)
                spawn(world, {x, -12}, {0, 1.6f}, {8, 8}, 340);
        }
    } else if (pattern == Pattern::rings) {
        if (local % 56)
            return;
        event(world, pattern);
        const float angle = world.gameplay_rng.range_float(kinematics::pi * 2.f);
        const float speed = 1.1f + world.gameplay_rng.range_float(.35f);
        const float x = (local / 56) % 2 ? 112.f : 272.f;
        const kinematics::Pattern ring{kinematics::Aim::circle, 24, 1, speed, speed, angle, 0};
        for (int spoke = 0; spoke < ring.count1; ++spoke) {
            kinematics::Launch launch{};
            if (kinematics::launch(ring, spoke, 0, 0, 1, {}, launch) != kinematics::Status::ready)
                throw std::logic_error("synthetic ring left the launch contract");
            spawn(world, {x, 72}, {launch.velocity_x, launch.velocity_y}, {7, 7}, 600);
        }
    } else if (pattern == Pattern::closing_gate && local == 0) {
        event(world, pattern);
        // Counterexample: an announced phase closes every center except x < 47.
        // Center-seeking bounded beams can discard all useful early escape paths.
        world.gameplay_rng.next_u16();
        const auto remaining = world.definition->phases[world.phase].end_frame - world.frame;
        spawn(world, {224, 224}, {0, 0}, {352, 448}, remaining);
    }
}
void trace_world(World &world) {
    hash(world.digests.world, world.frame);
    hash(world.digests.world, world.phase);
    hash(world.digests.world, world.bullets.size());
    for (const auto &bullet : world.bullets) {
        hash(world.digests.world, bullet.id);
        hash_float(world.digests.world, bullet.position.x);
        hash_float(world.digests.world, bullet.position.y);
        hash_float(world.digests.world, bullet.velocity.x);
        hash_float(world.digests.world, bullet.velocity.y);
        hash_float(world.digests.world, bullet.size.x);
        hash_float(world.digests.world, bullet.size.y);
        hash(world.digests.world, bullet.expires_after_frame);
    }
    hash_rng(world.digests.gameplay_rng, world.gameplay_rng);
    hash_rng(world.digests.visual_rng, world.visual_rng);
}
std::uint8_t encode(solver::Action action) {
    require(action.x >= -1 && action.x <= 1 && action.y >= -1 && action.y <= 1,
            "illegal player action");
    return std::uint8_t((action.y + 1) * 3 + action.x + 1);
}
solver::Action decode(std::uint8_t action) {
    require(action < 9, "illegal encoded player action");
    return {int(action % 3) - 1, int(action / 3) - 1};
}
void trace_route(std::uint64_t &digest, std::uint32_t frame, std::uint8_t action, Vec2 player) {
    hash(digest, frame);
    hash(digest, action);
    hash_float(digest, player.x);
    hash_float(digest, player.y);
}
bool same(Digests a, Digests b) {
    return a.world == b.world && a.events == b.events && a.gameplay_rng == b.gameplay_rng &&
           a.visual_rng == b.visual_rng;
}
} // namespace

std::uint32_t Definition::duration() const {
    return phases.empty() ? 0 : phases.back().end_frame;
}
std::vector<std::string> profile_names() {
    return {"relay", "lane-switch", "late-gate"};
}
std::shared_ptr<const Definition> profile(const std::string &name, std::uint32_t duration,
                                          std::uint32_t visual_draws_per_frame) {
    require(duration >= 3 && duration <= 10000000, "duration must be 3..10000000 frames");
    auto result = std::make_shared<Definition>();
    result->name = name;
    result->visual_draws_per_frame = visual_draws_per_frame;
    const auto first = duration / 3, second = duration * 2 / 3;
    if (name == "relay")
        result->phases = {
            {first, Pattern::curtain}, {second, Pattern::rings}, {duration, Pattern::lane_shift}};
    else if (name == "lane-switch")
        result->phases = {
            {first, Pattern::lane_shift}, {second, Pattern::curtain}, {duration, Pattern::rings}};
    else if (name == "late-gate")
        result->phases = {
            {first, Pattern::quiet}, {second, Pattern::closing_gate}, {duration, Pattern::quiet}};
    else
        throw std::invalid_argument("unknown synthetic profile: " + name);
    validate(*result);
    return result;
}
Checkpoint checkpoint(std::shared_ptr<const Definition> definition, std::uint16_t seed) {
    require(bool(definition), "missing scenario definition");
    validate(*definition);
    Checkpoint result;
    result.world.definition = std::move(definition);
    result.world.gameplay_rng = random::Rng(seed);
    result.world.visual_rng = random::Rng(std::uint16_t(seed ^ 0xa5a5U));
    result.world.bullets.reserve(512);
    result.world.transitions.reserve(result.world.definition->phases.size());
    result.player = result.world.definition->player_start;
    result.seed = seed;
    hash(result.world.digests.world, seed);
    hash_rng(result.world.digests.gameplay_rng, result.world.gameplay_rng);
    hash_rng(result.world.digests.visual_rng, result.world.visual_rng);
    return result;
}
void step(World &world) {
    validate_cursor(world, false);
    if (world.frame == world.definition->phases[world.phase].end_frame) {
        ++world.phase;
        world.transitions.push_back({world.frame, world.phase, world.bullets.size(),
                                     world.gameplay_rng.generation_count(),
                                     world.visual_rng.generation_count()});
        hash(world.digests.events, 0xffffffffffffffffULL);
        hash(world.digests.events, world.frame);
        hash(world.digests.events, world.phase);
    }
    const auto begin = world.phase ? world.definition->phases[world.phase - 1].end_frame : 0;
    emit(world, world.frame - begin);
    for (unsigned draw = 0; draw < world.definition->visual_draws_per_frame; ++draw)
        world.visual_rng.next_u16();
    for (auto &bullet : world.bullets) {
        bullet.position.x += bullet.velocity.x;
        bullet.position.y += bullet.velocity.y;
    }
    ++world.frame;
    world.bullets.erase(std::remove_if(world.bullets.begin(), world.bullets.end(),
                                       [&](const Bullet &bullet) {
                                           return world.frame > bullet.expires_after_frame ||
                                                  bullet.position.x < -64 ||
                                                  bullet.position.x > 448 ||
                                                  bullet.position.y < -64 ||
                                                  bullet.position.y > 512;
                                       }),
                        world.bullets.end());
    world.peak_live_bullets = std::max(world.peak_live_bullets, world.bullets.size());
    trace_world(world);
}
bool collision(const World &world, Vec2 player) {
    require(bool(world.definition), "missing collision world definition");
    geometry::coordinate(player);
    for (const auto &bullet : world.bullets)
        if (geometry::box_hit(player, world.definition->player_half,
                              {bullet.position, bullet.size}))
            return true;
    return false;
}
solver::Model forecast(const World &world, std::uint32_t horizon, ForecastStats *stats) {
    validate_checkpoint_world(world);
    require(horizon > 0 && horizon <= 4096, "invalid forecast horizon");
    if (stats)
        *stats = {};
    solver::Model model;
    model.dependency = solver::Dependency::independent;
    model.epoch = world.frame;
    const auto frames = std::min(horizon, world.definition->duration() - world.frame);
    model.frames.reserve(frames);
    auto future = world;
    for (std::uint32_t frame = 0; frame < frames; ++frame) {
        step(future);
        if (stats) {
            stats->bullet_references += future.bullets.size();
            stats->peak_live_bullets = std::max(stats->peak_live_bullets, future.bullets.size());
        }
        std::vector<geometry::Hazard> hazards;
        hazards.reserve(future.bullets.size());
        for (const auto &bullet : future.bullets)
            hazards.push_back(geometry::Hazard::bullet({bullet.position, bullet.size}));
        model.frames.emplace_back(std::move(hazards), world.definition->player_half);
    }
    return model;
}
ReplayResult replay(const Checkpoint &initial, const std::vector<std::uint8_t> &actions) {
    validate_checkpoint_world(initial.world);
    require(player_valid(initial.player), "invalid checkpoint player");
    require(actions.size() <= initial.world.definition->duration() - initial.world.frame,
            "route extends beyond scenario end");
    auto world = initial.world;
    ReplayResult result;
    result.player = initial.player;
    for (const auto encoded : actions) {
        const auto action = decode(encoded);
        result.player = solver::advance(result.player, action, world.definition->movement);
        step(world);
        ++result.completed_frames;
        trace_route(result.route_digest, world.frame, encoded, result.player);
        if (collision(world, result.player)) {
            result.death_frame = world.frame;
            break;
        }
    }
    result.digests = world.digests;
    return result;
}
RunResult run(const Checkpoint &initial, const RunOptions &options) {
    validate_checkpoint_world(initial.world);
    require(player_valid(initial.player) &&
                initial.world.frame < initial.world.definition->duration(),
            "invalid or completed checkpoint");
    require(options.horizon > 0 && options.horizon <= 4096 && options.commit_frames > 0 &&
                options.commit_frames <= options.horizon && options.beam > 0 &&
                options.beam <= 4096 && player_valid(options.goal) &&
                unsigned(options.strategy) <= unsigned(Strategy::rolling_beam),
            "invalid scenario run options");
    const auto start = Clock::now();
    auto world = initial.world;
    RunResult result;
    result.player = initial.player;
    result.actions.reserve(world.definition->duration() - world.frame);
    while (world.frame < world.definition->duration()) {
        std::vector<solver::Action> chosen;
        if (options.strategy == Strategy::rolling_beam) {
            auto timer = Clock::now();
            ForecastStats stats;
            auto model = forecast(world, options.horizon, &stats);
            result.generation_ms += elapsed(timer);
            result.forecast_frames += model.frames.size();
            result.peak_model_frames =
                std::max(result.peak_model_frames, std::uint32_t(model.frames.size()));
            result.peak_model_bullet_references =
                std::max(result.peak_model_bullet_references, stats.bullet_references);
            solver::Options search;
            search.beam = options.beam;
            search.expansions = options.expansions_per_plan;
            search.expected_epoch = model.epoch;
            search.terminal = {options.goal, {736, 832}};
            timer = Clock::now();
            auto proposal = solver::plan(model, result.player, world.definition->movement, search);
            result.search_ms += elapsed(timer);
            ++result.decisions;
            result.expansions += proposal.expansions;
            result.collision_queries += proposal.collision_queries;
            result.duplicate_successors += proposal.duplicate_successors;
            if (proposal.status != solver::Status::found) {
                result.outcome = Outcome::search_limit;
                result.search_status = proposal.status;
                result.search_stop_frame = world.frame;
                break;
            }
            const auto count =
                std::min<std::size_t>(proposal.actions.size(), options.commit_frames);
            chosen.assign(proposal.actions.begin(), proposal.actions.begin() + count);
        } else {
            chosen.push_back({});
        }
        const auto execution_start = Clock::now();
        const auto generation_before = result.generation_ms, search_before = result.search_ms;
        for (auto action : chosen) {
            const auto generation_start = Clock::now();
            step(world);
            result.generation_ms += elapsed(generation_start);
            if (options.strategy == Strategy::greedy) {
                const auto search_start = Clock::now();
                double best = std::numeric_limits<double>::infinity();
                for (int y = -1; y <= 1; ++y)
                    for (int x = -1; x <= 1; ++x) {
                        ++result.expansions;
                        ++result.collision_queries;
                        const auto candidate =
                            solver::advance(result.player, {x, y}, world.definition->movement);
                        if (collision(world, candidate))
                            continue;
                        const double dx = double(candidate.x) - options.goal.x;
                        const double dy = double(candidate.y) - options.goal.y;
                        const double score = dx * dx + dy * dy;
                        if (score < best) {
                            best = score;
                            action = {x, y};
                        }
                    }
                ++result.decisions;
                result.search_ms += elapsed(search_start);
            }
            result.player = solver::advance(result.player, action, world.definition->movement);
            result.actions.push_back(encode(action));
            trace_route(result.route_digest, world.frame, result.actions.back(), result.player);
            ++result.completed_frames;
            if (collision(world, result.player)) {
                result.outcome = Outcome::collision;
                result.death_frame = world.frame;
                break;
            }
        }
        result.execution_ms += elapsed(execution_start) -
                               (result.generation_ms - generation_before) -
                               (result.search_ms - search_before);
        if (result.death_frame)
            break;
        if (world.frame == world.definition->duration())
            result.outcome = Outcome::survived;
    }
    result.digests = world.digests;
    result.peak_live_bullets = world.peak_live_bullets;
    result.emitted_bullets = world.next_bullet_id - 1;
    result.emission_events = world.emission_events;
    result.gameplay_draws = world.gameplay_rng.generation_count();
    result.visual_draws = world.visual_rng.generation_count();
    result.transitions = world.transitions;
    const auto replay_start = Clock::now();
    result.replay = replay(initial, result.actions);
    result.replay_ms = elapsed(replay_start);
    result.replay_verified = result.replay.completed_frames == result.completed_frames &&
                             result.replay.death_frame == result.death_frame &&
                             result.replay.player.x == result.player.x &&
                             result.replay.player.y == result.player.y &&
                             result.replay.route_digest == result.route_digest &&
                             same(result.replay.digests, result.digests);
    result.total_ms = elapsed(start);
    if (!result.replay_verified)
        throw std::logic_error("fresh unindexed scenario replay disagrees with execution");
    return result;
}
const char *name(Strategy value) {
    switch (value) {
    case Strategy::stationary:
        return "stationary";
    case Strategy::greedy:
        return "greedy";
    case Strategy::rolling_beam:
        return "rolling-beam";
    }
    return "unknown";
}
const char *name(Outcome value) {
    switch (value) {
    case Outcome::survived:
        return "survived";
    case Outcome::collision:
        return "collision";
    case Outcome::search_limit:
        return "search_limit";
    }
    return "unknown";
}
const char *name(solver::Status value) {
    switch (value) {
    case solver::Status::found:
        return "found";
    case solver::Status::unsupported_dependency:
        return "unsupported_dependency";
    case solver::Status::invalidated:
        return "invalidated";
    case solver::Status::invalid_argument:
        return "invalid_argument";
    case solver::Status::expansion_limit:
        return "expansion_limit";
    case solver::Status::search_exhausted:
        return "search_exhausted";
    case solver::Status::no_terminal_witness:
        return "no_terminal_witness";
    }
    return "unknown";
}
const char *name(Pattern value) {
    switch (value) {
    case Pattern::curtain:
        return "curtain";
    case Pattern::rings:
        return "rings";
    case Pattern::lane_shift:
        return "lane_shift";
    case Pattern::quiet:
        return "quiet";
    case Pattern::closing_gate:
        return "closing_gate";
    }
    return "unknown";
}
} // namespace th08::scenario
