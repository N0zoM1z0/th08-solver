#include "score_audit.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "Spellcard.hpp"
#include "replay_check.hpp"
#include <algorithm>

namespace th08::headless {
namespace {
bool enabled = false;
ScoreAudit audit;
constexpr std::size_t no_open_spell = std::size_t(-1);
std::size_t open_spell = no_open_spell;
} // namespace

void score_audit_enable() {
    audit = {};
    open_spell = no_open_spell;
    enabled = true;
}
bool score_audit_enabled() {
    return enabled;
}
bool score_capture_failed(const ScoreAudit &record) {
    return record.unmatched_events ||
           std::any_of(record.spells.begin(), record.spells.end(), [](const auto &spell) {
               return spell.outcome != SpellAuditOutcome::Open &&
                      spell.outcome != SpellAuditOutcome::Captured;
           });
}
const ScoreAudit &score_audit() {
    return audit;
}
ScoreCaptureCoverage score_capture_coverage(const ScoreAudit &record, bool complete, int selected) {
    ScoreCaptureCoverage coverage;
    for (int stage = 0; stage < int(record.stages.size()); ++stage)
        if (record.stages[stage].seen && (selected < 0 || selected == stage))
            for (int i = 0; i < g_SpellcardCountPerStage[stage]; ++i) {
                const int spell = g_SpellcardNumbersPerStage[stage][i];
                if (Spellcard::GetDifficultyFromSpellCard(spell) == record.difficulty)
                    coverage.expected.insert(spell);
            }
    for (const auto &spell : record.spells) {
        if (selected >= 0 && spell.stage != selected)
            continue;
        coverage.encountered.insert(spell.spell);
        if (spell.outcome == SpellAuditOutcome::Captured)
            coverage.captured.insert(spell.spell);
        else
            ++coverage.unsuccessful;
    }
    coverage.all = complete && !coverage.expected.empty() && !coverage.unsuccessful &&
                   !record.unmatched_events &&
                   std::includes(coverage.captured.begin(), coverage.captured.end(),
                                 coverage.expected.begin(), coverage.expected.end());
    return coverage;
}
void score_audit_graze(bool suppress_extra_items) {
    if (!enabled)
        return;
    const bool human = g_GameManager.GaugeIsExtremelyHuman();
    const bool youkai = g_GameManager.GaugeIsExtremelyYoukai();
    const auto count = [&](auto &record) {
        ++record.graze_events;
        record.extreme_human_graze_events += human;
        record.extreme_youkai_graze_events += youkai;
        record.extra_item_suppressed_graze_events += suppress_extra_items;
        record.ordinary_extreme_youkai_graze_events += youkai && !suppress_extra_items;
    };
    count(audit);
    if (open_spell != no_open_spell)
        count(audit.spells[open_spell]);
}
void score_audit_observe() {
    if (!enabled || !g_GameManager.globals)
        return;
    const auto &g = *g_GameManager.globals;
    if (open_spell != no_open_spell && g_Spellcard.activeEnemy)
        audit.spells[open_spell].survival =
            audit.spells[open_spell].survival ||
            (g_Spellcard.activeEnemy->flags1 & ENEMY_FLAG_TIMEOUT_SPELL);
    audit.score_raw = g.score;
    audit.deaths = g.deaths;
    audit.bombs_used = g_GameManager.GetBombsUsed();
    audit.difficulty = g_GameManager.difficulty;
    audit.shot = g_GameManager.shotType;
    audit.death_rows += g.deaths != 0 || g_Player.playerState == PLAYER_STATE_DYING;
    const int stage = g_GameManager.currentStage;
    if (stage < 0 || stage >= int(audit.stages.size()))
        return;
    auto &s = audit.stages[stage];
    s.seen = true;
    ++s.updates;
    s.score_raw = g.score;
    s.point_items = g.pointItemsCollectedInStage;
    s.time_orbs = g.currentTimeOrbs;
    s.time_requirement = g.lastSpellTimeOrbThreshold;
    s.graze = g.grazeInStage;
    s.point_value = g.pointItemValue;
    s.clock = g.clockTime;
}
void score_audit_spell_start(const Spellcard &spell) {
    if (!enabled)
        return;
    if (open_spell != no_open_spell)
        ++audit.unmatched_events;
    SpellAuditRecord record;
    record.stage = g_GameManager.currentStage;
    record.spell = spell.spellCardNumber;
    record.start_update = replay_calculation_updates();
    record.limit = spell.timeLimit.current;
    record.time_orbs_start = g_GameManager.GetTimeOrbs();
    record.point_items_start = g_GameManager.globals->pointItemsCollectedInStage;
    record.score_raw_start = g_GameManager.globals->score;
    record.point_value_start = g_GameManager.globals->pointItemValue;
    record.last_spell = Spellcard::IsLastSpell(record.spell) != 0;
    record.survival = spell.activeEnemy && (spell.activeEnemy->flags1 & ENEMY_FLAG_TIMEOUT_SPELL);
    audit.spells.push_back(record);
    open_spell = audit.spells.size() - 1;
}
void score_audit_spell_end(const Spellcard &spell, bool abandoned) {
    if (!enabled)
        return;
    if (open_spell == no_open_spell || audit.spells[open_spell].spell != spell.spellCardNumber) {
        ++audit.unmatched_events;
        return;
    }
    auto &record = audit.spells[open_spell];
    // ECL can set the survival flag after starting the cut-in.
    record.survival = record.survival ||
                      (spell.activeEnemy && (spell.activeEnemy->flags1 & ENEMY_FLAG_TIMEOUT_SPELL));
    record.end_update = replay_calculation_updates();
    record.remaining = spell.timeRemaining.current;
    record.time_orbs = g_GameManager.GetTimeOrbs();
    record.point_items_end = g_GameManager.globals->pointItemsCollectedInStage;
    record.score_raw_end = g_GameManager.globals->score;
    record.point_value_end = g_GameManager.globals->pointItemValue;
    record.field_time_orbs = g_ItemManager.GetTimeOrbCount();
    record.time_requirement = g_GameManager.GetLastSpellTimeOrbThreshold();
    if (abandoned)
        record.outcome = SpellAuditOutcome::Abandoned;
    else if (spell.flags & SPELLCARD_FLAG_CAPTURED) {
        record.outcome = SpellAuditOutcome::Captured;
        record.bonus = spell.bonusAward;
        record.pending_time_orbs = spell.pendingTimeOrbs;
    } else if (spell.flags & SPELLCARD_FLAG_TIMER_CALLBACK_TRANSITION)
        record.outcome = SpellAuditOutcome::Timeout;
    else
        record.outcome = SpellAuditOutcome::Invalidated;
    open_spell = no_open_spell;
}
} // namespace th08::headless
