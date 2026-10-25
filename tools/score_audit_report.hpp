#pragma once
#include "Spellcard.hpp"
#include "modern/headless/score_audit.hpp"
#include <algorithm>
#include <ostream>
#include <set>

namespace th08::report {
inline const char *spell_outcome(headless::SpellAuditOutcome outcome) {
    using O = headless::SpellAuditOutcome;
    switch (outcome) {
    case O::Captured:
        return "captured";
    case O::Timeout:
        return "timeout";
    case O::Invalidated:
        return "invalidated";
    case O::Abandoned:
        return "abandoned";
    case O::Open:
        return "open";
    }
    return "unknown";
}
// Coverage is the source catalogue for the stages actually visited, including
// conditional Last Spells. Skipping a gate never silently shrinks the target.
inline bool write_score_audit(std::ostream &out, const headless::ScoreAudit &audit, bool complete) {
    using O = headless::SpellAuditOutcome;
    std::set<int> expected, encountered, captured;
    unsigned failures = 0;
    for (int stage = 0; stage < int(audit.stages.size()); ++stage)
        if (audit.stages[stage].seen)
            for (int i = 0; i < g_SpellcardCountPerStage[stage]; ++i) {
                const int spell = g_SpellcardNumbersPerStage[stage][i];
                if (Spellcard::GetDifficultyFromSpellCard(spell) == audit.difficulty)
                    expected.insert(spell);
            }
    for (const auto &spell : audit.spells) {
        encountered.insert(spell.spell);
        if (spell.outcome == O::Captured)
            captured.insert(spell.spell);
        else
            ++failures;
    }
    const bool all =
        complete && !expected.empty() && !failures && !audit.unmatched_events &&
        std::includes(captured.begin(), captured.end(), expected.begin(), expected.end());
    out << "{\"schema_version\":1,\"evidence\":\"native-source-lifecycle-audit\","
           "\"score_raw_unit\":10,\"complete\":"
        << (complete ? "true" : "false") << ",\"difficulty\":" << audit.difficulty
        << ",\"shot\":" << audit.shot << ",\"score_raw\":" << audit.score_raw
        << ",\"score_displayed\":" << std::uint64_t(audit.score_raw) * 10 << ",\"nmnb\":"
        << (!audit.death_rows && audit.deaths == 0 && !audit.bombs_used ? "true" : "false")
        << ",\"death_rows\":" << audit.death_rows << ",\"bombs_used\":" << audit.bombs_used
        << ",\"graze_events\":" << audit.graze_events
        << ",\"extreme_human_graze_events\":" << audit.extreme_human_graze_events
        << ",\"extreme_youkai_graze_events\":" << audit.extreme_youkai_graze_events
        << ",\"extra_item_suppressed_graze_events\":" << audit.extra_item_suppressed_graze_events
        << ",\"ordinary_extreme_youkai_graze_events\":"
        << audit.ordinary_extreme_youkai_graze_events
        << ",\"unmatched_events\":" << audit.unmatched_events
        << ",\"expected_unique\":" << expected.size()
        << ",\"encountered_unique\":" << encountered.size()
        << ",\"captured_unique\":" << captured.size() << ",\"unsuccessful_attempts\":" << failures
        << ",\"all_visited_stage_spells_captured\":" << (all ? "true" : "false") << ",\"stages\":[";
    bool first = true;
    for (int stage = 0; stage < int(audit.stages.size()); ++stage) {
        const auto &s = audit.stages[stage];
        if (!s.seen)
            continue;
        out << (first ? "" : ",") << "{\"stage\":" << stage << ",\"observations\":" << s.updates
            << ",\"score_displayed\":" << std::uint64_t(s.score_raw) * 10
            << ",\"point_items\":" << s.point_items << ",\"time_orbs\":" << s.time_orbs
            << ",\"time_requirement\":" << s.time_requirement << ",\"graze\":" << s.graze
            << ",\"point_value\":" << s.point_value << ",\"clock\":" << s.clock << '}';
        first = false;
    }
    out << "],\"missing\":[";
    first = true;
    for (int spell : expected)
        if (!encountered.count(spell)) {
            out << (first ? "" : ",") << "{\"spell\":" << spell << ",\"card\":" << spell + 1
                << ",\"last_spell\":" << (Spellcard::IsLastSpell(spell) ? "true" : "false") << '}';
            first = false;
        }
    out << "],\"spells\":[";
    first = true;
    for (const auto &s : audit.spells) {
        out << (first ? "" : ",") << "{\"stage\":" << s.stage << ",\"spell\":" << s.spell
            << ",\"card\":" << s.spell + 1 << ",\"outcome\":\"" << spell_outcome(s.outcome)
            << "\",\"survival\":" << (s.survival ? "true" : "false")
            << ",\"last_spell\":" << (s.last_spell ? "true" : "false")
            << ",\"start_update\":" << s.start_update << ",\"end_update\":" << s.end_update
            << ",\"limit_frames\":" << s.limit << ",\"remaining_frames\":" << s.remaining
            << ",\"bonus_displayed\":" << s.bonus
            << ",\"pending_time_orbs\":" << s.pending_time_orbs
            << ",\"time_orbs_at_start\":" << s.time_orbs_start
            << ",\"time_orbs_at_end\":" << s.time_orbs
            << ",\"point_items_at_start\":" << s.point_items_start
            << ",\"point_items_at_end\":" << s.point_items_end
            << ",\"point_value_at_start\":" << s.point_value_start
            << ",\"point_value_at_end\":" << s.point_value_end
            << ",\"score_displayed_at_start\":" << std::uint64_t(s.score_raw_start) * 10
            << ",\"score_displayed_at_end\":" << std::uint64_t(s.score_raw_end) * 10
            << ",\"graze_events\":" << s.graze_events
            << ",\"extreme_human_graze_events\":" << s.extreme_human_graze_events
            << ",\"extreme_youkai_graze_events\":" << s.extreme_youkai_graze_events
            << ",\"extra_item_suppressed_graze_events\":" << s.extra_item_suppressed_graze_events
            << ",\"ordinary_extreme_youkai_graze_events\":"
            << s.ordinary_extreme_youkai_graze_events
            << ",\"field_time_orbs_at_end\":" << s.field_time_orbs
            << ",\"time_requirement\":" << s.time_requirement << '}';
        first = false;
    }
    out << "]}\n";
    return all;
}
} // namespace th08::report
