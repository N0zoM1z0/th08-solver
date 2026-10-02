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
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
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

struct NativeShotArgs {
    std::int16_t bullet_type, color;
    std::int32_t count1, count2;
    float speed1, speed2, angle, angle_step;
    std::uint32_t transform_flags;
};
static_assert(sizeof(NativeShotArgs) == 0x20, "opcodes 96-104 payload layout changed");

struct NativeBulletTransformArgs {
    std::int32_t transform_index, kind, allow_while_active;
    std::int32_t int_payload0, int_payload1;
    float float_payload0, float_payload1;
};
static_assert(sizeof(NativeBulletTransformArgs) == 0x1c, "opcode 111 payload layout changed");

using LocalFloatSnapshot = std::array<float, 8>;

bool resolve_local_float(const LocalFloatSnapshot &locals, float raw, bool variable,
                         float &resolved) {
    if (!variable) {
        resolved = raw;
        return std::isfinite(resolved);
    }
    if (!std::isfinite(raw))
        return false;
    const int selector = static_cast<int>(raw);
    if (selector < ECL_OPERAND_LOCAL_FLOAT_0 || selector > ECL_OPERAND_LOCAL_FLOAT_7)
        return false;
    resolved = locals[selector - ECL_OPERAND_LOCAL_FLOAT_0];
    return std::isfinite(resolved);
}

// Execute only the local-float arithmetic used immediately before ID201's
// deterministic shot. Supporting any other writable selector would require a
// larger owned enemy snapshot and would blur this adapter's evidence boundary.
bool apply_local_float_binary(const EclRawInstruction &instruction, bool subtract,
                              LocalFloatSnapshot &locals) {
    if (instruction.nextOffset != 24 || (instruction.operandFlags & ~std::uint16_t(0x7)) ||
        !(instruction.operandFlags & 1))
        return false;
    std::array<float, 3> operands{};
    std::memcpy(operands.data(), instruction.operands, sizeof(operands));
    if (!std::isfinite(operands[0]))
        return false;
    const int selector = static_cast<int>(operands[0]);
    if (selector < ECL_OPERAND_LOCAL_FLOAT_0 || selector > ECL_OPERAND_LOCAL_FLOAT_7)
        return false;
    float left = 0, right = 0;
    if (!resolve_local_float(locals, operands[1], instruction.operandFlags & 2, left) ||
        !resolve_local_float(locals, operands[2], instruction.operandFlags & 4, right))
        return false;
    const float result = subtract ? left - right : left + right;
    if (!std::isfinite(result))
        return false;
    locals[selector - ECL_OPERAND_LOCAL_FLOAT_0] = result;
    return true;
}

// Apply opcode 111 to an owned descriptor copy. Integer selector operands are
// deliberately rejected; ID201 uses literals plus local-float payloads.
bool apply_bullet_transform(const EclRawInstruction &instruction, const LocalFloatSnapshot &locals,
                            BulletSpawnDescriptor &descriptor) {
    if (instruction.nextOffset != 12 + int(sizeof(NativeBulletTransformArgs)) ||
        (instruction.operandFlags & ~std::uint16_t(0x7f)) ||
        (instruction.operandFlags & std::uint16_t(0x1f)))
        return false;
    NativeBulletTransformArgs args{};
    std::memcpy(&args, instruction.operands, sizeof(args));
    if (args.transform_index < 0 || args.transform_index >= 18)
        return false;
    float float0 = 0, float1 = 0;
    if (!resolve_local_float(locals, args.float_payload0, instruction.operandFlags & 0x20,
                             float0) ||
        !resolve_local_float(locals, args.float_payload1, instruction.operandFlags & 0x40, float1))
        return false;
    auto &record = descriptor.transforms[args.transform_index];
    record.kind = args.kind;
    record.allowWhileActive = args.allow_while_active;
    record.payload.raw.int0 = args.int_payload0;
    record.payload.raw.int1 = args.int_payload1;
    record.payload.raw.float0 = float0;
    record.payload.raw.float1 = float1;
    return true;
}

struct PositionInterpolation {
    bool active = false;
    int timer = 0, duration = 0, callback_index = 0, easing = 0;
    std::array<float, 4> parameters{};
    float affected_variable = 0;
};

bool is_ecl_selector(float value) {
    if (!std::isfinite(value))
        return false;
    const int selector = static_cast<int>(value);
    return selector >= ECL_OPERAND_LOCAL_INT_0 && selector <= ECL_OPERAND_SPELL_TIMER_FRAMES;
}

float ease_interpolation(float progress, int easing) {
    switch (easing) {
    case ECL_EASING_IN_QUADRATIC:
        return progress * progress;
    case ECL_EASING_IN_CUBIC:
        return progress * progress * progress;
    case ECL_EASING_IN_QUARTIC:
        return progress * progress * progress * progress;
    case ECL_EASING_OUT_QUADRATIC:
        progress = 1.f - progress;
        return 1.f - progress * progress;
    case ECL_EASING_OUT_CUBIC:
        progress = 1.f - progress;
        return 1.f - progress * progress * progress;
    case ECL_EASING_OUT_QUARTIC:
        progress = 1.f - progress;
        return 1.f - progress * progress * progress * progress;
    default:
        return progress;
    }
}

float interpolation_value(const PositionInterpolation &slot, float progress) {
    if (slot.callback_index < 7)
        return (slot.parameters[1] - slot.parameters[0]) * progress + slot.parameters[0];
    const float weight0 = (progress - 1.f) * (progress - 1.f) * (2.f * progress + 1.f);
    const float weight1 = progress * progress * (3.f - 2.f * progress);
    const float weight2 = (1.f - progress) * (1.f - progress) * progress;
    const float weight3 = (progress - 1.f) * progress * progress;
    return weight0 * slot.parameters[0] + weight1 * slot.parameters[1] +
           weight2 * slot.parameters[2] + weight3 * slot.parameters[3];
}

// Forecast only the update integrations before a future instruction runs.
// RunEcl refreshes worldPosition before dispatch, while its interpolation tail
// changes velocity for EnemyManager's later integration.
bool forecast_shot_origin(const Enemy &enemy, const EnemyEclContext &context,
                          unsigned updates_until, float &origin_x, float &origin_y) {
    constexpr std::uint32_t unsupported_motion =
        ENEMY_FLAG_INHERIT_PARENT_POSITION | ENEMY_FLAG_MOVEMENT_MODE_MASK |
        ENEMY_FLAG_MIRROR_MOVEMENT_X | ENEMY_FLAG_CLAMP_POSITION | ENEMY_FLAG_SKIP_MOVEMENT;
    if (updates_until == 0 || context.secondaryTime.current != 0 ||
        enemy.pendingEclSubroutineIndex >= 0 || enemy.mainEclCallStackDepth != 0 ||
        context.perFrameCallback || (enemy.flags1 & unsupported_motion) ||
        g_Supervisor.framerateMultiplier != 1.f)
        return false;
    for (const auto *child : enemy.childEclBlocks)
        if (child)
            return false;

    std::array<PositionInterpolation, 8> slots{};
    bool owns_x = false, owns_y = false;
    for (std::size_t i = 0; i < slots.size(); ++i) {
        const auto &source = context.interpolationSlots[i];
        if (!source.callback)
            continue;
        const bool affects_x = source.affectedVariable == float(ECL_OPERAND_ENEMY_POSITION_X);
        const bool affects_y = source.affectedVariable == float(ECL_OPERAND_ENEMY_POSITION_Y);
        if ((!affects_x && !affects_y) || (affects_x && owns_x) || (affects_y && owns_y) ||
            source.duration <= 0 || source.timer.current < 0 ||
            source.timer.current > source.duration || source.callbackIndex < 0 ||
            source.callbackIndex > 7 || source.easing < ECL_EASING_LINEAR ||
            source.easing > ECL_EASING_OUT_QUARTIC)
            return false;
        for (float parameter : source.parameters)
            if (!std::isfinite(parameter) || is_ecl_selector(parameter))
                return false;
        auto &slot = slots[i];
        slot.active = true;
        slot.timer = source.timer.current;
        slot.duration = source.duration;
        slot.callback_index = source.callbackIndex;
        slot.easing = source.easing;
        std::copy(std::begin(source.parameters), std::end(source.parameters),
                  slot.parameters.begin());
        slot.affected_variable = source.affectedVariable;
        owns_x |= affects_x;
        owns_y |= affects_y;
    }

    float x = enemy.position.x, y = enemy.position.y;
    float vx = enemy.velocity.x, vy = enemy.velocity.y;
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(vx) || !std::isfinite(vy) ||
        !std::isfinite(enemy.positionOffset.x) || !std::isfinite(enemy.positionOffset.y) ||
        !std::isfinite(enemy.shootOffset.x) || !std::isfinite(enemy.shootOffset.y))
        return false;
    for (unsigned update = 1; update < updates_until; ++update) {
        float interpolated_x = x, interpolated_y = y;
        bool restored_position = false;
        for (auto &slot : slots) {
            if (!slot.active)
                continue;
            slot.timer = std::min(slot.timer + 1, slot.duration);
            float progress = float(slot.timer) / float(slot.duration);
            progress = ease_interpolation(progress, slot.easing);
            const float value = interpolation_value(slot, progress);
            if (slot.affected_variable == float(ECL_OPERAND_ENEMY_POSITION_X))
                interpolated_x = value;
            else
                interpolated_y = value;
            restored_position = true;
            if (slot.timer >= slot.duration)
                slot.active = false;
        }
        if (restored_position) {
            vx = interpolated_x - x;
            vy = interpolated_y - y;
        }
        x += vx;
        y += vy;
    }
    origin_x = x + enemy.positionOffset.x + enemy.shootOffset.x;
    origin_y = y + enemy.positionOffset.y + enemy.shootOffset.y;
    return std::isfinite(origin_x) && std::isfinite(origin_y);
}

// Count updates that preserve parent velocity after WAIT begins. A terminal
// child-pattern instruction still moves and collides once before its fade state
// takes effect. Non-terminal child records may immediately install another
// WAIT; all other enabled transforms end the proof before their update.
unsigned linear_updates_after_wait(const BulletTransformRecord *transforms,
                                   std::uint32_t transform_flags, int next_index, int wait_frames,
                                   unsigned limit) {
    constexpr std::uint32_t child_secondary = 0x2000000;
    std::uint64_t updates = 0;
    int index = next_index;
    int timer = wait_frames;
    bool wait_active = true;
    while (updates < limit) {
        if (wait_active) {
            // Native checks allowWhileActive before transformFlags. A disabled
            // disallowing record therefore still blocks the program.
            while (index >= 0 && index < 18) {
                const auto &record = transforms[index];
                if (record.kind == BULLET_TRANSFORM_NONE)
                    return limit;
                if (!record.allowWhileActive)
                    break;
                if (transform_flags & record.kind)
                    return unsigned(updates);
                ++index;
            }
            if (index < 0 || index >= 18)
                return limit;
            const auto segment = std::uint64_t(std::max(0, timer)) + 1;
            if (updates + segment >= limit)
                return limit;
            updates += segment;
            wait_active = false;
        }

        // With no active transform, AdvanceTransformProgram may consume
        // several disabled/child records during one native update.
        while (index >= 0 && index < 18) {
            const auto &record = transforms[index];
            if (record.kind == BULLET_TRANSFORM_NONE)
                return limit;
            if (!(transform_flags & record.kind)) {
                ++index;
                continue;
            }
            if (record.kind == BULLET_TRANSFORM_WAIT) {
                timer = record.payload.timed.frames;
                ++index;
                ++updates; // WAIT installation precedes this update's motion.
                if (updates >= limit)
                    return limit;
                if (timer > 0) {
                    --timer;
                    wait_active = true;
                }
                break;
            }
            if (record.kind != BULLET_TRANSFORM_SPAWN_CHILD_PATTERN || index + 1 >= 18 ||
                transforms[index + 1].kind != child_secondary)
                return unsigned(updates);
            const bool fade_parent = std::uint32_t(record.payload.childPrimary.packedPattern) >> 31;
            if (fade_parent)
                return unsigned(std::min<std::uint64_t>(limit, updates + 1));
            index += 2;
        }
        if (index < 0 || index >= 18)
            return limit;
    }
    return limit;
}

unsigned initial_bullet_linear_updates(const BulletSpawnDescriptor &descriptor, unsigned limit) {
    constexpr std::uint32_t spawn_style =
        BULLET_TRANSFORM_SPAWN_FAST | BULLET_TRANSFORM_SPAWN_NORMAL | BULLET_TRANSFORM_SPAWN_SLOW;
    if (descriptor.transformFlags & spawn_style || descriptor.transformStartIndex < 0 ||
        descriptor.transformStartIndex >= 18)
        return 0;
    for (int index = descriptor.transformStartIndex; index < 18; ++index) {
        const auto &record = descriptor.transforms[index];
        if (record.kind == BULLET_TRANSFORM_NONE)
            return limit;
        if (!(descriptor.transformFlags & record.kind))
            continue;
        if (record.kind != BULLET_TRANSFORM_WAIT)
            return 0;
        return linear_updates_after_wait(descriptor.transforms, descriptor.transformFlags,
                                         index + 1, record.payload.timed.frames, limit);
    }
    return limit;
}
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
    bullet_spawn_views_.reserve(256);
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
            int wait_linear_updates = 0;
            if (b.activeTransformFlags == BULLET_TRANSFORM_WAIT) {
                const int timer = b.exStates[BULLET_TRANSFORM_STATE_WAIT].timer.current;
                wait_linear_updates = int(
                    linear_updates_after_wait(b.transforms, b.transformFlags, b.transformIndex,
                                              timer, unsigned(std::numeric_limits<int>::max())));
            }
            views_.push_back({b.position.x, b.position.y, b.velocity.x, b.velocity.y, b.state, slot,
                              b.sprites.collisionSize.x, b.sprites.collisionSize.y,
                              b.activeTransformFlags, acceleration.vector.x, acceleration.vector.y,
                              acceleration.timer.current, acceleration.durationFrames,
                              wait_linear_updates});
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
const std::vector<UpcomingBulletSpawnView> &Session::upcoming_bullet_spawns(unsigned horizon) {
    constexpr std::int16_t shoot_first = ECL_OPCODE_SHOOT_FAN_AIMED;
    constexpr std::int16_t shoot_last = ECL_OPCODE_SHOOT_RANDOM;
    constexpr std::uint32_t alignment_flags =
        BULLET_TRANSFORM_ONLY_WHEN_PLAYER_YOUKAI | BULLET_TRANSFORM_ONLY_WHEN_PLAYER_HUMAN;
    constexpr std::uint32_t cancel_immune = BULLET_TRANSFORM_CANCEL_IMMUNE;

    bullet_spawn_views_.clear();
    if (!horizon)
        return bullet_spawn_views_;
    auto unsupported = [&](const Enemy &enemy, int opcode, unsigned update,
                           UpcomingBulletSpawnFailure failure) {
        UpcomingBulletSpawnView view;
        view.failure = failure;
        view.enemy_index = enemy.enemyIndex;
        view.opcode = opcode;
        view.update = update;
        bullet_spawn_views_.push_back(view);
    };
    for (const auto &enemy : g_EnemyManager.enemies) {
        if (!(enemy.flags1 & ENEMY_FLAG_ACTIVE) || enemy.life <= 0)
            continue;
        const auto &context = enemy.mainEclContextStorage;
        const auto *first = context.currentInstr;
        if (!first || (first->opcode != ECL_OPCODE_SET_DIRECTION_AND_SPEED &&
                       (first->opcode < shoot_first || first->opcode > shoot_last)))
            continue;
        if (first->time < context.time.current)
            continue;
        const auto update = unsigned(first->time - context.time.current) + 1;
        if (update > horizon)
            continue;

        std::array<const EclRawInstruction *, 16> shots{};
        std::size_t shot_count = 0;
        bool sequence_supported = true;
        LocalFloatSnapshot local_floats{};
        std::copy(std::begin(context.floatVariables), std::end(context.floatVariables),
                  local_floats.begin());
        BulletSpawnDescriptor forecast_descriptor = enemy.bulletSpawnDescriptor;
        const auto active_difficulty = static_cast<std::uint32_t>(g_GameManager.difficultyMask) |
                                       enemy.eclDifficultyMaskOverride;
        const auto *instruction = first;
        unsigned records = 0;
        for (; records < 64 && instruction->time == first->time; ++records) {
            if (instruction->nextOffset < 12) {
                sequence_supported = false;
                break;
            }
            const bool enabled =
                (instruction->difficultyMask & active_difficulty) == active_difficulty;
            if (enabled) {
                const int opcode = instruction->opcode;
                if (opcode >= shoot_first && opcode <= shoot_last) {
                    if (shot_count == shots.size()) {
                        sequence_supported = false;
                        break;
                    }
                    shots[shot_count++] = instruction;
                    sequence_supported &= opcode == ECL_OPCODE_SHOOT_FAN ||
                                          opcode == ECL_OPCODE_SHOOT_CIRCLE ||
                                          opcode == ECL_OPCODE_SHOOT_OFFSET_CIRCLE;
                    // Later same-time instructions cannot change a pattern
                    // already dispatched. Preview one source-ordered spawn;
                    // the refreshed cursor owns any following pattern.
                    break;
                } else if (opcode == ECL_OPCODE_FLOAT_ADD || opcode == ECL_OPCODE_FLOAT_SUBTRACT) {
                    sequence_supported = apply_local_float_binary(
                        *instruction, opcode == ECL_OPCODE_FLOAT_SUBTRACT, local_floats);
                    if (!sequence_supported)
                        break;
                } else if (opcode == ECL_OPCODE_SET_BULLET_TRANSFORM) {
                    sequence_supported =
                        apply_bullet_transform(*instruction, local_floats, forecast_descriptor);
                    if (!sequence_supported)
                        break;
                } else if (opcode != 0 && opcode != ECL_OPCODE_SET_DIRECTION_AND_SPEED &&
                           opcode != ECL_OPCODE_NOP_84 && opcode != ECL_OPCODE_NOP_85) {
                    sequence_supported = false;
                    break;
                }
            }
            instruction = reinterpret_cast<const EclRawInstruction *>(
                reinterpret_cast<const std::uint8_t *>(instruction) + instruction->nextOffset);
        }
        if (!shot_count) {
            if (!sequence_supported || (records == 64 && instruction->time == first->time))
                unsupported(enemy, first->opcode, update,
                            UpcomingBulletSpawnFailure::InstructionSequence);
            continue;
        }
        if (!sequence_supported || context.secondaryTime.current != 0 ||
            enemy.pendingEclSubroutineIndex >= 0 || !g_Spellcard.IsActive() ||
            g_GameManager.scriptedUpdateFreeze || enemy.minimumPlayerDistanceSquared > 0.f) {
            unsupported(enemy, shots[0]->opcode, update,
                        sequence_supported ? UpcomingBulletSpawnFailure::DynamicContext
                                           : UpcomingBulletSpawnFailure::InstructionSequence);
            continue;
        }

        float origin_x = 0, origin_y = 0;
        if (!forecast_shot_origin(enemy, context, update, origin_x, origin_y)) {
            unsupported(enemy, shots[0]->opcode, update, UpcomingBulletSpawnFailure::EnemyMotion);
            continue;
        }
        const auto sequence_begin = bullet_spawn_views_.size();
        bool decoded = true;
        auto failure = UpcomingBulletSpawnFailure::Operands;
        for (std::size_t shot_index = 0; shot_index < shot_count && decoded; ++shot_index) {
            const auto &raw = *shots[shot_index];
            if (raw.nextOffset != 12 + int(sizeof(NativeShotArgs)) ||
                (raw.operandFlags & ~std::uint16_t(0xff))) {
                decoded = false;
                break;
            }
            NativeShotArgs args{};
            std::memcpy(&args, raw.operands, sizeof(args));
            // Integer selectors and action-dependent alignment gates are outside
            // this narrow adapter. ID201 uses one local-float angle selector.
            if ((raw.operandFlags & 0xf) || (args.transform_flags & alignment_flags) ||
                args.bullet_type < 0 || args.bullet_type >= 0x20 || args.count1 <= 0 ||
                args.count1 > std::numeric_limits<std::int16_t>::max() || args.count2 <= 0 ||
                args.count2 > std::numeric_limits<std::int16_t>::max() ||
                std::uint64_t(args.count1) * std::uint64_t(args.count2) > 0x600) {
                decoded = false;
                break;
            }
            float speed1 = 0, speed2 = 0, angle = 0, angle_step = 0;
            if (!resolve_local_float(local_floats, args.speed1, raw.operandFlags & 0x10, speed1) ||
                !resolve_local_float(local_floats, args.speed2, raw.operandFlags & 0x20, speed2) ||
                !resolve_local_float(local_floats, args.angle, raw.operandFlags & 0x40, angle) ||
                !resolve_local_float(local_floats, args.angle_step, raw.operandFlags & 0x80,
                                     angle_step)) {
                decoded = false;
                break;
            }
            BulletSpawnDescriptor descriptor = forecast_descriptor;
            descriptor.transformFlags = args.transform_flags;
            const unsigned linear_limit = horizon - update + 1;
            const unsigned linear_updates = initial_bullet_linear_updates(descriptor, linear_limit);
            if (!linear_updates) {
                failure = UpcomingBulletSpawnFailure::TransformProgram;
                decoded = false;
                break;
            }
            const auto &sprites = g_BulletManager.bulletTypeSprites[args.bullet_type];
            if (!sprites.bulletVm.loadedSprite || !std::isfinite(sprites.collisionSize.x) ||
                !std::isfinite(sprites.collisionSize.y) || sprites.collisionSize.x < 0.f ||
                sprites.collisionSize.y < 0.f) {
                failure = UpcomingBulletSpawnFailure::SpriteGeometry;
                decoded = false;
                break;
            }
            const bool suppressed = g_BulletManager.spawnSuppressionFrames >= int(update) &&
                                    !(args.transform_flags & cancel_immune);
            const int aim_mode = raw.opcode - ECL_OPCODE_SHOOT_FAN_AIMED;
            for (int index2 = 0; index2 < args.count2 && decoded; ++index2) {
                const float speed = args.count2 > 1 ? speed1 - (speed1 - speed2) * float(index2) /
                                                                   float(args.count2)
                                                    : speed1;
                for (int index1 = 0; index1 < args.count1; ++index1) {
                    float bullet_angle = 0.f;
                    if (aim_mode == BULLET_AIM_FAN) {
                        if (args.count1 & 1)
                            bullet_angle += float((index1 + 1) / 2) * angle_step;
                        else
                            bullet_angle += float(index1 / 2) * angle_step + angle_step * .5f;
                        if (index1 & 1)
                            bullet_angle *= -1.f;
                        bullet_angle += angle;
                    } else if (aim_mode == BULLET_AIM_CIRCLE) {
                        bullet_angle += float(index1) * (ZUN_PI * 2.f) / float(args.count1);
                        bullet_angle += float(index2) * angle_step + angle;
                    } else {
                        bullet_angle += ZUN_PI / float(args.count1);
                        bullet_angle += float(index1) * (ZUN_PI * 2.f) / float(args.count1);
                        bullet_angle += angle;
                    }
                    Float3 velocity;
                    velocity.FromAngleMagnitude(bullet_angle, speed);
                    UpcomingBulletSpawnView view;
                    view.supported = true;
                    view.suppressed = suppressed;
                    view.enemy_index = enemy.enemyIndex;
                    view.opcode = raw.opcode;
                    view.update = update;
                    view.linear_updates = linear_updates;
                    view.vx = velocity.x;
                    view.vy = velocity.y;
                    view.x = origin_x + velocity.x;
                    view.y = origin_y + velocity.y;
                    view.full_width = sprites.collisionSize.x;
                    view.full_height = sprites.collisionSize.y;
                    if (!std::isfinite(view.x) || !std::isfinite(view.y) ||
                        !g_GameManager.IsWithinPlayfield(view.x, view.y,
                                                         sprites.bulletVm.loadedSprite->widthPx,
                                                         sprites.bulletVm.loadedSprite->heightPx)) {
                        failure = UpcomingBulletSpawnFailure::OffscreenCull;
                        decoded = false;
                        break;
                    }
                    // Stop before the first update where native offscreen
                    // culling removes this new bullet.
                    for (unsigned offset = 1; offset < view.linear_updates; ++offset)
                        if (!g_GameManager.IsWithinPlayfield(
                                view.x + float(offset) * view.vx, view.y + float(offset) * view.vy,
                                sprites.bulletVm.loadedSprite->widthPx,
                                sprites.bulletVm.loadedSprite->heightPx)) {
                            view.linear_updates = offset;
                            break;
                        }
                    bullet_spawn_views_.push_back(view);
                }
            }
        }
        if (!decoded) {
            bullet_spawn_views_.resize(sequence_begin);
            unsupported(enemy, shots[0]->opcode, update, failure);
        }
    }
    const auto unsuppressed = std::count_if(
        bullet_spawn_views_.begin(), bullet_spawn_views_.end(),
        [](const UpcomingBulletSpawnView &view) { return view.supported && !view.suppressed; });
    if (unsuppressed > std::max(0, 0x600 - g_BulletManager.activeBulletCount))
        for (auto &view : bullet_spawn_views_)
            if (view.supported && !view.suppressed) {
                view.supported = false;
                view.failure = UpcomingBulletSpawnFailure::PoolCapacity;
            }
    return bullet_spawn_views_;
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
