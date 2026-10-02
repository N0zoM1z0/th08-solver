#pragma once
#include "formats.hpp"
#include <optional>

namespace th08::practice::camera {
struct Vec3 {
    float x = 0, y = 0, z = 0;
};
struct Interpolation {
    Vec3 current, start, target;
    std::int32_t duration = 0, timer = 0, mode = 0;
};

// The added callback initializes all three copies before STD time zero runs.
// These are camera fields only, not a constructed Background or a world snapshot.
// Integer clocks project the source ZunTimers only at unit rate with zero
// fractional remainder; they are not substitutes for arbitrary source timers.
struct State {
    Interpolation position{{0, 0, 1000}, {0, 0, 1000}, {0, 0, 1000}};
    Interpolation look_at_offset;
    Interpolation up{{0, 1, 0}, {0, 1, 0}, {0, 1, 0}};
    Vec3 forward;
    // The certified script never starts FOV interpolation or camera motion.
    float fov = 0.5235987901687622f;
    std::uint32_t pc = 0;
    std::int32_t script_time = 0;
};
enum class Status {
    advanced,
    frozen,
    missing_context,
    needs_world_effect,
    unsupported_timing,
    invalid_state
};
struct Result {
    Status status;
    std::uint32_t pc, offset;
    std::int16_t opcode;
    std::int32_t script_time;
};

// Owns the hash-certified stage1_s.std instructions, including their source PCs.
// Only its unit-rate camera projection is executed. Stage origins, fog, clear color, object
// ANM, RNG and effect creation remain outside this component. In particular, the
// time1024 jump needs world-effect origin compensation and must stop, not loop.
class Program {
    std::vector<resources::StageInstruction> instructions_;
    Result identify(Status status, const State &state) const;

  public:
    explicit Program(resources::View stage1_std);
    const std::vector<resources::StageInstruction> &instructions() const {
        return instructions_;
    }
    // One unit-rate camera update. The caller must supply multiplier == 1; other
    // rates cannot preserve the source ZunTimer fractions in this projection.
    // A missing freeze observation blocks; true preserves the whole state and
    // does not inspect the multiplier, matching the source's early freeze gate.
    // Other failures also preserve state, a native retry contract rather than a
    // claim that the original Background rolls back a partially executed phase.
    Result advance(State &state, std::optional<bool> deathbomb_frozen, float multiplier) const;
};
} // namespace th08::practice::camera
