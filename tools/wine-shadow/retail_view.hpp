#pragma once
#include "BulletManager.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "Spellcard.hpp"
#include "Supervisor.hpp"
#include "th08/shadow_protocol.hpp"
#include <cmath>
#include <cstring>
#include <windows.h>
namespace wine_shadow {
namespace s = th08::shadow;
using namespace th08;
// Addresses and 32-bit layout assertions belong only to the pinned 1.00d ABI.
template <class T> T &at(unsigned address) {
    return *reinterpret_cast<T *>(address);
}
GameManager &game() {
    return at<GameManager>(0x160f508);
}
Player &player() {
    return at<Player>(0x17d5ef8);
}
BulletManager &bullets() {
    return at<BulletManager>(0xf54e90);
}
EnemyManager &enemies() {
    return at<EnemyManager>(0x577f20);
}
Supervisor &supervisor() {
    return at<Supervisor>(0x17ce758);
}
std::uint32_t bits(float x) {
    std::uint32_t n;
    memcpy(&n, &x, 4);
    return n;
}
void hash(std::uint64_t &h, std::uint64_t v) {
    h = (h ^ v) * 1099511628211ULL;
}
void pair(s::Projection &p, s::Field a, s::Field b, std::uint64_t v) {
    p[a] = std::uint32_t(v);
    p[b] = std::uint32_t(v >> 32);
}
// Match Session::bullets' declared relative-turn projection, including its
// validity gates. Unsupported views are zero on both sides; this hash is not a
// full bullet serialization. Native execution still owns every transformation.
struct Relative {
    float angle = 0, speed = 0;
    int timer = 0;
};
inline Relative relative_view(const Bullet &b) {
    if (b.state != BULLET_STATE_FIRED ||
        b.activeTransformFlags != BULLET_TRANSFORM_CHANGE_DIRECTION_RELATIVE ||
        b.transformIndex < 0 || supervisor().framerateMultiplier != 1.f ||
        game().scriptedUpdateFreeze)
        return {};
    for (int i = b.transformIndex; i >= 0 && i < 18; ++i) {
        auto &r = b.transforms[i];
        if (!r.kind || !r.allowWhileActive)
            break;
        if (b.transformFlags & r.kind)
            return {};
    }
    const auto &r = b.exStates[BULLET_TRANSFORM_STATE_DIRECTION_CHANGE];
    if (r.timer.current < 0 || r.timer.subFrame != 0.f || r.directionChangeIntervalFrames < 0 ||
        !std::isfinite(b.angle) || !std::isfinite(b.speed) ||
        !std::isfinite(r.directionChangeAngle) || !std::isfinite(r.directionChangeSpeed))
        return {};
    return {b.angle, b.speed, r.timer.current};
}
inline s::Projection capture(const s::Packet &packet, std::uint64_t calculations) {
    s::Projection p;
    auto &gm = game();
    auto &pl = player();
    auto &g = *gm.globals;
    pair(p, s::Field::FrameLow, s::Field::FrameHigh, packet.sequence);
    pair(p, s::Field::CalculationLow, s::Field::CalculationHigh, calculations);
    p[s::Field::Stage] = gm.currentStage;
    auto &sp = at<Spellcard>(0x4ea670);
    p[s::Field::Spell] = sp.spellCardNumber;
    p[s::Field::SpellActive] = reinterpret_cast<int(__thiscall *)(Spellcard *)>(0x4178a0)(&sp) != 0;
    p[s::Field::X] = bits(pl.position.x);
    p[s::Field::Y] = bits(pl.position.y);
    p[s::Field::Deaths] = bits(g.deaths);
    p[s::Field::Lives] = bits(g.livesRemaining);
    p[s::Field::PlayerState] = pl.playerState;
    p[s::Field::Bullets] = bullets().activeBulletCount;
    p[s::Field::Enemies] = enemies().activeEnemyCount;
    p[s::Field::RngSeed] = at<unsigned short>(0x164d520);
    p[s::Field::RngDraws] = at<unsigned>(0x164d524);
    p[s::Field::Score] = g.score;
    p[s::Field::Graze] = g.graze;
    p[s::Field::Gauge] = g.youkaiGauge;
    p[s::Field::HurtHalfX] = bits(pl.hurtboxHalfSize.x);
    p[s::Field::HurtHalfY] = bits(pl.hurtboxHalfSize.y);
    p[s::Field::LatchedInput] = at<unsigned short>(0x164d52c);
    p[s::Field::SampledInput] = at<unsigned short>(0x164d528);
    p[s::Field::Youkai] = reinterpret_cast<int(__thiscall *)(Player *)>(0x40bc40)(&pl) != 0;
    p[s::Field::RetryMenu] = gm.showRetryMenu != 0;
    p[s::Field::StageComplete] = gm.flags.stageTransitionState >= 2;
    auto &gui = at<Gui>(0x160f428);
    p[s::Field::RouteSelection] = gm.currentStage == STAGE5 && gui.impl &&
                                  gui.impl->message.currentInstr &&
                                  gui.impl->message.currentInstr->opcode == GUI_MSG_SHOW_SELECTION;
    p[s::Field::FinalRoute] = gui.impl ? gui.impl->message.selectedOption : 0;
    std::uint64_t actor = 1469598103934665603ULL, bs = actor, ls = actor;
    for (const auto &e : enemies().enemies)
        if (e.flags1 & ENEMY_FLAG_ACTIVE) {
            hash(actor, e.enemyIndex);
            hash(actor, e.flags1);
            hash(actor, e.life);
            hash(actor, e.bossTimer.current);
            hash(actor, e.mainEclContextStorage.subId);
            hash(actor, e.mainEclContextStorage.time.current);
        }
    int slot = 0;
    for (const auto &b : bullets().bullets) {
        if (b.state) {
            hash(bs, slot);
            hash(bs, b.state);
            hash(bs, b.activeTransformFlags);
            auto &a = b.exStates[BULLET_TRANSFORM_STATE_VECTOR_ACCELERATION];
            auto relative = relative_view(b);
            for (float v : {b.position.x, b.position.y, b.velocity.x, b.velocity.y,
                            b.sprites.collisionSize.x, b.sprites.collisionSize.y, a.vector.x,
                            a.vector.y, relative.angle, relative.speed})
                hash(bs, bits(v));
            hash(bs, a.timer.current);
            hash(bs, a.durationFrames);
            hash(bs, relative.timer);
        }
        ++slot;
    }
    slot = 0;
    for (const auto &l : bullets().lasers) {
        if (l.inUse) {
            hash(ls, slot);
            hash(ls, l.state);
            hash(ls, l.flags);
            hash(ls, l.timer.current);
            for (float v : {l.position.x, l.position.y, l.angle, l.startOffset, l.endOffset,
                            l.startLength, l.width, l.speed})
                hash(ls, bits(v));
            hash(ls, l.startTime);
            hash(ls, l.hitboxStartTime);
            hash(ls, l.duration);
            hash(ls, l.despawnDuration);
            hash(ls, l.hitboxEndDelay);
        }
        ++slot;
    }
    pair(p, s::Field::ActorLow, s::Field::ActorHigh, actor);
    pair(p, s::Field::BulletLow, s::Field::BulletHigh, bs);
    pair(p, s::Field::LaserLow, s::Field::LaserHigh, ls);
    p[s::Field::BombsUsed] = bits(g.bombsUsed);
    p[s::Field::Power] = bits(g.playerPower);
    p[s::Field::Rank] = gm.rank;
    p[s::Field::PointItems] = g.pointItemsCollected;
    p[s::Field::Orbs] = g.currentTimeOrbs;
    p[s::Field::Clock] = g.clockTime;
    p[s::Field::PointValue] = g.pointItemValue;
    return p;
}
} // namespace wine_shadow
