#pragma once
#include <cmath>
#include <cstdint>
#include <limits>

namespace th08::kinematics {
inline constexpr float pi = 3.14159265358979323846f;
// Preserve the source's capped loop, including its behavior beyond 18 turns.
inline float normalize_angle(float angle, float delta = 0) {
    int iterations = 0;
    angle += delta;
    while (angle > pi) {
        angle -= pi * 2.0f;
        if (iterations++ > 16)
            break;
    }
    while (angle < -pi) {
        angle += pi * 2.0f;
        if (iterations++ > 16)
            break;
    }
    return angle;
}
inline float point_angle(float y, float x) {
    return float(std::atan2(double(y), double(x)));
}
enum class Aim : std::uint8_t {
    fan_aimed,
    fan,
    circle_aimed,
    circle,
    offset_circle_aimed,
    offset_circle,
    random_angle,
    random_speed,
    random_angle_speed
};
struct Pattern {
    Aim aim;
    std::int32_t count1, count2;
    float speed1, speed2, angle, angle_step;
};
struct RandomPair {
    // Values already drawn in retail call order, not an independent per-bullet RNG.
    // NaN means no sample supplied. Non-random modes do not inspect these fields.
    float angle = std::numeric_limits<float>::quiet_NaN();
    float speed = std::numeric_limits<float>::quiet_NaN();
};
struct Launch {
    float raw_angle, angle, speed, velocity_x, velocity_y;
};
enum class Status { ready, invalid, missing_random };

// Pure launch kinematics, before transforms and pool/ANM/lifetime side effects.
// The velocity profile is TH08_MODERN_PORT float32 sinf/cosf, not retail x87 fsincos.
// Output remains unchanged on failure. No allocation or implicit RNG consumption.
inline Status launch(const Pattern &pattern, std::int32_t index1, std::int32_t index2,
                     float angle_to_player, float framerate_multiplier, RandomPair random,
                     Launch &output) {
    if (pattern.count1 <= 0 || pattern.count1 > 1536 || pattern.count2 <= 0 ||
        pattern.count2 > 1536 || index1 < 0 || index1 >= pattern.count1 || index2 < 0 ||
        index2 >= pattern.count2 || unsigned(pattern.aim) > 8 || !std::isfinite(pattern.speed1) ||
        !std::isfinite(pattern.speed2) || !std::isfinite(pattern.angle) ||
        !std::isfinite(pattern.angle_step) || !std::isfinite(framerate_multiplier) ||
        framerate_multiplier <= 0)
        return Status::invalid;
    const bool aimed = pattern.aim == Aim::fan_aimed || pattern.aim == Aim::circle_aimed ||
                       pattern.aim == Aim::offset_circle_aimed;
    if (aimed && !std::isfinite(angle_to_player))
        return Status::invalid;
    const bool random_angle =
        pattern.aim == Aim::random_angle || pattern.aim == Aim::random_angle_speed;
    const bool random_speed =
        pattern.aim == Aim::random_speed || pattern.aim == Aim::random_angle_speed;
    auto unit = [](float value) { return value >= 0 && value <= 1; };
    if ((random_angle && !unit(random.angle)) || (random_speed && !unit(random.speed)))
        return Status::missing_random;

    float angle = 0;
    float speed = pattern.count2 > 1 ? pattern.speed1 - (pattern.speed1 - pattern.speed2) *
                                                            float(index2) / float(pattern.count2)
                                     : pattern.speed1;
    switch (pattern.aim) {
    case Aim::fan_aimed:
    case Aim::fan:
        if (pattern.count1 & 1)
            angle += float((index1 + 1) / 2) * pattern.angle_step;
        else
            angle += float(index1 / 2) * pattern.angle_step + pattern.angle_step * 0.5f;
        if (index1 & 1)
            angle *= -1.0f;
        if (aimed)
            angle += angle_to_player;
        angle += pattern.angle;
        break;
    case Aim::circle_aimed:
    case Aim::circle:
    case Aim::random_speed:
        if (aimed)
            angle += angle_to_player;
        if (random_speed)
            speed = random.speed * (pattern.speed1 - pattern.speed2) + pattern.speed2;
        angle += float(index1) * (pi * 2.0f) / float(pattern.count1);
        angle += float(index2) * pattern.angle_step + pattern.angle;
        break;
    case Aim::offset_circle_aimed:
    case Aim::offset_circle:
        if (aimed)
            angle += angle_to_player;
        angle += pi / float(pattern.count1);
        angle += float(index1) * (pi * 2.0f) / float(pattern.count1);
        angle += pattern.angle;
        break;
    case Aim::random_angle:
    case Aim::random_angle_speed:
        angle = random.angle * (pattern.angle - pattern.angle_step) + pattern.angle_step;
        if (random_speed)
            speed = random.speed * (pattern.speed1 - pattern.speed2) + pattern.speed2;
        break;
    }
    const float magnitude = speed * framerate_multiplier;
    const Launch result{angle, normalize_angle(angle), speed, std::cos(angle) * magnitude,
                        std::sin(angle) * magnitude};
    if (!std::isfinite(result.angle) || !std::isfinite(result.speed) ||
        !std::isfinite(result.velocity_x) || !std::isfinite(result.velocity_y))
        return Status::invalid;
    output = result;
    return Status::ready;
}
} // namespace th08::kinematics
