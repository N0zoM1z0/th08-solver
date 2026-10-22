#include "th08/online_native.hpp"
#include "BulletManager.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "Supervisor.hpp"
#include "modern/headless/session.hpp"

namespace th08::online {
void capture_memory(headless::Session &session, std::uint64_t update, Observation &o) {
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
}
} // namespace th08::online
