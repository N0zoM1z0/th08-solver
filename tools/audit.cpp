#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <th08/resources.hpp>

namespace res = th08::resources;
namespace fs = std::filesystem;
constexpr const char *dat_hash = "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb";

std::ofstream report(const fs::path &directory, const char *name) {
    std::ofstream stream(directory / name);
    stream.exceptions(std::ios::badbit | std::ios::failbit);
    return stream;
}

int main(int argc, char **argv) try {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: th08_audit th08.dat [report-directory]\n";
        return 2;
    }
    const fs::path destination = argc == 3 ? argv[2] : "reports/native";
    const auto started = std::chrono::steady_clock::now();
    auto bytes = res::read_file(argv[1]);
    if (res::sha256(res::view(bytes)) != dat_hash)
        throw std::runtime_error("DAT identity differs from the supported TH08 1.00d resource");
    res::Archive archive(std::move(bytes));
    fs::create_directories(destination);
    report(destination, "summary.json") << "{\"status\":\"INCOMPLETE\"}\n";
    auto members = report(destination, "members.tsv");
    auto spells = report(destination, "spell_sites.tsv");
    auto subs = report(destination, "subprograms.tsv");
    auto shots = report(destination, "shot_sites.tsv");
    members << "name\tdecoded_bytes\tsha256\n";
    spells << "id\tfile\tsub\toffset\tmask\toffline_solution\n";
    subs << "file\tsub\tstart\tend\tinstructions\treference_world\n";
    shots << "file\tsub\toffset\topcode\tmask\toperand_flags\n";
    std::size_t ecl_count = 0, sub_count = 0, instruction_count = 0, spell_count = 0, jumps = 0;
    std::uint64_t decoded_bytes = 0;
    std::set<unsigned> ids, ex_ids;
    std::array<std::uint64_t, 185> opcode_counts{};
    for (std::size_t i = 0; i < archive.entries().size(); ++i) {
        const auto &entry = archive.entries()[i];
        const auto decoded = archive.decode(i);
        const auto data = res::view(decoded);
        members << entry.name << '\t' << decoded.size() << '\t' << res::sha256(data) << '\n';
        decoded_bytes += decoded.size();
        if (fs::path(entry.name).extension() != ".ecl")
            continue;
        const auto ecl = res::parse_ecl(data);
        ++ecl_count;
        sub_count += ecl.subs.size();
        jumps += ecl.jump_targets_checked;
        for (std::size_t sid = 0; sid < ecl.subs.size(); ++sid) {
            const auto &sub = ecl.subs[sid];
            std::uint32_t nonterminal = 0;
            for (std::uint32_t j = sub.first; j < sub.first + sub.count; ++j) {
                const auto &ins = ecl.instructions[j];
                if (ins.opcode < 0)
                    continue;
                ++nonterminal;
                ++opcode_counts[std::size_t(ins.opcode)];
                if (ins.opcode == 136 || ins.opcode == 137) {
                    if (ins.size < 20)
                        throw std::runtime_error("truncated EX operands");
                    const auto id = res::i32(data, ins.offset + 12);
                    if (id >= 0 && id < 32)
                        ex_ids.insert(unsigned(id));
                }
                if (ins.opcode >= 96 && ins.opcode <= 104)
                    shots << entry.name << '\t' << sid << '\t' << ins.offset << '\t' << ins.opcode
                          << '\t' << unsigned(ins.mask) << '\t' << ins.flags << '\n';
            }
            instruction_count += nonterminal;
            subs << entry.name << '\t' << sid << '\t' << sub.offset << '\t' << sub.end << '\t'
                 << nonterminal << "\tNOT_IMPLEMENTED\n";
        }
        for (const auto &spell : ecl.spells) {
            ++spell_count;
            ids.insert(spell.id);
            spells << spell.id << '\t' << entry.name << '\t' << spell.sub << '\t' << spell.offset
                   << '\t' << unsigned(spell.mask) << "\tNOT_VERIFIED\n";
        }
    }
    bool expected_ids = ids.size() == 222;
    for (unsigned id = 0; id < 222; ++id)
        expected_ids = expected_ids && ids.count(id) == 1;
    if (archive.entries().size() != 317 || ecl_count != 24 || sub_count != 1449 ||
        instruction_count != 36661 || spell_count != 431 || jumps != 2182 || !expected_ids ||
        ex_ids.size() != 32)
        throw std::runtime_error("native corpus counts disagree with the audited input contract");
    auto opcodes = report(destination, "opcodes.tsv");
    opcodes << "opcode\tobserved_sites\truntime_semantics\n";
    for (std::size_t i = 0; i < opcode_counts.size(); ++i)
        opcodes << i << '\t' << opcode_counts[i] << "\tNOT_IMPLEMENTED\n";
    const double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    auto summary = report(destination, "summary.json");
    summary
        << "{\n  \"status\": \"PASSED\",\n  \"implementation\": \"C++17\",\n  \"dat_sha256\": \""
        << dat_hash
        << "\",\n  \"scope\": \"all-member decoding and structural ECL audit; not world "
           "execution\",\n"
        << "  \"resources\": 317,\n  \"ecl_files\": 24,\n  \"subprograms\": 1449,\n"
        << "  \"nonterminal_instructions\": 36661,\n  \"jump_targets_checked\": 2182,\n"
        << "  \"spell_sites\": 431,\n  \"unique_spell_ids\": 222,\n  \"ex_ids\": 32,\n"
        << "  \"offline_full_spell_solutions_verified\": 0,\n  \"decoded_bytes\": " << decoded_bytes
        << ",\n  \"wall_seconds_including_hashing_and_reports\": " << std::setprecision(9)
        << elapsed << "\n}\n";
    std::cout << "317 members, 24 ECL, 1449 subs, 36661 instructions, 431 spell sites / 222 IDs\n"
              << "Decoded " << decoded_bytes << " bytes in " << elapsed
              << " seconds; reports: " << destination << '\n';
} catch (const std::exception &error) {
    std::cerr << "Audit failed: " << error.what() << '\n';
    return 1;
}
