#pragma once
// Included after the hash-pinned source SpawnEnemy1/2 bodies and narrow adapters.
inline std::uint64_t compare_spawn_order() {
    namespace p = th08::practice;
    namespace e = th08::emitter;
    namespace ref = spawn_reference;
    auto check = [](bool ok) {
        if (!ok)
            throw std::runtime_error("source spawn-order mismatch");
    };
    auto programs = std::make_shared<p::Programs>();
    programs->ecl.subs.resize(3);
    auto add = [](e::Program &program, int opcode, int time, int value = 0) {
        e::Operation op{};
        op.opcode = std::int16_t(opcode);
        op.time = time;
        op.mask = 255;
        op.payload_offset = std::uint32_t(program.payloads.size());
        if (opcode == 131) {
            op.payload_size = 4;
            op.words[0] = std::uint32_t(value);
            for (unsigned shift = 0; shift < 32; shift += 8)
                program.payloads.push_back(std::uint8_t(std::uint32_t(value) >> shift));
        }
        program.code.push_back(op);
    };
    add(programs->ecl.subs[0], 1, 100);
    add(programs->ecl.subs[1], 1, 0);
    add(programs->ecl.subs[2], 131, 0, 7);
    add(programs->ecl.subs[2], 1, 100);
    std::mt19937 inputs(0x08092026); // Test generator, never the shared game stream.
    std::uint64_t cases = 0;
    for (int batch = 0; batch < 8; ++batch) {
        p::SpawnPool native(programs);
        auto source = std::make_unique<ref::EnemyManager>();
        for (unsigned attempt = 0; attempt < 750; ++attempt) {
            p::SpawnRequest request;
            request.sub = attempt % 7 == 0 ? 1 : int(inputs() % 2) * 2;
            request.life = attempt % 3 ? int(inputs() % 2000) : -1;
            request.score = attempt % 4 ? int(inputs() % 100000) : -1;
            request.item_drop = int(inputs() % 1024) - 512;
            request.position = {float(int(inputs() % 1000) - 500), float(inputs() % 480), 3};
            request.kind = attempt % 2 ? p::SpawnKind::ordinary : p::SpawnKind::inherited_context;
            request.mirror_x = request.kind == p::SpawnKind::ordinary && attempt % 3;
            ref::i32 inherited[30]{};
            if (request.kind == p::SpawnKind::inherited_context) {
                request.inherited.emplace();
                request.inherited->initialized.fill(true);
                for (std::size_t i = 0; i < 30; ++i) {
                    const auto slot = e::context_slots[i];
                    const int value = int(inputs() % 1000) - 500;
                    request.inherited->values[i] = value;
                    if ((slot >= 16 && slot <= 23) || slot >= 94 || (slot >= 57 && slot <= 60)) {
                        const float scalar = float(value);
                        std::memcpy(&inherited[i], &scalar, 4);
                    } else {
                        inherited[i] = value;
                    }
                }
            }
            ref::ran = false;
            ref::replay.frameEventFlags = 0;
            const auto begun = native.begin(request, 1);
            auto *expected =
                request.kind == p::SpawnKind::ordinary
                    ? source->SpawnEnemy1(request.sub, &request.position, request.life,
                                          request.item_drop, request.score, request.mirror_x)
                    : source->SpawnEnemy2(request.sub, &request.position, request.life,
                                          request.item_drop, request.score, inherited);
            const auto slot = std::size_t(expected - source->enemies);
            check(native.spawn_event() && ref::replay.frameEventFlags == 1);
            check(begun.actor == slot);
            if (slot == 480) {
                check(begun.status == p::Status::pool_full && !ref::ran &&
                      native.last_spawn_failed() == source->lastSpawnFailed);
                ++cases;
                continue;
            }
            check(begun.status == p::Status::spawn_pending && ref::ran);
            const auto &before = native.actor(slot);
            check(before.life == ref::before_run.life && before.score == ref::before_run.score &&
                  before.max_life == ref::before_run.maxLife &&
                  before.item_drop == ref::before_run.itemDropType &&
                  before.motion.mirror_x == bool(ref::before_run.flags1 & (1U << 18)));
            if (request.inherited)
                for (std::size_t i = 0; i < 30; ++i) {
                    check(before.scalars.registers[e::context_slots[i]] ==
                          request.inherited->values[i]);
                    check(ref::before_run.mainEclContextStorage.intVariables[i] == inherited[i]);
                }
            const auto done = native.resume();
            check(done.status ==
                  (source->lastSpawnFailed ? p::Status::source_spawn_failed : p::Status::spawned));
            const auto &actual = native.actor(slot);
            check(actual.active == bool(expected->flags1 & 1) && actual.life == expected->life &&
                  actual.score == expected->score && actual.max_life == expected->maxLife &&
                  actual.phase_starting_life == expected->phaseStartingLife &&
                  actual.item_drop == expected->itemDropType &&
                  actual.display_color == expected->displayColor &&
                  native.last_spawn_failed() == source->lastSpawnFailed && !native.pending());
            ++cases;
        }
    }
    return cases;
}
