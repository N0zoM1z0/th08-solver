#include <cmath>
#include <stdexcept>
#include <th08/effect_pool.hpp>

namespace th08::effect {
namespace {
namespace cp = camera_particle;
bool valid_animation(const Effect51Animation &value) {
    const auto &fields = value.fields;
    const auto &control = value.control;
    if (!cp::detail::finite(fields.position_offset) ||
        !cp::detail::finite(fields.position_initial) ||
        !cp::detail::finite(fields.position_final) ||
        !cp::detail::finite(fields.rotation_initial) || !cp::detail::finite(value.rotation) ||
        !cp::detail::finite(value.angular_velocity) || !std::isfinite(value.sprite_width) ||
        !std::isfinite(value.sprite_height) || !std::isfinite(control.time.fraction) ||
        !std::isfinite(control.wait.fraction) || !std::isfinite(control.return_time.fraction))
        return false;
    for (const float scalar : control.floats)
        if (!std::isfinite(scalar))
            return false;
    return true;
}
} // namespace

const char *name(Status status) {
    switch (status) {
    case Status::spawned:
        return "SPAWNED";
    case Status::exhausted:
        return "EXHAUSTED";
    case Status::missing_context:
        return "MISSING_CONTEXT";
    case Status::invalid_state:
        return "INVALID_STATE";
    }
    return "INVALID_STATE";
}

PrimaryPool::PrimaryPool() {
    scratch_.reserve(primary_capacity);
}
PrimaryPool::PrimaryPool(const Occupancy &occupied, std::size_t cursor, bool spawn_event)
    : PrimaryPool() {
    if (cursor >= primary_capacity)
        throw std::invalid_argument("effect cursor outside primary pool");
    cursor_ = cursor;
    spawn_event_ = spawn_event;
    for (std::size_t i = 0; i < primary_capacity; ++i) {
        slots_[i].active = occupied[i];
        active_count_ += occupied[i] ? 1 : 0;
    }
}
PrimaryPool::PrimaryPool(const PrimaryPool &other)
    : slots_(other.slots_), cursor_(other.cursor_), active_count_(other.active_count_),
      spawn_event_(other.spawn_event_) {
    // Scratch is temporary work, not checkpoint state. Do not copy the previous
    // request's staged slots into every candidate snapshot.
    scratch_.reserve(primary_capacity);
}
PrimaryPool &PrimaryPool::operator=(const PrimaryPool &other) {
    if (this != &other) {
        slots_ = other.slots_;
        cursor_ = other.cursor_;
        active_count_ = other.active_count_;
        spawn_event_ = other.spawn_event_;
        scratch_.clear();
    }
    return *this;
}
const Slot &PrimaryPool::slot(std::size_t index) const {
    if (index >= primary_capacity)
        throw std::out_of_range("effect slot outside primary pool");
    return slots_[index];
}

Result PrimaryPool::spawn_effect51(const Effect51Request &request, const Effect51Inputs &inputs,
                                   random::Rng *rng) {
    scratch_.clear();
    // A full source scan returns to its starting cursor. In particular this
    // branch cannot require ANM, camera, multiplier, request position or RNG.
    if (active_count_ == primary_capacity) {
        spawn_event_ = true;
        return {Status::exhausted, 0, primary_capacity, exhausted_effect};
    }
    auto cursor = cursor_;
    auto remaining = std::int64_t(request.count);
    std::optional<random::Rng> stream;
    bool validated = false;
    Result result{Status::exhausted, 0, 0, exhausted_effect};
    for (std::size_t scan = 0; scan < primary_capacity; ++scan) {
        const auto selected = cursor;
        cursor = cursor + 1 == primary_capacity ? 0 : cursor + 1;
        ++result.inspected;
        if (slots_[selected].active)
            continue;
        // Inputs are demanded only at the first actual allocation. A later
        // initialization overflow can still roll back all earlier staged slots.
        if (!validated) {
            if (!inputs.post_time_zero || !inputs.camera || !inputs.multiplier || !rng)
                return {Status::missing_context, 0, result.inspected, exhausted_effect};
            if (!cp::detail::finite(request.position) || !valid_animation(*inputs.post_time_zero))
                return {Status::invalid_state, 0, result.inspected, exhausted_effect};
            stream = *rng;
            validated = true;
        }
        Slot next;
        next.active = true;
        next.effect51_known = true;
        next.animation = *inputs.post_time_zero;
        next.particle.position = request.position;
        next.particle.animation = next.animation.fields;
        next.particle.animation.flags |= 0x2000U;
        next.particle.animation.primary = request.color;
        next.particle.animation.position_offset = {0, 0, 0};
        // Slot clear establishes zero inherited velocity before the initializer.
        const auto initialized =
            cp::initialize(next.particle, inputs.camera, *inputs.multiplier, &*stream);
        if (initialized != cp::Status::initialized)
            return {initialized == cp::Status::missing_context ? Status::missing_context
                                                               : Status::invalid_state,
                    0, result.inspected, exhausted_effect};
        next.animation.fields = next.particle.animation;
        scratch_.push_back({selected, next});
        // Widened arithmetic preserves source count<=0 scanning without C++
        // signed-overflow undefined behavior at an INT32_MIN request.
        if (--remaining == 0) {
            result.status = Status::spawned;
            result.returned_slot = selected;
            break;
        }
    }
    for (const auto &delta : scratch_)
        slots_[delta.index] = delta.slot;
    result.allocated = scratch_.size();
    active_count_ += result.allocated;
    cursor_ = cursor;
    spawn_event_ = true;
    if (stream)
        *rng = *stream;
    return result;
}
} // namespace th08::effect
