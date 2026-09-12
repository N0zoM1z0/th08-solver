// Source-driven component fixtures, not complete Wriggle spell worlds.
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <th08/animation.hpp>
#include <th08/bullet_motion.hpp>
#include <th08/emitter.hpp>
#include <th08/planner.hpp>
#include <th08/transform_program.hpp>

namespace res = th08::resources;
namespace vm = th08::emitter;
namespace motion = th08::kinematics;
namespace bullet = th08::bullet;
namespace transform = th08::bullet::transform;
namespace solver = th08::solver;
namespace fs = std::filesystem;
namespace {
void require(bool valid, const char *message) {
    if (!valid)
        throw std::runtime_error(message);
}
float number(std::uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
std::int32_t integer(std::uint32_t bits) {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
std::uint32_t bits(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
res::Bytes member(const res::Archive &archive, const char *name) {
    for (std::size_t i = 0; i < archive.entries().size(); ++i)
        if (archive.entries()[i].name == name)
            return archive.decode(i);
    throw std::runtime_error("missing fixture resource");
}
struct Born {
    std::uint32_t tick;
    bullet::Particle particle;
};
std::vector<Born> expand(const vm::Workspace &workspace, const th08::animation::Timing &timing,
                         const res::Anm &anm) {
    std::vector<Born> births;
    births.reserve(840);
    // Explicit fixture entry state: empty transform table, cursor zero, no rank
    // adjustment, no suppression, fixed emitter at (192,96), empty bullet pool.
    transform::Program transforms;
    std::size_t applied = 0;
    for (const auto &event : workspace.emissions) {
        require(event.transform_write_count <= workspace.transforms.size(),
                "invalid transform history");
        while (applied < event.transform_write_count) {
            const auto &write = workspace.transforms[applied++].words;
            require(write[0] < transforms.records.size(), "invalid transform slot");
            transforms.records[write[0]] = {number(write[5]),  number(write[6]), integer(write[3]),
                                            integer(write[4]), write[1],         integer(write[2])};
        }
        const auto &words = event.words;
        const auto type = words[0] & 0xffffU, color = words[0] >> 16;
        require(type == 2 && (event.opcode == 97 || event.opcode == 99),
                "fixture gained an unsupported bullet type or aim mode");
        require((words[7] & ~0x2242U) == 0 && (words[7] & 2) != 0,
                "fixture gained unsupported spawn flags");
        const auto sprite_id = 32 + color;
        require(sprite_id < anm.sprites.size() && anm.sprites[sprite_id].raw_id == sprite_id,
                "fixture sprite mapping changed");
        const auto &sprite = anm.sprites[sprite_id];
        const motion::Pattern pattern{motion::Aim(event.opcode - 96),
                                      std::int32_t(words[1]),
                                      std::int32_t(words[2]),
                                      number(words[3]),
                                      number(words[4]),
                                      number(words[5]),
                                      number(words[6])};
        for (int layer = 0; layer < pattern.count2; ++layer)
            for (int spoke = 0; spoke < pattern.count1; ++spoke) {
                motion::Launch launch{};
                require(motion::launch(pattern, spoke, layer, 0, 1, {}, launch) ==
                            motion::Status::ready,
                        "fixture launch failed");
                bullet::InitialState initial{launch,
                                             192,
                                             96,
                                             sprite.width,
                                             sprite.height,
                                             bullet::Phase::spawning_fast,
                                             timing.calls_after_template,
                                             0,
                                             {}};
                transform::State installed;
                installed.flight = {initial.x - launch.velocity_x * 4.0f,
                                    initial.y - launch.velocity_y * 4.0f,
                                    launch.velocity_x,
                                    launch.velocity_y,
                                    launch.angle,
                                    launch.speed};
                installed.enabled_flags = words[7];
                const auto installation = transform::advance_program(transforms, installed, 1);
                require(installation.status == bullet::Status::advanced &&
                            installation.sound_count == 0 &&
                            (installed.active_flags & ~transform::relative) == 0 &&
                            (installed.pc == transforms.records.size() ||
                             transforms.records[installed.pc].kind == transform::none),
                        "fixture gained later transform scheduling or unsupported effects");
                require(installed.offscreen_cull_delay >= 0 &&
                            installed.offscreen_cull_delay <= 32767 &&
                            installed.turn.interval >= 0 && installed.turn.interval <= 32767 &&
                            installed.turn.repeats >= 0 && installed.turn.repeats <= 32767,
                        "fixture transform timing outside verified particle bounds");
                initial.cull_delay = installed.offscreen_cull_delay;
                initial.turn = installed.turn;
                initial.turn.active = (installed.active_flags & transform::relative) != 0;
                bullet::Particle particle;
                require(bullet::initialize(initial, particle) == bullet::Status::advanced,
                        "fixture particle initialization failed");
                births.push_back({event.tick, particle});
            }
    }
    require(births.size() == 840 && births.size() < 1536,
            "empty-pool no-contention fixture assumption failed");
    return births;
}
void solve_case(const res::Bytes &ecl_bytes, const res::Ecl &ecl, const res::Anm &anm,
                const th08::animation::Timing &timing, const res::ShtHeader &player, unsigned sub,
                const fs::path &output, std::ostream &summary) {
    constexpr unsigned horizon = 600;
    vm::Workspace workspace;
    const vm::Program program(res::view(ecl_bytes), ecl, sub);
    const auto schedule = vm::run(program, workspace, 8);
    require(schedule.status == vm::Status::returned && schedule.tick == 360,
            "fixture schedule changed");
    auto births = expand(workspace, timing, anm);
    solver::Model model;
    model.dependency = solver::Dependency::fixture;
    model.frames.reserve(horizon);
    const float half = player.hurtbox_size / 2.0f;
    std::uint64_t digest = 1469598103934665603ULL, particle_frames = 0;
    std::size_t peak_lethal = 0;
    std::ofstream frames(output / ("motion_sub" + std::to_string(sub) + "_frames.tsv"));
    frames.exceptions(std::ios::badbit | std::ios::failbit);
    frames << "tick\tallocated_total\tlive\tlethal\tstate_digest\n";
    const auto generation_start = std::chrono::steady_clock::now();
    std::size_t born = 0;
    for (unsigned tick = 0; tick < horizon; ++tick) {
        while (born < births.size() && births[born].tick <= tick)
            ++born;
        std::vector<th08::geometry::Hazard> hazards;
        hazards.reserve(born);
        std::size_t live = 0;
        for (std::size_t id = 0; id < born; ++id) {
            auto &particle = births[id].particle;
            if (particle.phase == bullet::Phase::unused)
                continue;
            const auto status = bullet::advance(particle);
            require(status == bullet::Status::advanced || status == bullet::Status::inactive,
                    "fixture particle execution stopped");
            ++particle_frames;
            for (auto word :
                 {std::uint32_t(id), std::uint32_t(particle.phase), bits(particle.flight.x),
                  bits(particle.flight.y), bits(particle.flight.velocity_x),
                  bits(particle.flight.velocity_y), std::uint32_t(particle.turn.timer)})
                digest = (digest ^ word) * 1099511628211ULL;
            if (particle.phase != bullet::Phase::unused)
                ++live;
            if (particle.phase == bullet::Phase::fired)
                hazards.push_back(th08::geometry::Hazard::bullet(
                    {{particle.flight.x, particle.flight.y}, {4, 4}}));
        }
        peak_lethal = std::max(peak_lethal, hazards.size());
        frames << tick << '\t' << born << '\t' << live << '\t' << hazards.size() << '\t' << digest
               << '\n';
        model.frames.emplace_back(std::move(hazards), th08::geometry::Vec2{half, half});
    }
    const auto generation_end = std::chrono::steady_clock::now();
    solver::Options options;
    options.beam = 128;
    options.expansions = horizon * options.beam * 9;
    options.terminal = {{192, 400}, {368, 416}};
    const auto path =
        solver::plan(model, {192, 400}, {player.focused_axis, player.focused_diagonal}, options);
    const auto solve_end = std::chrono::steady_clock::now();
    require(path.status == solver::Status::found, "bounded fixture search did not find a witness");
    std::ofstream route(output / ("motion_sub" + std::to_string(sub) + "_route.tsv"));
    route.exceptions(std::ios::badbit | std::ios::failbit);
    route << std::setprecision(17) << "tick\taction_x\taction_y\tx\ty\n";
    for (std::size_t tick = 0; tick < path.actions.size(); ++tick)
        route << tick << '\t' << path.actions[tick].x << '\t' << path.actions[tick].y << '\t'
              << path.positions[tick].x << '\t' << path.positions[tick].y << '\n';
    summary << "{\"sub\":" << sub << ",\"frames\":" << horizon << ",\"births\":" << births.size()
            << ",\"particle_frames\":" << particle_frames << ",\"peak_lethal\":" << peak_lethal
            << ",\"state_digest\":" << digest << ",\"expansions\":" << path.expansions
            << ",\"generation_ms\":"
            << std::chrono::duration<double, std::milli>(generation_end - generation_start).count()
            << ",\"solve_and_replay_ms\":"
            << std::chrono::duration<double, std::milli>(solve_end - generation_end).count()
            << ",\"route\":\"FOUND_AND_REPLAYED\"}";
}
} // namespace
int main(int argc, char **argv) try {
    require(argc == 2 || argc == 3, "Usage: th08_motion_cases th08.dat [report-directory]");
    auto bytes = res::read_file(argv[1]);
    require(res::sha256(res::view(bytes)) ==
                "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb",
            "DAT identity mismatch");
    res::Archive archive(std::move(bytes));
    const auto ecl_bytes = member(archive, "ecldata1.ecl");
    const auto anm_bytes = member(archive, "etama.anm");
    const auto sht_bytes = member(archive, "ply00a.sht");
    const auto ecl = res::parse_ecl(res::view(ecl_bytes));
    const auto anm = res::parse_anm(res::view(anm_bytes));
    const auto player = res::parse_sht(res::view(sht_bytes)).header;
    const auto timing = th08::animation::certify_timing(res::view(anm_bytes), anm, 21);
    const auto main = th08::animation::certify_timing(res::view(anm_bytes), anm, 2);
    require(timing.status == th08::animation::Status::certified &&
                timing.calls_after_template == 10 &&
                main.status == th08::animation::Status::certified && main.sprite == 32 &&
                main.completion_time == 0 && !main.hides_on_completion,
            "bullet template certificate mismatch");
    const fs::path output = argc == 3 ? argv[2] : "reports/native";
    fs::create_directories(output);
    std::ofstream summary(output / "motion_summary.json");
    summary.exceptions(std::ios::badbit | std::ios::failbit);
    summary
        << "{\"scope\":\"source-driven component fixtures; fixed emitter and explicit empty entry "
           "state; "
           "no damage, gates, suppression, cancellation, parent world or complete spell claim\","
           "\"profile\":\"modern-port float32, unit frame rate, focused ply00a.sht\",\"cases\":[";
    solve_case(ecl_bytes, ecl, anm, timing, player, 40, output, summary);
    summary << ',';
    solve_case(ecl_bytes, ecl, anm, timing, player, 41, output, summary);
    summary << "],\"status\":\"PASSED_COMPONENT_FIXTURES\",\"complete_spell_solutions\":0}\n";
    std::cout
        << "Two 600-frame source-driven particle fixtures solved and independently replayed\n";
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
