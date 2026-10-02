#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <th08/effect_animation.hpp>
#include <th08/practice_entry.hpp>

namespace p = th08::practice;
namespace e = th08::emitter;
namespace r = th08::resources;
namespace t = th08::timeline;
namespace fx = th08::effect;
namespace cp = th08::effect::camera_particle;
namespace {
void check(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
void append(e::Program &program, std::int16_t opcode, std::int32_t time,
            std::initializer_list<std::uint32_t> values = {}, std::uint16_t flags = 0) {
    e::Operation op{};
    op.time = time;
    op.opcode = opcode;
    op.flags = flags;
    op.mask = 255;
    op.offset = 100 + std::uint32_t(program.code.size()) * 64;
    op.payload_offset = std::uint32_t(program.payloads.size());
    op.payload_size = std::uint16_t(values.size() * 4);
    std::copy(values.begin(), values.end(), op.words.begin());
    for (const auto value : values)
        for (unsigned shift = 0; shift < 32; shift += 8)
            program.payloads.push_back(std::uint8_t(value >> shift));
    program.code.push_back(op);
}
std::shared_ptr<p::Programs> programs(std::uint32_t count = 2, std::uint32_t color = 0xe112a5f0U,
                                      std::uint16_t flags = 0, std::uint32_t effect = 51,
                                      bool random_prefix = false) {
    auto code = std::make_shared<p::Programs>();
    code->ecl.subs.resize(1);
    auto &sub = code->ecl.subs[0];
    if (random_prefix)
        append(sub, 6, 0, {10000, 10032}, 3);
    else
        append(sub, 80, 0, {56});
    append(sub, 139, 0, {effect, count, color}, flags);
    append(sub, 1, 100); // Genuine frame boundary, not a fake successful root return.
    return code;
}
void begin(p::SpawnPool &pool) {
    p::SpawnRequest request;
    request.position = {30, -16, 9};
    request.life = 20;
    request.score = 1000;
    check(pool.begin(request, 1).status == p::Status::spawn_pending, "spawn did not begin");
}
bool same_rng(const th08::random::Rng &left, const th08::random::Rng &right) {
    const auto a = left.state(), b = right.state();
    return a.seed == b.seed && a.generation_count == b.generation_count &&
           a.saved_seed == b.saved_seed && a.saved_seed_valid == b.saved_seed_valid;
}
void consume_particles(th08::random::Rng &rng, std::size_t count) {
    for (std::size_t i = 0; i < count * 16; ++i)
        rng.next_u16();
}
fx::Effect51Animation supplied_animation() {
    // Synthetic post-time-zero fixture; the DAT test uses the certified resource instead.
    fx::Effect51Animation result{};
    result.fields.flags = 7;
    result.control.pc = 2;
    result.control.time.current = 1;
    result.control.sprite = 121;
    result.rotation = result.angular_velocity = {0, 0, .25f};
    result.sprite_width = result.sprite_height = 16;
    return result;
}
const cp::Camera camera{{1, 2, 3}, {4, 5, -1}, {0, 0, 1}};
void missing_retry_and_fork() {
    const auto animation = supplied_animation();
    const fx::Effect51Inputs inputs{&animation, &camera, 1.0f};
    th08::random::Rng rng(42, 17);
    rng.save_seed();
    const auto checkpoint = rng;
    p::SpawnPool absent(programs());
    begin(absent);
    check(absent.resume(&rng, nullptr, 100000, &inputs).status == p::Status::missing_entry_state &&
              !absent.effects() && same_rng(rng, checkpoint),
          "unknown effect pool became empty storage or consumed RNG");

    p::SpawnPool pool(programs(), {}, fx::PrimaryPool{});
    begin(pool);
    auto blocked = [&](const fx::Effect51Inputs &context, th08::random::Rng *stream) {
        const auto result = pool.resume(stream, nullptr, 100000, &context);
        check(result.status == p::Status::missing_entry_state && result.actor == 0 &&
                  result.pc == 1 && result.offset == 164 && result.opcode == 139 &&
                  pool.pending() && pool.actor(0).execution.result.executed == 2 &&
                  pool.actor(0).score == 100 && pool.effects()->active_count() == 0 &&
                  pool.effects()->cursor() == 0 && !pool.effects()->spawn_event() &&
                  same_rng(rng, checkpoint),
              "missing input or retry mutated the pending transaction");
    };
    for (unsigned retry = 0; retry < 4; ++retry)
        blocked({&animation, nullptr, 1.0f}, &rng);
    blocked({nullptr, &camera, 1.0f}, &rng);
    blocked({&animation, &camera, std::nullopt}, &rng);
    blocked(inputs, nullptr);
    auto branch = pool;
    auto branch_rng = rng;
    check(branch.resume(&branch_rng, nullptr, 100000, &inputs).status == p::Status::spawned &&
              !branch.pending() && branch.effects()->active_count() == 2 &&
              pool.effects()->active_count() == 0 && pool.actor(0).execution.pc == 1 &&
              &branch.effects()->slot(0) != &pool.effects()->slot(0),
          "snapshot shares mutable effect slots or pending execution");
    auto expected = checkpoint;
    consume_particles(expected, 2);
    check(same_rng(branch_rng, expected) && branch.actor(0).score == 1000 &&
              branch.actor(0).max_life == 20 && branch.actor(0).execution.local_time == 1,
          "effect commit did not precede source post-spawn stores");
    const auto &slot = branch.effects()->slot(0);
    const auto color = slot.particle.animation.primary;
    check(color.a == 0xe1 && color.r == 0x12 && color.g == 0xa5 && color.b == 0xf0 &&
              slot.particle.position.x == 30 && slot.particle.position.z == 9 &&
              slot.animation.control.sprite == 121 && slot.animation.rotation.z == .25f,
          "raw signed ARGB, actor position or supplied ANM projection changed");
    check(branch.resume(&branch_rng, nullptr, 100000, &inputs).status == p::Status::invalid &&
              branch.effects()->active_count() == 2 && same_rng(branch_rng, expected),
          "completed transaction allocated or consumed RNG twice");
}
void random_operand_rollback() {
    // A scalar draw commits before a random count is decoded at the world boundary.
    p::SpawnPool pool(programs(10032, 0, 2, 51, true), {}, fx::PrimaryPool{});
    begin(pool);
    th08::random::Rng rng(1234, 19), committed(1234, 19);
    const auto scalar = committed.next_u32() & 0x7fffffffU;
    const auto animation = supplied_animation();
    fx::Effect51Inputs inputs{&animation, nullptr, 1.0f};
    for (unsigned retry = 0; retry < 4; ++retry)
        check(pool.resume(&rng, nullptr, 100000, &inputs).status ==
                      p::Status::missing_entry_state &&
                  pool.actor(0).scalars.registers[0] == scalar && same_rng(rng, committed) &&
                  pool.effects()->active_count() == 0,
              "failed world transaction leaked operand draws or reverted prior scalar RNG");
    auto expected = committed;
    check((expected.next_u32() & 0x7fffffffU) > fx::primary_capacity,
          "random count fixture no longer exhausts the source scan");
    consume_particles(expected, fx::primary_capacity);
    inputs.camera = &camera;
    check(pool.resume(&rng, nullptr, 100000, &inputs).status == p::Status::spawned &&
              pool.effects()->active_count() == fx::primary_capacity && same_rng(rng, expected),
          "recovery did not commit operand and allocation draws together");
}
void unsupported_fields_and_full_pool() {
    const auto animation = supplied_animation();
    const fx::Effect51Inputs inputs{&animation, &camera, 1.0f};
    th08::random::Rng rng(123), checkpoint = rng;
    for (const auto &code : {programs(1, 10000, 4), programs(1, 0, 0, 52)}) {
        p::SpawnPool pool(code, {}, fx::PrimaryPool{});
        begin(pool);
        check(pool.resume(&rng, nullptr, 100000, &inputs).status ==
                      p::Status::unsupported_world_effect &&
                  pool.pending() && pool.effects()->active_count() == 0 &&
                  same_rng(rng, checkpoint),
              "masked color or another effect ID was silently accepted");
    }
    p::SpawnPool raw(programs(1, 10032), {}, fx::PrimaryPool{});
    begin(raw);
    consume_particles(checkpoint, 1);
    check(raw.resume(&rng, nullptr, 100000, &inputs).status == p::Status::spawned &&
              raw.effects()->slot(0).particle.animation.primary.b == std::uint8_t(10032) &&
              same_rng(rng, checkpoint),
          "selector-shaped raw color consumed an extra RNG operand");
    fx::PrimaryPool::Occupancy occupied;
    occupied.fill(true);
    p::SpawnPool full(programs(), {}, fx::PrimaryPool(occupied, 319));
    begin(full);
    check(full.resume().status == p::Status::spawned && full.effects()->cursor() == 319 &&
              full.effects()->active_count() == fx::primary_capacity &&
              full.effects()->spawn_event() &&
              full.effects()->slot(319).kind == fx::SlotKind::unknown &&
              full.actor(0).score == 1000,
          "full pool demanded unused initialization inputs or fabricated particle fields");
}
void dat(const char *path) {
    auto bytes = r::read_file(path);
    const auto dat_hash = r::sha256(r::view(bytes));
    check(dat_hash == "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb",
          "DAT identity mismatch");
    r::Archive archive(std::move(bytes));
    r::Bytes ecl_bytes, anm_bytes;
    for (std::size_t member = 0; member < archive.entries().size(); ++member) {
        const auto &name = archive.entries()[member].name;
        if (name == "ecldata1sp.ecl")
            ecl_bytes = archive.decode(member);
        else if (name == "enemy.anm")
            anm_bytes = archive.decode(member);
    }
    const auto ecl_hash = r::sha256(r::view(ecl_bytes));
    const auto anm_hash = r::sha256(r::view(anm_bytes));
    check(ecl_hash == "aac506b4eaf8fdfaa90e876f74db711d3c0724f63798e7d62801b02bbb29e00e" &&
              anm_hash == "203902359e0d9f741356f48f5ea451f3ec5e95750542861012483a6d80483cb6",
          "practice resource identity mismatch");
    const auto animation = fx::compile_effect51_animation(r::view(anm_bytes));
    const auto decoded = r::parse_ecl(r::view(ecl_bytes));
    const auto code = std::make_shared<const p::Programs>(r::view(ecl_bytes), decoded);
    // Explicitly supplied checkpoint, not inferred background/player initialization.
    p::Entry entry(code, 1, fx::PrimaryPool{});
    t::Context observations;
    observations.gui_boss_present = observations.spawns_suppressed = t::KnownBool::no;
    const fx::Effect51Inputs inputs{&animation, &camera, 1.0f};
    th08::random::Rng rng(1234), expected(1234);
    check(entry.advance(observations, &rng, nullptr, 100000, &inputs).status ==
              p::Status::timeline_frame_complete,
          "initial actual timeline phase changed");
    const auto completed = entry.advance(observations, &rng, nullptr, 100000, &inputs);
    consume_particles(expected, 16);
    check(completed.status == p::Status::timeline_frame_complete &&
              entry.pool().effects()->active_count() == 16 && same_rng(rng, expected) &&
              !entry.pool().pending() && !entry.timeline_state().pending_effect &&
              entry.timeline_state().pc == 1 && entry.timeline_state().time.current == 2 &&
              entry.timeline_state().effect_token == 1 && entry.pool().active_count() == 1 &&
              entry.pool().actor(0).score == 1000 && entry.pool().actor(0).max_life == 20 &&
              entry.pool().actor(0).item_drop == -2 &&
              entry.pool().actor(0).execution.local_time == 1,
          "actual sub0 spawn did not commit effects, post-stores and its timeline token");
    std::cout << "DAT supplied checkpoint: dat_sha256=" << dat_hash << " ecl_sha256=" << ecl_hash
              << " enemy_anm_sha256=" << anm_hash << '\n'
              << "  mask=1 timeline_pc=1 timeline_time=2 status=" << p::name(completed.status)
              << " actor=0 sub=0 post_spawn_stores=completed effects=16 rng_draws=256\n";
    // Never reach wrapper42 by silently skipping absent manager/effect phases.
    std::cout << "Next required world phase is unimplemented. Supplied-state entry prefix only; "
                 "no complete spell or acceptance gate.\n";
}
} // namespace
int main(int argc, char **argv) try {
    check(argc <= 2, "usage: effect_entry_tests [th08.dat]");
    missing_retry_and_fork();
    random_operand_rollback();
    unsupported_fields_and_full_pool();
    if (argc == 2)
        dat(argv[1]);
    std::cout << "ECL effect51 entry transactions, ownership and retry tests passed\n";
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
