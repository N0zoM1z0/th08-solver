#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <th08/emitter.hpp>

namespace res = th08::resources;
namespace vm = th08::emitter;
namespace fs = std::filesystem;
constexpr const char *dat_hash = "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb";

void verify_examples(const res::Bytes &data, const res::Ecl &ecl, const fs::path &output) {
    struct Case {
        unsigned sub, mask, ticks, requests, commands;
        bool returned;
    };
    constexpr Case cases[] = {{40, 8, 400, 840, 160, true},
                              {41, 8, 400, 840, 160, true},
                              {42, 0x28, 120, 30, 6, false},
                              {42, 0x48, 120, 6, 3, false}};
    vm::Workspace workspace;
    workspace.emissions.reserve(1024);
    workspace.transforms.reserve(1024);
    std::ofstream examples(output / "emitter_examples.tsv");
    examples.exceptions(std::ios::badbit | std::ios::failbit);
    examples << "sub\tmask\tstatus\ttick\tcommands\trequested_bullets\texecuted\tdigest\n";
    for (const auto &test : cases) {
        vm::Program program(res::view(data), ecl, test.sub);
        const auto result = vm::run(program, workspace, std::uint8_t(test.mask), test.ticks);
        if ((test.sub == 40 && result.digest != 10406148969939241199ULL) ||
            (test.sub == 41 && result.digest != 7272853799573871479ULL))
            throw std::runtime_error("ordered emission digest differs from the baseline");
        if (result.requested_bullets != test.requests ||
            workspace.emissions.size() != test.commands ||
            (test.returned && (result.status != vm::Status::returned || result.tick != 360)) ||
            (!test.returned && result.status != vm::Status::horizon))
            throw std::runtime_error("real emitter case differs: sub" + std::to_string(test.sub) +
                                     " mask=" + std::to_string(test.mask) +
                                     " status=" + vm::name(result.status) +
                                     " requests=" + std::to_string(result.requested_bullets) +
                                     " pc=" + std::to_string(result.offset) +
                                     " opcode=" + std::to_string(result.opcode));
        examples << test.sub << '\t' << test.mask << '\t' << vm::name(result.status) << '\t'
                 << result.tick << '\t' << workspace.emissions.size() << '\t'
                 << result.requested_bullets << '\t' << result.executed << '\t' << result.digest
                 << '\n';
    }
    vm::Program program(res::view(data), ecl, 40);
    std::vector<double> nanoseconds;
    std::uint64_t checksum = 0;
    for (int warmup = 0; warmup < 100; ++warmup)
        checksum ^= vm::run(program, workspace, 8).digest;
    for (int batch = 0; batch < 41; ++batch) {
        const auto started = std::chrono::steady_clock::now();
        for (int iteration = 0; iteration < 1000; ++iteration)
            checksum ^= vm::run(program, workspace, 8).digest + std::uint64_t(iteration);
        nanoseconds.push_back(
            std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - started)
                .count() /
            1000);
    }
    std::sort(nanoseconds.begin(), nanoseconds.end());
    std::ofstream bench(output / "emitter_benchmark.json");
    bench.exceptions(std::ios::badbit | std::ios::failbit);
    bench << "{\"scope\":\"real sub40 restricted scheduling; excludes decode/compile, world, "
             "collision and planning\","
          << "\"batch_size\":1000,\"batches\":41,\"median_batch_average_ns\":" << nanoseconds[20]
          << ",\"p95_batch_average_ns\":" << nanoseconds[38] << ",\"checksum\":" << checksum
          << "}\n";
}

int main(int argc, char **argv) try {
    if (argc < 2 || argc > 3)
        throw std::runtime_error("Usage: th08_slices th08.dat [report-directory]");
    const fs::path output = argc == 3 ? argv[2] : "reports/native";
    auto data = res::read_file(argv[1]);
    if (res::sha256(res::view(data)) != dat_hash)
        throw std::runtime_error("DAT identity mismatch");
    res::Archive archive(std::move(data));
    fs::create_directories(output);
    {
        std::ofstream incomplete(output / "slice_summary.json");
        incomplete.exceptions(std::ios::badbit | std::ios::failbit);
        incomplete << "{\"status\":\"INCOMPLETE\"}\n";
    }
    std::ofstream matrix(output / "slice_matrix.tsv");
    matrix.exceptions(std::ios::badbit | std::ios::failbit);
    matrix << "file\tsub\tdifficulty_mask\talignment\tstatus\tlast_pc\topcode\texecuted\trequests_"
              "before_stop\n";
    std::map<vm::Status, std::size_t> counts;
    std::size_t attempts = 0;
    vm::Workspace workspace;
    workspace.emissions.reserve(1024);
    workspace.transforms.reserve(1024);
    for (std::size_t index = 0; index < archive.entries().size(); ++index) {
        const auto &entry = archive.entries()[index];
        if (fs::path(entry.name).extension() != ".ecl")
            continue;
        const auto decoded = archive.decode(index);
        const auto ecl = res::parse_ecl(res::view(decoded));
        if (entry.name == "ecldata1.ecl")
            verify_examples(decoded, ecl, output);
        for (std::size_t sub = 0; sub < ecl.subs.size(); ++sub) {
            const vm::Program program(res::view(decoded), ecl, sub);
            for (unsigned difficulty : {1, 2, 4, 8, 15})
                for (unsigned alignment : {0, 32, 64}) {
                    const auto result =
                        vm::run(program, workspace, std::uint8_t(difficulty | alignment), 360);
                    ++counts[result.status];
                    ++attempts;
                    matrix << entry.name << '\t' << sub << '\t' << difficulty << '\t' << alignment
                           << '\t' << vm::name(result.status) << '\t' << result.offset << '\t'
                           << result.opcode << '\t' << result.executed << '\t'
                           << result.requested_bullets << '\n';
                }
        }
    }
    if (attempts != 21735)
        throw std::runtime_error("slice matrix coverage gap");
    std::ofstream summary(output / "slice_summary.json");
    summary.exceptions(std::ios::badbit | std::ios::failbit);
    summary << "{\n  \"status\": \"PASSED\",\n  \"attempts\": " << attempts
            << ",\n  \"scope\": \"restricted native scalar scheduler, no complete worlds or spell "
               "solutions\",\n"
            << "  \"status_counts\": {";
    bool first = true;
    for (const auto &[status, count] : counts) {
        summary << (first ? "\n" : ",\n") << "    \"" << vm::name(status) << "\": " << count;
        first = false;
    }
    summary << "\n  }\n}\n";
    std::cout << "Native matrix: " << attempts
              << " attempts; real sub40/41/42 request contracts passed\n";
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
