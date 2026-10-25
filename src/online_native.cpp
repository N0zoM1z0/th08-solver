#include "th08/online_native.hpp"
#include "BulletManager.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "Spellcard.hpp"
#include "Supervisor.hpp"
#include "modern/headless/session.hpp"
#include "th08/animation.hpp"
#include "th08/direct_laser_policy.hpp"
#include "th08/ecl_turn_hint.hpp"
#include "th08/online_child_birth.hpp"
#include "th08/online_lifetime.hpp"
#include <cmath>
#include <limits>

namespace th08::online {
void capture_policy_memory(headless::Session &session, std::uint64_t update, Observation &o,
                           const Policy &policy) {
    capture_memory(session, update, o, true);
    auto &score = o.scoring;
    score.available = policy.needs_scoring_memory();
    score.pickups.clear();
    if (!score.available)
        return;
    score.has_boss = false;
    score.boss_x = 192;
    for (const auto *boss : g_EnemyManager.bosses)
        if (boss && (boss->flags1 & ENEMY_FLAG_ACTIVE)) {
            score.has_boss = true;
            score.boss_x = boss->worldPosition.x;
            break;
        }
    score.power = g_GameManager.globals->playerPower;
    score.can_switch_forms = !g_GameManager.IsSoloHuman() && !g_GameManager.IsSoloYoukai();
    score.survival_spell = g_Spellcard.IsActive() && g_Spellcard.activeEnemy &&
                           (g_Spellcard.activeEnemy->flags1 & ENEMY_FLAG_TIMEOUT_SPELL);
    score.collection_line = g_Player.primaryShtFile->pointItemValueLine;
    // Exact eligibility from ItemManager::OnUpdate; it is not a collection
    // promise, since the native world still owns movement and item lifecycle.
    score.auto_collect_eligible = score.power >= 128 ||
                                  g_Player.focusMode != PLAYER_FOCUS_MODE_UNFOCUSED ||
                                  g_GameManager.shotType == 1 || g_GameManager.shotType == 6;
    for (const auto *item = g_ItemManager.itemListHead.next; item; item = item->next) {
        if (!item->isInUse)
            continue;
        const bool point = item->itemType == ITEM_POINT;
        const bool power = item->itemType == ITEM_POWER_SMALL || item->itemType == ITEM_POWER_BIG ||
                           item->itemType == ITEM_POWER_FULL;
        if (point || (power && score.power < 128))
            score.pickups.push_back({item->currentPosition.x, item->currentPosition.y,
                                     point ? PickupKind::Point : PickupKind::Power});
    }
}
namespace {
static_assert(detail::kLifetimeWaitKind == BULLET_TRANSFORM_WAIT &&
              detail::kLifetimeDespawnKind == BULLET_TRANSFORM_DESPAWN);
// A returned parent cutoff reuses the successful child-constructor validation.
// Fade changes the parent state, but its current Fired callback still collides.
unsigned capture_child_birth(const th08::Bullet &parent, int slot, unsigned horizon,
                             Observation &observation) {
    if (parent.state != BULLET_STATE_FIRED || parent.transformIndex < 0 ||
        parent.transformIndex >= 18)
        return 0;
    std::array<detail::ChildCursorRecord, 18> records;
    for (std::size_t i = 0; i < records.size(); ++i)
        records[i] = {parent.transforms[i].kind, parent.transforms[i].allowWhileActive != 0};
    const auto &wait = parent.exStates[BULLET_TRANSFORM_STATE_WAIT].timer;
    const auto cursor = detail::child_birth_cursor(
        records, parent.transformIndex, parent.transformFlags, parent.activeTransformFlags,
        wait.current, wait.subFrame, horizon);
    if (cursor.unsupported)
        ++observation.unsupported_child_births;
    if (cursor.index < 0)
        return 0;
    const int index = cursor.index;
    const unsigned update = cursor.update;
    const auto &record = parent.transforms[index];
    const auto unsupported = [&] { ++observation.unsupported_child_births; };
    if (g_Supervisor.framerateMultiplier != 1.f || g_GameManager.scriptedUpdateFreeze ||
        g_GameManager.flags.deathbombFreezeActive || g_Supervisor.flags.forceExtraTimerStep) {
        unsupported();
        return 0;
    }
    const auto packed = std::uint32_t(record.payload.childPrimary.packedPattern);
    const int type = int((packed >> 16) & 255), mode = int((packed >> 24) & 127);
    const int start = int(packed & 255);
    const auto &secondary = parent.transforms[index + 1].payload.childSecondary;
    // Match the source's signed-short count conversion without evaluating RNG.
    const auto count1 = std::int16_t(record.payload.childPrimary.count1);
    const auto count2 = std::int16_t(secondary.count2);
    if (count1 <= 0 || count2 <= 0)
        return 0;
    if (type >= int(std::size(g_BulletManager.bulletTypeSprites)) || mode > BULLET_AIM_RANDOM ||
        start >= 18 ||
        (secondary.transformFlags & (BULLET_TRANSFORM_SPAWN_FAST | BULLET_TRANSFORM_SPAWN_NORMAL |
                                     BULLET_TRANSFORM_SPAWN_SLOW))) {
        unsupported();
        return 0;
    }
    const auto &first = parent.transforms[start];
    // A currently copied WAIT keeps child velocity unchanged throughout these
    // first two pool-order possibilities. Later transforms are outside coverage.
    if (first.kind != BULLET_TRANSFORM_WAIT || first.payload.timed.frames < 1 ||
        !(secondary.transformFlags & BULLET_TRANSFORM_WAIT)) {
        unsupported();
        return 0;
    }
    for (int next = start + 1; next < 18; ++next) {
        const auto &pending = parent.transforms[next];
        if (!pending.kind || !pending.allowWhileActive)
            break;
        if (pending.kind & secondary.transformFlags) {
            unsupported();
            return 0;
        }
    }
    const auto &size = g_BulletManager.bulletTypeSprites[type].collisionSize;
    const float speed1 = record.payload.childPrimary.speed1;
    const float speed2 = record.payload.childPrimary.speed2;
    if (!std::isfinite(speed1) || !std::isfinite(speed2) || !std::isfinite(secondary.angle) ||
        !std::isfinite(secondary.angleStep) || !std::isfinite(size.x) || !std::isfinite(size.y) ||
        size.x < 0 || size.y < 0) {
        unsupported();
        return 0;
    }
    // Native interpolation rounds the subtraction/product/add separately. The
    // triangle bound encloses all interpolation fractions, including RNG modes.
    const float difference = speed1 - speed2;
    const float bound = std::nextafter(std::abs(difference) + std::abs(speed2),
                                       std::numeric_limits<float>::infinity());
    if (!std::isfinite(bound)) {
        unsupported();
        return 0;
    }
    float x = parent.position.x, y = parent.position.y;
    for (unsigned step = 1; step < update; ++step)
        x += parent.velocity.x, y += parent.velocity.y;
    observation.child_births.push_back({update, x, y, bound, size.x, size.y, slot});
    return (packed & 0x80000000u) && (!parent.activeTransformFlags ||
                                      parent.activeTransformFlags == BULLET_TRANSFORM_WAIT)
               ? update
               : 0;
}
detail::CollisionLifetime capture_collision_lifetime(const th08::Bullet &bullet, unsigned horizon) {
    if (bullet.state != BULLET_STATE_FIRED)
        return {};
    std::array<detail::LifetimeRecord, 18> records;
    for (std::size_t i = 0; i < records.size(); ++i)
        records[i] = {bullet.transforms[i].kind, bullet.transforms[i].allowWhileActive != 0};
    const auto &wait = bullet.exStates[BULLET_TRANSFORM_STATE_WAIT].timer;
    const bool clock =
        g_Supervisor.framerateMultiplier == 1.f && !g_GameManager.scriptedUpdateFreeze &&
        !g_GameManager.flags.deathbombFreezeActive && !g_Supervisor.flags.forceExtraTimerStep;
    return detail::scheduled_collision_lifetime(records, bullet.transformIndex,
                                                bullet.transformFlags, bullet.activeTransformFlags,
                                                wait.current, wait.subFrame, horizon, clock);
}
unsigned native_spawn_clock(const th08::Bullet &bullet) {
    if (g_Supervisor.framerateMultiplier != 1 || g_GameManager.scriptedUpdateFreeze ||
        g_GameManager.flags.deathbombFreezeActive)
        return 0;
    if (bullet.state < BULLET_STATE_SPAWNING_FAST || bullet.state > BULLET_STATE_SPAWNING_SLOW)
        return 0;
    const auto &vm = bullet.state == BULLET_STATE_SPAWNING_FAST     ? bullet.sprites.spawnFastVm
                     : bullet.state == BULLET_STATE_SPAWNING_NORMAL ? bullet.sprites.spawnNormalVm
                                                                    : bullet.sprites.spawnSlowVm;
    animation::TerminalClock clock;
    clock.context_supported = true;
    clock.cursor =
        vm.currentInstruction ? animation::Cursor::present : animation::Cursor::null_instruction;
    clock.blocked = vm.flag19 || vm.stopped || vm.pendingInterrupt;
    clock.current_time = vm.currentTimeInScript.current;
    clock.subframe = vm.currentTimeInScript.subFrame;
    if (vm.currentInstruction && !clock.blocked && clock.current_time >= 0 && clock.subframe == 0) {
        const auto &instruction = *vm.currentInstruction;
        clock.opcode = instruction.opcode;
        clock.scheduled_time = instruction.time;
        clock.size = instruction.instructionSize;
        clock.mask = instruction.varMask;
    }
    return animation::remaining_terminal_calls(clock);
}
TurnHint native_turn_hint(const headless::EclContextView &view) {
    if (view.enemy_index < 0 || view.enemy_index >= int(std::size(g_EnemyManager.enemies)))
        return {};
    const auto &enemy = g_EnemyManager.enemies[view.enemy_index];
    if (enemy.enemyIndex != view.enemy_index)
        return {};
    const auto *context = &enemy.mainEclContextStorage;
    if (view.child_slot) {
        if (view.child_slot < 1 || view.child_slot > 4 ||
            !enemy.childEclBlocks[view.child_slot - 1])
            return {};
        context = &enemy.childEclBlocks[view.child_slot - 1]->eclContext;
    }
    // Adjacent sub-table entries bound the immutable byte span. The final sub
    // has no following bound in this native interface and supplies no cue.
    if (!g_EclManager.eclFile || !g_EclManager.subTable || context->subId < 0 ||
        context->subId + 1 >= g_EclManager.eclFile->subCount)
        return {};
    const auto begin = std::uintptr_t(g_EclManager.subTable[context->subId]);
    const auto end = std::uintptr_t(g_EclManager.subTable[context->subId + 1]);
    const auto cursor = reinterpret_cast<std::uintptr_t>(context->currentInstr);
    if (cursor < begin || cursor >= end || end - cursor > 1048576)
        return {};
    const auto difficulty =
        std::uint32_t(g_GameManager.difficultyMask) | enemy.eclDifficultyMaskOverride;
    return current_batch_turn_hint(reinterpret_cast<const std::uint8_t *>(cursor),
                                   std::size_t(end - cursor), difficulty);
}
} // namespace
void capture_memory(headless::Session &session, std::uint64_t update, Observation &o,
                    bool include_actors, unsigned queued_horizon, unsigned queued_laser_horizon,
                    bool include_turn_hints, bool include_motion_alternatives,
                    unsigned child_horizon, unsigned lifetime_horizon) {
    const auto state = session.state();
    o.update = update;
    o.stage = state.stage;
    o.spell = state.spell;
    o.spell_active = state.spell_active;
    o.youkai = state.youkai;
    o.x = state.x;
    o.y = state.y;
    o.latched_input = state.latched_input;
    auto &m = o.movement;
    m.axis[0] = g_Player.primaryShtFile->normalAxisSpeed;
    m.diag[0] = g_Player.primaryShtFile->normalDiagonalSpeed;
    m.axis[1] = g_Player.secondaryShtFile->focusedAxisSpeed;
    m.diag[1] = g_Player.secondaryShtFile->focusedDiagonalSpeed;
    m.hmul = g_Player.horizontalSpeedMultiplier;
    m.vmul = g_Player.verticalSpeedMultiplier;
    m.frm = g_Supervisor.framerateMultiplier;
    m.minx = g_GameManager.playerMovementTopLeftPos.x;
    m.miny = g_GameManager.playerMovementTopLeftPos.y;
    m.maxx = m.minx + g_GameManager.playerMovementAreaSize.x;
    m.maxy = m.miny + g_GameManager.playerMovementAreaSize.y;
    m.half = {g_Player.hurtboxHalfSize.x, g_Player.hurtboxHalfSize.y, 0};
    o.bullets.clear();
    o.graze_half = {g_Player.grazeHalfSize.x, g_Player.grazeHalfSize.y, g_Player.grazeHalfSize.z};
    o.child_births.clear();
    o.unsupported_child_births = 0;
    o.unsupported_lifetimes = 0;
    for (const auto &b : session.bullets()) {
        // The final pool entry is a source sentinel, never a gameplay bullet.
        if (b.slot == 0x600 && b.state == BULLET_STATE_SENTINEL)
            continue;
        const auto &native = g_BulletManager.bullets[b.slot];
        const unsigned parent_fade =
            child_horizon ? capture_child_birth(native, b.slot, child_horizon, o) : 0;
        o.bullets.push_back({b.x, b.y, b.vx, b.vy, b.state, b.slot,
                             int(native.sprites.bulletVm.activeSpriteIndex), b.full_width,
                             b.full_height, b.active_transforms, b.vector_acceleration_x,
                             b.vector_acceleration_y, b.vector_acceleration_timer,
                             b.vector_acceleration_duration, native.collisionDisabled != 0});
        o.bullets.back().activation_update = native_spawn_clock(native);
        o.bullets.back().grazed = native.isGrazed != 0;
        o.bullets.back().graze_ready_in = unsigned(std::max(0, 16 - native.activeTimer.current));
        if (lifetime_horizon) {
            const auto lifetime = capture_collision_lifetime(native, lifetime_horizon);
            auto &copied = o.bullets.back();
            copied.collision_until = lifetime.collision_until;
            o.unsupported_lifetimes += lifetime.unsupported;
            if (parent_fade && parent_fade <= lifetime_horizon)
                copied.collision_until = parent_fade;
        }
        if (include_motion_alternatives && native.collisionDisabled &&
            native.state == BULLET_STATE_FIRED) {
            // Source mode-restoration callbacks can replace the displayed ghost
            // velocity with this retained canonical flight. Read it without
            // decoding a spell schedule or mutating the native bullet.
            Float3 alternative;
            alternative.FromAngleMagnitude(native.angle,
                                           g_Supervisor.framerateMultiplier * native.speed);
            auto &copied = o.bullets.back();
            copied.alternate_motion = true;
            copied.alternate_vx = alternative.x;
            copied.alternate_vy = alternative.y;
        }
        if (b.relative_direction.updates) {
            const auto &v = b.relative_direction;
            const auto &turn = native.exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE];
            auto &r = o.bullets.back().relative_turn;
            r.updates = v.updates;
            r.angle = v.angle;
            r.speed = v.speed;
            r.state = {bullet::TurnMode::relative,
                       true,
                       v.interval,
                       turn.directionChangeRepeatCount,
                       turn.directionChangesCompleted,
                       v.timer,
                       v.turn_angle,
                       v.turn_speed,
                       0};
        }
    }
    o.lasers.clear();
    for (const auto &v : session.lasers()) {
        LaserState l;
        l.endOffset = v.end_offset;
        l.startOffset = v.start_offset;
        l.startLength = v.start_length;
        l.width = v.width;
        l.position = {v.origin_x, v.origin_y, 0};
        l.angle = v.angle;
        l.speed = v.speed;
        l.timer = v.timer;
        l.state = v.state;
        l.startTime = v.start_time;
        l.hitboxStartTime = v.hitbox_start_time;
        l.duration = v.duration;
        l.despawnDuration = v.despawn_duration;
        l.hitboxEndDelay = v.hitbox_end_delay;
        l.flags = v.flags;
        l.origin_delta_x = v.origin_delta_x;
        l.origin_delta_y = v.origin_delta_y;
        l.angle_delta = v.angle_delta;
        l.motion_observed = v.motion_observed;
        o.lasers.push_back(l);
    }
    o.laser_hitboxes.clear();
    for (const auto &l : session.laser_hitboxes())
        o.laser_hitboxes.push_back({l.center_x, l.center_y, l.full_width, l.full_height, l.origin_x,
                                    l.origin_y, l.angle, l.pooled_slot});
    o.lethal_regions.clear();
    for (const auto &r : session.lethal_regions())
        o.lethal_regions.push_back({r.bounds.left, r.bounds.top, r.bounds.right, r.bounds.bottom});
    o.actors.clear();
    if (include_actors) {
        int slot = 0;
        for (const auto &enemy : g_EnemyManager.enemies) {
            if ((enemy.flags1 & ENEMY_FLAG_ACTIVE) && enemy.life > 0) {
                // DispatchShotInstruction uses the cached world position plus
                // shootOffset. Motion is observed, not a certified future origin.
                o.actors.push_back({slot, enemy.worldPosition.x + enemy.shootOffset.x,
                                    enemy.worldPosition.y + enemy.shootOffset.y,
                                    enemy.lastFrameDisplacement.x, enemy.lastFrameDisplacement.y});
            }
            ++slot;
        }
    }
    o.queued_shots.clear();
    o.unsupported_queued_shots = 0;
    if (queued_horizon)
        for (const auto &shot : session.upcoming_bullet_spawns(queued_horizon)) {
            if (!shot.supported) {
                ++o.unsupported_queued_shots;
            } else if (!shot.suppressed) {
                o.queued_shots.push_back({shot.update, shot.linear_updates, shot.x, shot.y, shot.vx,
                                          shot.vy, shot.full_width, shot.full_height});
            }
        }
    o.queued_lasers.clear();
    o.unsupported_queued_lasers = 0;
    o.queued_pooled_lasers.clear();
    o.unsupported_queued_pooled_lasers = 0;
    o.turn_hint_records = 0;
    o.unsupported_turn_hints = 0;
    if (queued_laser_horizon) {
        for (const auto &v : session.imminent_laser_spawns()) {
            if (!v.supported) {
                ++o.unsupported_queued_pooled_lasers;
                continue;
            }
            if (v.suppressed)
                continue;
            LaserState l;
            l.endOffset = v.end_offset;
            l.startOffset = v.start_offset;
            l.startLength = v.start_length;
            l.width = v.width;
            l.position = {v.origin_x, v.origin_y, 0};
            l.angle = v.angle;
            l.speed = v.speed;
            l.startTime = v.start_time;
            l.hitboxStartTime = v.hitbox_start_time;
            l.duration = v.duration;
            l.despawnDuration = v.despawn_duration;
            l.hitboxEndDelay = v.hitbox_end_delay;
            l.flags = v.flags;
            l.state = v.start_time == 0 ? 1 : 0;
            o.queued_pooled_lasers.push_back(l);
        }
        std::vector<policy::DirectLaserWarning> warnings;
        policy::DirectLaserStats stats;
        const auto append = [&](const TurnHint &hint) {
            for (const auto &warning : warnings) {
                o.queued_lasers.push_back(
                    {warning.first_update,
                     warning.last_update,
                     {warning.center_x, warning.center_y, warning.full_width, warning.full_height,
                      warning.origin_x, warning.origin_y, warning.angle, -1},
                     hint.supported ? hint.radians : 0.f});
                if (include_turn_hints) {
                    o.turn_hint_records += hint.supported && hint.radians > 0;
                    o.unsupported_turn_hints += !hint.supported;
                }
            }
        };
        if (include_turn_hints) {
            for (const auto &view : session.ecl_contexts()) {
                const std::array<headless::EclContextView, 1> context{view};
                policy::collect_direct_laser_warnings(context, queued_laser_horizon, warnings,
                                                      stats);
                append(warnings.empty() ? TurnHint{} : native_turn_hint(view));
            }
        } else {
            policy::collect_direct_laser_warnings(session.ecl_contexts(), queued_laser_horizon,
                                                  warnings, stats);
            append({});
        }
        o.unsupported_queued_lasers = stats.variable_selectors + stats.dynamic_geometry;
    }
}
} // namespace th08::online
