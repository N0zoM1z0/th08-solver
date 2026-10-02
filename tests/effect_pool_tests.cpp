#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <th08/effect_pool.hpp>

namespace effect = th08::effect;
namespace cp = effect::camera_particle;
namespace rng_api = th08::random;

void check(bool value, const char *message) {
    if (!value) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
bool same(cp::Vec3 left, cp::Vec3 right) {
    return left.x == right.x && left.y == right.y && left.z == right.z;
}
bool same(cp::Color left, cp::Color right) {
    return left.r == right.r && left.g == right.g && left.b == right.b && left.a == right.a;
}
bool same(const cp::Animation &left, const cp::Animation &right) {
    return same(left.position_offset, right.position_offset) &&
           same(left.position_initial, right.position_initial) &&
           same(left.position_final, right.position_final) &&
           same(left.rotation_initial, right.rotation_initial) &&
           same(left.primary, right.primary) && same(left.secondary, right.secondary) &&
           left.flags == right.flags;
}
bool same(const cp::State &left, const cp::State &right) {
    return same(left.position, right.position) &&
           same(left.inherited_velocity, right.inherited_velocity) &&
           same(left.velocity, right.velocity) && same(left.acceleration, right.acceleration) &&
           same(left.particle_position, right.particle_position) &&
           same(left.animation, right.animation) && left.draw_group == right.draw_group;
}
bool same(rng_api::State left, rng_api::State right) {
    return left.seed == right.seed && left.generation_count == right.generation_count &&
           left.saved_seed == right.saved_seed && left.saved_seed_valid == right.saved_seed_valid;
}
effect::Effect51Animation prepared() {
    // Explicit fixture projection, not a claim about enemy.anm script73.
    effect::Effect51Animation value{};
    value.fields = {{1, 2, 3},        {4, 5, 6},        {7, 8, 9},  {10, 11, 12},
                    {13, 14, 15, 16}, {17, 18, 19, 20}, 0x80500041U};
    value.control.pc = 19;
    value.control.time = {27, 28, .5f};
    value.control.wait = {29, 30, .25f};
    value.control.sprite = 101;
    value.control.pending_interrupt = 37;
    value.control.floats = {.5f, 1.5f, 2.5f, 3.5f};
    value.rotation = {.1f, .2f, .3f};
    value.angular_velocity = {.01f, .02f, .03f};
    value.sprite_width = 32;
    value.sprite_height = 24;
    return value;
}
void check_unchanged(const effect::PrimaryPool &pool, const effect::PrimaryPool &before) {
    check(pool.cursor() == before.cursor() && pool.active_count() == before.active_count() &&
              pool.spawn_event() == before.spawn_event(),
          "blocked pool transaction changed manager state");
    for (std::size_t i = 0; i < effect::primary_capacity; ++i) {
        const auto &slot = pool.slot(i);
        const auto &prior = before.slot(i);
        check(slot.active == prior.active && slot.effect51_known == prior.effect51_known,
              "blocked pool transaction changed occupancy or known state");
        if (slot.effect51_known)
            check(same(slot.particle, prior.particle) &&
                      same(slot.animation.fields, prior.animation.fields) &&
                      slot.animation.control.pc == prior.animation.control.pc,
                  "blocked pool transaction changed an initialized effect");
    }
}

int main() {
    const auto animation = prepared();
    const cp::Camera camera{{10, -20, 30}, {4, 8, -12}, {0, 0, 1}};
    const effect::Effect51Inputs inputs{&animation, &camera, .5f};
    const effect::Effect51Request single{1, {30, -16, 7}, {255, 0x41, 0x42, 0x43}};
    rng_api::Rng rng(5123, 27), expected_rng(5123, 27);
    rng.save_seed();
    expected_rng.save_seed();

    effect::PrimaryPool pool;
    const auto first = pool.spawn_effect51(single, inputs, &rng);
    check(first.status == effect::Status::spawned && first.allocated == 1 && first.inspected == 1 &&
              first.returned_slot == 0 && first.committed() && pool.cursor() == 1 &&
              pool.active_count() == 1 && pool.spawn_event(),
          "single effect allocation or source cursor order is wrong");
    cp::State expected{};
    expected.position = single.position;
    expected.animation = animation.fields;
    expected.animation.flags |= 0x2000U;
    expected.animation.primary = single.color;
    expected.animation.position_offset = {0, 0, 0};
    check(cp::initialize(expected, &camera, .5f, &expected_rng) == cp::Status::initialized,
          "explicit initializer fixture failed");
    const auto &created = pool.slot(0);
    check(created.active && created.effect51_known && same(created.particle, expected) &&
              same(created.animation.fields, expected.animation) &&
              same(rng.state(), expected_rng.state()) && rng.generation_count() == 43,
          "pool slot initialization, ANM overrides or shared RNG differs from callback");
    check(created.animation.control.pc == animation.control.pc &&
              created.animation.control.time.current == animation.control.time.current &&
              created.animation.control.time.fraction == animation.control.time.fraction &&
              created.animation.control.wait.current == animation.control.wait.current &&
              created.animation.control.sprite == animation.control.sprite &&
              created.animation.control.pending_interrupt == animation.control.pending_interrupt &&
              created.animation.control.floats == animation.control.floats &&
              same(created.animation.rotation, animation.rotation) &&
              same(created.animation.angular_velocity, animation.angular_velocity) &&
              created.animation.sprite_width == animation.sprite_width &&
              created.animation.sprite_height == animation.sprite_height,
          "pool discarded supplied ancillary ANM state");
    check(same(created.particle.animation.primary, single.color) &&
              same(created.particle.animation.secondary, animation.fields.secondary) &&
              created.particle.animation.position_initial.y == 5 &&
              created.particle.animation.position_initial.z == 6 &&
              same(created.particle.animation.position_offset, {-9999, 0, 0}) &&
              created.particle.animation.flags == (animation.fields.flags | 0x2000U),
          "source initialization order or untouched ANM fields changed");

    // Occupied slots are skipped after advancing the cursor, including wrap.
    effect::PrimaryPool::Occupancy occupancy{};
    occupancy[511] = occupancy[0] = true;
    effect::PrimaryPool wrapping(occupancy, 510);
    const auto wrapped = wrapping.spawn_effect51({2, single.position, single.color}, inputs, &rng);
    check(wrapped.status == effect::Status::spawned && wrapped.allocated == 2 &&
              wrapped.inspected == 4 && wrapped.returned_slot == 1 && wrapping.cursor() == 2 &&
              wrapping.occupied(510) && wrapping.occupied(1) &&
              !wrapping.slot(511).effect51_known && !wrapping.slot(0).effect51_known,
          "wrap, occupied skipping or checkpoint validity changed");

    occupancy.fill(true);
    occupancy[509] = occupancy[1] = false;
    effect::PrimaryPool partial(occupancy, 510);
    rng_api::Rng partial_rng(17);
    const auto exhausted =
        partial.spawn_effect51({3, single.position, single.color}, inputs, &partial_rng);
    check(exhausted.status == effect::Status::exhausted && exhausted.committed() &&
              exhausted.returned_slot == effect::exhausted_effect && exhausted.allocated == 2 &&
              exhausted.inspected == 512 && partial.cursor() == 510 &&
              partial.active_count() == 512 && partial_rng.generation_count() == 32,
          "partial allocation did not commit or did not return source exhaustion sentinel");
    const auto invalid_float = std::numeric_limits<float>::quiet_NaN();
    const auto full_before = partial_rng.state();
    const auto full = partial.spawn_effect51({1, {invalid_float, invalid_float, invalid_float}, {}},
                                             {}, &partial_rng);
    check(full.status == effect::Status::exhausted && full.allocated == 0 &&
              full.inspected == 512 && partial.cursor() == 510 && partial.spawn_event() &&
              same(partial_rng.state(), full_before),
          "full pool consumed unavailable inputs or RNG");
    occupancy.fill(true);
    effect::PrimaryPool full_fresh(occupancy, 173);
    check(!full_fresh.spawn_event(), "supplied full occupancy fabricated prior spawn event");
    check(full_fresh.spawn_effect51(single, {}, nullptr).committed() && full_fresh.spawn_event() &&
              full_fresh.cursor() == 173,
          "full pool failed without ANM/camera/RNG or did not set spawn event");

    for (const auto count : {0, -1, std::numeric_limits<std::int32_t>::min()}) {
        occupancy.fill(true);
        occupancy[500] = occupancy[511] = occupancy[0] = false;
        effect::PrimaryPool signed_count(occupancy, 500);
        rng_api::Rng signed_rng(83);
        const auto result = signed_count.spawn_effect51({count, single.position, single.color},
                                                        inputs, &signed_rng);
        check(result.status == effect::Status::exhausted && result.allocated == 3 &&
                  result.inspected == 512 && result.returned_slot == effect::exhausted_effect &&
                  signed_count.cursor() == 500 && signed_count.active_count() == 512 &&
                  signed_rng.generation_count() == 48,
              "nonpositive count became a no-op or overflowed instead of scanning once");
    }
    effect::PrimaryPool exactly;
    rng_api::Rng exactly_rng(82);
    const auto all =
        exactly.spawn_effect51({512, single.position, single.color}, inputs, &exactly_rng);
    check(all.status == effect::Status::spawned && all.returned_slot == 511 &&
              all.allocated == 512 && all.inspected == 512 && exactly.cursor() == 0 &&
              exactly_rng.generation_count() == 8192,
          "exact capacity count incorrectly returned scan exhaustion");

    // Missing context remains missing after a previous successful request, and
    // retries cannot use stale scratch contents or consume duplicate draws.
    const auto checkpoint = pool;
    const auto rng_checkpoint = rng.state();
    auto blocked = inputs;
    blocked.post_time_zero = nullptr;
    check(pool.spawn_effect51(single, blocked, &rng).status == effect::Status::missing_context,
          "missing ANM projection invented a template");
    blocked = inputs;
    blocked.camera = nullptr;
    check(pool.spawn_effect51(single, blocked, &rng).status == effect::Status::missing_context,
          "missing camera invented an input");
    blocked = inputs;
    blocked.multiplier.reset();
    check(pool.spawn_effect51(single, blocked, &rng).status == effect::Status::missing_context,
          "missing frame multiplier defaulted to unit rate");
    check(pool.spawn_effect51(single, inputs, nullptr).status == effect::Status::missing_context,
          "missing RNG invented a stream");
    check_unchanged(pool, checkpoint);
    check(same(rng.state(), rng_checkpoint), "missing context advanced RNG");

    auto invalid_animation = animation;
    invalid_animation.angular_velocity.y = invalid_float;
    auto invalid_inputs = inputs;
    invalid_inputs.post_time_zero = &invalid_animation;
    check(pool.spawn_effect51(single, invalid_inputs, &rng).status == effect::Status::invalid_state,
          "invalid ancillary ANM projection was accepted");
    invalid_inputs = inputs;
    invalid_inputs.multiplier = invalid_float;
    check(pool.spawn_effect51(single, invalid_inputs, &rng).status == effect::Status::invalid_state,
          "nonfinite frame multiplier was accepted");
    auto overflowing_camera = camera;
    overflowing_camera.position.x = std::numeric_limits<float>::max();
    overflowing_camera.look_at_offset.x = std::numeric_limits<float>::max();
    invalid_inputs = inputs;
    invalid_inputs.camera = &overflowing_camera;
    const auto invalid =
        pool.spawn_effect51({16, single.position, single.color}, invalid_inputs, &rng);
    check(invalid.status == effect::Status::invalid_state && invalid.allocated == 0 &&
              !invalid.committed(),
          "post-draw overflow was converted into source deactivation or success");
    check(pool.spawn_effect51({1, {invalid_float, 0, 0}, {}}, inputs, &rng).status ==
              effect::Status::invalid_state,
          "nonfinite request position was accepted");
    check_unchanged(pool, checkpoint);
    check(same(rng.state(), rng_checkpoint), "invalid initialization leaked provisional RNG draws");
    auto fork = checkpoint;
    auto fork_rng = rng_api::Rng(rng_checkpoint);
    const auto retried = pool.spawn_effect51(single, inputs, &rng);
    const auto forked = fork.spawn_effect51(single, inputs, &fork_rng);
    check(retried.returned_slot == 1 && retried.returned_slot == forked.returned_slot &&
              same(pool.slot(1).particle, fork.slot(1).particle) &&
              same(rng.state(), fork_rng.state()) && checkpoint.active_count() == 1 &&
              !checkpoint.occupied(1),
          "retry/fork repeated draws, aliased storage or reused stale scratch");
    effect::PrimaryPool assigned;
    assigned = pool;
    const auto assigned_before = assigned;
    check(fork.spawn_effect51(single, inputs, &fork_rng).returned_slot == 2 && !pool.occupied(2) &&
              !assigned.occupied(2),
          "snapshot branches alias mutable effect slots");
    assigned = assigned;
    check_unchanged(assigned, assigned_before);

    // Independent occupancy/count/cursor model, all starting cursors. It uses
    // explicit circular indices rather than the implementation's staged scan.
    for (std::size_t start = 0; start < effect::primary_capacity; ++start) {
        for (const std::int32_t count : {1, 7, 300, 0, -17}) {
            std::size_t expected_allocated = 0, inspected = 512;
            std::size_t returned = effect::exhausted_effect, next_cursor = start;
            occupancy.fill(false);
            for (std::size_t i = 0; i < effect::primary_capacity; ++i)
                occupancy[i] = (i * 37 + start * 11) % 7 < 3;
            auto expected_occupancy = occupancy;
            for (std::size_t distance = 0; distance < effect::primary_capacity; ++distance) {
                const auto index = (start + distance) % effect::primary_capacity;
                if (occupancy[index])
                    continue;
                expected_occupancy[index] = true;
                ++expected_allocated;
                if (count > 0 && expected_allocated == std::size_t(count)) {
                    inspected = distance + 1;
                    returned = index;
                    next_cursor = (index + 1) % effect::primary_capacity;
                    break;
                }
            }
            effect::PrimaryPool actual(occupancy, start);
            rng_api::Rng actual_rng{std::uint16_t(start)}, draw_reference{std::uint16_t(start)};
            const auto result =
                actual.spawn_effect51({count, single.position, single.color}, inputs, &actual_rng);
            for (std::size_t draw = 0; draw < expected_allocated * 16; ++draw)
                draw_reference.next_u16();
            check(result.allocated == expected_allocated && result.inspected == inspected &&
                      result.returned_slot == returned && actual.cursor() == next_cursor &&
                      result.status == (returned == effect::exhausted_effect
                                            ? effect::Status::exhausted
                                            : effect::Status::spawned) &&
                      same(actual_rng.state(), draw_reference.state()),
                  "circular occupancy differential or exact sixteen-draw allocation failed");
            for (std::size_t i = 0; i < effect::primary_capacity; ++i)
                check(actual.occupied(i) == expected_occupancy[i] &&
                          actual.slot(i).effect51_known == (!occupancy[i] && expected_occupancy[i]),
                      "circular differential modified a skipped or unreachable slot");
        }
    }
    bool bad_cursor = false, bad_slot = false;
    try {
        const effect::PrimaryPool rejected(occupancy, effect::primary_capacity);
    } catch (const std::invalid_argument &) {
        bad_cursor = true;
    }
    try {
        pool.slot(effect::primary_capacity);
    } catch (const std::out_of_range &) {
        bad_slot = true;
    }
    check(bad_cursor && bad_slot, "effect pool accepted an out-of-bounds cursor or slot");
    std::cout << "Effect51 primary pool scan, ANM ownership, RNG and atomic retries: passed\n";
}
