#include <charconv>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string_view>
#include <th08/animation.hpp>
#include <th08/animation_control.hpp>
namespace res = th08::resources;
namespace ac = th08::animation::control;
namespace fs = std::filesystem;
int main(int argc, char **argv) try {
    if (argc != 3 && argc != 4)
        throw std::runtime_error(
            "Usage: th08_animation_cases th08.dat report-directory [explicit-seed]");
    std::optional<std::uint16_t> seed;
    if (argc == 4) {
        const std::string_view argument = argv[3];
        unsigned value = 0;
        const auto parsed =
            std::from_chars(argument.data(), argument.data() + argument.size(), value);
        if (parsed.ec != std::errc{} || parsed.ptr != argument.data() + argument.size() ||
            value > 65535)
            throw std::runtime_error("ANM seed must be an explicit integer from 0 to 65535");
        seed = std::uint16_t(value);
    }
    auto bytes = res::read_file(argv[1]);
    const auto hash = res::sha256(res::view(bytes));
    if (hash != "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb")
        throw std::runtime_error("DAT identity mismatch");
    res::Archive archive(std::move(bytes));
    const fs::path output = argv[2];
    fs::create_directories(output);
    {
        std::ofstream pending(output / "animation_control_summary.json");
        pending.exceptions(std::ios::badbit | std::ios::failbit);
        pending << "{\"status\":\"INCOMPLETE\"}\n";
    }
    std::ofstream matrix(output / "animation_control_cases.tsv");
    matrix.exceptions(std::ios::badbit | std::ios::failbit);
    matrix << "file\tscript\tentry\traw_"
              "id\tstatus\tcalls\tpc\toffset\topcode\ttime\tsprite\tvisible\tstopped"
              "\thit_animation_type\trng_seed\trng_draws\n";
    std::size_t cases = 0, certificates_checked = 0;
    std::map<ac::Status, std::size_t> counts;
    for (std::size_t member = 0; member < archive.entries().size(); ++member) {
        const auto &entry = archive.entries()[member];
        if (fs::path(entry.name).extension() != ".anm")
            continue;
        const auto decoded = archive.decode(member);
        const auto anm = res::parse_anm(res::view(decoded));
        for (std::size_t script = 0; script < anm.scripts.size(); ++script) {
            const ac::Program program(res::view(decoded), anm, script);
            ac::State state;
            std::optional<th08::random::Rng> rng;
            if (seed)
                rng.emplace(*seed);
            ac::Result result;
            unsigned calls = 0;
            do {
                result = ac::advance(program, state, 1, false, 100000, rng ? &*rng : nullptr);
                ++calls;
            } while (result.status == ac::Status::advanced && calls < 600);
            const auto certificate =
                th08::animation::certify_timing(res::view(decoded), anm, script);
            if (certificate.status == th08::animation::Status::certified) {
                // Some static bullet scripts end at frame 30000. Verify those
                // certificates separately without relabeling the 600-call matrix.
                auto checked_state = state;
                auto checked_result = result;
                auto checked_calls = calls;
                const auto expected_calls = unsigned(certificate.completion_time) + 1;
                while (checked_result.status == ac::Status::advanced &&
                       checked_calls < expected_calls) {
                    checked_result = ac::advance(program, checked_state);
                    ++checked_calls;
                }
                if (checked_result.status != ac::Status::completed ||
                    checked_calls != expected_calls || checked_state.sprite != certificate.sprite ||
                    (certificate.hides_on_completion && checked_state.visible))
                    throw std::runtime_error(
                        "ANM runtime disagrees with timing certificate: " + entry.name +
                        " script=" + std::to_string(script) + " status=" + ac::name(result.status) +
                        " calls=" + std::to_string(calls) +
                        " expected_time=" + std::to_string(certificate.completion_time));
                ++certificates_checked;
            }
            ++counts[result.status];
            ++cases;
            const auto &source = anm.scripts[script];
            const auto &stop = program.code.at(result.pc);
            matrix << entry.name << '\t' << script << '\t' << source.entry << '\t' << source.raw_id
                   << '\t'
                   << (result.status == ac::Status::advanced ? "BOUNDED_PREFIX"
                                                             : ac::name(result.status))
                   << '\t' << calls << '\t' << result.pc << '\t' << stop.offset << '\t'
                   << stop.opcode << '\t' << state.time.current << '\t' << state.sprite << '\t'
                   << state.visible << '\t' << state.stopped << '\t'
                   << state.player_bullet_hit_animation_type << '\t';
            if (rng)
                matrix << rng->seed() << '\t' << rng->generation_count();
            else
                matrix << "UNAVAILABLE\tUNAVAILABLE";
            matrix << '\n';
        }
    }
    if (cases != 1151 || certificates_checked != 42 || counts[ac::Status::invalid])
        throw std::runtime_error("ANM runtime coverage or validity regression");
    std::ofstream summary(output / "animation_control_summary.json");
    summary.exceptions(std::ios::badbit | std::ios::failbit);
    summary << "{\n  \"status\": \"PASSED\",\n  \"dat_sha256\": \"" << hash
            << "\",\n  \"scope\": \"600 unit-rate calls from fresh lifecycle projection; no "
               "external interrupts, renderer or complete spell worlds\",\n"
            << "  \"rng_profile\": \""
            << (seed ? "explicit seed independently reset for each script; not world draw order"
                     : "no RNG context; random instructions stop")
            << "\",\n  \"initial_seed\": " << (seed ? std::to_string(*seed) : "null")
            << ",\n  \"scripts\": " << cases
            << ",\n  \"certificates_checked\": " << certificates_checked
            << ",\n  \"status_counts\": {";
    bool first = true;
    for (const auto &[status, count] : counts) {
        summary << (first ? "\n" : ",\n") << "    \""
                << (status == ac::Status::advanced ? "BOUNDED_PREFIX" : ac::name(status))
                << "\": " << count;
        first = false;
    }
    summary << "\n  }\n}\n";
    std::cout << "ANM control projection: " << cases << " scripts; " << certificates_checked
              << " existing timing certificates matched\n";
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
