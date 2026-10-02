#pragma once

#include "native_policy.hpp"

namespace th08::policy {
struct NativeSpellPolicy {
    const char *name;
    HazardReactiveOptions hazards;
    bool direct_ecl_lasers = false;
};

// Spell-specific selection belongs here rather than in the generic projection
// kernel. ID198 needs a short initial leg before its constant continuation to
// avoid a measured bullet/rotating-beam trap. ID199 instead needs the measured
// constant-velocity ablation. The native runtime remains the acceptance oracle.
inline NativeSpellPolicy native_spell_policy(int spell_id) {
    if (spell_id == 85)
        return {"id85-rigid-laser-motion", {12, 120, true, 0x1ff, true}};
    if (spell_id == 89)
        return {"id89-direct-ecl-laser", {12, 120, true}, true};
    if (spell_id == 198)
        return {"id198-two-leg-rigid-laser", {12, 120, true, 0x1ff, true, 4}};
    if (spell_id == 199)
        return {"id199-linear-ranking", {12, 120, false}};
    return {"source-vector-ranking", {12, 120, true}};
}
} // namespace th08::policy
