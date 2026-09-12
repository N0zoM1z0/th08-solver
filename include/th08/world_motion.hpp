#pragma once
#include "emitter.hpp"
#include "enemy_motion.hpp"

namespace th08::world {
enum class EffectStatus { applied, not_handled, missing_context, unsupported, invalid };
// Publish only movement/player-derived ECL rvalues. Missing player input invalidates
// its computed slots. Other actor/global/call registers are preserved untouched.
// Uses the caller's phase-correct world_position; does not republish local+offset.
EffectStatus publish_motion(const enemy::State &motion, emitter::ScalarStorage &storage,
                            const enemy::Vec3 *player = nullptr);
// Execute and acknowledge one pending movement instruction (63..76 or 178).
// ECL, actor storage, motion and RNG commit together; any failure leaves all intact.
// This is the instruction effect phase, NOT update_velocity or integrate_position.
// The caller still owns actor initialization, child/callback/shot/ANM scheduling.
EffectStatus apply_motion_effect(emitter::Execution &execution, emitter::Workspace &workspace,
                                 enemy::State &motion, random::Rng *rng = nullptr,
                                 const enemy::Vec3 *player = nullptr);
} // namespace th08::world
