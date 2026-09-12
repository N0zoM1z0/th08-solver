#include <cstdlib>
#include <iostream>
#include <th08/bullet_motion.hpp>

namespace bullet = th08::bullet;
void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
int main() {
    bullet::Particle particle;
    bullet::InitialState input{{0, 0, 2, 2, 0},
                               100,
                               100,
                               14,
                               16,
                               bullet::Phase::spawning_fast,
                               10,
                               3,
                               {bullet::TurnMode::relative, true, 90, 2, 0, 0, 1, 1.4f}};
    check(bullet::initialize(input, particle) == bullet::Status::advanced &&
              particle.flight.x == 92,
          "spawn rewind mismatch");
    for (int i = 0; i < 9; ++i)
        check(bullet::advance(particle) == bullet::Status::advanced &&
                  particle.phase == bullet::Phase::spawning_fast && particle.turn.timer == 0 &&
                  particle.cull_delay == 3,
              "spawn phase advanced transform or cull timer");
    check(particle.flight.x == 101, "spawn displacement mismatch");
    check(bullet::advance(particle) == bullet::Status::advanced &&
              particle.phase == bullet::Phase::fired && particle.flight.x == 104 &&
              particle.turn.timer == 1 && particle.cull_delay == 2,
          "activation frame omitted fired update");
    check(bullet::initialize(input, particle) == bullet::Status::advanced,
          "reinitialization failed");
    check(bullet::advance(particle, 1, true) == bullet::Status::advanced && particle.flight.x == 93,
          "scripted freeze incorrectly froze spawn displacement");
    const auto unchanged = particle.flight.x;
    check(bullet::advance(particle, .5f) == bullet::Status::unsupported &&
              particle.flight.x == unchanged,
          "unit-clock timing certificate used for non-unit clock");
    input.phase = bullet::Phase::fired;
    input.turn = {};
    bullet::initialize(input, particle);
    check(bullet::advance(particle, 1, true) == bullet::Status::advanced &&
              particle.flight.x == 100,
          "fired displacement ignored scripted freeze");

    bullet::Flight flight{0, 0, 2, 0, 0, 2};
    bullet::DirectionChange turn{bullet::TurnMode::aimed, true, 1, 1, 0, 0, 0, 3};
    check(bullet::advance_direction(flight, turn, 1) == bullet::Status::advanced && turn.timer == 1,
          "target requested before aimed turn fired");
    check(bullet::advance_direction(flight, turn, 1) == bullet::Status::requires_target &&
              turn.timer == 1 && turn.completed == 0,
          "missing re-aim target changed transform state");
    check(bullet::advance_direction(flight, turn, 1, 0) == bullet::Status::advanced &&
              !turn.active && turn.completed == 1 && turn.timer == 1 && flight.velocity_x == 3,
          "last re-aim or post-turn timer mismatch");

    input.x = 500;
    input.launch = {0, 0, 0, 0, 0};
    input.cull_delay = 0;
    input.turn = {bullet::TurnMode::relative, true, 1000, 2, 0, 0, 0, 0};
    bullet::initialize(input, particle);
    for (int i = 0; i < 127; ++i)
        check(bullet::advance(particle) == bullet::Status::advanced,
              "turning bullet culled before 128 offscreen frames");
    check(bullet::advance(particle) == bullet::Status::inactive,
          "turning bullet failed 128-frame cull");
    input.turn = {};
    bullet::initialize(input, particle);
    check(bullet::advance(particle) == bullet::Status::inactive,
          "linear offscreen bullet retained an invented grace window");
    std::cout << "Spawn activation, freeze, re-aim dependency and offscreen lifecycle: passed\n";
}
