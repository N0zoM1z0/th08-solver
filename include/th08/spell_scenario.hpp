#pragma once
#include "animation.hpp"
#include "emitter.hpp"
#include "planner.hpp"
#include "transform_program.hpp"
#include <string>

namespace th08::spell {
inline constexpr std::uint32_t duration = 1200;
// A DAT-derived controlled survival projection, not retail practice initialization.
// Only movement is a search action. Graze/score/item feedback and visual RNG are
// isolated; player shots, bombs, form transitions and death continuations are absent.
struct Program {
    emitter::Module ecl;
    std::array<geometry::Vec2, 2> sprite_sizes{}; // type2 colors2 and6
    animation::Timing spawn;
    resources::ShtHeader player{};
    std::string dat_hash, ecl_hash, anm_hash, sht_hash;
    explicit Program(const std::filesystem::path &dat);
};
struct Statistics {
    std::uint32_t frame = 0, shot_commands = 0, allocated = 0, retired = 0, despawning = 0;
    std::uint32_t peak_lethal = 0, visual_requests = 0, child_start = 0;
    std::uint32_t first_lethal = duration;
    std::uint32_t last_shot = 0, gameplay_draws_before_callback = 0;
    std::uint64_t hazard_digest = 1469598103934665603ULL;
    std::uint64_t visual_digest = 1469598103934665603ULL;
    bool timeout_entered = false, spell_ended = false, boss_removed = false;
};
class World {
    // Programs are immutable during execution and must outlive all World copies.
    struct Particle {
        bullet::transform::Program program;
        bullet::transform::State state;
        geometry::Vec2 sprite_size;
        std::uint32_t spawn_remaining;
        bool live = true, despawning = false;
    };
    const Program *program_;
    emitter::Workspace main_storage_, child_storage_;
    emitter::Execution main_, child_;
    random::Rng gameplay_, visual_;
    bullet::transform::Program transforms_;
    std::vector<Particle> particles_;
    std::vector<geometry::Hazard> hazards_;
    Statistics stats_;
    bool child_active_ = false, active_spell_ = false, timeout_spell_ = false;
    bool player_terminal_ = false, capture_valid_ = true;
    bool bonus_updates_disabled_ = false, timer_paused_ = false;
    std::int32_t boss_slot_ = 0, death_callback_ = 1, death_mode_ = 3;
    std::int32_t damage_reduction_ = 0;
    std::uint32_t interaction_flags_ = 0, timer_limit_ = duration;
    float minimum_distance_ = 32;
    float departure_angle_ = 0, departure_speed_ = 0;
    std::int32_t departure_duration_ = 0;
    std::uint64_t sound_requests_ = 0;
    void execute(emitter::Execution &execution, emitter::Workspace &storage, bool child);
    void effect(emitter::Execution &execution, emitter::Workspace &storage, bool child);
    void visual_request(std::int32_t kind, std::uint32_t count);
    void shoot(const emitter::Operation &operation, const emitter::Workspace &storage);
    void move_particles();

  public:
    explicit World(const Program &program, std::uint16_t gameplay_seed = 0,
                   std::uint16_t visual_seed = 0);
    // One lethal phase for boss timers0..1199. The player action precedes this phase.
    const std::vector<geometry::Hazard> &advance();
    // At timer1200 run ECL/children first, then actual callback1 through boss removal.
    // The resulting player state3 is the survival-segment terminal boundary.
    void finish_timeout();
    const Statistics &statistics() const {
        return stats_;
    }
    random::State gameplay_rng() const {
        return gameplay_.state();
    }
    random::State visual_rng() const {
        return visual_.state();
    }
    // Trace plus selected lifecycle-state digest, not a complete equivalence key
    // for arbitrary state merging. Replay regenerates every hazard independently.
    std::uint64_t state_digest() const;
};
struct Options {
    std::size_t horizon = duration, commit = duration, beam = 128;
    std::uint64_t expansion_budget = 2000000;
};
struct Result {
    std::string status = "SEARCH_LIMIT";
    solver::Status search_status = solver::Status::invalid_argument;
    std::vector<solver::Action> actions;
    std::vector<geometry::Vec2> positions;
    std::uint64_t expansions = 0, collision_queries = 0, replans = 0;
    std::uint64_t world_digest = 0;
    Statistics statistics;
    random::State gameplay{}, visual{};
    bool replayed = false;
};
Result solve(const Program &program, const Options &options, std::uint16_t gameplay_seed = 0,
             std::uint16_t visual_seed = 0);
Result baseline(const Program &program, bool greedy, std::uint16_t gameplay_seed = 0,
                std::uint16_t visual_seed = 0);
bool replay(const Program &program, const Result &result, std::uint16_t gameplay_seed = 0,
            std::uint16_t visual_seed = 0);
} // namespace th08::spell
