// Stream the pinned upstream replay-state rows from the native playback path.
// Process exit, natural completion and successful row writes are all required.
#include "modern/headless/replay_check.hpp"
#include "modern/headless/session.hpp"
#include <array>
#include <charconv>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace {
std::string sha256(const std::filesystem::path &path) {
    std::ifstream input(path, std::ios::binary);
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(),
                                                                    EVP_MD_CTX_free);
    if (!input || !context || EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("identity_read_failed");
    std::array<char, 65536> buffer{};
    while (input.read(buffer.data(), buffer.size()) || input.gcount()) {
        if (EVP_DigestUpdate(context.get(), buffer.data(), std::size_t(input.gcount())) != 1)
            throw std::runtime_error("identity_hash_failed");
    }
    std::array<unsigned char, EVP_MAX_MD_SIZE> bytes{};
    unsigned size = 0;
    if (!input.eof() || EVP_DigestFinal_ex(context.get(), bytes.data(), &size) != 1 || size != 32)
        throw std::runtime_error("identity_hash_failed");
    std::ostringstream result;
    result << std::hex << std::setfill('0');
    for (unsigned i = 0; i < size; ++i)
        result << std::setw(2) << unsigned(bytes[i]);
    return result.str();
}
} // namespace

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
    // Identify the actual immutable inputs before playback; private paths never
    // enter the report. Archive consumers bind their bytes to this capture.
    std::string dat_sha256, replay_sha256, binary_sha256;
    try {
        dat_sha256 = sha256(dat);
        replay_sha256 = sha256(replay);
        binary_sha256 = sha256("/proc/self/exe");
    } catch (...) {
        std::fprintf(stderr, "replay input identity unavailable\n");
        return 1;
    }
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
    std::printf("{\"source_revision\":\"02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415\","
                "\"replay_sha256\":\"%s\",\"dat_sha256\":\"%s\",\"binary_sha256\":\"%s\","
                "\"rows\":%u,\"finished\":%s,\"natural\":%s,\"steps\":%llu,"
                "\"runtime_error\":%s,\"io_error\":%s,\"death_rows\":%u,"
                "\"bomb_input_rows\":%u,\"stages\":[",
                replay_sha256.c_str(), dat_sha256.c_str(), binary_sha256.c_str(), summary.rows,
                summary.finished ? "true" : "false", summary.natural ? "true" : "false",
                static_cast<unsigned long long>(steps), runtime_error ? "true" : "false",
                io_error ? "true" : "false", summary.death_rows, summary.bomb_input_rows);
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
