// Stream the pinned upstream replay-state rows from the native playback path.
// Process exit, natural completion and successful row writes are all required.
#include "modern/headless/replay_check.hpp"
#include "modern/headless/session.hpp"
#include <charconv>
#include <cstdio>
#include <filesystem>
#include <string>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 4 && argc != 5) {
        std::fprintf(stderr, "usage: th08_replay_check DAT REPLAY ROWS_OUT [START_STAGE]\n");
        return 1;
    }
    int stage = 0;
    if (argc == 5) {
        const std::string value = argv[4];
        const auto parsed = std::from_chars(value.data(), value.data() + value.size(), stage);
        if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || stage < 0 ||
            stage > 8)
            return 1;
    }
    namespace fs = std::filesystem;
    const std::string dat = fs::absolute(argv[1]), replay = fs::absolute(argv[2]);
    const auto rows_path = fs::absolute(argv[3]);
    std::FILE *rows = std::fopen(rows_path.c_str(), "wb");
    if (!rows)
        return 1;
    char scratch[] = "/tmp/th08-replay-XXXXXX";
    if (!mkdtemp(scratch)) {
        std::fclose(rows);
        return 1;
    }
    std::uint64_t steps = 0;
    bool runtime_error = false;
    try {
        fs::current_path(scratch);
        th08::headless::replay_check_enable(rows);
        th08::headless::Config config;
        config.dat_path = dat.c_str();
        config.replay = replay.c_str();
        config.stage = stage;
        // The title screen may install a headless-incompatible setup thread at
        // replay end. The process owns this session until _exit, without teardown.
        auto *session = new th08::headless::Session(config);
        while (!th08::headless::replay_check_summary().finished && steps < 400000) {
            session->step(0);
            ++steps;
        }
    } catch (...) {
        runtime_error = true;
    }
    const auto &summary = th08::headless::replay_check_summary();
    const bool flush_ok = std::fflush(rows) == 0;
    const bool close_ok = std::fclose(rows) == 0;
    const bool io_error = summary.write_failed || !flush_ok || !close_ok;
    const bool ok =
        !runtime_error && !io_error && summary.finished && summary.natural && summary.rows > 0;
    std::printf("{\"rows\":%u,\"finished\":%s,\"natural\":%s,\"steps\":%llu,"
                "\"runtime_error\":%s,\"io_error\":%s,\"death_rows\":%u,"
                "\"bomb_input_rows\":%u,\"stages\":[",
                summary.rows, summary.finished ? "true" : "false",
                summary.natural ? "true" : "false", static_cast<unsigned long long>(steps),
                runtime_error ? "true" : "false", io_error ? "true" : "false", summary.death_rows,
                summary.bomb_input_rows);
    for (int i = 0; i < summary.stage_count; ++i)
        std::printf("%s{\"stage\":%d,\"frames\":%u,\"endScore\":%u}", i ? "," : "",
                    summary.stages[i], summary.stage_frames[summary.stages[i]],
                    summary.stage_scores[summary.stages[i]]);
    std::printf("]}\n");
    const bool report_ok = std::fflush(stdout) == 0;
    std::error_code ignored;
    fs::current_path("/", ignored);
    fs::remove_all(scratch, ignored);
    _exit(ok && report_ok ? 0 : 2);
}
