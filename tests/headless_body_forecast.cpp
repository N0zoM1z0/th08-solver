#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "Supervisor.hpp"
#include "modern/headless/body_math.hpp"
#include "modern/headless/runtime.hpp"
#include "modern/headless/session.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
namespace th08 {
extern EnemyManager g_EnemyManager;
extern Supervisor g_Supervisor;
} // namespace th08
namespace {
class RunDirectory {
  public:
    RunDirectory() : previous(std::filesystem::current_path()) {
        char pattern[] = "/tmp/th08-body-bounds-XXXXXX";
        const char *created = mkdtemp(pattern);
        if (!created)
            throw std::runtime_error("cannot create body test directory");
        path = created;
        std::filesystem::current_path(path);
    }
    ~RunDirectory() {
        std::error_code error;
        std::filesystem::current_path(previous, error);
        std::filesystem::remove_all(path, error);
    }

  private:
    std::filesystem::path previous, path;
};
void interval_cases() {
    using namespace th08;
    using namespace th08::headless::body_detail;
    Float3 size(5, 5, 0);
    const float native_half = (size / 1.5f).x / 2.f;
    const auto touching = body_bounds({native_half, native_half}, 5);
    if (touching.lo > 0)
        throw std::runtime_error("reciprocal-multiply endpoint missed exact touching");
    auto motion = std::make_unique<Enemy>();
    for (int duration : {1, 2, 12, 60})
        for (unsigned easing = 0; easing < 7; ++easing)
            for (float angle : {-2.f, 0.f, .71f}) {
                motion->flags1 =
                    (ENEMY_MOVEMENT_MODE_INTERPOLATED << ENEMY_FLAG_MOVEMENT_MODE_SHIFT) |
                    (easing << ENEMY_FLAG_MOVEMENT_EASING_SHIFT) | ENEMY_FLAG_CLAMP_POSITION;
                motion->position = motion->previousPosition =
                    motion->movementInterpolationOrigin = {192, 128, 0};
                motion->velocity = {0, 0, 0};
                motion->movementInterpolationDelta = {cosf(angle) * float(duration),
                                                      sinf(angle) * float(duration), 0};
                motion->movementDuration = duration;
                motion->movementTimer = duration;
                motion->movementBounds.lower = {80, 64};
                motion->movementBounds.upper = {304, 128};
                Interval x{192, 192}, y{128, 128};
                auto delta = random_delta(1, duration);
                int timer = duration;
                bool moving = true;
                for (int step = 1; step <= 12; ++step) {
                    motion->UpdateMovement();
                    motion->ClampPosition();
                    motion->IntegrateVelocity();
                    motion->ClampPosition();
                    if (moving) {
                        --timer;
                        float p = eased_progress(timer, duration, easing);
                        x = interpolated_position(x, 192, delta, p, timer <= 0);
                        y = interpolated_position(y, 128, delta, p, timer <= 0);
                        if (timer <= 0)
                            moving = false;
                    }
                    const auto bx = body_bounds(x, 5), by = body_bounds(y, 5);
                    if (motion->position.x - native_half < bx.lo ||
                        motion->position.x + native_half > bx.hi ||
                        motion->position.y - native_half < by.lo ||
                        motion->position.y + native_half > by.hi)
                        throw std::runtime_error(
                            "random movement envelope failed native containment");
                }
            }
}
} // namespace
int main(int argc, char **argv) {
    try {
        using namespace th08;
        if (argc != 2)
            throw std::runtime_error("usage: th08_headless_body_forecast DAT");
        const auto dat = std::filesystem::absolute(argv[1]).string();
        RunDirectory directory;
        headless::Config c;
        c.dat_path = dat.c_str();
        c.stage = 8;
        c.spell = 192;
        c.difficulty = 4;
        headless::Session s(c);
        while (s.state().frame < 200)
            s.step(4);
        Enemy *boss = nullptr, *passive = nullptr;
        for (auto &e : g_EnemyManager.enemies)
            if (e.flags1 & ENEMY_FLAG_ACTIVE) {
                if (e.flags1 & ENEMY_FLAG_BOSS)
                    boss = &e;
                else if (e.flags1 & ENEMY_FLAG_NO_SPRITE)
                    passive = &e;
            }
        if (!boss || !passive)
            return 9;
        unsigned tests = 0, fails = 0;
        auto expect = [&](const char *name, bool reject) {
            auto before = s.state();
            auto f = s.enemy_body_forecast(12);
            auto after = s.state();
            ++tests;
            bool actual = f.failure != headless::BodyForecastFailure::None;
            if (actual != reject || before.rng_seed != after.rng_seed ||
                before.rng_draws != after.rng_draws) {
                printf("FAIL %s rejection=%d reason=%d\n", name, actual, int(f.failure));
                ++fails;
            }
        };
#define CHECK(field, value, name)                                                                  \
    {                                                                                              \
        auto save = (field);                                                                       \
        (field) = (value);                                                                         \
        expect(name, true);                                                                        \
        (field) = save;                                                                            \
    }
        interval_cases();
        expect("baseline", false);
        {
            // A pending timeline can create the first body: no active owner
            // must not be mistaken for certified empty future space.
            std::vector<unsigned> flags;
            for (auto &enemy : g_EnemyManager.enemies) {
                flags.push_back(enemy.flags1);
                enemy.flags1 = 0;
            }
            auto &timeline = g_EnemyManager.timelines[0];
            auto *instruction = timeline.instruction;
            const auto opcode = instruction->opcode;
            const auto time = instruction->time;
            instruction->time = timeline.timer.current;
            instruction->opcode = ECL_TIMELINE_OPCODE_SPAWN_ENEMY;
            expect("empty world imminent timeline spawn", true);
            instruction->opcode = ECL_TIMELINE_OPCODE_WAIT_FOR_BOSS_DEFEAT;
            expect("empty world unprotected boss wait", true);
            instruction->opcode = opcode;
            instruction->time = time;
            for (std::size_t i = 0; i < flags.size(); ++i)
                g_EnemyManager.enemies[i].flags1 = flags[i];
        }
        CHECK(g_Supervisor.flags.forceExtraTimerStep, 1, "extra timer step");
        CHECK(g_Supervisor.framerateMultiplier, .5f, "nonunit clock");
        CHECK(boss->flags2, boss->flags2 | ENEMY_FLAG2_FORCE_PAUSE, "force pause");
        CHECK(boss->flags1, boss->flags1 | ENEMY_FLAG_LINKED_CHILD, "linked child");
        CHECK(boss->movementTimer.subFrame, .5f, "movement subframe");
        CHECK(boss->mainEclContextStorage.time.subFrame, .5f, "ECL subframe");
        CHECK(boss->hitboxDimensions.x, std::numeric_limits<float>::quiet_NaN(), "NaN size");
        CHECK(boss->hitboxDimensions.x, -1.f, "negative size");
        // Finite source scalars can still overflow the final lethal endpoints.
        {
            const auto flags = boss->flags1;
            const auto position = boss->position;
            const auto velocity = boss->velocity;
            const auto size = boss->hitboxDimensions;
            boss->flags1 &= ~(ENEMY_FLAG_CLAMP_POSITION | ENEMY_FLAG_MOVEMENT_MODE_MASK);
            boss->position.x = std::numeric_limits<float>::max();
            boss->velocity.x = boss->velocity.y = 0;
            boss->hitboxDimensions.x = std::numeric_limits<float>::max();
            expect("finite endpoint overflow", true);
            boss->flags1 = flags;
            boss->position = position;
            boss->velocity = velocity;
            boss->hitboxDimensions = size;
        }
        CHECK(boss->pendingEclSubroutineIndex, 11, "pending sub");
        CHECK(passive->mainEclCallStackDepth, 1, "passive call stack");
        CHECK(passive->timerCallbackThresholdFrames, 10, "passive timer callback");
        CHECK(passive->lifeCallbackThresholds[0], 100, "passive life callback");
        CHECK(passive->mainEclContextStorage.interpolationSlots[0].callback,
              reinterpret_cast<EnemyEclInterpolatorCallback>(uintptr_t(1)), "passive interpolator");
        CHECK(g_EnemyManager.timelines[0].instruction->opcode,
              ECL_TIMELINE_OPCODE_SET_BOSS_PENDING_ECL_SUBROUTINE, "timeline remote");
        auto *p = boss->mainEclContextStorage.currentInstr;
        for (int n = 0; n < 20; n++) {
            if (p->opcode == ECL_OPCODE_SET_INT) {
                int save;
                memcpy(&save, p->operands, 4);
                int bad = 10016;
                memcpy(p->operands, &bad, 4);
                expect("invalid int lvalue", true);
                memcpy(p->operands, &save, 4);
                break;
            }
            p = reinterpret_cast<EclRawInstruction *>(reinterpret_cast<char *>(p) + p->nextOffset);
        }
        {
            auto save = boss->timerCallbackThresholdFrames;
            boss->timerCallbackThresholdFrames = boss->bossTimer.current + 1;
            auto f = s.enemy_body_forecast(12);
            ++tests;
            if (f.failure != headless::BodyForecastFailure::None || f.warnings.size() != 1) {
                printf("FAIL callback cut count=%zu reason=%d\n", f.warnings.size(),
                       int(f.failure));
                ++fails;
            }
            boss->timerCallbackThresholdFrames = save;
        }
        // A blocked event cannot be published while every timeline is blocked.
        {
            auto *instruction = g_EnemyManager.timelines[0].instruction;
            const auto opcode = instruction->opcode;
            const int event = instruction->args.ints[0];
            instruction->opcode = ECL_TIMELINE_OPCODE_WAIT_FOR_EVENT;
            instruction->args.ints[0] = 12345;
            expect("blocked timeline event", false);
            CHECK(g_EnemyManager.timelineEventSlots[0], 12345, "ready timeline event");
            instruction->opcode = opcode;
            instruction->args.ints[0] = event;
        }
        // Continuous Extra uses a noninteractive death followed next update
        // by an unconditional EndSpell prefix. Guard both sides of that handoff.
        {
            const auto flags = boss->flags1;
            const int life = boss->life;
            const int callback = boss->deathCallbackSubId;
            boss->flags1 = (flags & ~ENEMY_FLAG_DEATH_MODE_MASK) |
                           (ENEMY_DEATH_MODE_PERSIST_NONINTERACTIVE << ENEMY_FLAG_DEATH_MODE_SHIFT);
            boss->life = 1;
            // Spell practice overrides its timeout callback with a wrapper.
            // Select a real loaded phase-EndSpell prefix for this state fixture.
            int phase_callback = -1;
            for (int id = 0; id + 1 < g_EclManager.eclFile->subCount; ++id) {
                const auto *first =
                    reinterpret_cast<const EclRawInstruction *>(g_EclManager.subTable[id]);
                if (first->time != 0 || first->opcode != ECL_OPCODE_SET_TIMER_CALLBACK ||
                    first->nextOffset != 20)
                    continue;
                const auto *second = reinterpret_cast<const EclRawInstruction *>(
                    reinterpret_cast<const char *>(first) + 20);
                if (second->time != 0 || second->opcode != ECL_OPCODE_SET_DAMAGE_REDUCTION_TIMER ||
                    second->nextOffset != 16)
                    continue;
                const auto *third = reinterpret_cast<const EclRawInstruction *>(
                    reinterpret_cast<const char *>(second) + 16);
                if (third->time == 0 && third->opcode == ECL_OPCODE_END_SPELL &&
                    third->nextOffset == 12) {
                    phase_callback = id;
                    break;
                }
            }
            if (phase_callback < 0)
                throw std::runtime_error("missing phase callback fixture");
            boss->deathCallbackSubId = phase_callback;
            auto *entry = reinterpret_cast<EclRawInstruction *>(
                g_EclManager.subTable[boss->deathCallbackSubId]);
            auto *end = reinterpret_cast<EclRawInstruction *>(reinterpret_cast<char *>(entry) + 36);
            expect("future death immunity prefix", false);
            CHECK(end->opcode, ECL_OPCODE_NOP, "corrupt death immunity prefix");
            auto &context = boss->mainEclContextStorage;
            const auto sub = context.subId;
            const auto time = context.time;
            auto *cursor = context.currentInstr;
            context.subId = boss->deathCallbackSubId;
            context.time = 0;
            context.currentInstr = entry;
            boss->life = 0;
            boss->flags1 &= ~ENEMY_FLAG_COLLISION;
            expect("installed death immunity prefix", false);
            CHECK(boss->pendingEclSubroutineIndex, 11, "pending replacement of immunity prefix");
            CHECK(context.time.subFrame, .5f, "fractional immunity prefix clock");
            CHECK(context.interpolationSlots[0].callback,
                  reinterpret_cast<EnemyEclInterpolatorCallback>(uintptr_t(1)),
                  "immunity prefix interpolator");
            context.subId = sub;
            context.time = time;
            context.currentInstr = cursor;
            boss->flags1 = flags;
            boss->life = life;
            boss->deathCallbackSubId = callback;
        }
        expect("restored", false);
        {
            // Native observation includes a miss and an invulnerable overlap;
            // neither should disappear merely because no death was recorded.
            headless::begin_update_observation();
            const auto player_state = g_Player.playerState;
            g_Player.playerState = PLAYER_STATE_INVULNERABLE;
            Float3 far{-1000.f, -1000.f, 0.f}, size{5.f, 7.f, 0.f};
            const auto missed = g_Player.CheckLethalCollision(&far, &size);
            Float3 touching = g_Player.position;
            const auto overlapped = g_Player.CheckLethalCollision(&touching, &size);
            const auto &regions = s.lethal_regions();
            ++tests;
            if (missed != 0 || overlapped != 1 || regions.size() != 2 ||
                regions[0].bounds.left != far.x - size.x / 2.f ||
                regions[0].bounds.bottom != size.y / 2.f + far.y ||
                regions[1].bounds.right != size.x / 2.f + touching.x ||
                regions[1].bounds.top != touching.y - size.y / 2.f ||
                s.collision().kind != headless::CollisionKind::None) {
                printf("FAIL native lethal-region observation\n");
                ++fails;
            }
            headless::begin_update_observation();
            ++tests;
            if (!s.lethal_regions().empty()) {
                printf("FAIL stale lethal-region observation\n");
                ++fails;
            }
            g_Player.playerState = player_state;
        }
        printf("tests=%u failures=%u\n", tests, fails);
        return fails ? 1 : 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
}
