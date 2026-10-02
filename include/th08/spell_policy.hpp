#pragma once

#include "native_policy.hpp"

namespace th08::policy {
struct NativeSpellPolicy {
    const char *name;
    HazardReactiveOptions hazards;
};

// Spell-specific selection belongs here rather than in the generic projection
// kernel. ID199 is the only measured exception so far: constant-velocity hazard
// ranking completed seeds 0, 1 and 65535, while vector ranking failed seed 0.
// The native runtime remains the acceptance oracle for both proposal models.
inline NativeSpellPolicy native_spell_policy(int spell_id) {
    if (spell_id == 199)
        return {"id199-linear-ranking", {12, 120, false}};
    return {"source-vector-ranking", {12, 120, true}};
}
} // namespace th08::policy
