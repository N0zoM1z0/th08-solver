#include <cmath>
#include <stdexcept>
#include <th08/practice_entry.hpp>

namespace th08::practice {
namespace {
Status scalar_status(emitter::Status status) {
    switch (status) {
    case emitter::Status::missing_context:
        return Status::missing_entry_state;
    case emitter::Status::instruction_limit:
        return Status::instruction_limit;
    case emitter::Status::unsupported:
        return Status::unsupported_world_effect;
    default:
        return Status::invalid;
    }
}
void publish(Actor &actor) {
    actor.scalars.registers[51] = actor.life;
    actor.scalars.initialized[51] = true;
    // Other computed/global selectors remain unknown until their owner exists.
}
bool source_spawn_finished(Status status) {
    return status == Status::spawned || status == Status::pool_full ||
           status == Status::source_spawn_failed;
}
std::int32_t signed_low_word(std::int32_t value) {
    const auto low = std::uint32_t(value) & 0xffffU;
    return low < 0x8000U ? std::int32_t(low) : std::int32_t(low) - 0x10000;
}
} // namespace

Programs::Programs(resources::View bytes, const resources::Ecl &decoded, std::size_t timeline_id)
    : ecl(bytes, decoded), timeline(bytes, decoded, timeline_id) {}

const char *name(Status status) {
    switch (status) {
    case Status::timeline_frame_complete:
        return "TIMELINE_FRAME_COMPLETE";
    case Status::spawn_pending:
        return "SPAWN_PENDING";
    case Status::spawned:
        return "SPAWNED";
    case Status::pool_full:
        return "POOL_FULL";
    case Status::source_spawn_failed:
        return "SOURCE_SPAWN_FAILED";
    case Status::unsupported_world_effect:
        return "UNSUPPORTED_WORLD_EFFECT";
    case Status::missing_entry_state:
        return "MISSING_ENTRY_STATE";
    case Status::unsupported_root_return:
        return "UNSUPPORTED_ROOT_RETURN";
    case Status::instruction_limit:
        return "INSTRUCTION_LIMIT";
    case Status::invalid:
        return "INVALID";
    case Status::busy:
        return "BUSY";
    }
    return "INVALID";
}

SpawnPool::SpawnPool(std::shared_ptr<const Programs> programs, SharedCallParameters shared_calls)
    : programs_(std::move(programs)), actors_(enemy_capacity), shared_calls_(shared_calls) {
    if (!programs_)
        throw std::invalid_argument("spawn pool requires immutable programs");
}
const Actor &SpawnPool::actor(std::size_t index) const {
    return actors_.at(index);
}
std::size_t SpawnPool::active_count() const {
    std::size_t count = 0;
    for (const auto &actor : actors_)
        count += actor.active;
    return count;
}
Result SpawnPool::identify(Status status) const {
    Result result;
    result.status = status;
    if (!pending_)
        return result;
    result.actor = pending_->actor;
    const auto &execution = actors_[result.actor].execution;
    result.execution_mask = execution.mask;
    result.local_time = execution.local_time;
    result.pc = execution.pc;
    for (std::size_t sub = 0; sub < programs_->ecl.subs.size(); ++sub)
        if (&programs_->ecl.subs[sub] == execution.active) {
            result.sub = sub;
            break;
        }
    if (execution.active && execution.pc < execution.active->code.size()) {
        const auto &op = execution.active->code[execution.pc];
        result.opcode = op.opcode;
        result.offset = op.offset;
        result.instruction_mask = op.mask;
    }
    return result;
}

Result SpawnPool::begin(const SpawnRequest &request, std::uint8_t mask) {
    if (pending_)
        return identify(Status::busy);
    const auto sub = signed_low_word(request.sub);
    if (mask == 0 || !enemy::detail::finite(request.position) ||
        (request.kind != SpawnKind::ordinary && request.kind != SpawnKind::inherited_context))
        return {Status::invalid};
    if (request.kind == SpawnKind::inherited_context && !request.inherited)
        return {Status::missing_entry_state};
    if (request.kind == SpawnKind::inherited_context && request.mirror_x)
        return {Status::invalid}; // SpawnEnemy2 has no mirror argument.
    // Match source scan order. A full pool never resolves/dereferences a sub ID.
    std::size_t slot = 0;
    while (slot < enemy_capacity && actors_[slot].active)
        ++slot;
    if (slot == enemy_capacity) {
        spawn_event_ = last_spawn_failed_ = true;
        return {Status::pool_full, no_actor};
    }
    // Source would leave a null instruction for a negative narrowed ID or index
    // outside its table. Reject instead of interpreting that undefined behavior.
    if (sub < 0 || std::size_t(sub) >= programs_->ecl.subs.size() ||
        programs_->ecl.subs[sub].code.empty())
        return {Status::invalid};
    Actor next;
    next.active = true;
    next.index = slot;
    next.motion.timer.previous = 0; // Source template memset, not ZunTimer construction.
    next.motion.position = request.position;
    next.motion.mirror_x = request.mirror_x;
    if (request.life >= 0)
        next.life = request.life;
    next.execution = emitter::begin(programs_->ecl, std::size_t(sub), next.scalars, mask);
    emitter::initialize_spawn_scalars(next.scalars);
    if (request.kind == SpawnKind::inherited_context)
        emitter::restore_context(*request.inherited, next.scalars);
    for (std::size_t i = 0; i < shared_calls_.values.size(); ++i) {
        next.scalars.registers[61 + i] = shared_calls_.values[i];
        next.scalars.initialized[61 + i] = shared_calls_.known[i];
    }
    publish(next);
    actors_[slot] = std::move(next);
    pending_ = PendingSpawn{request, slot};
    spawn_event_ = true;
    // lastSpawnFailed is written only AFTER immediate RunEcl, not at selection.
    return identify(Status::spawn_pending);
}

world::EffectStatus SpawnPool::apply_effect(Actor &actor, random::Rng *rng,
                                            const enemy::Vec3 *player) {
    const auto *op = emitter::pending_operation(actor.execution);
    if (!op)
        return world::EffectStatus::invalid;
    const auto motion =
        world::apply_motion_effect(actor.execution, actor.scalars, actor.motion, rng, player);
    if (motion != world::EffectStatus::not_handled)
        return motion;
    if (op->opcode != 77 && op->opcode != 78 && op->opcode != 79 && op->opcode != 80 &&
        op->opcode != 81 && op->opcode != 131)
        return world::EffectStatus::not_handled;

    const bool hitbox = op->opcode == 77 || op->opcode == 78;
    const emitter::OperandField fields[] = {
        {0, hitbox ? emitter::OperandType::float32 : emitter::OperandType::signed32, 0},
        {4, emitter::OperandType::float32, 1}};
    double values[2];
    const auto decoded =
        emitter::decode_operands(*op, actor.scalars, fields, values, hitbox ? 2 : 1, rng,
                                 emitter::OperandOrder::source_ordered_fields,
                                 actor.execution.active->payload(actor.execution.pc));
    if (decoded != emitter::Status::operands_decoded)
        return decoded == emitter::Status::missing_context ? world::EffectStatus::missing_context
               : decoded == emitter::Status::unsupported   ? world::EffectStatus::unsupported
                                                           : world::EffectStatus::invalid;
    if (hitbox) {
        auto &box = op->opcode == 77 ? actor.hitbox : actor.secondary_hitbox;
        box.x = float(values[0]);
        box.y = float(values[1]);
    } else if (op->opcode == 131) {
        // No accepted prefix opcode can install boss ownership. The GUI branch
        // is therefore unreachable in this restricted spawn domain.
        actor.life = actor.max_life = actor.phase_starting_life = std::int32_t(values[0]);
        publish(actor);
    } else {
        const auto bits = std::uint32_t(std::int32_t(values[0]));
        auto &flags = actor.interaction;
        const bool set = op->opcode == 79, enable = op->opcode == 81;
        if (set || (bits & 1))
            flags.accepts_damage = set ? !(bits & 1) : enable;
        if (set || (bits & 2))
            flags.collision = set ? !(bits & 2) : enable;
        if (set || (bits & 4))
            flags.damageable = set ? !(bits & 4) : enable;
        if (set || (bits & 8))
            flags.no_sprite = set ? bool(bits & 8) : !enable;
        if (set || (bits & 16))
            flags.allow_offscreen = set ? bool(bits & 16) : !enable;
        if (set || (bits & 32))
            flags.no_death = set ? bool(bits & 32) : !enable;
        // Alignment-effect creation is unsupported, so there is no attached VM
        // whose collision flag would also need changing here.
    }
    return emitter::acknowledge_effect(actor.execution, actor.execution.result.executed)
               ? world::EffectStatus::applied
               : world::EffectStatus::invalid;
}

Result SpawnPool::resume(random::Rng *rng, const enemy::Vec3 *player,
                         std::uint32_t instruction_limit) {
    if (!pending_)
        return {Status::invalid};
    auto &actor = actors_[pending_->actor];
    for (;;) {
        if (enemy::refresh_world(actor.motion) != enemy::Status::advanced ||
            world::publish_motion(actor.motion, actor.scalars, player) !=
                world::EffectStatus::applied)
            return identify(Status::invalid);
        publish(actor);
        const auto result = emitter::advance(actor.execution, actor.scalars, rng, instruction_limit,
                                             emitter::Effects::yield_to_world);
        // These eight source globals are shared between actors, not part of the
        // inherited thirty-word block. Completed writes survive a later stop or
        // source termination, exactly like other committed prefix effects.
        for (std::size_t i = 0; i < shared_calls_.values.size(); ++i) {
            shared_calls_.values[i] = actor.scalars.registers[61 + i];
            shared_calls_.known[i] = actor.scalars.initialized[61 + i];
        }
        if (result.status == emitter::Status::external_effect) {
            const auto status = apply_effect(actor, rng, player);
            if (status != world::EffectStatus::applied)
                return identify(
                    status == world::EffectStatus::missing_context ? Status::missing_entry_state
                    : status == world::EffectStatus::invalid       ? Status::invalid
                                                             : Status::unsupported_world_effect);
            continue;
        }
        if (result.terminated) {
            actor.active = false;
            last_spawn_failed_ = true;
            auto stopped = identify(Status::source_spawn_failed);
            pending_.reset();
            return stopped;
        }
        if (result.status == emitter::Status::returned)
            return identify(Status::unsupported_root_return);
        if (result.status != emitter::Status::frame_complete)
            return identify(scalar_status(result.status));

        // Every supported opcode preserves the source template's null callback,
        // eight null interpolators, absent child blocks, zero periodic-shot
        // interval and moveLeft=-1. Thus their RunEcl tail branches do no work.
        // Velocity update still runs; displacement belongs to EnemyManager, NOT
        // SpawnEnemy1/2. Do not add a new opcode without rechecking this closure.
        if (enemy::update_velocity(actor.motion, 1, false) != enemy::Status::advanced)
            return identify(Status::invalid);
        actor.display_color = actor.vm_color;
        const auto &request = pending_->request;
        const auto low = std::uint32_t(request.item_drop) & 255U;
        actor.item_drop = std::int8_t(low < 128 ? int(low) : int(low) - 256);
        if (request.kind == SpawnKind::inherited_context && request.life >= 0)
            actor.life = request.life;
        if (request.score >= 0)
            actor.score = request.score;
        actor.max_life = actor.phase_starting_life = actor.life;
        last_spawn_failed_ = false;
        auto done = identify(Status::spawned);
        pending_.reset();
        return done;
    }
}

Entry::Entry(std::shared_ptr<const Programs> programs, std::uint8_t mask)
    : programs_(std::move(programs)), pool_(programs_), mask_(mask) {
    if (!mask_)
        throw std::invalid_argument("entry requires an explicit nonzero execution mask");
}
Result Entry::advance(timeline::Context &observations, random::Rng *rng, const enemy::Vec3 *player,
                      std::uint32_t instruction_limit) {
    // This budget bounds same-frame external operations as well as scalar loops.
    for (std::uint32_t handoffs = 0; handoffs < instruction_limit; ++handoffs) {
        if (awaiting_spawn_) {
            const auto result = pool_.resume(rng, player, instruction_limit);
            if (!source_spawn_finished(result.status))
                return result;
            if (!timeline::acknowledge_effect(programs_->timeline, timeline_,
                                              timeline_.effect_token))
                return {Status::invalid};
            awaiting_spawn_ = false;
        }
        const auto step = timeline::advance(programs_->timeline, timeline_, observations, mask_, 1,
                                            false, instruction_limit);
        Result result;
        result.pc = step.pc;
        result.offset = step.offset;
        result.opcode = std::int16_t(step.opcode);
        result.local_time = timeline_.time.current;
        result.execution_mask = mask_;
        if (step.pc < programs_->timeline.code.size())
            result.instruction_mask = programs_->timeline.code[step.pc].mask;
        if (step.status != timeline::Status::external_effect) {
            switch (step.status) {
            case timeline::Status::frame_complete:
                result.status = Status::timeline_frame_complete;
                break;
            case timeline::Status::requires_context:
                result.status = Status::missing_entry_state;
                break;
            case timeline::Status::unsupported:
                result.status = Status::unsupported_world_effect;
                break;
            case timeline::Status::instruction_limit:
                result.status = Status::instruction_limit;
                break;
            default:
                result.status = Status::invalid;
                break;
            }
            return result;
        }
        const auto *op = timeline::pending_operation(programs_->timeline, timeline_);
        if (!op || (op->opcode != 0 && op->opcode != 1 && op->opcode != 15)) {
            result.status = Status::unsupported_world_effect;
            return result;
        }
        const auto payload = programs_->timeline.payload(timeline_.pc);
        SpawnRequest request;
        request.sub = resources::i32(payload, 0);
        request.position = {resources::f32(payload, 4), resources::f32(payload, 8), 0};
        request.life = resources::i32(payload, 12);
        request.item_drop = resources::i32(payload, 16);
        request.score = resources::i32(payload, 20);
        request.mirror_x = op->opcode == 1;
        const auto begun = pool_.begin(request, mask_);
        if (begun.status == Status::spawn_pending) {
            awaiting_spawn_ = true;
        } else if (begun.status == Status::pool_full) {
            if (!timeline::acknowledge_effect(programs_->timeline, timeline_,
                                              timeline_.effect_token))
                return {Status::invalid};
        } else {
            result.status = begun.status;
            return result;
        }
    }
    return {Status::instruction_limit};
}
} // namespace th08::practice
