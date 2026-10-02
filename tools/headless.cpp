#include "modern/headless/session.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <th08/reactive.hpp>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;
namespace {
unsigned number(const std::string &text, unsigned maximum) {
    std::size_t end = 0;
    const auto value = std::stoul(text, &end);
    if (end != text.size() || value > maximum || text.empty() || text.front() == '-')
        throw std::runtime_error("invalid numeric option: " + text);
    return unsigned(value);
}
int stage_index(const std::string &name) {
    const std::array<std::string, 9> names{"1", "2", "3", "4a", "4b", "5", "6a", "6b", "extra"};
    const auto found = std::find(names.begin(), names.end(), name);
    if (found == names.end())
        throw std::runtime_error("unknown stage: " + name);
    return int(found - names.begin());
}
std::string dat_hash(const std::string &path) {
    std::ifstream input(path, std::ios::binary);
    if (!input)
        throw std::runtime_error("cannot read DAT");
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!ctx || EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("SHA256 initialization failed");
    std::array<char, 65536> buffer;
    while (input.read(buffer.data(), buffer.size()) || input.gcount())
        if (EVP_DigestUpdate(ctx.get(), buffer.data(), input.gcount()) != 1)
            throw std::runtime_error("SHA256 update failed");
    if (!input.eof())
        throw std::runtime_error("DAT read failed");
    std::array<unsigned char, 32> digest;
    unsigned size = 0;
    if (EVP_DigestFinal_ex(ctx.get(), digest.data(), &size) != 1 || size != digest.size())
        throw std::runtime_error("SHA256 finalization failed");
    std::ostringstream out;
    for (auto byte : digest)
        out << std::hex << std::setw(2) << std::setfill('0') << unsigned(byte);
    return out.str();
}
class RunDirectory {
  public:
    RunDirectory() : previous(fs::current_path()) {
        char scratch[] = "/tmp/th08-headless-XXXXXX";
        if (!mkdtemp(scratch))
            throw std::runtime_error("cannot create isolated runtime directory");
        path = scratch;
        fs::current_path(path);
    }
    ~RunDirectory() {
        std::error_code error;
        fs::current_path(previous, error);
        fs::remove_all(path, error);
    }

  private:
    fs::path previous, path;
};
void hash(std::uint64_t &digest, std::uint64_t value) {
    digest = (digest ^ value) * 1099511628211ULL;
}
std::uint32_t bits(float value) {
    std::uint32_t out;
    std::memcpy(&out, &value, sizeof(out));
    return out;
}
std::vector<std::uint16_t> read_tape(const std::string &path) {
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("cannot open action tape");
    std::vector<std::uint16_t> actions;
    std::string token;
    while (input >> token) {
        if (actions.size() == 1000000)
            throw std::runtime_error("action tape exceeds frame bound");
        actions.push_back(number(token, 0xffff));
    }
    if (!input.eof() || actions.empty())
        throw std::runtime_error("invalid or empty action tape");
    return actions;
}
} // namespace

// The engine adapter owns the game. This CLI owns options, tapes and reports.
int main(int argc, char **argv) {
    try {
        th08::headless::Config config;
        std::string dat, output, strategy = "stationary", tape_path, replay_path, stage_name = "1";
        unsigned limit = 20000;
        bool shoot = false, shoot_set = false;
        for (int i = 1; i < argc; i += 2) {
            if (i + 1 >= argc)
                throw std::runtime_error("option requires a value");
            const std::string key = argv[i], value = argv[i + 1];
            if (key == "--dat")
                dat = fs::absolute(value).string();
            else if (key == "--spell-id")
                config.spell = number(value, 221);
            else if (key == "--stage") {
                config.stage = stage_index(value);
                stage_name = value;
            } else if (key == "--difficulty")
                config.difficulty = number(value, 4);
            else if (key == "--seed")
                config.seed = number(value, 65535);
            else if (key == "--frames")
                limit = number(value, 1000000);
            else if (key == "--output")
                output = fs::absolute(value).string();
            else if (key == "--actions")
                tape_path = fs::absolute(value).string();
            else if (key == "--replay")
                replay_path = fs::absolute(value).string();
            else if (key == "--strategy")
                strategy = value;
            else if (key == "--shoot") {
                shoot = number(value, 1);
                shoot_set = true;
            } else
                throw std::runtime_error("unknown option: " + key);
        }
        if (dat.empty() || !limit)
            throw std::runtime_error("--dat and a positive frame budget are required");
        if (strategy != "stationary" && strategy != "reactive")
            throw std::runtime_error("unknown strategy");
        if (!shoot_set)
            shoot = config.spell < 0;
        const auto identity = dat_hash(dat);
        if (identity != "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb")
            throw std::runtime_error("DAT identity mismatch");
        auto replay = replay_path.empty() ? std::vector<std::uint16_t>{} : read_tape(replay_path);
        config.dat_path = dat.c_str();
        RunDirectory scratch;
        th08::headless::Session session(config);
        std::vector<std::uint16_t> actions;
        actions.reserve(limit);
        const auto initial_io_ns = session.file_io_time_ns();
        const auto start = std::chrono::steady_clock::now();
        double update_ms = 0, decision_ms = 0;
        auto state = session.state();
        bool started = false, complete = false;
        unsigned peak = 0, first_hit = 0, spell_start = 0;
        std::string outcome = "frame_limit";
        std::uint64_t digest = 1469598103934665603ULL;
        for (unsigned i = 0; i < limit; ++i) {
            const auto decision_start = std::chrono::steady_clock::now();
            const auto &observed = session.bullets();
            std::uint16_t action = 4;
            if (!replay_path.empty()) {
                if (i == replay.size()) {
                    outcome = "tape_end";
                    break;
                }
                action = replay[i];
            } else {
                if (strategy == "reactive")
                    action = th08::policy::reactive(state.x, state.y, session.focused_axis_speed(),
                                                    session.focused_diagonal_speed(), observed);
                if (shoot)
                    action |= 1;
                // Alternating confirm advances real message scripts; timeline
                // and dialogue updates remain in the original game calc chain.
                if (config.spell < 0 && i % 2)
                    action |= 4096;
            }
            const auto update_start = std::chrono::steady_clock::now();
            decision_ms +=
                std::chrono::duration<double, std::milli>(update_start - decision_start).count();
            state = session.step(action);
            update_ms += std::chrono::duration<double, std::milli>(
                             std::chrono::steady_clock::now() - update_start)
                             .count();
            actions.push_back(action);
            peak = std::max(peak, unsigned(state.bullets));
            hash(digest, state.frame);
            hash(digest, action);
            hash(digest, state.rng_seed);
            hash(digest, bits(state.x));
            hash(digest, bits(state.y));
            hash(digest, state.player_state);
            hash(digest, state.spell_active);
            hash(digest, state.rng_draws);
            hash(digest, state.score);
            hash(digest, state.graze);
            hash(digest, state.gauge);
            hash(digest, bits(state.lives));
            hash(digest, session.actor_digest());
            for (const auto &b : session.bullets()) {
                hash(digest, b.state);
                hash(digest, bits(b.x));
                hash(digest, bits(b.y));
                hash(digest, bits(b.vx));
                hash(digest, bits(b.vy));
            }
            if (state.spell_active && !started) {
                started = true;
                spell_start = state.frame;
                if (config.spell >= 0 && state.spell != config.spell)
                    throw std::runtime_error(
                        "practice checkpoint selected a different spell; check stage/difficulty");
            }
            if (state.player_state == 2) {
                first_hit = state.frame;
                outcome = "collision";
                break;
            }
            complete = config.spell >= 0 ? started && !state.spell_active : state.stage_complete;
            if (complete) {
                outcome = "complete";
                break;
            }
            if (state.retry_menu) {
                outcome = "retry_menu";
                break;
            }
        }
        const auto ms =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
                .count();
        const double io_ms = double(session.file_io_time_ns() - initial_io_ns) / 1000000;
        if (!replay_path.empty() && actions.size() != replay.size())
            throw std::runtime_error("action tape extends beyond the execution boundary");
        if (!tape_path.empty()) {
            std::ofstream tape(tape_path);
            if (!tape)
                throw std::runtime_error("cannot open action tape output");
            for (auto action : actions)
                tape << action << '\n';
            if (!tape)
                throw std::runtime_error("cannot write action tape");
        }
        std::ofstream file;
        if (!output.empty())
            file.open(output);
        auto &out = output.empty() ? std::cout : file;
        if (!out)
            throw std::runtime_error("cannot open report");
        out << "{\"source_revision\":\"861bec908b84fa4658382d7526e5a0075f520846\","
            << "\"profile\":\"native-headless-float32\",\"dat_sha256\":\"" << identity
            << "\",\"stage\":\"" << stage_name << "\",\"requested_spell_id\":" << config.spell
            << ",\"difficulty\":" << config.difficulty << ",\"seed\":" << config.seed
            << ",\"optimization\":" << TH08_HEADLESS_OPTIMIZATION << ",\"compiler\":\""
            << __VERSION__ << "\""
            << ",\"strategy\":\"" << (replay_path.empty() ? strategy : "replay")
            << "\",\"frame_budget\":" << limit << ",\"outcome\":\"" << outcome
            << "\",\"frames\":" << state.frame << ",\"spell_id\":" << state.spell
            << ",\"spell_start\":" << spell_start << ",\"first_hit\":" << first_hit
            << ",\"deaths\":" << state.deaths << ",\"player_state\":" << state.player_state
            << ",\"rng_draws\":" << state.rng_draws << ",\"graze\":" << state.graze
            << ",\"rng_seed\":" << state.rng_seed << ",\"score\":" << state.score
            << ",\"gauge\":" << state.gauge << ",\"peak_bullets\":" << peak
            << ",\"trace_digest\":\"" << digest
            << "\",\"simulation_ms\":" << std::max(0., update_ms - io_ms)
            << ",\"file_io_ms\":" << io_ms << ",\"decision_ms\":" << decision_ms
            << ",\"execution_ms\":" << ms << "}\n";
        if (!out)
            throw std::runtime_error("cannot write report");
        return complete ? 0 : 2;
    } catch (const std::exception &e) {
        std::cerr << "th08_headless: " << e.what() << '\n';
        return 1;
    }
}
