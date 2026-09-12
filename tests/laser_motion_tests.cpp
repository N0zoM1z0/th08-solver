#include <cstdlib>
#include <iostream>
#include <th08/laser_motion.hpp>

namespace laser = th08::laser;
void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
int main() {
    // Minimized from the pinned source oracle's scenario 66. The literal keeps
    // the exact float32 width; the source expression rounds below zero at t=12.
    for (float y : {0.0f, 20.0f, 0.0f}) {
        laser::State edge;
        edge.position = {-50, y};
        edge.width = 0x1.db89b4p+3f;
        edge.end_offset = edge.length = 100;
        edge.timer = edge.despawn_duration = 12;
        edge.hitbox_end_delay = 34;
        edge.phase = laser::Phase::despawning;
        const auto result = laser::advance(edge);
        check(result.status == laser::Status::inactive && result.count == 1 &&
                  result.calls[0].geometry.box.size.x == -0x1p-21f,
              "despawn roundoff lost the signed terminal hitbox");
        const auto shape = result.calls[0].geometry;
        check(!th08::geometry::laser_hit({1, y}, {1, 1}, shape),
              "signed terminal hitbox was clamped to zero");
        th08::geometry::Snapshot snapshot({th08::geometry::Hazard::laser(shape)}, {1, 1});
        check(snapshot.query({0, y}) && !snapshot.query({1, y}) && snapshot.scan({0, y}),
              "signed terminal hitbox disagreed across geometry layers");
    }
    laser::State state;
    state.duration = 100;
    auto fractional = laser::advance(state, .5f);
    check(fractional.status == laser::Status::advanced && state.timer == 0 && state.subframe == .5f,
          "fractional laser clock advanced an integer frame early");
    laser::advance(state, 1);
    check(state.timer == 1 && state.subframe == .5f,
          "unit-rate timer branch discarded the existing fraction");
    laser::advance(state, .5f);
    check(state.timer == 2 && state.subframe == 0, "fractional laser clock failed carry");
    state = {};
    state.position = {20, 30};
    state.end_offset = 100;
    state.length = 100;
    state.width = 16;
    state.start_time = 10;
    state.duration = 0;
    state.despawn_duration = 5;
    state.hitbox_end_delay = 3;
    state.phase = laser::Phase::starting;
    auto result = laser::advance(state);
    check(result.count == 1 && result.calls[0].geometry.box.size.x == .6f &&
              result.calls[0].geometry.box.size.y == 8 && !result.calls[0].allow_graze,
          "startup collision dimensions or graze gate mismatch");
    state.timer = 10;
    result = laser::advance(state);
    check(result.count == 3 && !result.calls[0].allow_graze && result.calls[1].allow_graze &&
              !result.calls[2].allow_graze && state.phase == laser::Phase::despawning &&
              state.timer == 1,
          "transition-frame ordered collision calls lost");
    state.timer = 3;
    result = laser::advance(state);
    check(result.count == 0 && result.status == laser::Status::advanced,
          "despawn hitbox end delay treated as inclusive");
    state.timer = 5;
    result = laser::advance(state);
    check(result.status == laser::Status::inactive && state.timer == 5,
          "retired laser incremented its timer");
    state.in_use = true;
    state.phase = laser::Phase::active;
    state.timer = 0;
    state.despawn_duration = 0;
    result = laser::advance(state);
    check(result.status == laser::Status::inactive && result.count == 1,
          "instant retirement discarded active-frame hitbox");
    state.in_use = true;
    state.duration = 100;
    state.phase = laser::Phase::active;
    state.end_offset = 200;
    state.start_offset = 0;
    result = laser::advance(state);
    check(state.start_offset == 100 && result.calls[0].geometry.box.size.x == 70 &&
              result.calls[0].geometry.box.center.x == 170,
          "moving laser tail or shortened hitbox mismatch");
    state.phase = laser::Phase::starting;
    state.start_time = 0;
    const auto old_end = state.end_offset;
    result = laser::advance(state);
    check(result.status == laser::Status::invalid && state.end_offset == old_end,
          "invalid startup modified laser state");
    std::cout << "Laser phase fallthrough, collision gates, shape and retirement: passed\n";
}
