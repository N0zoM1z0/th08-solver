#pragma once
#include "timeline.hpp"
#include "world_motion.hpp"
#include <memory>
#include <optional>

namespace th08::practice {
inline constexpr std::size_t enemy_capacity = 480;
inline constexpr std::size_t no_actor = enemy_capacity;

// Shared immutable programs keep every Execution/CallFrame pointer alive when
// runtime snapshots are copied. No decrypted-resource view survives compilation.
struct Programs {
    emitter::Module ecl;
    timeline::Program timeline;
    Programs() = default;
    Programs(resources::View bytes, const resources::Ecl &decoded, std::size_t timeline_id = 0);
};

struct Interaction {
    bool accepts_damage = true, collision = true, damageable = true;
    bool no_sprite = false, allow_offscreen = false, no_death = false;
};
// Restricted source-template projection. Only the implemented spawn-prefix
// consumers may use it. ANM loading, children, callbacks and manager updates are
// not fabricated by these defaults; instructions installing them stop first.
struct Actor {
    bool active = false;
    std::size_t index = no_actor;
    std::int32_t life = 1, score = 100, max_life = 0, phase_starting_life = 0;
    std::int8_t item_drop = 0;
    std::uint32_t vm_color = 0, display_color = 0;
    Interaction interaction;
    enemy::Vec3 hitbox{24, 24, 24}, secondary_hitbox;
    enemy::State motion;
    emitter::Workspace scalars;
    emitter::Execution execution;
};

enum class SpawnKind { ordinary, inherited_context };
struct SpawnRequest {
    std::int32_t sub = 0;
    enemy::Vec3 position;
    std::int32_t life = -1, item_drop = -1, score = -1;
    bool mirror_x = false;
    SpawnKind kind = SpawnKind::ordinary;
    // SpawnEnemy2 copies these thirty words after CallEclSub and BEFORE RunEcl.
    // An absent block is missing context, not permission to supply zero words.
    std::optional<emitter::ContextScalars> inherited;
};
enum class Status {
    timeline_frame_complete,
    spawn_pending,
    spawned,
    pool_full,
    source_spawn_failed,
    unsupported_world_effect,
    missing_entry_state,
    unsupported_root_return,
    instruction_limit,
    invalid,
    busy
};
const char *name(Status status);
struct Result {
    Status status = Status::invalid;
    std::size_t actor = no_actor, sub = 0;
    std::uint32_t pc = 0, offset = 0;
    std::int16_t opcode = 0;
    std::uint8_t instruction_mask = 0, execution_mask = 0;
    std::int64_t local_time = 0;
};
struct PendingSpawn {
    SpawnRequest request;
    std::size_t actor = no_actor;
};
struct SharedCallParameters {
    std::array<double, 8> values{};
    std::array<bool, 8> known{};
};

// Owns first-free slot selection, template initialization, immediate ECL and
// restricted RunEcl tails, then source-ordered post-spawn stores. This is NOT an
// EnemyManager frame loop. Unsupported effects retain their actor and pending
// transaction; resume never allocates a second actor or repeats committed RNG.
class SpawnPool {
    std::shared_ptr<const Programs> programs_;
    std::vector<Actor> actors_;
    std::optional<PendingSpawn> pending_;
    SharedCallParameters shared_calls_;
    bool last_spawn_failed_ = false, spawn_event_ = false;

    Result identify(Status status) const;
    world::EffectStatus apply_effect(Actor &actor, random::Rng *rng, const enemy::Vec3 *player);

  public:
    explicit SpawnPool(std::shared_ptr<const Programs> programs,
                       SharedCallParameters shared_calls = {});
    Result begin(const SpawnRequest &request, std::uint8_t mask);
    Result resume(random::Rng *rng = nullptr, const enemy::Vec3 *player = nullptr,
                  std::uint32_t instruction_limit = 100000);
    const Actor &actor(std::size_t index) const;
    const std::optional<PendingSpawn> &pending() const {
        return pending_;
    }
    bool last_spawn_failed() const {
        return last_spawn_failed_;
    }
    bool spawn_event() const {
        return spawn_event_;
    }
    std::size_t active_count() const;
    const SharedCallParameters &shared_calls() const {
        return shared_calls_;
    }
};

// Connects actual timeline spawn packets to the owned pool. Context contains
// explicit GUI/global observations; this controller does not run the preceding
// background/player phases or the subsequent enemy-manager frame. Consequently
// timeline_frame_complete is never a complete-world frame or a spell result.
class Entry {
    std::shared_ptr<const Programs> programs_;
    SpawnPool pool_;
    timeline::State timeline_;
    bool awaiting_spawn_ = false;
    std::uint8_t mask_;

  public:
    Entry(std::shared_ptr<const Programs> programs, std::uint8_t mask);
    Result advance(timeline::Context &observations, random::Rng *rng = nullptr,
                   const enemy::Vec3 *player = nullptr, std::uint32_t instruction_limit = 100000);
    const SpawnPool &pool() const {
        return pool_;
    }
    const timeline::State &timeline_state() const {
        return timeline_;
    }
};
} // namespace th08::practice
