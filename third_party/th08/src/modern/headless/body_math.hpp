#pragma once
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <stdexcept>

namespace th08::headless::body_detail {
struct Interval {
    float lo, hi;
};
inline float outward(double value, bool upper) {
    const float rounded = static_cast<float>(value);
    if (!std::isfinite(value) || !std::isfinite(rounded))
        throw std::overflow_error("nonfinite body interval");
    const float bound = std::nextafter(rounded, upper ? INFINITY : -INFINITY);
    if (!std::isfinite(bound))
        throw std::overflow_error("body interval overflow");
    return bound;
}
inline Interval add(Interval a, Interval b) {
    return {outward(double(a.lo) + b.lo, false), outward(double(a.hi) + b.hi, true)};
}
inline Interval multiply(Interval a, Interval b) {
    const double values[]{double(a.lo) * b.lo, double(a.lo) * b.hi, double(a.hi) * b.lo,
                          double(a.hi) * b.hi};
    const auto bounds = std::minmax_element(std::begin(values), std::end(values));
    return {outward(*bounds.first, false), outward(*bounds.second, true)};
}
inline double rounding_error(double magnitude) {
    // A whole ulp at an outward-rounded magnitude bounds round-to-nearest
    // error for either sign, including subnormals. Overflow rejects the view.
    const float upper = outward(std::abs(magnitude), true);
    const float next = std::nextafter(upper, INFINITY);
    if (!std::isfinite(next))
        throw std::overflow_error("body rounding bound overflow");
    return double(next) - upper;
}
inline Interval random_delta(float speed, int duration) {
    // Native: (cosf/sinf(angle) * speed) * float(duration). Retain both
    // rounded products; an all-angle bound covers player-dependent direction.
    return multiply(multiply({-1.f, 1.f}, {speed, speed}), {float(duration), float(duration)});
}
inline float eased_progress(int timer, int duration, unsigned easing) {
    float p = std::max(0.f, 1.f - float(timer) / float(duration));
    switch (easing) {
    case 0:
        return p;
    case 1:
        return p * p;
    case 2:
        return p * p * p;
    case 3:
        return p * p * p * p;
    case 4:
        p = 1.f - p;
        return 1.f - p * p;
    case 5:
        p = 1.f - p;
        return 1.f - p * p * p;
    case 6:
        p = 1.f - p;
        return 1.f - p * p * p * p;
    default:
        throw std::domain_error("unsupported body easing");
    }
}
inline Interval interpolated_position(Interval old, float origin, Interval delta, float progress,
                                      bool terminal) {
    const auto target =
        add({origin, origin}, terminal ? delta : multiply(delta, {progress, progress}));
    if (terminal)
        return target; // Native snaps to origin+delta and sets velocity to zero.
    // Native computes v=fl(target-old), then x=fl(old+v). Preserve the old
    // position's correlation instead of tripling an interval each update.
    // Callers prove the intervening pre-integration clamp is inactive here.
    const double difference =
        std::max(std::abs(double(target.lo) - old.hi), std::abs(double(target.hi) - old.lo));
    const double subtract_error = rounding_error(difference);
    const double magnitude = std::max(std::abs(double(target.lo)), std::abs(double(target.hi)));
    const double integrate_error = rounding_error(magnitude + subtract_error);
    const double error = subtract_error + integrate_error;
    return {outward(double(target.lo) - error, false), outward(double(target.hi) + error, true)};
}
inline Interval clamp(Interval value, float lower, float upper) {
    return {std::clamp(value.lo, lower, upper), std::clamp(value.hi, lower, upper)};
}
inline Interval body_bounds(Interval position, float full_size) {
    // Same reciprocal-multiplication order as CheckPlayerCollision/CheckLethalCollision,
    // then outward endpoints; no center/width reconstruction can narrow them.
    const float half = (full_size * (1.f / 1.5f)) / 2.f;
    return {outward(double(position.lo) - half, false), outward(double(position.hi) + half, true)};
}
} // namespace th08::headless::body_detail
