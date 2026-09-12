#include <cstdlib>
#include <iostream>
#include <th08/kinematics.hpp>

namespace motion = th08::kinematics;
void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
int main() {
    motion::Pattern pattern{motion::Aim::fan, 5, 4, 8, 0, 0, 0.25f};
    motion::Launch result{};
    const float expected[] = {0, -0.25f, 0.25f, -0.5f, 0.5f};
    for (int i = 0; i < 5; ++i) {
        check(motion::launch(pattern, i, 3, 0, 1, {}, result) == motion::Status::ready,
              "fan launch failed");
        check(result.raw_angle == expected[i] && result.speed == 2,
              "fan order or layer denominator mismatch");
    }
    pattern.count1 = 4;
    check(motion::launch(pattern, 1, 0, 0, 1, {}, result) == motion::Status::ready &&
              result.angle == -0.125f,
          "even fan half-step mismatch");
    pattern.aim = motion::Aim::random_angle_speed;
    result.speed = 123;
    check(motion::launch(pattern, 0, 0, 0, 1, {}, result) == motion::Status::missing_random &&
              result.speed == 123,
          "missing randomness invented values or modified output");
    check(motion::launch(pattern, 0, 0, 0, 1, {1, 0}, result) == motion::Status::ready &&
              result.angle == 0 && result.speed == 0,
          "inclusive float RNG endpoints rejected");
    pattern.count1 = 0;
    check(motion::launch(pattern, 0, 0, 0, 1, {0, 0}, result) == motion::Status::invalid,
          "zero count accepted");
    pattern.count1 = 4;
    check(motion::launch(pattern, 4, 0, 0, 1, {0, 0}, result) == motion::Status::invalid,
          "out-of-range launch index accepted");
    check(motion::normalize_angle(motion::pi) == motion::pi &&
              motion::normalize_angle(-motion::pi) == -motion::pi,
          "angle endpoint policy mismatch");
    check(motion::normalize_angle(200) > motion::pi,
          "capped retail normalization replaced by complete modulo");
    std::cout << "Launch ordering, layer spacing, explicit RNG and angle boundaries: passed\n";
}
