#include <cstdlib>
#include <iostream>
#include <th08/acceleration.hpp>

namespace bullet = th08::bullet;
void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
int main() {
    bullet::Flight flight{0, 0, 0, 0, 0, 2};
    bullet::Acceleration state;
    check(bullet::install_vector_acceleration(state, flight, 4, -990, 2, .5f) ==
                  bullet::Status::advanced &&
              state.vector_x == 2 && state.vector_y == 0,
          "vector installation lost angle sentinel or installation multiplier");
    check(bullet::advance_acceleration(flight, state, .5f) == bullet::Status::advanced &&
              flight.velocity_x == 1 && flight.speed == 2 && state.timer == 0 &&
              state.subframe == .5f,
          "vector update omitted second multiplier or changed scalar speed");
    state.timer = 2;
    check(bullet::advance_acceleration(flight, state) == bullet::Status::advanced &&
              !state.active && state.timer == 3 && state.subframe == .5f && flight.velocity_x == 1,
          "vector expiry changed velocity or skipped timer increment");
    state = {};
    state.active = true;
    state.timer = 16;
    state.subframe = .5f;
    flight.angle = 0;
    check(bullet::advance_acceleration(flight, state, .5f) == bullet::Status::advanced &&
              state.active && flight.velocity_x == .921875f && state.timer == 17 &&
              state.subframe == 0,
          "deceleration dropped the inclusive terminal fractional ramp");
    check(bullet::advance_acceleration(flight, state, 2) == bullet::Status::advanced &&
              !state.active && flight.velocity_x == .921875f && state.timer == 18,
          "deceleration expiry recomputed velocity using the new multiplier");
    state = {};
    state.mode = bullet::AccelerationMode::vector;
    state.active = true;
    state.duration = 10;
    flight.angle = 1;
    flight.velocity_x = .0001f;
    flight.velocity_y = -.0001f;
    check(bullet::advance_acceleration(flight, state) == bullet::Status::advanced &&
              flight.angle == 1,
          "vector angle dead zone changed at exact threshold");
    state.mode = bullet::AccelerationMode::polar;
    state.angle_delta = 0;
    state.speed_delta = -2;
    flight.angle = 0;
    flight.speed = 1;
    check(bullet::advance_acceleration(flight, state) == bullet::Status::advanced &&
              flight.speed == -1 && flight.velocity_x == -1,
          "polar acceleration clamped a signed speed");
    const auto before = flight;
    const auto timer = state.timer;
    state.speed_delta = std::numeric_limits<float>::infinity();
    check(bullet::advance_acceleration(flight, state) == bullet::Status::invalid &&
              flight.velocity_x == before.velocity_x && state.timer == timer,
          "invalid acceleration changed state");
    std::cout
        << "Acceleration installation, fractional clocks, terminal frames and dead zone: passed\n";
}
