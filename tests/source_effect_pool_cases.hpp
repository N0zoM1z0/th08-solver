#pragma once
namespace {
void require_effect(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
bool equal_float(float a, float b) {
    return std::memcmp(&a, &b, sizeof(float)) == 0;
}
template <class A, class B> bool equal_vec(const A &a, const B &b) {
    return equal_float(a.x, b.x) && equal_float(a.y, b.y) && equal_float(a.z, b.z);
}
template <class A> bool equal_color(const th08::effect::camera_particle::Color &a, const A &b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}
} // namespace
int main(int argc, char **argv) try {
    namespace e = th08::effect;
    namespace c = th08::effect::camera_particle;
    namespace r = effect_pool_reference;
    namespace cr = camera_particle_reference;
    e::Effect51Animation initial{};
    initial.fields.primary = {255, 255, 255, 255};
    initial.fields.flags = 7;
    auto background = initial;
    background.script = e::ScriptCertificate::script75;
    background.control.pc = 2;
    background.control.sprite = 123;
    background.control.visible = true;
    background.control.time = {0, 1, 0};
    if (argc == 2) {
        th08::resources::Archive archive(th08::resources::read_file(argv[1]));
        bool found = false;
        for (std::size_t i = 0; i < archive.entries().size(); ++i)
            if (archive.entries()[i].name == "enemy.anm") {
                const auto bytes = archive.decode(i);
                initial = e::compile_effect51_animation(th08::resources::view(bytes));
                background = e::compile_background62_animation(th08::resources::view(bytes));
                found = true;
            }
        require_effect(found, "enemy.anm missing");
        effect_anm_reference::AnmVmBase source{};
        source.Initialize();
        // Independently recorded opcode13 words from the pinned DAT, rather
        // than feeding the implementation's decoded operands back as expected.
        const std::uint32_t z_word = 0x3e860a92U;
        float z_velocity;
        std::memcpy(&z_velocity, &z_word, sizeof(z_velocity));
        const float angular[] = {0, 0, z_velocity};
        effect_anm_reference::time_zero(&source, angular);
        require_effect(equal_vec(initial.rotation, source.rotation) &&
                           equal_vec(initial.angular_velocity, source.angleVel) &&
                           initial.fields.flags == source.flags &&
                           source.color1.d3dColor == 0xffffffffU,
                       "source ANM time-zero projection mismatch");
        require_effect(equal_vec(background.rotation, source.rotation) &&
                           background.control.sprite == 123 && background.control.time.current == 1,
                       "background particle ANM75 projection mismatch");
    }
    std::mt19937 gen(0x0851);
    const c::Camera camera{{1, 2, 3}, {20, -30, 60}, {0, 0, 1}};
    cr::g_Background.cameraCurrent.position = {1, 2, 3};
    cr::g_Background.cameraCurrent.lookAtOffset = {20, -30, 60};
    cr::g_Background.cameraCurrent.forward = {0, 0, 1};
    cr::g_Supervisor.framerateMultiplier = 1;
    std::size_t allocations = 0;
    for (unsigned trial = 0; trial < 6000; ++trial) {
        e::PrimaryPool::Occupancy occupied{};
        for (auto &value : occupied)
            value = (gen() % 5) != 0;
        if (trial % 8 == 0)
            occupied.fill(false);
        if (trial % 8 == 1)
            occupied.fill(true);
        const std::size_t cursor = gen() % 512;
        e::PrimaryPool actual(occupied, cursor);
        auto reference = std::make_unique<r::EffectManager>();
        reference->nextEffectIndex = int(cursor);
        for (std::size_t i = 0; i < 512; ++i)
            reference->effects[i].active = occupied[i];
        r::AnmLoaded anm{};
        anm.initial.flags = initial.fields.flags;
        anm.initial.color1.d3dColor = 0xffffffffU;
        reference->effectAnm = &anm;
        r::g_EffectTemplates[51] = {73, nullptr, r::initialize};
        r::replay.frameEventFlags = 0;
        const std::uint16_t seed = std::uint16_t(gen());
        th08::random::Rng rng(seed);
        cr::g_Rng.SetSeed(seed);
        cr::g_Rng.ResetGenerationCount();
        const int counts[] = {-3, 0, 1, 2, 16, 511, 512, 513};
        const int count = counts[trial % 8];
        e::Effect51Request request{count, {30, -16, 3}, {0x12, 0x34, 0x56, 0x78}};
        const auto result = actual.spawn_effect51(request, {&initial, &camera, 1}, &rng);
        cr::D3DXVECTOR3 position{30, -16, 3};
        const auto *returned = reference->SpawnEffect(51, &position, count, 0x78123456);
        require_effect(result.committed() &&
                           result.returned_slot == std::size_t(returned - reference->effects) &&
                           actual.cursor() == std::size_t(reference->nextEffectIndex) &&
                           actual.spawn_event() && r::replay.frameEventFlags == 1 &&
                           rng.seed() == cr::g_Rng.GetSeed(),
                       "source effect scan/cursor/return/RNG mismatch");
        std::size_t added = 0;
        for (std::size_t i = 0; i < 512; ++i) {
            require_effect(actual.occupied(i) == bool(reference->effects[i].active),
                           "source effect occupancy mismatch");
            if (occupied[i] || !actual.occupied(i))
                continue;
            ++added;
            const auto &a = actual.slot(i).particle;
            const auto &b = reference->effects[i];
            require_effect(
                equal_vec(a.position, b.position) && equal_vec(a.inherited_velocity, b.vector1) &&
                    equal_vec(a.velocity, b.vector2) && equal_vec(a.acceleration, b.vector3) &&
                    equal_vec(a.particle_position, b.vector4) && a.draw_group == b.drawGroup &&
                    equal_vec(a.animation.position_offset, b.vm.pos2) &&
                    equal_vec(a.animation.position_initial, b.vm.posInitial) &&
                    equal_vec(a.animation.position_final, b.vm.posFinal) &&
                    equal_vec(a.animation.rotation_initial, b.vm.rotateInitial) &&
                    equal_color(a.animation.primary, b.vm.color1) &&
                    equal_color(a.animation.secondary, b.vm.color2) &&
                    a.animation.flags == b.vm.flags,
                "source effect initialized fields mismatch");
        }
        require_effect(added == result.allocated && rng.generation_count() == 16 * added,
                       "source effect allocation/draw count mismatch");
        allocations += added;
    }
    // Background62 and ECL51 contend for the SAME cursor. Reuse the unchanged
    // SpawnEffect body with its real callback/no-callback template distinction.
    for (unsigned trial = 0; trial < 32; ++trial) {
        e::PrimaryPool::Occupancy occupied{};
        for (std::size_t i = 0; i < occupied.size(); ++i)
            occupied[i] = trial == 0 || (i + trial) % 7 != 0;
        e::PrimaryPool actual(occupied, trial * 16);
        auto reference = std::make_unique<r::EffectManager>();
        reference->nextEffectIndex = int(trial * 16);
        for (std::size_t i = 0; i < 512; ++i)
            reference->effects[i].active = occupied[i];
        r::AnmLoaded anm{};
        anm.initial.flags = 7;
        anm.initial.color1.d3dColor = 0xffffffffU;
        reference->effectAnm = &anm;
        r::g_EffectTemplates[62] = {75, nullptr, nullptr};
        th08::random::Rng rng{std::uint16_t(trial)};
        cr::g_Rng.SetSeed(std::uint16_t(trial));
        cr::g_Rng.ResetGenerationCount();
        for (unsigned call = 0; call < 16; ++call) {
            const bool background_call = call % 3 != 0;
            cr::D3DXVECTOR3 position{30, -16, 3};
            const auto result = background_call
                                    ? actual.spawn_background62({30, -16, 3}, &background)
                                    : actual.spawn_effect51({4, {30, -16, 3}, {255, 255, 255, 255}},
                                                            {&initial, &camera, 1}, &rng);
            auto *returned =
                reference->SpawnEffect(background_call ? 62 : 51, &position,
                                       background_call ? 1 : 4, background_call ? 0x20ffffff : -1);
            if (background_call)
                returned->drawGroup = 4; // Background writes even to sentinel653.
            require_effect(result.committed() && rng.seed() == cr::g_Rng.GetSeed() &&
                               actual.cursor() == std::size_t(reference->nextEffectIndex) &&
                               result.returned_slot == std::size_t(returned - reference->effects),
                           "shared effect62/51 allocation or RNG ordering mismatch");
            if (background_call && result.returned_slot == e::exhausted_effect)
                require_effect(actual.exhausted_draw_group() == returned->drawGroup,
                               "background sentinel post-store missing");
            for (std::size_t i = 0; i < 512; ++i) {
                const auto &slot = actual.slot(i);
                const auto &expected = reference->effects[i];
                require_effect(slot.active == bool(expected.active), "mixed occupancy mismatch");
                if (!occupied[i] && slot.active)
                    require_effect(
                        equal_vec(slot.particle.position, expected.position) &&
                            equal_color(slot.particle.animation.primary, expected.vm.color1) &&
                            slot.particle.draw_group == expected.drawGroup,
                        "mixed slot initialization mismatch");
            }
        }
    }
    std::cout << "effect allocation source transactions=6000 allocations=" << allocations
              << " mixed_background_calls=512"
              << " mismatches=0 ANM_DAT=" << (argc == 2 ? "checked" : "not_supplied") << '\n';
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
