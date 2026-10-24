#pragma once
#include "th08/shadow_protocol.hpp"
#include <vector>

namespace th08::headless {
class Session;
struct State;
} // namespace th08::headless
namespace th08::shadow {
// Copies current values only. Hashes cover the declared views, not all manager
// storage. No native update, operand resolution or RNG write occurs here.
Projection capture(headless::Session &session);
struct BulletDetail {
    int slot;
    float angle, speed, acceleration_angle, acceleration_magnitude;
};
struct NativeDiagnostics {
    std::uint16_t x87_control = 0;
    std::vector<BulletDetail> bullets;
};
// Current source arguments for a stopped differential case; these are neither
// policy features nor a serialized checkpoint.
NativeDiagnostics diagnostics(headless::Session &session);
// Story stage-clear can precede a control-only handoff. Only the original
// Final/Extra boundary terminates the complete story case.
bool stage_terminal(bool story, const headless::State &state);
} // namespace th08::shadow
