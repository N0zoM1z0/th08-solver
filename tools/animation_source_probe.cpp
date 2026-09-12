#include "animation_source_probe.hpp"
#include <stdexcept>
// Shared pinned-source readers supplied by source_probe.cpp.
std::string source(const std::filesystem::path &, const char *);
std::string function(const std::string &, const std::string &);

std::string animation_reference(const std::filesystem::path &repo) {
    source(repo / "src/AsciiManager.cpp",
           "86c0d3cca5040036f16de762e80b3126b7037c89b526044cbb74bcc4bc6abdb1");
    const auto text = source(repo / "src/AnmManager.cpp",
                             "c82bb37c19af4ccaabfa4bf4606d92c72e180f5f2fdd642cf3ec2131c85cecce");
    const auto header = source(repo / "src/AnmManager.hpp",
                               "df96ae2abd43ffc64a5967451fcd3ca5b83b75f6ad6ed37ba852370855c7f582");
    const auto body = function(text, "ZunBool AnmManager::ExecuteScript(");
    auto section = [&](const std::string &first, const std::string &last) {
        const auto begin = body.find(first), end = body.find(last, begin);
        if (begin == std::string::npos || end == std::string::npos)
            throw std::runtime_error("missing pinned ANM control block");
        return body.substr(begin, end - begin);
    };
    return "\nnamespace anm_reference {\n" + function(header, "enum AnmOpcode") + R"CPP(;
enum { FALSE=0, TRUE=1 };
using u8=std::uint8_t;
struct AnmRawInstr {
    i16 opcode; u16 instructionSize; i16 time; u16 varMask;
    i32 intArgs[5];
};
struct AnmVm;
struct AnmLoaded { void SetSprite(AnmVm*,int); };
struct AnmVm {
    AnmRawInstr* currentInstruction=nullptr;
    AnmRawInstr* beginningOfScript=nullptr;
    AnmRawInstr* interruptReturnInstruction=nullptr;
    ZunTimer currentTimeInScript,waitTimer,interruptReturnTime;
    i32 sprite=-1,timeOfLastSpriteSet=0;
    i16 pendingInterrupt=0;
    u32 visible:1; u32 stopped:1;
    bool flag19=false;
    AnmLoaded* anmFile=nullptr;
    // Initialize zeroes the base, then initializes only currentTimeInScript.
    AnmVm():visible(0),stopped(0){waitTimer.previous=0;}
    int GetIntVar(int) { throw std::runtime_error("ANM oracle variable outside control scope"); }
};
void AnmLoaded::SetSprite(AnmVm* vm,int sprite) {vm->sprite=sprite;}
struct AnmManager { ZunBool ExecuteScript(AnmVm*); };
)CPP" + section("ZunBool AnmManager::ExecuteScript(", "        case AnmOpcode_Scale:") +
           section("        case AnmOpcode_Jmp:", "        case AnmOpcode_JmpDec:") +
           section("        case AnmOpcode_Wait:", "        case AnmOpcode_AnchorTopLeft:") +
           // Visual writes and scalar instructions are deliberately outside this
           // oracle's control-only input domain, not silently accepted stubs.
           "        case AnmOpcode_Nop:\n        case AnmOpcode_InterruptLabel: break;\n"
           "        default: throw std::runtime_error(\"ANM oracle opcode outside control "
           "scope\");\n"
           "        }\n" +
           section("#undef GET_FLOAT_VAR_PTR", "stop:") + "stop:\n" +
           section("    vm->currentTimeInScript++;", "    this->scriptsExecutedThisFrame++;") +
           "    return FALSE;\n}\n}\n";
}
