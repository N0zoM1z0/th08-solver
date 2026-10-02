#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <th08/spell_scenario.hpp>

namespace th08::spell {
namespace {
namespace r = resources;
namespace vm = emitter;
namespace tr = bullet::transform;
void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}
std::uint32_t bits(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
void hash(std::uint64_t &digest, std::uint64_t value) {
    digest = (digest ^ value) * 1099511628211ULL;
}
r::Bytes member(const r::Archive &archive, const char *name) {
    for (std::size_t i = 0; i < archive.entries().size(); ++i)
        if (archive.entries()[i].name == name)
            return archive.decode(i);
    throw std::runtime_error(std::string("missing resource: ") + name);
}
bool collision(const std::vector<geometry::Hazard> &hazards, geometry::Vec2 player,
               geometry::Vec2 half_size) {
    for (const auto &hazard : hazards) {
        require(hazard.kind == geometry::Hazard::Kind::bullet,
                "spell projection gained a non-box hazard");
        if (geometry::box_hit(player, half_size, hazard.box))
            return true;
    }
    return false;
}
std::array<double, 9> operands(const vm::Operation &op, const vm::Workspace &storage,
                               random::Rng &rng, bool shot = false) {
    std::array<vm::OperandField, 9> fields{};
    const std::size_t count = shot ? 9 : op.payload_size / 4;
    require(count <= fields.size(), "unexpected spell operand payload");
    for (std::size_t i = 0; i < count; ++i) {
        const bool floating = shot               ? i >= 4 && i <= 7
                              : op.opcode == 111 ? i >= 5
                              : op.opcode == 66  ? i >= 2
                              : op.opcode == 140 ? i >= 3
                                                 : op.opcode == 82;
        fields[i] = {std::uint16_t(i * 4),
                     floating ? vm::OperandType::float32 : vm::OperandType::signed32,
                     std::int8_t(i)};
        if (shot) {
            fields[i].byte_offset = std::uint16_t(i < 2 ? i * 2 : (i - 1) * 4);
            if (i < 2)
                fields[i].type = vm::OperandType::signed16;
            if (i == 8)
                fields[i].flag_index = -1;
        }
    }
    std::array<double, 9> values{};
    require(vm::decode_operands(op, storage, fields.data(), values.data(), count, &rng) ==
                vm::Status::operands_decoded,
            "spell operands require unsupported context");
    return values;
}
} // namespace

Program::Program(const std::filesystem::path &path) {
    auto bytes = r::read_file(path);
    dat_hash = r::sha256(r::view(bytes));
    require(dat_hash == "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb",
            "spell DAT identity mismatch");
    r::Archive archive(std::move(bytes));
    const auto ecl_bytes = member(archive, "ecldata7sp.ecl");
    ecl_hash = r::sha256(r::view(ecl_bytes));
    require(ecl_hash == "7f1a847fdd7ceb5e35dfd3529a54961ab4d1c9e7607fbcfb577936465326ab0e",
            "spell ECL identity mismatch");
    const auto parsed = r::parse_ecl(r::view(ecl_bytes));
    ecl = vm::Module(r::view(ecl_bytes), parsed);
    const auto &start = ecl.subs.at(72).code.at(10);
    require(start.opcode == 122 && start.offset == 56084 && start.mask == 0xf1 &&
                r::u16(ecl.subs[72].payload(10), 2) == 179,
            "ID179 occurrence mismatch");
    const auto anm_bytes = member(archive, "etama.anm");
    const auto anm = r::parse_anm(r::view(anm_bytes));
    anm_hash = r::sha256(r::view(anm_bytes));
    spawn = animation::certify_timing(r::view(anm_bytes), anm, 21);
    const auto main = animation::certify_timing(r::view(anm_bytes), anm, 2);
    require(spawn.status == animation::Status::certified && spawn.calls_after_template == 10 &&
                main.status == animation::Status::certified && main.sprite == 32 &&
                main.completion_time == 0 && !main.hides_on_completion,
            "type2 ANM projection is not certified");
    for (std::size_t i = 0; i < sprite_sizes.size(); ++i) {
        const auto sprite_id = i == 0 ? 34U : 38U;
        const auto &sprite = anm.sprites.at(sprite_id);
        require(sprite.raw_id == sprite_id, "type2 sprite identity mismatch");
        sprite_sizes[i] = {sprite.width, sprite.height};
    }
    const auto sht_bytes = member(archive, "ply00a.sht");
    sht_hash = r::sha256(r::view(sht_bytes));
    player = r::parse_sht(r::view(sht_bytes)).header;
}

World::World(const Program &program, std::uint16_t gameplay_seed, std::uint16_t visual_seed)
    : program_(&program), gameplay_(gameplay_seed), visual_(visual_seed) {
    main_ = vm::begin(program.ecl, 72, main_storage_, 1);
    vm::initialize_spawn_scalars(main_storage_);
    // Supplied checkpoint AFTER sub72 PC0..9 and BEFORE the exact spell occurrence.
    // Prefix has no nonzero displacement: move-to target equals position(192,224).
    main_.pc = 10;
    main_storage_.registers[39] = 0;
    main_storage_.initialized[39] = true;
    particles_.reserve(1120);
    hazards_.reserve(1120);
}
void World::visual_request(std::int32_t kind, std::uint32_t count) {
    // Named controlled hook: one separate-stream U32 per requested visual particle.
    // This is intentionally not the retail shared-stream effect/ANM implementation.
    stats_.visual_requests += count;
    hash(stats_.visual_digest, std::uint32_t(kind));
    hash(stats_.visual_digest, stats_.frame);
    for (std::uint32_t i = 0; i < count; ++i)
        hash(stats_.visual_digest, visual_.next_u32());
}
void World::shoot(const vm::Operation &op, const vm::Workspace &storage) {
    const auto v = operands(op, storage, gameplay_, true);
    require(active_spell_ && !player_terminal_ && minimum_distance_ == 0 && op.opcode == 99 &&
                v[0] == 2 && (v[1] == 2 || v[1] == 6) && v[2] == 8 && v[3] == 1 && v[4] == 1 &&
                v[5] == .5 && v[8] == 546,
            "shot is outside the controlled ID179 dispatch contract");
    require(stats_.frame >= 162 && (stats_.frame - 162) % 15 == 0 &&
                op.offset == (stats_.shot_commands % 2 == 0 ? 57448U : 57568U),
            "DAT shot event timing or pair ordering changed");
    stats_.last_shot = stats_.frame;
    // Active spell disables rank adjustment; circle mode never reads player position.
    const kinematics::Pattern pattern{
        kinematics::Aim::circle, 8, 1, float(v[4]), float(v[5]), float(v[6]), float(v[7])};
    ++stats_.shot_commands;
    ++sound_requests_; // Flag0x200 requests sound; no simulation RNG or geometry effect.
    for (int spoke = 0; spoke < 8; ++spoke) {
        require(stats_.allocated < 1120 && particles_.size() < 1536,
                "source-derived empty-pool allocation bound was exceeded");
        kinematics::Launch launch{};
        require(kinematics::launch(pattern, spoke, 0, 0, 1, {}, launch) ==
                    kinematics::Status::ready,
                "circle launch failed");
        Particle p;
        p.program = transforms_;
        p.state.flight = {192 - launch.velocity_x * 4,
                          224 - launch.velocity_y * 4,
                          launch.velocity_x,
                          launch.velocity_y,
                          launch.angle,
                          launch.speed};
        p.state.enabled_flags = 546;
        p.sprite_size = program_->sprite_sizes[v[1] == 2 ? 0 : 1];
        p.state.sprite_width = p.sprite_size.x;
        p.state.sprite_height = p.sprite_size.y;
        p.spawn_remaining = program_->spawn.calls_after_template;
        const auto installed = tr::advance_program(p.program, p.state, 1);
        require(installed.status == bullet::Status::advanced && installed.sound_count == 0 &&
                    p.state.active_flags == tr::polar && p.state.pc == 1,
                "polar installation failed");
        // Total births<1536, no wrap: source circular allocation selects successive
        // never-used slots even when earlier slots have retired. No pool-full shortcut.
        particles_.push_back(p);
        ++stats_.allocated;
    }
}
void World::effect(vm::Execution &execution, vm::Workspace &storage, bool child_context) {
    const auto &op = *vm::pending_operation(execution);
    if (op.opcode == 122) {
        require(!child_context && execution.active == &program_->ecl.subs[72] &&
                    execution.pc == 10 && op.offset == 56084 && !active_spell_,
                "unexpected spell start");
        active_spell_ = true;
        capture_valid_ = true;
        visual_request(122, 1);
        return;
    }
    if (op.opcode == 99) {
        require(child_context, "unexpected main-context shot");
        shoot(op, storage); // Decode once, preserving source operand read order.
        return;
    }
    const auto v = operands(op, storage, gameplay_);
    switch (op.opcode) {
    case 155:
        require(v[0] == 1 && active_spell_, "unexpected timeout-spell flag");
        timeout_spell_ = true;
        break;
    case 160:
        require(v[0] == 120, "unexpected damage-reduction timer");
        damage_reduction_ = 120;
        break;
    case 139:
        require(!child_context && execution.active == &program_->ecl.subs[33] && v[0] == 17 &&
                    v[1] == 4 && v[2] == -1,
                "unsupported visual-charge effect");
        visual_request(17, 4);
        break;
    case 135:
        require(!child_context && !child_active_ && v[0] == 0 && v[1] == 73,
                "unsupported child-context installation");
        child_ = vm::begin(program_->ecl, 73, child_storage_, 1);
        vm::restore_context(vm::capture_context(storage), child_storage_);
        child_active_ = true;
        stats_.child_start = stats_.frame;
        break;
    case 82:
        require(v[0] == 0, "unexpected distance gate");
        minimum_distance_ = 0;
        break;
    case 111: {
        require(child_context && v[0] == 0 && v[1] == 32 && v[2] == 0 && v[3] == 640 &&
                    v[4] == -1 && float(v[5]) == .02f,
                "unsupported bullet transform");
        transforms_.records[0] = {float(v[5]),        float(v[6]),         std::int32_t(v[3]),
                                  std::int32_t(v[4]), std::uint32_t(v[1]), std::int32_t(v[2])};
        break;
    }
    case 184:
        require(stats_.timeout_entered && v[0] == 1, "unexpected bonus-update control");
        bonus_updates_disabled_ = true;
        break;
    case 129:
        require(stats_.timeout_entered && v[0] == 0, "unexpected death mode");
        death_mode_ = 0;
        break;
    case 130:
        require(stats_.timeout_entered && v[0] == -1, "unexpected death callback");
        death_callback_ = -1;
        break;
    case 80:
        require(stats_.timeout_entered && v[0] == 3, "unexpected interaction change");
        interaction_flags_ &= ~3U;
        break;
    case 66:
        require(stats_.timeout_entered && v[0] == 60 && v[1] == 4 && float(v[3]) == .15f,
                "unexpected terminal movement");
        // The handler installs the departure before ending this same ECL phase.
        // No subsequent actor movement phase belongs to the survival segment.
        departure_duration_ = std::int32_t(v[0]);
        departure_angle_ = float(v[2]);
        departure_speed_ = float(v[3]);
        break;
    case 173:
        require(stats_.timeout_entered && v[0] == 0, "unexpected terminal timer control");
        timer_paused_ = false;
        break;
    case 124:
        require(stats_.timeout_entered && (v[0] == 7 || v[0] == 18), "unexpected terminal sound");
        ++sound_requests_;
        break;
    case 140:
        require(stats_.timeout_entered && v[0] == 26 && (v[1] == 1 || v[1] == 64),
                "unsupported terminal visual effect");
        visual_request(26, std::uint32_t(v[1]));
        break;
    case 123:
        require(stats_.timeout_entered && active_spell_, "unexpected spell end");
        active_spell_ = false;
        timeout_spell_ = false;
        bonus_updates_disabled_ = false;
        // Timeout-spell skipped the ordinary timeout transition flag. EndSpell
        // therefore starts bullet cancellation, retaining occupied despawn slots.
        // Reward/item effects and post-spell despawn animation are outside this
        // controlled survival boundary; never pretend those slots became unused.
        for (auto &particle : particles_)
            if (particle.live) {
                particle.despawning = true;
                ++stats_.despawning;
            }
        stats_.spell_ended = true;
        break;
    case 127:
        require(stats_.spell_ended && v[0] == -1, "unexpected boss removal");
        boss_slot_ = -1;
        stats_.boss_removed = true;
        break;
    default: {
        std::ostringstream message;
        message << "unsupported ID179 world opcode " << op.opcode << " ecldata7sp.ecl sub"
                << (execution.active - program_->ecl.subs.data()) << " PC" << execution.pc
                << " offset" << op.offset << " instruction_mask=" << unsigned(op.mask)
                << " execution_mask=" << unsigned(execution.mask) << " frame=" << stats_.frame
                << " gameplay_seed=" << gameplay_.seed()
                << " gameplay_draws=" << gameplay_.generation_count();
        throw std::runtime_error(message.str());
    }
    }
}
void World::execute(vm::Execution &execution, vm::Workspace &storage, bool child_context) {
    for (;;) {
        const auto result =
            vm::advance(execution, storage, &gameplay_, 1000000, vm::Effects::yield_to_world);
        if (result.status == vm::Status::frame_complete)
            return;
        if (result.status != vm::Status::external_effect) {
            std::ostringstream message;
            message << "ID179 scalar stop " << vm::name(result.status) << " ecldata7sp.ecl sub"
                    << (execution.active - program_->ecl.subs.data()) << " PC" << execution.pc
                    << " offset" << result.offset << " opcode" << result.opcode
                    << " execution_mask=" << unsigned(execution.mask) << " frame=" << stats_.frame
                    << " gameplay_seed=" << gameplay_.seed()
                    << " gameplay_draws=" << gameplay_.generation_count();
            throw std::runtime_error(message.str());
        }
        effect(execution, storage, child_context);
        require(vm::acknowledge_effect(execution, result.executed), "world token mismatch");
        if (stats_.boss_removed)
            return; // Explicit end-of-spell boundary, before menu/engine-field10051 tail.
    }
}
void World::move_particles() {
    hazards_.clear();
    for (auto &p : particles_) {
        if (!p.live)
            continue;
        auto &flight = p.state.flight;
        if (p.spawn_remaining != 0) {
            flight.x += flight.velocity_x * .5f;
            flight.y += flight.velocity_y * .5f;
            if (--p.spawn_remaining != 0)
                continue;
        }
        const auto result = tr::step(p.program, p.state, 1, false);
        require(result.status == bullet::Status::advanced && result.sound_count == 0 &&
                    (p.state.active_flags & ~tr::polar) == 0,
                "unsupported fired polar motion");
        flight.x += flight.velocity_x;
        flight.y += flight.velocity_y;
        // Polar acceleration does not grant the turn/bounce128-frame outside grace.
        if (flight.x + p.sprite_size.x / 2 < 0 || flight.x - p.sprite_size.x / 2 > 384 ||
            flight.y + p.sprite_size.y / 2 < 0 || flight.y - p.sprite_size.y / 2 > 448) {
            p.live = false;
            ++stats_.retired;
            continue;
        }
        hazards_.push_back(geometry::Hazard::bullet({{flight.x, flight.y}, {4, 4}}));
    }
    stats_.peak_lethal = std::max(stats_.peak_lethal, std::uint32_t(hazards_.size()));
    if (!hazards_.empty() && stats_.first_lethal == duration)
        stats_.first_lethal = stats_.frame;
    hash(stats_.hazard_digest, stats_.frame);
    hash(stats_.hazard_digest, hazards_.size());
    for (const auto &hazard : hazards_) {
        hash(stats_.hazard_digest, bits(hazard.box.center.x));
        hash(stats_.hazard_digest, bits(hazard.box.center.y));
    }
}
const std::vector<geometry::Hazard> &World::advance() {
    require(stats_.frame < duration && !player_terminal_, "survival frame outside segment");
    execute(main_, main_storage_, false);
    if (child_active_)
        execute(child_, child_storage_, true);
    if (damage_reduction_ > 0)
        --damage_reduction_;
    move_particles();
    ++stats_.frame;
    return hazards_;
}
void World::finish_timeout() {
    require(stats_.frame == duration && timer_limit_ == duration && timeout_spell_ &&
                death_callback_ == 1 && !stats_.timeout_entered,
            "invalid timeout boundary");
    execute(main_, main_storage_, false);
    if (child_active_)
        execute(child_, child_storage_, true);
    stats_.gameplay_draws_before_callback = gameplay_.generation_count();
    require(stats_.gameplay_draws_before_callback == 4 && stats_.last_shot == 1197,
            "pre-callback gameplay RNG or emission schedule changed");
    // Source order: timeout after ECL/movement; child blocks are released before
    // callback re-entry. Timeout-spell path preserves bullets but player is state3.
    child_active_ = false;
    player_terminal_ = true;
    stats_.timeout_entered = true;
    timer_limit_ = 0;
    transforms_ = {};
    // CallEclSub resets PC/clocks and the callback clears the call stack, but it
    // does not zero the actor's context/entity scalars like a new spawn would.
    const vm::ScalarStorage inherited = main_storage_;
    main_ = vm::begin(program_->ecl, 1, main_storage_, 1);
    main_storage_.registers = inherited.registers;
    main_storage_.initialized = inherited.initialized;
    main_storage_.registers[99] = capture_valid_ ? 1 : 0;
    main_storage_.initialized[99] = true;
    execute(main_, main_storage_, false);
    require(stats_.spell_ended && stats_.boss_removed && !active_spell_ && boss_slot_ == -1 &&
                stats_.allocated == 1120 && stats_.shot_commands == 140 &&
                stats_.child_start == 162,
            "complete controlled spell boundary mismatch");
}
std::uint64_t World::state_digest() const {
    std::uint64_t value = stats_.hazard_digest;
    hash(value, stats_.visual_digest);
    hash(value, stats_.frame);
    hash(value, stats_.allocated);
    hash(value, stats_.retired);
    hash(value, stats_.despawning);
    hash(value, gameplay_.seed());
    hash(value, gameplay_.generation_count());
    hash(value, visual_.seed());
    hash(value, visual_.generation_count());
    hash(value, main_.pc);
    hash(value, main_.local_time);
    hash(value, child_active_);
    hash(value, player_terminal_);
    hash(value, active_spell_);
    hash(value, stats_.boss_removed);
    hash(value, bonus_updates_disabled_);
    hash(value, timer_paused_);
    hash(value, bits(departure_angle_));
    hash(value, bits(departure_speed_));
    hash(value, departure_duration_);
    for (const auto &p : particles_) {
        hash(value, p.live);
        hash(value, p.despawning);
        hash(value, p.spawn_remaining);
        hash(value, bits(p.state.flight.x));
        hash(value, bits(p.state.flight.y));
        hash(value, bits(p.state.flight.speed));
        hash(value, bits(p.state.flight.angle));
        hash(value, p.state.acceleration[2].timer);
        hash(value, p.state.active_flags);
    }
    return value;
}

namespace {
void record(const World &world, Result &result) {
    result.statistics = world.statistics();
    result.gameplay = world.gameplay_rng();
    result.visual = world.visual_rng();
    result.world_digest = world.state_digest();
}
} // namespace
Result solve(const Program &program, const Options &options, std::uint16_t gameplay_seed,
             std::uint16_t visual_seed) {
    require(options.horizon > 0 && options.horizon <= duration && options.commit > 0 &&
                options.commit <= options.horizon && options.beam > 0 && options.beam <= 4096,
            "invalid controlled spell planning options");
    World world(program, gameplay_seed, visual_seed);
    Result result;
    geometry::Vec2 position{192, 400};
    const geometry::Vec2 half{program.player.hurtbox_size / 2, program.player.hurtbox_size / 2};
    const solver::Movement movement{program.player.focused_axis, program.player.focused_diagonal};
    while (world.statistics().frame < duration) {
        World forecast = world;
        solver::Model model;
        model.dependency = solver::Dependency::independent; // Controlled projection only.
        model.epoch = world.statistics().frame;
        const auto horizon = std::min(options.horizon, std::size_t(duration - model.epoch));
        model.frames.reserve(horizon);
        for (std::size_t i = 0; i < horizon; ++i)
            model.frames.emplace_back(forecast.advance(), half);
        solver::Options planning;
        planning.beam = options.beam;
        planning.expansions = options.expansion_budget;
        planning.expected_epoch = model.epoch;
        planning.terminal = {{192, 400}, {368, 416}};
        const auto path = solver::plan(model, position, movement, planning);
        ++result.replans;
        result.expansions += path.expansions;
        result.collision_queries += path.collision_queries;
        result.search_status = path.status;
        if (path.status != solver::Status::found)
            break;
        for (std::size_t i = 0; i < std::min(options.commit, path.actions.size()); ++i) {
            position = solver::advance(position, path.actions[i], movement);
            const auto &hazards = world.advance();
            result.actions.push_back(path.actions[i]);
            result.positions.push_back(position);
            if (collision(hazards, position, half)) {
                result.status = "PLAYER_DEAD";
                record(world, result);
                result.replayed = replay(program, result, gameplay_seed, visual_seed);
                require(result.replayed, "regenerated death-prefix replay failed");
                return result;
            }
        }
    }
    if (world.statistics().frame == duration) {
        world.finish_timeout();
        result.status = "SURVIVED_CONTROLLED_TIMEOUT";
    }
    record(world, result);
    result.replayed = replay(program, result, gameplay_seed, visual_seed);
    require(result.replayed, "regenerated unindexed spell replay failed");
    return result;
}
Result baseline(const Program &program, bool greedy, std::uint16_t gameplay_seed,
                std::uint16_t visual_seed) {
    World world(program, gameplay_seed, visual_seed);
    Result result;
    geometry::Vec2 position{192, 400};
    const geometry::Vec2 half{program.player.hurtbox_size / 2, program.player.hurtbox_size / 2};
    const solver::Movement movement{program.player.focused_axis, program.player.focused_diagonal};
    while (world.statistics().frame < duration) {
        const auto &hazards = world.advance();
        solver::Action selected{};
        double best = std::numeric_limits<double>::infinity();
        if (greedy) {
            for (int y = -1; y <= 1; ++y)
                for (int x = -1; x <= 1; ++x) {
                    const auto next = solver::advance(position, {x, y}, movement);
                    ++result.expansions;
                    ++result.collision_queries;
                    if (collision(hazards, next, half))
                        continue;
                    const double dx = double(next.x) - 192, dy = double(next.y) - 400;
                    const double score = dx * dx + dy * dy;
                    if (score < best) {
                        best = score;
                        selected = {x, y};
                    }
                }
        }
        position = solver::advance(position, selected, movement);
        result.actions.push_back(selected);
        result.positions.push_back(position);
        if (collision(hazards, position, half)) {
            result.status = "PLAYER_DEAD";
            record(world, result);
            result.replayed = replay(program, result, gameplay_seed, visual_seed);
            require(result.replayed, "baseline regenerated death-prefix replay failed");
            return result;
        }
    }
    world.finish_timeout();
    result.status = "SURVIVED_CONTROLLED_TIMEOUT";
    record(world, result);
    result.replayed = replay(program, result, gameplay_seed, visual_seed);
    require(result.replayed, "baseline regenerated replay failed");
    return result;
}
bool replay(const Program &program, const Result &result, std::uint16_t gameplay_seed,
            std::uint16_t visual_seed) {
    if (result.actions.size() > duration || result.positions.size() != result.actions.size() ||
        result.statistics.frame != result.actions.size())
        return false;
    World replayed(program, gameplay_seed, visual_seed);
    geometry::Vec2 position{192, 400};
    const geometry::Vec2 half{program.player.hurtbox_size / 2, program.player.hurtbox_size / 2};
    const solver::Movement movement{program.player.focused_axis, program.player.focused_diagonal};
    bool died = false;
    for (std::size_t tick = 0; tick < result.actions.size(); ++tick) {
        const auto action = result.actions[tick];
        if (action.x < -1 || action.x > 1 || action.y < -1 || action.y > 1)
            return false;
        position = solver::advance(position, action, movement);
        if (position.x != result.positions[tick].x || position.y != result.positions[tick].y)
            return false;
        if (collision(replayed.advance(), position, half)) {
            if (result.status != "PLAYER_DEAD" || tick + 1 != result.actions.size())
                return false;
            died = true;
        }
    }
    if (result.status == "SURVIVED_CONTROLLED_TIMEOUT") {
        if (result.actions.size() != duration || died)
            return false;
        replayed.finish_timeout();
    } else if (result.status == "PLAYER_DEAD") {
        if (!died)
            return false;
    } else if (result.status != "SEARCH_LIMIT" || died || result.actions.size() == duration) {
        return false;
    }
    return replayed.state_digest() == result.world_digest &&
           replayed.statistics().hazard_digest == result.statistics.hazard_digest;
}
} // namespace th08::spell
