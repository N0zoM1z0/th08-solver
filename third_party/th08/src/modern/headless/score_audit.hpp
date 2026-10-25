#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace th08 {
struct Spellcard;
namespace headless {
// Read-only instrumentation. Raw score units are ten displayed points; spell
// bonuses and point-item values already use displayed points.
struct ScoreAuditStage {
    bool seen = false;
    std::uint64_t updates = 0;
    std::uint32_t score_raw = 0;
    int point_items = 0, time_orbs = 0, time_requirement = 0, graze = 0;
    int point_value = 0, clock = 0;
};
enum class SpellAuditOutcome { Open, Captured, Timeout, Invalidated, Abandoned };
struct SpellAuditRecord {
    int stage = 0, spell = -1;
    // Monotonic GameManager calculations, excluding control-only handoffs.
    // These are neither replay input-clock values nor Session API-call counts.
    std::uint64_t start_update = 0, end_update = 0;
    int limit = 0, remaining = 0, bonus = 0, pending_time_orbs = 0;
    int time_orbs = 0, field_time_orbs = 0, time_requirement = 0;
    bool survival = false, last_spell = false;
    SpellAuditOutcome outcome = SpellAuditOutcome::Open;
};
struct ScoreAudit {
    std::array<ScoreAuditStage, 9> stages{};
    std::vector<SpellAuditRecord> spells;
    std::uint64_t death_rows = 0;
    // AwardGraze invocations, including repeated laser contacts. The displayed
    // counter awards 3/2/1 per event and cannot substitute for these counts.
    std::uint64_t graze_events = 0, extreme_human_graze_events = 0;
    std::uint64_t extreme_youkai_graze_events = 0;
    int bombs_used = 0, difficulty = 0, shot = 0;
    float deaths = 0;
    std::uint32_t score_raw = 0;
    // Unmatched lifecycle events cannot certify complete capture coverage.
    unsigned unmatched_events = 0;
};
void score_audit_enable();
const ScoreAudit &score_audit();
// Call after each owner update; it neither steps the game nor evaluates ECL.
void score_audit_observe();
void score_audit_graze();
// Narrow lifecycle hooks preserve spells that start/end in the same update.
// Disabled unless explicitly enabled by a reporting tool, never by a planner.
void score_audit_spell_start(const Spellcard &spell);
void score_audit_spell_end(const Spellcard &spell, bool abandoned = false);
} // namespace headless
} // namespace th08
