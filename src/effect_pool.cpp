#include <cmath>
#include <limits>
#include <stdexcept>
#include <th08/effect_pool.hpp>
#include <th08/kinematics.hpp>
#include <th08/timing.hpp>

namespace th08::effect {
namespace {
namespace cp = camera_particle;
bool valid_animation(const ParticleAnimation &value) {
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
bool particle_script_waiting(const ParticleAnimation &value, SlotKind kind) {
    const auto &control = value.control;
    // This projection begins AFTER executing the certified time-zero instructions
    // and remains at its static-completion instruction. Interrupts, external
    // ANM freezes and arbitrary restored PCs need the full VM, not this phase.
    const bool matches = (kind == SlotKind::effect51 &&
                          value.script == ScriptCertificate::script73 && control.sprite == 121) ||
                         (kind == SlotKind::background62 &&
                          value.script == ScriptCertificate::script75 && control.sprite == 123);
    return matches && control.pc == 2 && control.active && control.visible && !control.stopped &&
           !control.frozen && !control.has_return && control.pending_interrupt == 0;
}
void tick_unit(animation::control::Clock &clock) {
    clock.previous = clock.current;
    timing::tick(clock.current, clock.fraction, 1.0f);
}
UpdateStatus advance_animation(Slot &next) {
    if (!particle_script_waiting(next.animation, next.kind))
        return UpdateStatus::unsupported_animation;
    auto &control = next.animation.control;
    if (!valid_animation(next.animation) || control.time.current < 1 ||
        control.time.fraction != 0.0f)
        return UpdateStatus::invalid_state;
    if (control.time.current >= 30000) {
        // Opcode2 is static completion: visibility is retained and the source
        // returns before angular motion and either timer tail.
        control.active = false;
        next.active = false;
        return UpdateStatus::advanced;
    }
    auto rotate = [](float current, float velocity) {
        return velocity != 0.0f ? kinematics::normalize_angle(current, velocity) : current;
    };
    auto &rotation = next.animation.rotation;
    const auto &velocity = next.animation.angular_velocity;
    rotation = {rotate(rotation.x, velocity.x), rotate(rotation.y, velocity.y),
                rotate(rotation.z, velocity.z)};
    // ExecuteScript marks the rotation dirty on every nonzero angular update;
    // a renderer may have cleared this bit. Keep both owned field views aligned.
    if (velocity.x != 0.0f || velocity.y != 0.0f || velocity.z != 0.0f) {
        next.animation.fields.flags |= 4U;
        next.particle.animation.flags = next.animation.fields.flags;
    }
    if (!cp::detail::finite(rotation) || next.timer.current < 0 ||
        next.timer.current == std::numeric_limits<std::int32_t>::max() ||
        next.timer.fraction != 0.0f)
        return UpdateStatus::invalid_state;
    tick_unit(control.time);
    tick_unit(next.timer);
    return UpdateStatus::advanced;
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
const char *name(UpdateStatus status) {
    switch (status) {
    case UpdateStatus::advanced:
        return "ADVANCED";
    case UpdateStatus::missing_context:
        return "MISSING_CONTEXT";
    case UpdateStatus::unsupported_animation:
        return "UNSUPPORTED_ANIMATION";
    case UpdateStatus::invalid_state:
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
      spawn_event_(other.spawn_event_), exhausted_draw_group_(other.exhausted_draw_group_) {
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
        exhausted_draw_group_ = other.exhausted_draw_group_;
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
    return spawn_particle(SlotKind::effect51, request, inputs, rng);
}
Result PrimaryPool::spawn_background62(cp::Vec3 position, const ParticleAnimation *post_time_zero) {
    const auto result = spawn_particle(SlotKind::background62, {1, position, {255, 255, 255, 32}},
                                       {post_time_zero, nullptr, std::nullopt}, nullptr);
    if (result.committed()) {
        // The source caller writes through SpawnEffect's returned pointer even
        // on full-pool exhaustion. Slot653 is metadata, never slots_[653].
        if (result.returned_slot == exhausted_effect)
            exhausted_draw_group_ = 4;
        else
            slots_[result.returned_slot].particle.draw_group = 4;
    }
    return result;
}
Result PrimaryPool::spawn_particle(SlotKind kind, const Effect51Request &request,
                                   const Effect51Inputs &inputs, random::Rng *rng) {
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
            if (!inputs.post_time_zero ||
                (kind == SlotKind::effect51 && (!inputs.camera || !inputs.multiplier || !rng)))
                return {Status::missing_context, 0, result.inspected, exhausted_effect};
            if (!cp::detail::finite(request.position) || !valid_animation(*inputs.post_time_zero))
                return {Status::invalid_state, 0, result.inspected, exhausted_effect};
            if (kind == SlotKind::effect51)
                stream = *rng;
            validated = true;
        }
        Slot next;
        next.active = true;
        next.kind = kind;
        next.animation = *inputs.post_time_zero;
        next.particle.position = request.position;
        next.particle.animation = next.animation.fields;
        next.particle.animation.flags |= 0x2000U;
        next.particle.animation.primary = request.color;
        next.particle.animation.position_offset = {0, 0, 0};
        // Only effect51 has an initializer. Effect62 keeps the common cleared
        // storage and spawn overrides, reading no camera, multiplier or RNG.
        if (kind == SlotKind::effect51) {
            const auto initialized =
                cp::initialize(next.particle, inputs.camera, *inputs.multiplier, &*stream);
            if (initialized != cp::Status::initialized)
                return {initialized == cp::Status::missing_context ? Status::missing_context
                                                                   : Status::invalid_state,
                        0, result.inspected, exhausted_effect};
        }
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

UpdateResult PrimaryPool::advance_particles(const ParticleUpdateInputs &inputs) {
    scratch_.clear();
    if (!inputs.deathbomb_freeze)
        return {UpdateStatus::missing_context};
    UpdateResult result{UpdateStatus::advanced};
    for (std::size_t index = 0; index < primary_capacity; ++index) {
        const auto &slot = slots_[index];
        if (!slot.active)
            continue;
        // The source increments activeCount before callback/ANM retirement, so
        // this observation can exceed the live occupancy after a successful phase.
        ++result.source_active_count;
        if (slot.kind == SlotKind::unknown)
            return {UpdateStatus::missing_context};
        if (*inputs.deathbomb_freeze)
            continue;

        auto next = slot;
        ++result.updated;
        if (next.kind == SlotKind::effect51) {
            const auto callback =
                cp::update(next.particle, inputs.camera, inputs.bosses, inputs.stage_tint);
            if (callback == cp::Status::missing_context)
                return {UpdateStatus::missing_context};
            if (callback == cp::Status::invalid_state)
                return {UpdateStatus::invalid_state};
            next.animation.fields = next.particle.animation;
            if (callback == cp::Status::culled) {
                // Motion before the view-cone test remains observable on the
                // dead slot. Neither ANM nor its effect timer has run here.
                next.active = false;
            }
        }
        if (next.active) {
            const auto status = advance_animation(next);
            if (status != UpdateStatus::advanced)
                return {status};
        }
        result.retired += next.active ? 0 : 1;
        scratch_.push_back({index, next});
    }
    // As with spawn staging, a later blocked slot discards all provisional
    // earlier writes. This rollback is our interface contract, not engine behavior.
    for (const auto &delta : scratch_)
        slots_[delta.index] = delta.slot;
    active_count_ -= result.retired;
    return result;
}
} // namespace th08::effect
