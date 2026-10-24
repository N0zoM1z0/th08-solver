#include "th08/shadow_native.hpp"
#include "BulletManager.hpp"
#include "GameManager.hpp"
#include "modern/headless/session.hpp"
#include "th08/plan_route.hpp"
#include <cstring>

namespace th08::shadow {
bool stage_terminal(bool story, const headless::State &state) {
    return story ? plan::story_terminal(state.stage, state.stage_complete, 8)
                 : state.stage_complete;
}
namespace {
std::uint32_t bits(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
void pair(Projection &p, Field low, Field high, std::uint64_t value) {
    p[low] = std::uint32_t(value);
    p[high] = std::uint32_t(value >> 32);
}
void hash(std::uint64_t &digest, std::uint64_t value) {
    digest = (digest ^ value) * 1099511628211ULL;
}
} // namespace
Projection capture(headless::Session &session) {
    const auto s = session.state();
    Projection p;
    pair(p, Field::FrameLow, Field::FrameHigh, s.frame);
    pair(p, Field::CalculationLow, Field::CalculationHigh, s.calculation_updates);
    p[Field::Stage] = s.stage;
    p[Field::Spell] = s.spell;
    p[Field::SpellActive] = s.spell_active;
    p[Field::X] = bits(s.x);
    p[Field::Y] = bits(s.y);
    p[Field::Deaths] = bits(s.deaths);
    p[Field::Lives] = bits(s.lives);
    p[Field::PlayerState] = s.player_state;
    p[Field::Bullets] = s.bullets;
    p[Field::Enemies] = s.enemies;
    p[Field::RngSeed] = s.rng_seed;
    p[Field::RngDraws] = s.rng_draws;
    p[Field::Score] = s.score;
    p[Field::Graze] = s.graze;
    p[Field::Gauge] = s.gauge;
    p[Field::HurtHalfX] = bits(s.hurt_half_x);
    p[Field::HurtHalfY] = bits(s.hurt_half_y);
    p[Field::LatchedInput] = s.latched_input;
    p[Field::SampledInput] = s.sampled_input;
    p[Field::Youkai] = s.youkai;
    p[Field::RetryMenu] = s.retry_menu;
    p[Field::StageComplete] = s.stage_complete;
    p[Field::RouteSelection] = s.final_route_selection;
    p[Field::FinalRoute] = s.selected_final_route;
    pair(p, Field::ActorLow, Field::ActorHigh, session.actor_digest());
    std::uint64_t bullets = 1469598103934665603ULL, lasers = bullets;
    for (const auto &b : session.bullets()) {
        hash(bullets, b.slot);
        hash(bullets, b.state);
        hash(bullets, b.active_transforms);
        for (float value :
             {b.x, b.y, b.vx, b.vy, b.full_width, b.full_height, b.vector_acceleration_x,
              b.vector_acceleration_y, b.relative_direction.angle, b.relative_direction.speed})
            hash(bullets, bits(value));
        hash(bullets, b.vector_acceleration_timer);
        hash(bullets, b.vector_acceleration_duration);
        hash(bullets, b.relative_direction.timer);
    }
    for (const auto &l : session.lasers()) {
        hash(lasers, l.slot);
        hash(lasers, l.state);
        hash(lasers, l.flags);
        hash(lasers, l.timer);
        for (float value : {l.origin_x, l.origin_y, l.angle, l.start_offset, l.end_offset,
                            l.start_length, l.width, l.speed})
            hash(lasers, bits(value));
        hash(lasers, l.start_time);
        hash(lasers, l.hitbox_start_time);
        hash(lasers, l.duration);
        hash(lasers, l.despawn_duration);
        hash(lasers, l.hitbox_end_delay);
    }
    pair(p, Field::BulletLow, Field::BulletHigh, bullets);
    pair(p, Field::LaserLow, Field::LaserHigh, lasers);
    if (!g_GameManager.globals)
        throw std::runtime_error("missing_shadow_globals");
    const auto &g = *g_GameManager.globals;
    p[Field::BombsUsed] = bits(g.bombsUsed);
    p[Field::Power] = bits(g.playerPower);
    p[Field::Rank] = g_GameManager.rank;
    p[Field::PointItems] = g.pointItemsCollected;
    p[Field::Orbs] = g.currentTimeOrbs;
    p[Field::Clock] = g.clockTime;
    p[Field::PointValue] = g.pointItemValue;
    return p;
}
NativeDiagnostics diagnostics(headless::Session &session) {
    NativeDiagnostics result;
    result.bullets.reserve(std::size_t(session.state().bullets));
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    __asm__ volatile("fnstcw %0" : "=m"(result.x87_control));
#endif
    int slot = 0;
    for (const auto &bullet : g_BulletManager.bullets) {
        if (bullet.state) {
            const auto &a = bullet.exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION];
            result.bullets.push_back(
                {slot, bullet.angle, bullet.speed, a.accelerationAngle, a.accelerationMagnitude});
        }
        ++slot;
    }
    return result;
}
} // namespace th08::shadow
