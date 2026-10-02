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
    if (argc == 2) {
        th08::resources::Archive archive(th08::resources::read_file(argv[1]));
        bool found = false;
        for (std::size_t i = 0; i < archive.entries().size(); ++i)
            if (archive.entries()[i].name == "enemy.anm") {
                const auto bytes = archive.decode(i);
                initial = e::compile_effect51_animation(th08::resources::view(bytes));
                found = true;
            }
        require_effect(found, "enemy.anm missing");
        effect_anm_reference::AnmVmBase source{};
        source.Initialize();
        const float angular[] = {initial.angular_velocity.x, initial.angular_velocity.y,
                                 initial.angular_velocity.z};
        effect_anm_reference::time_zero(&source, angular);
        require_effect(equal_vec(initial.rotation, source.rotation) &&
                           equal_vec(initial.angular_velocity, source.angleVel) &&
                           initial.fields.flags == source.flags &&
                           source.color1.d3dColor == 0xffffffffU,
                       "source ANM time-zero projection mismatch");
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
    std::cout << "effect allocation source transactions=6000 allocations=" << allocations
              << " mismatches=0 ANM_DAT=" << (argc == 2 ? "checked" : "not_supplied") << '\n';
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
