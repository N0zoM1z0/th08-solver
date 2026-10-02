#include <fstream>
#include <iostream>
#include <th08/effect_animation.hpp>
#include <th08/practice_camera.hpp>
#include <th08/practice_entry.hpp>

namespace r = th08::resources;
namespace p = th08::practice;
namespace fs = std::filesystem;
namespace {
struct Pin {
    const char *file, *sha256;
};
constexpr Pin source_pins[] = {
    {"EnemyTimeline.cpp", "920ee34725aa6aad9f113d43454731acadab456abddac73256b2ba9a29e8e94b"},
    {"EnemyManager.cpp", "e8febe94a833472b33f732e83ee39ee48fdc5097c5d69ff094fd1f1bb8629a7d"},
    {"EnemyManager.hpp", "e56633232cfb8e0934fb9e83f592989b577cd045e623df0c2c294eed9b2bf256"},
    {"EclManager.cpp", "18f06e5aa827b52eee16755b2fb07e780362b057426f6492050c13db1dd6e93d"},
    {"EclRun.cpp", "010049211263e47d8245c7335f56b17a8502ca0f84595c8b035926a495d90b57"},
    {"EclRunLow.inl", "8c6d23bf4e9daf8f96dbd344f4a03d3ed32d1d200483959682e962cd41ec0045"},
    {"EclRunHigh.inl", "5e8c0b8ac1bd35f92cf2c3eb8792b2fb62d00feb97a21dc27e935101c5a45914"},
    {"EclDependencies.cpp", "019f9cd6abdb73223d3d41cc8a6317641e6fe6bfbd7777d126a4bace3e14e2e4"},
    {"EffectManager.cpp", "63d45a213956008b44874bc4707c971a7799a9c551b07e732bf1f55282c2209e"},
    {"AnmManager.cpp", "c82bb37c19af4ccaabfa4bf4606d92c72e180f5f2fdd642cf3ec2131c85cecce"},
    {"AsciiManager.cpp", "86c0d3cca5040036f16de762e80b3126b7037c89b526044cbb74bcc4bc6abdb1"},
    {"Background.cpp", "36889a17a6f0c314eaf3751a2238e778c82d13bffd3a18e55051b49f3d8fd285"},
    {"Background.hpp", "bbfa9022f52c5b5332f8e690d42c7338ec97f062b43a3bfcd6dc33190484efe8"}};
void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}
} // namespace

int main(int argc, char **argv) try {
    require(argc == 4, "Usage: th08_first_spell th08.dat pinned-source report-directory");
    auto bytes = r::read_file(argv[1]);
    const auto dat_hash = r::sha256(r::view(bytes));
    require(dat_hash == "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb",
            "DAT identity mismatch");
    for (const auto &pin : source_pins) {
        const auto source = r::read_file(fs::path(argv[2]) / "src" / pin.file);
        require(r::sha256(r::view(source)) == pin.sha256, "source identity mismatch");
    }
    r::Archive archive(std::move(bytes));
    for (std::size_t member = 0; member < archive.entries().size(); ++member) {
        if (archive.entries()[member].name != "ecldata1sp.ecl")
            continue;
        const auto data = archive.decode(member);
        const auto member_hash = r::sha256(r::view(data));
        require(member_hash == "aac506b4eaf8fdfaa90e876f74db711d3c0724f63798e7d62801b02bbb29e00e",
                "practice member identity mismatch");
        const auto decoded = r::parse_ecl(r::view(data));
        bool selected_site = false;
        for (const auto &site : decoded.spells)
            selected_site |=
                site.id == 2 && site.sub == 24 && site.offset == 18116 && site.mask == 0xf1;
        require(selected_site, "selected ID2 spell site mismatch");
        const auto code = std::make_shared<const p::Programs>(r::view(data), decoded);
        p::Entry entry(code, 1);
        th08::timeline::Context observations;
        // This is an explicitly supplied gate observation for the entry-prefix
        // test, not a claim to have run GUI/background/player initialization.
        observations.gui_boss_present = observations.spawns_suppressed =
            th08::timeline::KnownBool::no;
        const auto initial = entry.advance(observations);
        require(initial.status == p::Status::timeline_frame_complete,
                "unexpected initial timeline stop");
        const auto stop = entry.advance(observations);
        require(stop.status == p::Status::missing_entry_state && stop.sub == 0 && stop.pc == 1 &&
                    stop.offset == 260 && stop.opcode == 139 && stop.actor == 0 &&
                    entry.pool().pending(),
                "first blocker changed; review and refresh producer assertions with the "
                "implementation");
        const auto &timeline = entry.timeline_state();
        const auto &actor = entry.pool().actor(stop.actor);
        const auto before = actor.execution.result.executed;
        for (int retry = 0; retry < 8; ++retry)
            require(entry.advance(observations).offset == stop.offset &&
                        entry.pool().active_count() == 1 &&
                        actor.execution.result.executed == before && timeline.pc == 0 &&
                        timeline.time.current == 1 && timeline.effect_token == 1,
                    "blocked resumption duplicated state changes");

        fs::create_directories(argv[3]);
        {
            std::ofstream pending(fs::path(argv[3]) / "first_spell_summary.json");
            pending.exceptions(std::ios::failbit | std::ios::badbit);
            pending << "{\"status\":\"INCOMPLETE\"}\n";
        }
        std::ofstream trace(fs::path(argv[3]) / "first_spell_trace.tsv");
        trace.exceptions(std::ios::failbit | std::ios::badbit);
        trace
            << "phase\ttimeline_time\ttimeline_pc\ttimeline_offset\tactor\tsub\tpc\toffset\topcode"
               "\tinstruction_mask\texecution_mask\tlocal_time\tstatus\n"
            << "timeline_control\t1\t0\t39684\t-1\t-1\t-1\t-1\t-1\t255\t1\t-1\t"
            << p::name(initial.status) << '\n'
            << "immediate_spawn_ecl\t" << timeline.time.current << '\t' << timeline.pc << '\t'
            << code->timeline.code[timeline.pc].offset << '\t' << stop.actor << '\t' << stop.sub
            << '\t' << stop.pc << '\t' << stop.offset << '\t' << stop.opcode << '\t'
            << unsigned(stop.instruction_mask) << '\t' << unsigned(stop.execution_mask) << '\t'
            << stop.local_time << '\t' << p::name(stop.status) << '\n';
        trace.close();
        std::ofstream report(fs::path(argv[3]) / "first_spell_summary.json");
        report.exceptions(std::ios::failbit | std::ios::badbit);
        report << "{\n  \"status\": \"" << p::name(stop.status)
               << "\",\n  \"scope\": \"owned timeline-to-spawn prefix with supplied GUI gates; "
                  "not a complete entry, world, or solution\",\n"
               << "  \"dat_sha256\": \"" << dat_hash << "\",\n  \"member\": \"ecldata1sp.ecl\",\n"
               << "  \"member_sha256\": \"" << member_hash << "\",\n"
               << "  \"reference_revision\": \"a45e99fb1942714e6edded20847e32a654d56f97\",\n"
               << "  \"source_sha256\": {\n";
        bool first = true;
        for (const auto &pin : source_pins) {
            report << (first ? "" : ",\n") << "    \"src/" << pin.file << "\": \"" << pin.sha256
                   << '"';
            first = false;
        }
        report << "\n  },\n  \"selected_case\": {\"spell_id\": 2, \"difficulty\": \"Easy\", "
                  "\"difficulty_mask\": 1, \"entry\": \"spell_practice\", \"timeline\": 0, "
                  "\"wrapper_sub\": 42, \"spell_sub\": 24, \"spell_pc\": 10, \"spell_offset\": "
                  "18116},\n"
               << "  \"input_state\": {\"provenance\": \"source manager template plus explicitly "
                  "supplied "
                  "GUI gate observations; surrounding world phases not executed\", "
                  "\"gui_boss_present\": false, \"timeline_spawns_suppressed\": false, "
                  "\"rng_seed\": null, \"rng_draw_counter\": null, \"player_state\": null},\n"
               << "  \"numerical_profile\": \"native float32, no fast-math, unit-rate "
                  "ECL/timeline\",\n"
               << "  \"first_blocker\": {\"phase\": \"immediate_spawn_ecl\", \"timeline_pc\": "
               << timeline.pc
               << ", \"timeline_offset\": " << code->timeline.code[timeline.pc].offset
               << ", \"timeline_time\": " << timeline.time.current << ", \"actor\": " << stop.actor
               << ", \"sub\": " << stop.sub << ", \"pc\": " << stop.pc
               << ", \"offset\": " << stop.offset << ", \"opcode\": " << stop.opcode
               << ", \"instruction_mask\": " << unsigned(stop.instruction_mask)
               << ", \"execution_mask\": " << unsigned(stop.execution_mask)
               << ", \"local_time\": " << stop.local_time
               << ", \"reason\": \"effect51 pool occupancy, camera and shared RNG "
                  "must be supplied by the missing entry-world owner\"},\n"
               << "  \"pending_spawn\": {\"active_slots\": " << entry.pool().active_count()
               << ", \"life\": " << actor.life << ", \"score\": " << actor.score
               << ", \"item_drop\": " << int(actor.item_drop)
               << ", \"max_life\": " << actor.max_life
               << ", \"ecl_instructions_examined\": " << actor.execution.result.executed
               << ", \"post_spawn_stores_applied\": false, \"timeline_acknowledged\": false},\n"
               << "  \"complete_worlds\": 0,\n  \"complete_spell_solutions\": 0,\n"
               << "  \"acceptance_gates_passed\": 0,\n  \"blocked_resumptions_checked\": 8\n}\n";
        report.close();
        for (std::size_t anm_member = 0; anm_member < archive.entries().size(); ++anm_member) {
            if (archive.entries()[anm_member].name != "enemy.anm")
                continue;
            const auto anm_bytes = archive.decode(anm_member);
            const auto effect_anm = th08::effect::compile_effect51_animation(r::view(anm_bytes));
            const auto background_anm =
                th08::effect::compile_background62_animation(r::view(anm_bytes));
            r::Bytes stage_bytes;
            for (std::size_t i = 0; i < archive.entries().size(); ++i)
                if (archive.entries()[i].name == "stage1_s.std")
                    stage_bytes = archive.decode(i);
            const th08::practice::camera::Program camera_program(r::view(stage_bytes));
            th08::practice::camera::State camera_state;
            // Camera runs before EnemyManager on both the initial empty
            // timeline phase and the following phase that first spawns sub0.
            for (int phase = 0; phase < 2; ++phase)
                require(camera_program.advance(camera_state, false, 1).status ==
                            th08::practice::camera::Status::advanced,
                        "practice camera prefix blocked");
            const auto cp_vector = [](th08::practice::camera::Vec3 value) {
                return th08::effect::camera_particle::Vec3{value.x, value.y, value.z};
            };
            const th08::effect::camera_particle::Camera camera{
                cp_vector(camera_state.position.current),
                cp_vector(camera_state.look_at_offset.current), cp_vector(camera_state.forward)};
            // A separately supplied component checkpoint: Background's first
            // 12 effect62 requests see reset specialEffectPoints (all zero).
            // Their first EffectManager phase advances ANM but consumes no RNG.
            // This does NOT execute omitted player/background-object phases.
            th08::effect::PrimaryPool prepared_pool;
            for (int i = 0; i < 12; ++i)
                require(prepared_pool.spawn_background62({}, &background_anm).committed(),
                        "background effect checkpoint failed");
            require(prepared_pool.advance_particles({false}).committed(),
                    "background effect update failed");
            const th08::effect::Effect51Inputs inputs{&effect_anm, &camera, 1};
            // Seed0 is explicitly supplied at the immediate-ECL checkpoint,
            // not inferred from game startup or omitted global RNG consumers.
            th08::random::Rng rng(0);
            p::Entry supplied(code, 1, std::move(prepared_pool));
            require(supplied.advance(observations, &rng, nullptr, 100000, &inputs).status ==
                        p::Status::timeline_frame_complete,
                    "supplied first timeline mismatch");
            const auto next = supplied.advance(observations, &rng, nullptr, 100000, &inputs);
            require(next.status == p::Status::timeline_frame_complete &&
                        supplied.timeline_state().pc == 1 && !supplied.pool().pending() &&
                        supplied.pool().effects()->active_count() == 28 &&
                        rng.generation_count() == 256,
                    "supplied effect51 entry regression");
            std::ofstream extra(fs::path(argv[3]) / "first_spell_effect51_summary.json");
            extra.exceptions(std::ios::failbit | std::ios::badbit);
            extra << "{\n  \"status\": \"" << p::name(next.status)
                  << "\",\n  \"scope\": \"source-derived camera and background62 component "
                     "checkpoint; supplied GUI gates and seed0 at immediate ECL; not a full "
                     "world\",\n"
                  << "  \"dat_sha256\": \"" << dat_hash << "\",\n"
                  << "  \"ecl_member_sha256\": \"" << member_hash << "\",\n"
                  << "  \"reference_revision\": \"a45e99fb1942714e6edded20847e32a654d56f97\",\n"
                  << "  \"source_provenance\": \"first_spell_summary.json source_sha256; "
                     "all listed files verified during this same command\",\n"
                  << "  \"anm_sha256\": \"" << r::sha256(r::view(anm_bytes)) << "\",\n"
                  << "  \"std_sha256\": \"" << r::sha256(r::view(stage_bytes)) << "\",\n"
                  << "  \"camera_steps\": 2,\n  \"background_effect62_slots\": 12,\n"
                     "  \"effect51_slots\": 16,\n"
                  << "  \"effect_slots\": " << supplied.pool().effects()->active_count()
                  << ",\n  \"rng_seed\": " << rng.seed()
                  << ",\n  \"rng_draws\": " << rng.generation_count()
                  << ",\n  \"timeline_boundary\": {\"pc\": " << next.pc
                  << ", \"offset\": " << next.offset << ", \"opcode\": " << next.opcode
                  << "},\n  \"next_missing_behavior\": \"EnemyManager/effect/background/player "
                     "frame phases before advancing another timeline frame\",\n"
                     "  \"complete_worlds\": 0,\n  \"complete_spell_solutions\": 0,\n"
                     "  \"acceptance_gates_passed\": 0\n}\n";
            std::cout << "Supplied effect51 prefix: " << p::name(next.status) << " timeline PC"
                      << next.pc << " effects=" << supplied.pool().effects()->active_count()
                      << " draws=" << rng.generation_count() << '\n';
        }
        std::cout << "ID2 Easy practice: " << p::name(stop.status)
                  << " at sub0 PC1 offset260 opcode139; pending timeline39684 retained. "
                     "No complete spell solution.\n";
        // Diagnostic report generation succeeded. Consult status, not this exit
        // code, for world/solver completion; malformed inputs still return1.
        return 0;
    }
    throw std::runtime_error("practice member missing");
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
