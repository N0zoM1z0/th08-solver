#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <th08/practice_entry.hpp>

namespace p = th08::practice;
namespace e = th08::emitter;
namespace r = th08::resources;
namespace t = th08::timeline;
namespace {
void check(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
std::uint32_t bits(float value) {
    std::uint32_t word;
    std::memcpy(&word, &value, 4);
    return word;
}
void word(r::Bytes &bytes, std::uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8)
        bytes.push_back(std::uint8_t(value >> shift));
}
void append(e::Program &program, std::int16_t opcode, std::int32_t time,
            std::initializer_list<std::uint32_t> values = {}, std::uint16_t flags = 0,
            std::uint8_t mask = 255) {
    e::Operation op{};
    op.time = time;
    op.opcode = opcode;
    op.flags = flags;
    op.mask = mask;
    op.offset = 100 + std::uint32_t(program.code.size()) * 64;
    op.payload_offset = std::uint32_t(program.payloads.size());
    op.payload_size = std::uint16_t(values.size() * 4);
    std::copy(values.begin(), values.end(), op.words.begin());
    for (auto value : values)
        word(program.payloads, value);
    program.code.push_back(op);
}
void timeline_append(t::Program &program, unsigned opcode, int time,
                     std::initializer_list<std::uint32_t> values = {}) {
    t::Operation op;
    op.opcode = std::uint16_t(opcode);
    op.time = time;
    op.mask = 255;
    op.offset = 10000 + std::uint32_t(program.code.size()) * 32;
    op.payload_offset = std::uint32_t(program.payloads.size());
    op.payload_size = std::uint16_t(values.size() * 4);
    std::size_t i = 0;
    for (auto value : values) {
        word(program.payloads, value);
        op.words[i++] = r::i32(r::view(program.payloads), program.payloads.size() - 4);
    }
    program.code.push_back(op);
}
std::shared_ptr<p::Programs> programs() {
    auto code = std::make_shared<p::Programs>();
    code->ecl.subs.resize(5);
    // A future command leaves the current context alive at a genuine ECL frame
    // boundary. A root return is NOT used as a fake successful initialization.
    append(code->ecl.subs[0], 1, 100);
    append(code->ecl.subs[1], 1, 0);
    append(code->ecl.subs[2], 131, 0, {7});
    append(code->ecl.subs[2], 6, 0, {10001, 10051}, 3);
    append(code->ecl.subs[2], 65, 0, {bits(0), bits(2)});
    append(code->ecl.subs[2], 1, 100);
    append(code->ecl.subs[3], 80, 0, {56});
    append(code->ecl.subs[3], 139, 0, {51, 16, 0});
    append(code->ecl.subs[3], 1, 100);
    append(code->ecl.subs[4], 53, 0);
    return code;
}
void metadata_and_order() {
    auto code = programs();
    p::SpawnPool pool(code);
    p::SpawnRequest request;
    request.sub = 2;
    request.life = 20;
    request.item_drop = 510;
    request.score = 1000;
    request.position = {10, 20, 3};
    request.mirror_x = true;
    auto result = pool.begin(request, 1);
    check(result.status == p::Status::spawn_pending && result.actor == 0 && pool.pending() &&
              pool.spawn_event() && !pool.last_spawn_failed(),
          "source selection boundary");
    const auto &before = pool.actor(0);
    check(before.life == 20 && before.score == 100 && before.item_drop == 0 &&
              before.max_life == 0 && before.phase_starting_life == 0,
          "post-spawn stores leaked before immediate ECL");
    check(before.hitbox.x == 24 && before.secondary_hitbox.x == 0 &&
              before.scalars.initialized[0] && !before.scalars.initialized[40],
          "template fields or unknown external state changed");
    result = pool.resume();
    const auto &after = pool.actor(0);
    check(result.status == p::Status::spawned && !pool.pending() && after.active &&
              after.life == 7 && after.max_life == 7 && after.phase_starting_life == 7 &&
              after.score == 1000 && after.item_drop == -2 && after.scalars.registers[1] == 7,
          "ordinary spawn must retain immediate ECL life and apply metadata afterward");
    check(after.motion.velocity.x == 2 && after.motion.mirror_x && after.motion.position.x == 10 &&
              after.motion.position.y == 20 && after.motion.position.z == 3 &&
              after.execution.local_time == 1,
          "RunEcl tail updates velocity but must not run manager displacement");
    check(pool.resume().status == p::Status::invalid, "completed spawn resumed twice");

    request.kind = p::SpawnKind::inherited_context;
    request.mirror_x = false;
    request.inherited = e::capture_context(after.scalars);
    request.inherited->values[0] = 93;
    request.inherited->initialized[2] = false;
    check(pool.begin(request, 1).status == p::Status::spawn_pending, "inherited spawn begins");
    check(pool.actor(1).scalars.registers[0] == 93 && !pool.actor(1).scalars.initialized[2],
          "inherited words and validity must precede immediate execution");
    result = pool.resume();
    const auto &inherited = pool.actor(1);
    check(result.status == p::Status::spawned && inherited.life == 20 && inherited.max_life == 20 &&
              inherited.phase_starting_life == 20 && inherited.scalars.registers[1] == 7,
          "SpawnEnemy2 must overwrite life AFTER immediate ECL, unlike SpawnEnemy1");
    request.inherited.reset();
    check(pool.begin(request, 1).status == p::Status::missing_entry_state &&
              pool.active_count() == 2,
          "missing inherited context must not allocate");
}
void failure_and_capacity() {
    auto code = programs();
    p::SpawnPool pool(code);
    p::SpawnRequest request;
    request.sub = 1;
    request.life = 50;
    request.score = 900;
    check(pool.begin(request, 1).status == p::Status::spawn_pending, "terminating spawn begins");
    auto result = pool.resume();
    check(result.status == p::Status::source_spawn_failed && result.actor == 0 &&
              pool.last_spawn_failed() && !pool.actor(0).active && !pool.pending() &&
              pool.actor(0).score == 100 && pool.actor(0).max_life == 0,
          "source failure must retain selected identity but clear active without post-stores");
    request.sub = 0x10000; // Source narrows the sub ID to signed16 before CallEclSub.
    request.life = request.score = -1;
    for (std::size_t i = 0; i < p::enemy_capacity; ++i) {
        result = pool.begin(request, 1);
        check(result.status == p::Status::spawn_pending && result.actor == i,
              "first-free pool order or signed16 narrowing changed");
        if (i == 0)
            check(pool.last_spawn_failed(), "lastSpawnFailed changed before RunEcl returned");
        check(pool.resume().status == p::Status::spawned && pool.actor(i).life == 1 &&
                  pool.actor(i).score == 100,
              "negative life/score must retain template values");
    }
    request.sub = -1; // A full pool does not dereference the sub table.
    result = pool.begin(request, 1);
    check(result.status == p::Status::pool_full && result.actor == p::no_actor &&
              pool.last_spawn_failed() && pool.active_count() == p::enemy_capacity &&
              !pool.pending(),
          "full-pool sentinel must not alias a real actor");
    p::SpawnPool empty(code);
    check(empty.begin(request, 1).status == p::Status::invalid && !empty.spawn_event(),
          "negative narrowed sub must not execute a null instruction");
    request.sub = 0;
    request.position.x = std::numeric_limits<float>::quiet_NaN();
    check(empty.begin(request, 1).status == p::Status::invalid && empty.active_count() == 0,
          "invalid spawn position mutated ownership");
}
void pause_and_fork() {
    auto code = programs();
    p::SpawnPool pool(code);
    p::SpawnRequest request;
    request.sub = 3;
    request.life = 20;
    request.item_drop = -2;
    request.score = 1000;
    check(pool.begin(request, 1).status == p::Status::spawn_pending, "paused spawn begins");
    th08::random::Rng rng(42, 17);
    auto blocked = pool.resume(&rng);
    check(blocked.status == p::Status::unsupported_world_effect && blocked.sub == 3 &&
              blocked.pc == 1 && blocked.opcode == 139 && blocked.local_time == 0 &&
              pool.actor(0).interaction.no_sprite && pool.actor(0).interaction.allow_offscreen &&
              pool.actor(0).interaction.no_death && pool.actor(0).interaction.collision &&
              pool.actor(0).score == 100 && pool.actor(0).max_life == 0,
          "blocked effect must retain the executed prefix but no post-spawn stores");
    check(pool.begin(request, 1).status == p::Status::busy && pool.active_count() == 1,
          "pending spawn must not allocate another actor");
    auto branch = pool;
    code.reset(); // Snapshots, not caller locals, keep immutable programs alive.
    for (int i = 0; i < 20; ++i) {
        auto again = pool.resume(&rng);
        check(again.status == blocked.status && again.pc == blocked.pc &&
                  pool.actor(0).execution.result.executed == 2 && pool.active_count() == 1 &&
                  rng.seed() == 42 && rng.generation_count() == 17,
              "repeated blocked resume repeated ECL, RNG or allocation");
    }
    check(branch.resume().pc == 1 && &branch.actor(0) != &pool.actor(0) &&
              branch.actor(0).execution.active == pool.actor(0).execution.active,
          "runtime copy must own mutable actors and share only immutable code");
    p::SpawnPool root(programs());
    request.sub = 4;
    check(root.begin(request, 1).status == p::Status::spawn_pending &&
              root.resume().status == p::Status::unsupported_root_return && root.pending(),
          "main-context underflow cannot become successful initialization");
}
void interaction_and_missing_rng() {
    auto code = programs();
    code->ecl.subs.emplace_back();
    auto &sequence = code->ecl.subs.back();
    append(sequence, 79, 0, {63});
    append(sequence, 81, 0, {63});
    append(sequence, 77, 0, {bits(40), bits(30)});
    append(sequence, 78, 0, {bits(10), bits(20)});
    append(sequence, 1, 100);
    p::SpawnPool pool(code);
    p::SpawnRequest request;
    request.sub = 5;
    check(pool.begin(request, 1).status == p::Status::spawn_pending &&
              pool.resume().status == p::Status::spawned,
          "interaction sequence");
    const auto &actor = pool.actor(0);
    check(actor.interaction.accepts_damage && actor.interaction.collision &&
              actor.interaction.damageable && !actor.interaction.no_sprite &&
              !actor.interaction.allow_offscreen && !actor.interaction.no_death &&
              actor.hitbox.x == 40 && actor.hitbox.y == 30 && actor.hitbox.z == 24 &&
              actor.secondary_hitbox.x == 10 && actor.secondary_hitbox.y == 20,
          "source interaction bit polarity or hitbox z preservation changed");
    auto random_code = programs();
    auto &random_sub = random_code->ecl.subs[0];
    random_sub = {};
    append(random_sub, 80, 0, {10032}, 1);
    append(random_sub, 1, 100);
    p::SpawnPool random_pool(random_code);
    request.sub = 0;
    check(random_pool.begin(request, 1).status == p::Status::spawn_pending &&
              random_pool.resume().status == p::Status::missing_entry_state &&
              random_pool.actor(0).execution.pending_effect,
          "missing RNG is not a zero random result or a source spawn failure");
    th08::random::Rng rng(0), expected(0);
    expected.next_u32();
    check(random_pool.resume(&rng).status == p::Status::spawned && rng.seed() == expected.seed() &&
              rng.generation_count() == 2,
          "world operand resumes with exactly one ordered draw");
}
void timeline_connection() {
    auto code = programs();
    timeline_append(code->timeline, 0, 0, {3, bits(30), bits(-16), 20, std::uint32_t(-2), 1000});
    timeline_append(code->timeline, 16, 0);
    timeline_append(code->timeline, 0, -1);
    p::Entry entry(code, 1);
    t::Context observations;
    check(entry.advance(observations).status == p::Status::missing_entry_state &&
              !entry.pool().spawn_event(),
          "unknown GUI gate must stop before pool access");
    observations.gui_boss_present = observations.spawns_suppressed = t::KnownBool::no;
    const auto result = entry.advance(observations);
    check(result.status == p::Status::unsupported_world_effect && result.opcode == 139 &&
              entry.timeline_state().pending_effect && entry.timeline_state().pc == 0 &&
              entry.timeline_state().time.current == 0 && entry.pool().active_count() == 1,
          "timeline was acknowledged before spawn's immediate ECL completed");
    auto branch = entry;
    for (int i = 0; i < 10; ++i)
        check(entry.advance(observations).offset == result.offset &&
                  entry.pool().active_count() == 1 && entry.timeline_state().effect_token == 1,
              "same-frame entry resumption duplicated pending spawn");
    check(branch.advance(observations).offset == result.offset,
          "entry snapshot loses pending transaction");

    auto success = programs();
    timeline_append(success->timeline, 1, 0, {2, bits(10), bits(20), 20, 254, 1000});
    timeline_append(success->timeline, 16, 0);
    timeline_append(success->timeline, 0, -1);
    p::Entry successful(success, 1);
    check(successful.advance(observations).opcode == 16 && successful.pool().active_count() == 1 &&
              !successful.pool().pending() && successful.timeline_state().pc == 1 &&
              successful.timeline_state().time.current == 0 &&
              successful.pool().actor(0).motion.velocity.x == 2 &&
              successful.pool().actor(0).motion.mirror_x,
          "completed mirrored spawn must resume same timeline frame before next external effect");
}
void shared_parameters_across_spawns() {
    auto code = programs();
    code->ecl.subs.resize(8);
    append(code->ecl.subs[5], 6, 0, {10061, 73}, 1);
    append(code->ecl.subs[5], 7, 0, {bits(10065), bits(1.25f)}, 1);
    append(code->ecl.subs[5], 1, 0); // Source failure does not undo earlier global writes.
    append(code->ecl.subs[6], 6, 0, {10000, 10061}, 3);
    append(code->ecl.subs[6], 7, 0, {bits(10016), bits(10065)}, 3);
    append(code->ecl.subs[6], 1, 100);
    append(code->ecl.subs[7], 6, 0, {10061, 99}, 1);
    append(code->ecl.subs[7], 1, 100);
    p::SpawnPool pool(code);
    p::SpawnRequest request;
    request.sub = 5;
    check(pool.begin(request, 1).status == p::Status::spawn_pending &&
              pool.resume().status == p::Status::source_spawn_failed &&
              pool.shared_calls().known[0] && pool.shared_calls().values[0] == 73,
          "source initialization failure discarded committed shared call parameters");
    request.sub = 6;
    check(pool.begin(request, 1).status == p::Status::spawn_pending &&
              pool.resume().status == p::Status::spawned &&
              pool.actor(0).scalars.registers[0] == 73 &&
              pool.actor(0).scalars.registers[16] == 1.25,
          "shared call parameters were isolated per actor or overwritten by the template");
    auto branch = pool;
    request.sub = 7;
    check(branch.begin(request, 1).status == p::Status::spawn_pending &&
              branch.resume().status == p::Status::spawned &&
              branch.shared_calls().values[0] == 99 && pool.shared_calls().values[0] == 73,
          "snapshot branches alias mutable shared call parameters");
    p::SpawnPool unknown(code);
    request.sub = 6;
    check(unknown.begin(request, 1).status == p::Status::spawn_pending &&
              unknown.resume().status == p::Status::missing_entry_state,
          "unprovided initial shared parameters silently became zero");
}
void dat(const char *path) {
    auto bytes = r::read_file(path);
    check(r::sha256(r::view(bytes)) ==
              "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb",
          "DAT identity mismatch");
    r::Archive archive(std::move(bytes));
    for (std::size_t member = 0; member < archive.entries().size(); ++member) {
        if (archive.entries()[member].name != "ecldata1sp.ecl")
            continue;
        const auto data = archive.decode(member);
        const auto decoded = r::parse_ecl(r::view(data));
        const auto code = std::make_shared<const p::Programs>(r::view(data), decoded);
        for (const std::uint8_t mask : {1, 2, 4, 8}) {
            p::Entry entry(code, mask);
            t::Context observations;
            observations.gui_boss_present = observations.spawns_suppressed = t::KnownBool::no;
            check(entry.advance(observations).status == p::Status::timeline_frame_complete,
                  "initial empty timeline phase");
            const auto stopped = entry.advance(observations);
            check(stopped.status == p::Status::unsupported_world_effect && stopped.actor == 0 &&
                      stopped.sub == 0 && stopped.pc == 1 && stopped.offset == 260 &&
                      stopped.opcode == 139 && stopped.instruction_mask == 255 &&
                      stopped.execution_mask == mask && stopped.local_time == 0 &&
                      entry.timeline_state().pc == 0 && entry.timeline_state().time.current == 1 &&
                      entry.timeline_state().pending_effect && entry.pool().pending() &&
                      entry.pool().actor(0).life == 20 && entry.pool().actor(0).score == 100 &&
                      entry.pool().actor(0).motion.position.x == 30 &&
                      entry.pool().actor(0).motion.position.y == -16,
                  "real practice prefix must stop at effect51 before completing timeline spawn");
        }
        std::cout << "DAT practice entry: four masks retain timeline39684 / sub0 PC1 offset260 "
                     "opcode139\n";
        return;
    }
    throw std::runtime_error("practice member missing");
}
} // namespace
int main(int argc, char **argv) try {
    check(argc <= 2, "usage: practice_entry_tests [th08.dat]");
    metadata_and_order();
    failure_and_capacity();
    pause_and_fork();
    interaction_and_missing_rng();
    timeline_connection();
    shared_parameters_across_spawns();
    if (argc == 2)
        dat(argv[1]);
    std::cout << "practice entry ownership and restricted spawn tests passed\n";
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
