#pragma once

#include "native_policy.hpp"

namespace th08::policy {
struct NativeSpellPolicy {
    const char *name;
    HazardReactiveOptions hazards;
    bool direct_ecl_lasers = false;
    bool imminent_pooled_lasers = false;
    bool upcoming_ecl_bullets = false;
};

// Spell-specific selection belongs here rather than in the generic projection
// kernel. ID93 needs its currently due opcode-114 spawn because native input
// latency makes the first observed pooled-laser state too late. ID198 needs a
// short initial leg before its constant continuation to avoid a measured
// bullet/rotating-beam trap. ID199 instead needs the measured constant-velocity
// ablation. ID201 combines bounded WAIT motion with deterministic bullets from
// its current future ECL cursor. ID202 reuses only the observed WAIT bound:
// its random future emissions cannot use the deterministic ECL adapter.
// ID204 needs the active relative-direction deceleration/turn recurrence.
// The native runtime remains the acceptance oracle.
inline NativeSpellPolicy native_spell_policy(int spell_id) {
    if (spell_id == 85)
        return {"id85-rigid-laser-motion", {12, 120, true, 0x1ff, true}};
    if (spell_id == 89)
        return {"id89-direct-ecl-laser", {12, 120, true}, true};
    if (spell_id == 93)
        return {"id93-imminent-pooled-laser", {12, 120, true}, false, true};
    if (spell_id == 198)
        return {"id198-two-leg-rigid-laser", {12, 120, true, 0x1ff, true, 4}};
    if (spell_id == 199)
        return {"id199-linear-ranking", {12, 120, false}};
    if (spell_id == 201)
        return {
            "id201-wait-and-ecl-shot", {32, 120, true, 0x1ff, false, 0, true}, false, false, true};
    if (spell_id == 202)
        return {"id202-observed-wait", {12, 120, true, 0x1ff, false, 0, true}};
    if (spell_id == 204)
        return {"id204-relative-direction", {12, 120, true, 0x1ff, false, 0, false, true}};
    return {"source-vector-ranking", {12, 120, true}};
}
} // namespace th08::policy
