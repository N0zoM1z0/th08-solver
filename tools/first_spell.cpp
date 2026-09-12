#include <fstream>
#include <iostream>
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
    {"EclDependencies.cpp", "019f9cd6abdb73223d3d41cc8a6317641e6fe6bfbd7777d126a4bace3e14e2e4"}};
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
        require(stop.status == p::Status::unsupported_world_effect && stop.sub == 0 &&
                    stop.pc == 1 && stop.offset == 260 && stop.opcode == 139 && stop.actor == 0 &&
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
               << ", \"reason\": \"effect51 allocation, "
                  "ANM initialization and camera/shared-RNG ownership are not integrated\"},\n"
               << "  \"pending_spawn\": {\"active_slots\": " << entry.pool().active_count()
               << ", \"life\": " << actor.life << ", \"score\": " << actor.score
               << ", \"item_drop\": " << int(actor.item_drop)
               << ", \"max_life\": " << actor.max_life
               << ", \"ecl_instructions_examined\": " << actor.execution.result.executed
               << ", \"post_spawn_stores_applied\": false, \"timeline_acknowledged\": false},\n"
               << "  \"complete_worlds\": 0,\n  \"complete_spell_solutions\": 0,\n"
               << "  \"acceptance_gates_passed\": 0,\n  \"blocked_resumptions_checked\": 8\n}\n";
        report.close();
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
