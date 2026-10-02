#include "session.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"
#include "Spellcard.hpp"
#include "Supervisor.hpp"
#include "TextHelper.hpp"
#include "pbg/PbgArchive.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace th08::headless {
namespace {
void require(bool ok, const char *message) {
    if (!ok)
        throw std::runtime_error(message);
}
// Serialized opcode 114 payload. Keep this private to the observation adapter;
// the policy consumes only resolved scalar geometry, never native pointers.
struct NativeLaserSpawnArgs {
    std::uint16_t bullet_type;
    std::int16_t color;
    float angle, speed, start_offset, end_offset, start_length, width;
    std::int32_t start_time, duration, despawn_duration, hitbox_start_time, hitbox_end_delay;
    std::uint32_t transform_flags;
};
static_assert(sizeof(NativeLaserSpawnArgs) == 0x34, "opcode 114 payload layout changed");
} // namespace
Session::Session(const Config &config) {
    static bool used = false;
    require(!used, "one native session per process; start a fresh process for replay");
    used = true;
    require(config.dat_path && config.stage >= 0 && config.stage < 9 && config.spell >= -1 &&
                config.spell < 222 && config.difficulty >= 0 && config.difficulty < 5,
            "invalid native checkpoint");
    g_Supervisor.InitializeCriticalSections();
    require(g_Supervisor.LoadConfig(const_cast<char *>("th08.cfg")) == ZUN_SUCCESS,
            "configuration initialization failed");
    g_Supervisor.cfg.musicMode = OFF;
    g_Supervisor.cfg.playSounds = 0;
    g_Supervisor.cfg.opts.dontUseDirectInput = 1;
    g_Supervisor.cfg.opts.dontUseVertexBuf = 1;
    g_Supervisor.disableVsync = 1;
    g_Supervisor.framerateMultiplier = 1;
    g_Supervisor.d3dIface = Direct3DCreate8(D3D_SDK_VERSION);
    require(g_Supervisor.d3dIface->CreateDevice(0, D3DDEVTYPE_HAL, nullptr, 0,
                                                &g_Supervisor.presentParameters,
                                                &g_Supervisor.d3dDevice) == S_OK,
            "CPU resource device initialization failed");
    g_AnmManager = new AnmManager();
    require(g_PbgArchive.Load(config.dat_path), "cannot load TH08 archive");
    TextHelper::CreateTextBuffer();
    g_Supervisor.loadingAnm = g_AnmManager->LoadAnm(ANM_FILE_SLOT_NOW_LOADING, "nowloading.anm");
    g_Supervisor.textAnm = g_AnmManager->LoadAnm(ANM_FILE_SLOT_TEXT, "text.anm");
    require(g_Supervisor.loadingAnm && g_Supervisor.textAnm, "common ANM initialization failed");
    require(AsciiManager::RegisterChain() == ZUN_SUCCESS, "ASCII manager initialization failed");
    require(g_AnmManager->ServicePreloadedAnims() == ZUN_SUCCESS, "common ANM load failed");
    g_Supervisor.curState = g_Supervisor.wantedState = SupervisorState_GameManager;
    g_Supervisor.wantedState2 = SupervisorState_GameManager;
    g_GameManager.flags.isPracticeMode = config.spell < 0;
    g_GameManager.flags.isSpellPractice = config.spell >= 0;
    g_GameManager.currentStage = config.stage;
    g_GameManager.currentSpellCardNumber = config.spell;
    g_GameManager.difficulty = config.difficulty;
    g_GameManager.shotType = 0;
    g_GameManager.fullShotType = 0;
    g_Rng.SetSeed(config.seed);
    g_Rng.ResetGenerationCount();
    auto *supervisor = g_Chain.CreateElem(reinterpret_cast<ChainCallback>(Supervisor::OnUpdate));
    supervisor->arg = &g_Supervisor;
    require(g_Chain.AddToCalcChain(supervisor, CHAIN_PRIO_CALC_SUPERVISOR) == ZUN_SUCCESS,
            "supervisor update registration failed");
    require(GameManager::RegisterChain() == ZUN_SUCCESS &&
                g_GameManager.gameplaySetupState == GAMEPLAY_SETUP_COMPLETE,
            "native gameplay setup failed");
    require(g_AnmManager->ServicePreloadedAnims() == ZUN_SUCCESS, "stage ANM load failed");
    prepare_observation_storage();
    views_.reserve(1536);
    laser_views_.reserve(256);
    ecl_views_.reserve(512);
    laser_spawn_views_.reserve(64);
}
Session::~Session() {
    g_Chain.Release();
    TextHelper::ReleaseTextBuffer();
    delete g_AnmManager;
    g_AnmManager = nullptr;
    g_Supervisor.d3dDevice->Release();
    g_Supervisor.d3dDevice = nullptr;
    g_Supervisor.d3dIface->Release();
    g_Supervisor.d3dIface = nullptr;
    g_Supervisor.DeleteCriticalSections();
}
State Session::step(std::uint16_t input) {
    require((input & ~std::uint16_t(0x10f7)) == 0, "unsupported headless input bits");
    th08_headless_input = input;
    begin_update_observation();
    ++th08_headless_frame;
    require(g_Chain.RunCalcChain() > 0, "native update chain stopped");
    g_SoundPlayer.ProcessQueues();
    // The draw chain is not run: its jobs are presentation-only. Gameplay,
    // including stage/menu gates, belongs to the original calc chain.
    return state();
}
State Session::state() const {
    return {th08_headless_frame,
            g_Player.position.x,
            g_Player.position.y,
            g_GameManager.globals ? g_GameManager.globals->deaths : 0,
            g_GameManager.currentStage,
            g_Player.playerState,
            g_BulletManager.activeBulletCount,
            g_EnemyManager.activeEnemyCount,
            g_Spellcard.spellCardNumber,
            bool(g_Spellcard.IsActive()),
            bool(g_GameManager.showRetryMenu),
            g_GameManager.flags.stageTransitionState >= 2,
            g_Rng.GetSeed(),
            g_Rng.GetGenerationCount(),
            g_GameManager.globals->score,
            g_GameManager.globals->graze,
            g_GameManager.globals->youkaiGauge,
            g_GameManager.globals->livesRemaining,
            g_Player.hurtboxHalfSize.x,
            g_Player.hurtboxHalfSize.y,
            g_GuiMessageInputCurrent,
            g_CurFrameInput};
}
const std::vector<BulletView> &Session::bullets() {
    views_.clear();
    int slot = 0;
    for (const auto &b : g_BulletManager.bullets) {
        if (b.state) {
            const auto &acceleration = b.exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION];
            views_.push_back({b.position.x, b.position.y, b.velocity.x, b.velocity.y, b.state, slot,
                              b.sprites.collisionSize.x, b.sprites.collisionSize.y,
                              b.activeTransformFlags, acceleration.vector.x, acceleration.vector.y,
                              acceleration.timer.current, acceleration.durationFrames});
        }
        ++slot;
    }
    return views_;
}
const std::vector<LaserView> &Session::lasers() {
    laser_views_.clear();
    int slot = 0;
    for (const auto &laser : g_BulletManager.lasers) {
        if (laser.inUse) {
            const auto motion = current_laser_motion(slot);
            laser_views_.push_back(
                {laser.position.x,      laser.position.y,    laser.angle,
                 laser.startOffset,     laser.endOffset,     laser.startLength,
                 laser.width,           laser.speed,         laser.startTime,
                 laser.hitboxStartTime, laser.duration,      laser.despawnDuration,
                 laser.hitboxEndDelay,  laser.timer.current, slot,
                 laser.flags,           laser.state,         motion.origin_delta_x,
                 motion.origin_delta_y, motion.angle_delta,  motion.continuous});
        }
        ++slot;
    }
    return laser_views_;
}
const std::vector<LaserHitboxView> &Session::laser_hitboxes() const {
    return current_laser_hitboxes();
}
const std::vector<EclContextView> &Session::ecl_contexts() {
    ecl_views_.clear();
    auto append = [&](const Enemy &enemy, const EnemyEclContext &context, int child_slot) {
        const auto *instruction = context.currentInstr;
        int raw_int0 = 0;
        const bool has_raw_int0 = instruction && instruction->nextOffset >= 16;
        if (has_raw_int0)
            std::memcpy(&raw_int0, instruction->operands, sizeof(raw_int0));
        int active_interpolations = 0;
        for (const auto &slot : context.interpolationSlots)
            active_interpolations += slot.callback != nullptr;
        int per_frame_ex = -1;
        for (int i = 0; i < 32; ++i)
            if (context.perFrameCallback == g_EclExInsn[i]) {
                per_frame_ex = i;
                break;
            }
        const auto active_difficulty = static_cast<std::uint32_t>(g_GameManager.difficultyMask) |
                                       enemy.eclDifficultyMaskOverride;
        ecl_views_.push_back(
            {enemy.enemyIndex,
             child_slot,
             context.subId,
             context.time.current,
             instruction ? instruction->time : 0,
             instruction ? instruction->opcode : 0,
             instruction ? instruction->nextOffset : 0,
             context.secondaryTime.current,
             enemy.pendingEclSubroutineIndex,
             active_interpolations,
             per_frame_ex,
             instruction ? instruction->difficultyMask : std::uint8_t(0),
             instruction ? instruction->operandFlags : std::uint16_t(0),
             enemy.flags1,
             instruction && (instruction->difficultyMask & active_difficulty) == active_difficulty,
             has_raw_int0,
             enemy.parentEnemy != nullptr,
             raw_int0,
             enemy.position.x,
             enemy.position.y,
             enemy.positionOffset.x,
             enemy.positionOffset.y,
             enemy.velocity.x,
             enemy.velocity.y,
             enemy.vm.rotation.z,
             enemy.vm.angleVel.z,
             context.floatVariables[0],
             context.floatVariables[1]});
    };
    for (const auto &enemy : g_EnemyManager.enemies) {
        if (!(enemy.flags1 & ENEMY_FLAG_ACTIVE))
            continue;
        append(enemy, enemy.mainEclContextStorage, 0);
        for (int slot = 0; slot < 4; ++slot)
            if (enemy.childEclBlocks[slot])
                append(enemy, enemy.childEclBlocks[slot]->eclContext, slot + 1);
    }
    return ecl_views_;
}
const std::vector<ImminentLaserSpawnView> &Session::imminent_laser_spawns() {
    constexpr std::int16_t create_laser = 114, create_laser_aimed = 115;
    constexpr std::uint16_t variable_angle = 1u << 2;
    constexpr std::uint16_t variable_geometry_or_lifecycle = 0x7f8;
    constexpr std::uint32_t spawn_during_suppression = 0x4;
    constexpr int local_float_0 = 0x2720, local_float_7 = 0x2727;

    laser_spawn_views_.clear();
    const bool pool_has_space =
        std::any_of(std::begin(g_BulletManager.lasers), std::end(g_BulletManager.lasers),
                    [](const Laser &laser) { return !laser.inUse; });
    auto append = [&](const Enemy &enemy, const EnemyEclContext &context, bool main_context) {
        const auto *instruction = context.currentInstr;
        if (!instruction ||
            (instruction->opcode != create_laser && instruction->opcode != create_laser_aimed))
            return;
        const auto active_difficulty = static_cast<std::uint32_t>(g_GameManager.difficultyMask) |
                                       enemy.eclDifficultyMaskOverride;
        if ((instruction->difficultyMask & active_difficulty) != active_difficulty ||
            context.time.current != instruction->time || context.secondaryTime.current > 0 ||
            enemy.pendingEclSubroutineIndex >= 0)
            return;

        ImminentLaserSpawnView view;
        view.enemy_index = enemy.enemyIndex;
        view.opcode = instruction->opcode;
        // Child execution follows the main context and may be invalidated by it;
        // this narrow preview deliberately owns only the next main instruction.
        // A full pool may also change before this call, so a snapshot cannot
        // prove that native SpawnLaserPattern will suppress the instruction.
        if (!main_context || !pool_has_space || instruction->opcode == create_laser_aimed ||
            instruction->nextOffset != 0x40 ||
            (instruction->operandFlags & variable_geometry_or_lifecycle)) {
            laser_spawn_views_.push_back(view);
            return;
        }

        NativeLaserSpawnArgs args{};
        std::memcpy(&args, instruction->operands, sizeof(args));
        float angle = args.angle;
        if (instruction->operandFlags & variable_angle) {
            if (!std::isfinite(args.angle) || std::trunc(args.angle) != args.angle ||
                args.angle < local_float_0 || args.angle >= local_float_7 + 1) {
                laser_spawn_views_.push_back(view);
                return;
            }
            const int selector = static_cast<int>(args.angle);
            angle = context.floatVariables[selector - local_float_0];
        }
        const float origin_x = enemy.position.x + enemy.positionOffset.x + enemy.shootOffset.x;
        const float origin_y = enemy.position.y + enemy.positionOffset.y + enemy.shootOffset.y;
        const bool finite = std::isfinite(origin_x) && std::isfinite(origin_y) &&
                            std::isfinite(angle) && std::isfinite(args.speed) &&
                            std::isfinite(args.start_offset) && std::isfinite(args.end_offset) &&
                            std::isfinite(args.start_length) && std::isfinite(args.width);
        if (!finite || args.start_offset < 0 || args.end_offset < args.start_offset ||
            args.start_length < 0 || args.width < 0 || args.speed < 0 || args.start_time < 0 ||
            args.duration < 0 || args.despawn_duration < 0 || args.hitbox_start_time < 0 ||
            args.hitbox_end_delay < 0) {
            laser_spawn_views_.push_back(view);
            return;
        }

        view.suppressed = g_BulletManager.spawnSuppressionFrames != 0 &&
                          !(args.transform_flags & spawn_during_suppression);
        view.supported = true;
        view.origin_x = origin_x;
        view.origin_y = origin_y;
        view.angle = angle;
        view.start_offset = args.start_offset;
        view.end_offset = args.end_offset;
        view.start_length = args.start_length;
        view.width = args.width;
        view.speed = args.speed;
        view.start_time = args.start_time;
        view.hitbox_start_time = args.hitbox_start_time;
        view.duration = args.duration;
        view.despawn_duration = args.despawn_duration;
        view.hitbox_end_delay = args.hitbox_end_delay;
        view.flags = static_cast<std::uint16_t>(args.transform_flags);
        laser_spawn_views_.push_back(view);
    };
    for (const auto &enemy : g_EnemyManager.enemies) {
        if (!(enemy.flags1 & ENEMY_FLAG_ACTIVE))
            continue;
        append(enemy, enemy.mainEclContextStorage, true);
        for (int slot = 0; slot < 4; ++slot)
            if (enemy.childEclBlocks[slot])
                append(enemy, enemy.childEclBlocks[slot]->eclContext, false);
    }
    return laser_spawn_views_;
}
CollisionEvent Session::collision() const {
    return current_collision();
}
float Session::focused_axis_speed() const {
    return g_Player.secondaryShtFile->focusedAxisSpeed;
}
float Session::focused_diagonal_speed() const {
    return g_Player.secondaryShtFile->focusedDiagonalSpeed;
}
std::uint64_t Session::file_io_time_ns() const {
    return file_io_ns;
}
std::uint64_t Session::actor_digest() const {
    std::uint64_t digest = 1469598103934665603ULL;
    auto hash = [&](std::uint64_t value) { digest = (digest ^ value) * 1099511628211ULL; };
    for (const auto &enemy : g_EnemyManager.enemies) {
        if (!(enemy.flags1 & ENEMY_FLAG_ACTIVE))
            continue;
        hash(enemy.enemyIndex);
        hash(enemy.flags1);
        hash(enemy.life);
        hash(enemy.bossTimer.current);
        hash(enemy.mainEclContextStorage.subId);
        hash(enemy.mainEclContextStorage.time.current);
    }
    return digest;
}
} // namespace th08::headless
