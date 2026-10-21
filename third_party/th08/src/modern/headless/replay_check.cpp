#include "replay_check.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "modern/linux/replay_trace.hpp"
#include <cstring>

namespace th08::headless {
namespace {
constexpr int field_count = 25;
std::FILE *rows_file = nullptr;
bool initialized = false;
ReplayCheckSummary summary;
std::uint32_t float_bits(const f32 &value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}
} // namespace
void replay_check_enable(std::FILE *rows) {
    rows_file = rows;
}
const ReplayCheckSummary &replay_check_summary() {
    return summary;
}
} // namespace th08::headless

namespace th08::modern {
using namespace th08::headless;
// Mirrors upstream Initialize(): playback mode 2 is installed on the first
// GameManager update, exactly where the published retail captures set it.
void ReplayTraceBeginCalculation() {
    if (!rows_file || summary.finished)
        return;
    if (!initialized) {
        initialized = true;
        g_GameManager.replayMode = 2;
    }
    if (g_ReplayManager && g_ReplayManager->frameCounter == 0) {
        const int stage = g_GameManager.currentStage;
        if (stage >= 0 && stage < MAX_STAGES)
            summary.rng_origins[stage] = g_Rng.ReplayTraceGenerationCount();
    }
}
bool ReplayTraceRecord(ReplayManager *manager) {
    if (!rows_file || summary.finished || !g_GameManager.flags.isReplay ||
        g_GameManager.flags.isDemoMode)
        return false;
    const int stage = g_GameManager.currentStage;
    if (stage < 0 || stage >= MAX_STAGES || !g_GameManager.globals) {
        summary.finished = true;
        return true;
    }
    if (!summary.stage_count || summary.stages[summary.stage_count - 1] != stage) {
        if (summary.stage_count == MAX_STAGES || summary.stage_frames[stage]) {
            summary.finished = true;
            return true;
        }
        summary.stages[summary.stage_count++] = stage;
    }
    const ZunGlobals &globals = *g_GameManager.globals;
    const std::uint32_t row[field_count] = {summary.rows + 1,
                                            std::uint32_t(stage),
                                            std::uint32_t(manager->frameCounter),
                                            g_GameManager.gameplayFrameCounter,
                                            globals.score,
                                            std::uint32_t(globals.graze),
                                            float_bits(globals.deaths),
                                            float_bits(globals.livesRemaining),
                                            float_bits(globals.bombsRemaining),
                                            float_bits(globals.playerPower),
                                            float_bits(g_Player.position.x),
                                            float_bits(g_Player.position.y),
                                            float_bits(g_Player.position.z),
                                            std::uint8_t(g_Player.playerState),
                                            g_Rng.GetSeed(),
                                            g_Rng.ReplayTraceGenerationCount() -
                                                summary.rng_origins[stage],
                                            g_GuiMessageInputCurrent,
                                            std::uint32_t(globals.pointItemsCollected),
                                            std::uint32_t(globals.currentTimeOrbs),
                                            std::uint32_t(globals.youkaiGauge),
                                            std::uint8_t(globals.clockTime),
                                            std::uint32_t(globals.pointItemValue),
                                            g_GameManager.character,
                                            g_GameManager.shotType,
                                            std::uint32_t(g_GameManager.difficulty)};
    if (std::fwrite(row, sizeof(std::uint32_t), field_count, rows_file) != field_count) {
        summary.write_failed = true;
        summary.finished = true;
        return true;
    }
    ++summary.rows;
    if (globals.deaths != 0 || g_Player.playerState == PLAYER_STATE_DYING)
        ++summary.death_rows;
    if (g_GuiMessageInputCurrent & 2)
        ++summary.bomb_input_rows;
    summary.stage_frames[stage] = row[2];
    summary.stage_scores[stage] = row[4];
    return false;
}
void ReplayTraceFinish() {
    if (rows_file && !summary.finished && summary.rows) {
        summary.finished = true;
        summary.natural = true;
    }
}
bool ReplayTraceShouldExit() {
    return rows_file && summary.finished;
}
} // namespace th08::modern
