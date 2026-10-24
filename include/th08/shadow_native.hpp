#pragma once
#include "th08/shadow_protocol.hpp"

namespace th08::headless {
class Session;
struct State;
} // namespace th08::headless
namespace th08::shadow {
// Copies current values only. Hashes cover the declared views, not all manager
// storage. No native update, operand resolution or RNG write occurs here.
Projection capture(headless::Session &session);
// Story stage-clear can precede a control-only handoff. Only the original
// Final/Extra boundary terminates the complete story case.
bool stage_terminal(bool story, const headless::State &state);
} // namespace th08::shadow
