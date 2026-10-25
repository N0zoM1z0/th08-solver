#include "modern/headless/session.hpp"
#include "th08/plan_contracts.hpp"
#include "th08/plan_prefix.hpp"
#include "th08/shadow_native.hpp"
#include "th08/shadow_process.hpp"
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <openssl/evp.h>
#include <sstream>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;
namespace sh = th08::shadow;
namespace hl = th08::headless;
namespace plan = th08::plan;
namespace {
std::string file_hash(const std::string &path) {
    std::ifstream input(path, std::ios::binary);
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!input || !ctx || EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("shadow_identity_failed");
    std::array<char, 65536> bytes;
    while (input) {
        input.read(bytes.data(), bytes.size());
        if (EVP_DigestUpdate(ctx.get(), bytes.data(), std::size_t(input.gcount())) != 1)
            throw std::runtime_error("shadow_identity_failed");
    }
    std::array<unsigned char, 32> digest;
    unsigned length = 0;
    if (!input.eof() || EVP_DigestFinal_ex(ctx.get(), digest.data(), &length) != 1 || length != 32)
        throw std::runtime_error("shadow_identity_failed");
    std::ostringstream output;
    for (auto byte : digest)
        output << std::hex << std::setw(2) << std::setfill('0') << unsigned(byte);
    return output.str();
}
struct Options {
    std::string dat, actions, output;
    hl::Config config;
    std::uint64_t frames = 1000000;
    int follower_seed = -1;
};
Options options(int argc, char **argv) {
    Options o;
    const std::array<std::string, 9> stages{"1", "2", "3", "4a", "4b", "5", "6a", "6b", "extra"};
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 == argc)
            throw std::runtime_error("missing_shadow_option");
        const std::string key = argv[i], value = argv[i + 1];
        const auto n = [&] { return plan::integer<unsigned>(value, key); };
        const auto bounded = [&](unsigned maximum) {
            const auto number = n();
            if (number > maximum)
                throw std::runtime_error("invalid_shadow_option_range");
            return number;
        };
        if (key == "--dat")
            o.dat = fs::absolute(value).string();
        else if (key == "--actions")
            o.actions = fs::absolute(value).string();
        else if (key == "--output")
            o.output = fs::absolute(value).string();
        else if (key == "--frames")
            o.frames = n();
        else if (key == "--seed" || key == "--follower-seed") {
            auto seed = n();
            if (seed > 65535)
                throw std::runtime_error("invalid_shadow_seed");
            if (key == "--seed")
                o.config.seed = std::uint16_t(seed);
            else
                o.follower_seed = int(seed);
        } else if (key == "--difficulty")
            o.config.difficulty = int(bounded(4));
        else if (key == "--input-profile") {
            if (value == "sampled-gameplay")
                o.config.input_profile = hl::InputProfile::SampledGameplay;
            else if (value != "controller")
                throw std::runtime_error("invalid_shadow_input_profile");
        } else if (key == "--prior-final-a-clear")
            o.config.prior_final_a_clear = bounded(1);
        else if (key == "--shot")
            o.config.shot = int(bounded(11));
        else if (key == "--spell")
            o.config.spell = int(bounded(221));
        else if (key == "--story") {
            auto v = n();
            if (v > 1)
                throw std::runtime_error("invalid_shadow_story");
            o.config.story = v;
        } else if (key == "--stage") {
            bool found = false;
            for (std::size_t s = 0; s < stages.size(); ++s)
                if (value == stages[s]) {
                    o.config.stage = int(s);
                    found = true;
                }
            if (!found)
                throw std::runtime_error("invalid_shadow_stage");
        } else
            throw std::runtime_error("unknown_shadow_option");
    }
    if (o.dat.empty() || o.actions.empty() || o.output.empty() || !o.frames || o.frames > 1000000 ||
        o.config.difficulty > 4 || o.config.shot > 11 || o.config.spell > 221 ||
        (o.config.story && o.config.spell >= 0))
        throw std::runtime_error("invalid_shadow_configuration");
    return o;
}
class Scratch {
    fs::path previous = fs::current_path(), directory;

  public:
    Scratch() {
        char path[] = "/tmp/th08-shadow-owner-XXXXXX";
        if (!mkdtemp(path))
            throw std::runtime_error("shadow_owner_scratch_failed");
        directory = path;
        fs::current_path(directory);
    }
    ~Scratch() {
        std::error_code e;
        fs::current_path(previous, e);
        fs::remove_all(directory, e);
    }
};
int run(Options o) {
    std::ifstream tape(o.actions);
    if (!tape)
        throw std::runtime_error("shadow_actions_unreadable");
    plan::PrefixParser parser(1000000);
    std::array<char, 65536> buffer;
    while (tape) {
        tape.read(buffer.data(), buffer.size());
        parser.append(buffer.data(), std::size_t(tape.gcount()));
    }
    if (!tape.eof())
        throw std::runtime_error("shadow_actions_read_failed");
    const auto actions = parser.finish();
    const auto dat_hash = file_hash(o.dat), actions_hash = file_hash(o.actions);
    const auto binary_hash = file_hash("/proc/self/exe");
    o.config.dat_path = o.dat.c_str();
    auto other = o.config;
    if (o.follower_seed >= 0)
        other.seed = std::uint16_t(o.follower_seed);
    const auto begin = std::chrono::steady_clock::now();
    sh::Follower follower(other);
    Scratch scratch;
    hl::Session owner(o.config);
    sh::Packet receipt;
    receipt.state = sh::capture(owner);
    auto observed = follower.initial();
    const char *mismatch = sh::difference(receipt, observed);
    bool spell_seen = owner.state().spell_active;
    std::uint64_t handoffs = 0;
    std::string outcome = mismatch ? "desynchronized" : "budget";
    for (std::size_t i = 0; !mismatch && i < actions.size() && i < o.frames; ++i) {
        auto before = owner.state();
        if (sh::stage_terminal(o.config.story, before) ||
            (o.config.spell >= 0 && spell_seen && !before.spell_active)) {
            outcome = "complete";
            break;
        }
        if (before.player_state == 2 || before.deaths != 0) {
            outcome = "death";
            break;
        }
        observed = follower.advance(receipt.sequence + 1, receipt.epoch, actions[i]);
        owner.step(actions[i]);
        const auto epoch = receipt.epoch;
        sh::advance_receipt(receipt, actions[i], sh::capture(owner));
        handoffs += receipt.epoch != epoch;
        mismatch = sh::difference(receipt, observed);
        const auto after = owner.state();
        spell_seen = spell_seen || after.spell_active;
        if (mismatch)
            outcome = "desynchronized";
        else if (after.player_state == 2 || after.deaths != 0) {
            outcome = "death";
            break;
        } else if (sh::stage_terminal(o.config.story, after) ||
                   (o.config.spell >= 0 && spell_seen && !after.spell_active)) {
            outcome = "complete";
            break;
        }
    }
    const auto state = owner.state();
    std::ofstream out(o.output);
    if (!out)
        throw std::runtime_error("shadow_report_failed");
    out << "{\"schema_version\":1,\"profile\":\"native-common-start-follow-v1\",\"outcome\":\""
        << outcome << "\",\"first_mismatch\":\"" << (mismatch ? mismatch : "")
        << "\",\"updates\":" << receipt.sequence
        << ",\"calculation_updates\":" << state.calculation_updates << ",\"handoffs\":" << handoffs
        << ",\"seed\":" << o.config.seed << ",\"follower_seed\":" << other.seed
        << ",\"stage\":" << o.config.stage << ",\"shot\":" << o.config.shot
        << ",\"input_profile\":\""
        << (o.config.input_profile == hl::InputProfile::SampledGameplay ? "sampled-gameplay"
                                                                        : "controller")
        << "\",\"prior_final_a_clear\":" << (o.config.prior_final_a_clear ? "true" : "false")
        << ",\"difficulty\":" << o.config.difficulty << ",\"story\":" << o.config.story
        << ",\"dat_sha256\":\"" << dat_hash << "\",\"actions_sha256\":\"" << actions_hash
        << "\",\"binary_sha256\":\"" << binary_hash << '"' << ",\"spell\":" << o.config.spell
        << ",\"score\":" << state.score << ",\"rng_seed\":" << state.rng_seed
        << ",\"rng_draws\":" << state.rng_draws << ",\"deaths\":" << state.deaths
        << ",\"player_state\":" << state.player_state
        << ",\"future_native_updates\":0,\"rewinds\":0,\"deadline_enforced\":false,\"retail_"
           "validated\":false"
        << ",\"projection_words\":" << sh::field_names.size() << ",\"wall_seconds\":"
        << std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count();
    if (mismatch) {
        out << ",\"owner_projection\":[";
        for (std::size_t i = 0; i < receipt.state.values.size(); ++i)
            out << (i ? "," : "") << receipt.state.values[i];
        out << "],\"follower_projection\":[";
        for (std::size_t i = 0; i < observed.state.values.size(); ++i)
            out << (i ? "," : "") << observed.state.values[i];
        out << ']';
    }
    out << ",\"terminal_projection\":[";
    for (std::size_t i = 0; i < receipt.state.values.size(); ++i)
        out << (i ? "," : "") << receipt.state.values[i];
    out << "]}\n";
    if (!out)
        throw std::runtime_error("shadow_report_failed");
    return mismatch ? 3 : (outcome == "complete" || outcome == "budget" ? 0 : 4);
}
} // namespace
int main(int argc, char **argv) try {
    return run(options(argc, argv));
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 2;
}
