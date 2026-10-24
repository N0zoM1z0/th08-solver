#include "ReplayManager.hpp"
#include "modern/headless/session.hpp"
#include "th08/shadow_bridge.hpp"
#include "th08/shadow_compare.hpp"
#include "th08/shadow_native.hpp"
#include "th08/shadow_start.hpp"
#include "wine-shadow/file_hash.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;
namespace s = th08::shadow;
namespace h = th08::headless;
namespace b = s::bridge;
namespace {
struct Scratch {
    fs::path previous = fs::current_path(), directory;
    Scratch() {
        char name[] = "/tmp/th08-recorded-replay-XXXXXX";
        if (!mkdtemp(name))
            throw std::runtime_error("shadow_export_scratch_failed");
        directory = name;
        fs::current_path(directory);
    }
    ~Scratch() {
        std::error_code ignored;
        fs::current_path(previous, ignored);
        fs::remove_all(directory, ignored);
    }
};
unsigned completed_rows(const fs::path &path) {
    std::array<std::uint32_t, b::map_words> commands{};
    std::ifstream input(path, std::ios::binary);
    if (!input.read(reinterpret_cast<char *>(commands.data()), b::map_bytes) ||
        input.peek() != std::char_traits<char>::eof() || commands[b::Magic] != b::magic ||
        commands[b::Version] != b::version ||
        commands[b::Outcome] != unsigned(b::OutcomeCode::Complete) || !commands[b::Sequence])
        throw std::runtime_error("shadow_export_requires_completed_owner");
    return commands[b::Sequence];
}
int run(int argc, char **argv) {
    if (argc != 4)
        throw std::runtime_error("usage: th08_shadow_export DAT RECORDING_DIRECTORY OUTPUT.rpy");
    const auto dat = fs::absolute(argv[1]).string();
    const auto run = fs::absolute(argv[2]), output = fs::absolute(argv[3]);
    if (fs::exists(output))
        throw std::runtime_error("shadow_export_output_exists");
    std::ifstream start(run / "start.txt");
    const auto record = s::read_start(start);
    const auto expected_rows = completed_rows(run / "commands.bin");
    if (expected_rows > record.maximum_updates || record.spell >= 0)
        throw std::runtime_error("shadow_export_requires_complete_stage_or_story");
    std::ifstream journal(run / "frames.bin", std::ios::binary);
    if (!journal || fs::file_size(run / "frames.bin") != expected_rows * b::trace_bytes)
        throw std::runtime_error("shadow_export_incomplete_journal");
    h::Config config;
    config.seed = std::uint16_t(record.seed);
    config.stage = record.stage;
    config.difficulty = record.difficulty;
    config.shot = record.shot;
    config.story = record.story;
    config.prior_final_a_clear = record.prior_final_a_clear;
    config.dat_path = dat.c_str();
    config.input_profile = h::InputProfile::SampledGameplay;
    Scratch scratch;
    h::Session session(config);
    s::Packet native;
    native.state = s::capture(session);
    b::Trace bytes;
    std::uint64_t rows = 0, numeric = 0;
    while (journal.read(reinterpret_cast<char *>(bytes.data()), bytes.size())) {
        const auto actual = s::decode(bytes.data(), s::Bytes{}.size());
        if (actual.sequence != rows + 1 || s::stage_terminal(config.story, session.state()))
            throw std::runtime_error("shadow_export_bad_receipt_index");
        session.step(std::uint16_t(actual.input));
        s::advance_receipt(native, actual.input, s::capture(session));
        const auto comparison = s::compare_live(actual, native, false);
        if (comparison.failure)
            throw std::runtime_error(comparison.failure);
        if (native.state[s::Field::Deaths] || native.state[s::Field::BombsUsed] ||
            native.state[s::Field::PlayerState] == 2)
            throw std::runtime_error("shadow_export_requires_nmnb");
        numeric += comparison.numeric_words != 0;
        ++rows;
    }
    if (!journal.eof() || journal.gcount() || rows != expected_rows ||
        !s::stage_terminal(config.story, session.state()) ||
        (record.prior_final_a_clear && session.state().stage != 7))
        throw std::runtime_error("shadow_export_not_complete_nmnb");
    // Upstream SaveReplay frees/mutates the recorder. Keep that destruction in a
    // child after the entire actual journal has agreed, without planning again.
    const auto child = fork();
    if (child < 0)
        throw std::runtime_error("shadow_export_writer_fork_failed");
    if (!child) {
        th08::ReplayManager::SaveReplay(output.c_str(), "CAPTURE ");
        _exit(fs::is_regular_file(output) && fs::file_size(output) ? 0 : 1);
    }
    int status = 0;
    if (waitpid(child, &status, 0) != child || !WIFEXITED(status) || WEXITSTATUS(status))
        throw std::runtime_error("shadow_export_writer_failed");
    std::cout << "{\"outcome\":\"complete\",\"rows\":" << rows << ",\"seed\":" << record.seed
              << ",\"score\":" << native.state[s::Field::Score]
              << ",\"numeric_drift_updates\":" << numeric
              << ",\"prior_final_a_clear\":" << (record.prior_final_a_clear ? "true" : "false")
              << ",\"replay_bytes\":" << fs::file_size(output) << ",\"trace_sha256\":\""
              << wine_shadow::file_hash((run / "frames.bin").string()) << "\",\"replay_sha256\":\""
              << wine_shadow::file_hash(output.string())
              << "\",\"retail_playback_checked\":false}\n";
    return 0;
}
} // namespace
int main(int argc, char **argv) try {
    return run(argc, argv);
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 2;
}
