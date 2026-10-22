#include "th08/online_native.hpp"
#include "BulletManager.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "Supervisor.hpp"
#include "modern/headless/session.hpp"
#include "th08/direct_laser_policy.hpp"
#include "th08/ecl_turn_hint.hpp"

namespace th08::online {
namespace {
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
                    bool include_turn_hints) {
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
    for (const auto &b : session.bullets()) {
        // The final pool entry is a source sentinel, never a gameplay bullet.
        if (b.slot == 0x600 && b.state == BULLET_STATE_SENTINEL)
            continue;
        const auto &native = g_BulletManager.bullets[b.slot];
        o.bullets.push_back({b.x, b.y, b.vx, b.vy, b.state, b.slot,
                             int(native.sprites.bulletVm.activeSpriteIndex), b.full_width,
                             b.full_height, b.active_transforms, b.vector_acceleration_x,
                             b.vector_acceleration_y, b.vector_acceleration_timer,
                             b.vector_acceleration_duration, native.collisionDisabled != 0});
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
    o.turn_hint_records = 0;
    o.unsupported_turn_hints = 0;
    if (queued_laser_horizon) {
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
