#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <th08/timeline.hpp>

namespace res = th08::resources;
namespace tl = th08::timeline;
namespace fs = std::filesystem;
int main(int argc, char **argv) try {
    if (argc != 3)
        throw std::runtime_error("Usage: th08_timeline_cases th08.dat report-directory");
    auto bytes = res::read_file(argv[1]);
    const auto hash = res::sha256(res::view(bytes));
    if (hash != "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb")
        throw std::runtime_error("DAT identity mismatch");
    res::Archive archive(std::move(bytes));
    const fs::path output = argv[2];
    fs::create_directories(output);
    {
        std::ofstream pending(output / "timeline_control_summary.json");
        pending.exceptions(std::ios::badbit | std::ios::failbit);
        pending << "{\"status\":\"INCOMPLETE\"}\n";
    }
    std::ofstream matrix(output / "timeline_control_cases.tsv");
    matrix.exceptions(std::ios::badbit | std::ios::failbit);
    matrix << "file\ttimeline\tstart\tmask\tstatus\tframes\tpc\toffset\topcode\ttime"
              "\tfraction\tpending_token\n";
    std::size_t timelines = 0, records = 0, attempts = 0;
    std::map<std::string, std::size_t> counts;
    for (std::size_t member = 0; member < archive.entries().size(); ++member) {
        const auto &entry = archive.entries()[member];
        if (fs::path(entry.name).extension() != ".ecl")
            continue;
        const auto resource = archive.decode(member);
        const auto ecl = res::parse_ecl(res::view(resource));
        for (std::size_t index = 0; index < ecl.timelines.size(); ++index) {
            const tl::Program program(res::view(resource), ecl, index);
            ++timelines;
            for (std::uint32_t pc = 0; pc + 1 < program.code.size(); ++pc) {
                const auto payload = program.payload(pc);
                if (payload.size &&
                    std::memcmp(payload.data, resource.data() + program.code[pc].offset + 8,
                                payload.size) != 0)
                    throw std::runtime_error("compiled timeline payload mismatch");
                ++records;
            }
            for (std::uint8_t mask : {1, 2, 4, 8, 16}) {
                tl::State state;
                tl::Context context; // Unknown world observations, not fabricated false gates.
                tl::Result result;
                unsigned frames = 0;
                for (; frames < 600;) {
                    result = tl::advance(program, state, context, mask);
                    if (result.status != tl::Status::frame_complete)
                        break;
                    ++frames;
                    if (result.at_end)
                        break;
                }
                const std::string status =
                    result.status == tl::Status::frame_complete
                        ? (result.at_end ? "SENTINEL_REACHED" : "BOUNDED_PREFIX")
                        : tl::name(result.status);
                if (result.status == tl::Status::invalid ||
                    result.status == tl::Status::unsupported ||
                    result.status == tl::Status::instruction_limit)
                    throw std::runtime_error("unexpected timeline stop in " + entry.name);
                ++counts[status];
                ++attempts;
                matrix << entry.name << '\t' << index << '\t' << ecl.timelines[index].offset << '\t'
                       << unsigned(mask) << '\t' << status << '\t' << frames << '\t' << result.pc
                       << '\t' << result.offset << '\t' << result.opcode << '\t'
                       << state.time.current << '\t' << state.time.fraction << '\t'
                       << state.effect_token << '\n';
            }
        }
    }
    if (timelines != 32 || records != 2003 || attempts != 160)
        throw std::runtime_error("pinned timeline coverage mismatch");
    matrix.close();
    std::ofstream summary(output / "timeline_control_summary.json");
    summary.exceptions(std::ios::badbit | std::ios::failbit);
    summary
        << "{\n  \"status\": \"PASSED\",\n  \"dat_sha256\": \"" << hash
        << "\",\n  \"scope\": \"fresh timeline clocks, five difficulty masks, at most 600 frames; "
           "unknown world gates and unexecuted effects stop; not complete worlds\",\n"
        << "  \"timelines\": " << timelines << ",\n  \"payloads_checked\": " << records
        << ",\n  \"attempts\": " << attempts << ",\n  \"status_counts\": {";
    bool first = true;
    for (const auto &count : counts) {
        summary << (first ? "\n" : ",\n") << "    \"" << count.first << "\": " << count.second;
        first = false;
    }
    summary << "\n  }\n}\n";
    std::cout << "Timeline control: " << attempts << " explicit-boundary attempts; " << records
              << " owned DAT payloads checked\n";
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
