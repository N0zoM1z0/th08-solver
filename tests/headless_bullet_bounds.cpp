#include "BulletManager.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "Supervisor.hpp"
#include "modern/headless/runtime.hpp"
#include "modern/headless/session.hpp"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <th08/native_policy.hpp>

namespace {
void check(bool valid, const char *message) {
    if (!valid)
        throw std::runtime_error(message);
}
class RunDirectory {
  public:
    RunDirectory() : previous(std::filesystem::current_path()) {
        char pattern[] = "/tmp/th08-bullet-bounds-XXXXXX";
        const auto *created = mkdtemp(pattern);
        check(created, "cannot create native test directory");
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
struct Fixture {
    th08::AnmLoadedSprite sprite{};
    th08::Bullet &bullet = th08::g_BulletManager.bullets[0];
    void reset() {
        using namespace th08;
        for (auto &b : g_BulletManager.bullets)
            b.state = BULLET_STATE_UNUSED;
        for (auto &laser : g_BulletManager.lasers)
            laser.inUse = 0;
        g_GameManager.scriptedUpdateFreeze = 0;
        g_GameManager.flags.deathbombFreezeActive = 0;
        g_Supervisor.flags.forceExtraTimerStep = 0;
        g_Supervisor.framerateMultiplier = 1;
        auto &b = bullet;
        b.state = BULLET_STATE_FIRED;
        b.stateTimer = 0;
        b.activeTimer = 0;
        b.position = {100, 100, 0};
        b.velocity = {.125f, -.25f, 0};
        b.angle = .71f;
        b.collisionDisabled = 1;
        b.offscreenCullDelayFrames = 10000;
        b.activeTransformFlags = BULLET_TRANSFORM_WAIT;
        b.transformFlags = BULLET_TRANSFORM_WAIT | BULLET_TRANSFORM_ACCELERATE_VECTOR;
        b.transformIndex = 1;
        b.transformSound = -1;
        for (auto &record : b.transforms)
            record = {};
        b.transforms[1].kind = BULLET_TRANSFORM_ACCELERATE_VECTOR;
        b.transforms[1].payload.vectorAcceleration.durationFrames = 2;
        b.transforms[1].payload.vectorAcceleration.magnitude = .033333333f;
        b.transforms[1].payload.vectorAcceleration.angle = -.0278272f;
        b.exStates[BULLET_TRANSFORM_STATE_WAIT].timer = 0;
        sprite.widthPx = sprite.heightPx = 8;
        b.sprites.bulletVm.loadedSprite = &sprite;
        b.sprites.bulletVm.currentInstruction = nullptr;
        b.sprites.despawnVm.currentInstruction = nullptr;
        b.sprites.collisionSize = {4, 4, 0};
    }
};
void check_vector(th08::headless::Session &session, Fixture &fixture) {
    using namespace th08;
    struct Case {
        int wait, duration;
        float angle;
    };
    const std::array<Case, 6> cases{{{0, 0, 0},
                                     {0, 1, 1.57f},
                                     {1, 2, -.0278272f},
                                     {3, 2, -2.1f},
                                     {-1, -1, 0},
                                     {60, 60, -999.f}}};
    for (const auto &test : cases) {
        fixture.reset();
        auto &b = fixture.bullet;
        b.exStates[BULLET_TRANSFORM_STATE_WAIT].timer = test.wait;
        b.transforms[1].payload.vectorAcceleration.durationFrames = test.duration;
        b.transforms[1].payload.vectorAcceleration.angle = test.angle;
        const auto view = session.bullets().front();
        check(view.wait_vector.updates != 0, "missing WAIT/vector certificate");
        for (unsigned step = 1; step <= view.wait_vector.updates; ++step) {
            BulletManager::OnUpdate(&g_BulletManager);
            const auto p =
                policy::detail::project_bullet(view, step, true, false, false, false, true);
            check(p.x == b.position.x && p.y == b.position.y,
                  "WAIT/vector projection changed native update order");
        }
        check(policy::detail::project_bullet(view, view.wait_vector.updates + 1, true, false, false,
                                             false, true)
                      .kind == policy::detail::BulletProjectionKind::unsupported,
              "WAIT/vector crossed the final clear update");
    }
    auto supported = [&]() { return session.bullets().front().wait_vector.updates != 0; };
    fixture.reset();
    fixture.bullet.transforms[1].allowWhileActive = 1;
    check(!supported(), "concurrent vector record accepted");
    fixture.reset();
    fixture.bullet.transforms[2].kind = BULLET_TRANSFORM_CHANGE_DIRECTION_AIMED;
    check(!supported(), "unsupported future record accepted");
    fixture.reset();
    fixture.bullet.exStates[BULLET_TRANSFORM_STATE_WAIT].timer.subFrame = .5f;
    check(!supported(), "fractional WAIT timer accepted");
    fixture.reset();
    g_Supervisor.flags.forceExtraTimerStep = 1;
    check(!supported(), "extra timer step accepted");
    fixture.reset();
    g_GameManager.flags.deathbombFreezeActive = 1;
    check(!supported(), "deathbomb freeze accepted");
    fixture.reset();
    fixture.bullet.position.x = std::numeric_limits<float>::infinity();
    check(!supported(), "nonfinite position accepted");
}
void check_despawn(th08::headless::Session &session, Fixture &fixture) {
    using namespace th08;
    fixture.reset();
    auto &b = fixture.bullet;
    b.velocity = {1, 0, 0};
    b.transformFlags = BULLET_TRANSFORM_WAIT | BULLET_TRANSFORM_DESPAWN;
    b.transforms[1].kind = BULLET_TRANSFORM_DESPAWN;
    b.exStates[BULLET_TRANSFORM_STATE_WAIT].timer = 1;
    const auto view = session.bullets().front();
    check(view.wait_linear_updates == 3, "WAIT/DESPAWN omitted its final lethal movement");
    BulletManager::OnUpdate(&g_BulletManager);
    BulletManager::OnUpdate(&g_BulletManager);
    check(b.state == BULLET_STATE_FIRED && b.activeTransformFlags == 0 && b.position.x == 102,
          "WAIT cleared on the wrong native update");
    b.collisionDisabled = 0;
    g_Player.playerState = PLAYER_STATE_ALIVE;
    g_Player.hurtboxBoundsMin = {102.5f, 99.5f, 0};
    g_Player.hurtboxBoundsMax = {103.5f, 100.5f, 0};
    headless::begin_update_observation();
    BulletManager::OnUpdate(&g_BulletManager);
    check(b.state == BULLET_STATE_DESPAWNING && b.position.x == 103 &&
              headless::current_collision().kind == headless::CollisionKind::Bullet,
          "DESPAWN lost its current FIRED-branch collision");
    check(policy::detail::project_bullet(view, 3, true, true).x == 103 &&
              policy::detail::project_bullet(view, 4, true, true).kind ==
                  policy::detail::BulletProjectionKind::unsupported,
          "WAIT/DESPAWN certificate crossed its final lethal update");
    g_GameManager.flags.deathbombFreezeActive = 0;
    g_Player.playerState = PLAYER_STATE_ALIVE;
    headless::begin_update_observation();
    BulletManager::OnUpdate(&g_BulletManager);
    check(headless::current_collision().kind == headless::CollisionKind::None,
          "later despawn animation was incorrectly lethal");
}
} // namespace
int main(int argc, char **argv) {
    try {
        check(argc == 2, "usage: th08_headless_bullet_bounds DAT");
        const auto dat = std::filesystem::absolute(argv[1]).string();
        RunDirectory directory;
        th08::headless::Session session({dat.c_str(), 8, 203, 4, 0});
        Fixture fixture;
        check_vector(session, fixture);
        check_despawn(session, fixture);
        std::cout << "Native WAIT/vector bounds and terminal DESPAWN collision passed\n";
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
