#pragma once
#include "animation_control.hpp"
#include "camera_particle.hpp"
#include <array>
#include <cstddef>
#include <optional>
#include <vector>

namespace th08::effect {
inline constexpr std::size_t primary_capacity = 512;
// SpawnEffect returns the address of effects[653] after exhausting its scan,
// including when earlier iterations initialized some primary-pool slots.
inline constexpr std::size_t exhausted_effect = 653;

// Supplied, validated post-SetAndExecuteScriptIdx(73) time-zero projection.
// This pool does not load sprite resources or execute arbitrary ANM programs.
// Ancillary fields remain owned for later world consumers.
// Renderer matrices/textures and the complete ANM VM are not represented here.
struct Effect51Animation {
    camera_particle::Animation fields;
    animation::control::State control;
    camera_particle::Vec3 rotation, angular_velocity;
    float sprite_width, sprite_height;
    // The compiler sets this after checking the pinned script73 resource. A
    // caller supplying a projection directly must explicitly assert the same
    // unit-rate script contract; an arbitrary spawn template is not certified.
    bool unit_rate_script73 = false;
};
using Template = Effect51Animation;
struct Slot {
    bool active = false;
    // Occupancy-only checkpoints do not imply known particle/ANM fields.
    // Consumers must check this flag before reading either projection below.
    bool effect51_known = false;
    camera_particle::State particle{};
    Effect51Animation animation{};
    // SpawnEffect clears the whole Effect before its initializer. Unlike an
    // ANM clock's default sentinel, all three effect-timer fields start at zero.
    animation::control::Clock timer{0, 0, 0};
};
struct Effect51Request {
    std::int32_t count;
    camera_particle::Vec3 position;
    camera_particle::Color color;
};
struct Effect51Inputs {
    const Effect51Animation *post_time_zero = nullptr;
    const camera_particle::Camera *camera = nullptr;
    std::optional<float> multiplier;
};
enum class Status { spawned, exhausted, missing_context, invalid_state };
const char *name(Status status);
struct Result {
    Status status = Status::invalid_state;
    std::size_t allocated = 0, inspected = 0, returned_slot = exhausted_effect;
    bool committed() const {
        return status == Status::spawned || status == Status::exhausted;
    }
};
struct Effect51UpdateInputs {
    std::optional<bool> deathbomb_freeze;
    const camera_particle::Camera *camera = nullptr;
    const camera_particle::Bosses *bosses = nullptr;
    const camera_particle::Color *stage_tint = nullptr;
};
enum class UpdateStatus { advanced, missing_context, unsupported_animation, invalid_state };
const char *name(UpdateStatus status);
struct UpdateResult {
    UpdateStatus status = UpdateStatus::invalid_state;
    // Updated counts callbacks executed, including callbacks that retire a
    // slot. Source activeCount counts occupied slots BEFORE either cull path.
    std::size_t updated = 0, retired = 0, source_active_count = 0;
    bool committed() const {
        return status == UpdateStatus::advanced;
    }
};

// Restricted EffectManager primary-pool ownership. Construction explicitly
// supplies empty storage or known occupancy; it is not world initialization.
// Copy this owner and the external RNG separately to fork a checkpoint. Queries
// are immutable; only the explicitly supported effect51 manager phase is owned.
class PrimaryPool {
  public:
    using Occupancy = std::array<bool, primary_capacity>;

  private:
    struct Delta {
        std::size_t index;
        Slot slot;
    };
    std::array<Slot, primary_capacity> slots_{};
    std::vector<Delta> scratch_;
    std::size_t cursor_ = 0, active_count_ = 0;
    bool spawn_event_ = false;

  public:
    PrimaryPool();
    explicit PrimaryPool(const Occupancy &occupied, std::size_t cursor, bool spawn_event = false);
    PrimaryPool(const PrimaryPool &other);
    PrimaryPool &operator=(const PrimaryPool &other);
    PrimaryPool(PrimaryPool &&) noexcept = default;
    PrimaryPool &operator=(PrimaryPool &&) noexcept = default;

    // Missing or invalid required inputs roll back slots, cursor, spawn event
    // and shared RNG together. This atomicity is a native interface guarantee,
    // not a claim about source-engine rollback. A full pool reads no request
    // initialization fields or context and consumes no RNG.
    //
    // A single source-ordered scan visits at most 512 slots. Nonpositive counts
    // initialize every free slot in that scan; they are not no-op requests.
    // Effect51's source callback always succeeds. Native context/finite-domain
    // failures are blockers, never source callback failure/deactivation events.
    Result spawn_effect51(const Effect51Request &request, const Effect51Inputs &inputs,
                          random::Rng *rng = nullptr);
    // One unit-rate primary-pool phase, in ascending slot order. Known effect51
    // slots retain their cleared updateDuringFreeze=0 behavior: a supplied
    // deathbomb freeze skips callback, ANM and effect timer. Unknown occupied
    // checkpoint slots always block, even during freeze.
    //
    // Callback culling precedes script73 advancement; static ANM completion
    // precedes angular motion and the effect timer. Camera/boss/tint inputs are
    // required only on the callback branches that actually read them. Any
    // missing/invalid state rolls back the WHOLE phase using native staging.
    // This is not a full EffectManager/world phase: draw-list grouping,
    // tamper counters, renderer state and non-primary effects are not represented.
    UpdateResult advance_effect51(const Effect51UpdateInputs &inputs);
    const Slot &slot(std::size_t index) const;
    bool occupied(std::size_t index) const {
        return slot(index).active;
    }
    std::size_t cursor() const {
        return cursor_;
    }
    std::size_t active_count() const {
        return active_count_;
    }
    bool spawn_event() const {
        return spawn_event_;
    }
};
} // namespace th08::effect
