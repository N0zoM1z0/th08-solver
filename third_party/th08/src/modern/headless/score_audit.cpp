#include "score_audit.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "Spellcard.hpp"
#include "replay_check.hpp"

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
const ScoreAudit &score_audit() {
    return audit;
}
void score_audit_graze() {
    if (!enabled)
        return;
    ++audit.graze_events;
    audit.extreme_human_graze_events += g_GameManager.GaugeIsExtremelyHuman();
    audit.extreme_youkai_graze_events += g_GameManager.GaugeIsExtremelyYoukai();
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
