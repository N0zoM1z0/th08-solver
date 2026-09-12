#include "ecl_source_probe.hpp"
#include <stdexcept>

// Shared extraction helpers in source_probe.cpp check identities before slicing.
std::string source(const std::filesystem::path &path, const char *expected);
std::string function(const std::string &text, const std::string &signature);

namespace {
std::string section(const std::string &text, const char *first, const char *after) {
    const auto begin = text.find(first);
    const auto end = text.find(after, begin);
    if (begin == std::string::npos || end == std::string::npos)
        throw std::runtime_error("missing pinned ECL section");
    return text.substr(begin, end - begin);
}
std::string random_cases(const std::string &text, bool floating) {
    auto result = section(
        text, "    case ECL_OPERAND_RANDOM_NONNEGATIVE_INT:", "    case ECL_OPERAND_DIFFICULTY:");
    // The contiguous float block additionally includes RANDOM_ANGLE; integer
    // resolution has no such case. Preserve both original blocks unchanged.
    if ((result.find("ECL_OPERAND_RANDOM_ANGLE") != std::string::npos) != floating)
        throw std::runtime_error("unexpected pinned random selector block");
    return result;
}
constexpr const char *adapter = R"CPP(
struct Instruction {unsigned short operandFlags; alignas(4) unsigned char operands[32];};
struct Enemy {
    i32 ints[8]{};
    f32 floats[8]{};
    f32 ResolveFloat(f32 operand);
};
Rng g_Rng;
namespace EclOperands {
i32 ResolveInt(Enemy*,i32 operand);
i32* ResolveIntLValue(Enemy* enemy,i32* operand,u16 flags,i32 index) {
    if(!(flags&(1U<<index)) || *operand<10000 || *operand>=10008)
        throw std::runtime_error("source adapter integer destination outside fixture");
    return &enemy->ints[*operand-10000];
}
f32* ResolveFloatLValue(Enemy* enemy,f32* operand,u16 flags,i32 index) {
    if(!(flags&(1U<<index)) || *operand<10016 || *operand>=10024)
        throw std::runtime_error("source adapter float destination outside fixture");
    return &enemy->floats[i32(*operand)-10016];
}
}
)CPP";
constexpr const char *tests = R"CPP(
#undef RawInt
#undef RawFloat
#undef ReadInt
#undef ReadFloat
#undef ReadFloatRawArg
#undef WriteInt
#undef WriteFloat
struct Comparison {std::uint64_t operations=0,mismatches=0;};
Comparison compare() {
    namespace vm=th08::emitter;
    Comparison result;
    auto bits=[](float value) {u32 word;std::memcpy(&word,&value,4);return word;};
    vm::Workspace workspace;
    vm::Program program;
    program.code.resize(2);
    program.code[1]={0,53,0,0,255,0,0,{}};
    for(unsigned seed=0;seed<65536;++seed) {
        g_Rng.SetSeed(u16(seed));
        g_Rng.ResetGenerationCount();
        th08::random::Rng actual{u16(seed)};
        for(unsigned sample=0;sample<11;++sample) {
            const bool floating=(sample>=4 && sample<9) || sample==10;
            const int opcode=sample>=9?(floating?9:8):(floating?7:6);
            const unsigned selector=sample==8?10082:10032+sample%4;
            vm::Operation operation{0,std::int16_t(opcode),u16(sample<9?3:1),8,255,0,0,{}};
            operation.words[0]=floating?bits(10016.0f):10000;
            operation.words[1]=sample<9?(floating?bits(float(selector)+0.75f):selector)
                                      :(floating?bits(-2.5f):u32(-13));
            program.code[0]=operation;
            Instruction instruction{};
            instruction.operandFlags=operation.flags;
            std::memcpy(instruction.operands,operation.words.data(),32);
            Enemy enemy;
            execute(&enemy,&instruction,opcode);
            const auto execution=vm::run(program,workspace,8,400,100000,&actual);
            bool equal=execution.status==vm::Status::returned && actual.seed()==g_Rng.GetSeed();
            if(floating) equal=equal && bits(float(workspace.registers[16]))==bits(enemy.floats[0]);
            else equal=equal && workspace.registers[0]==enemy.ints[0];
            if(!equal) {
                if(result.mismatches==0)
                    std::cerr<<"ECL RNG mismatch seed="<<seed<<" sample="<<sample<<'\n';
                ++result.mismatches;
            }
            ++result.operations;
        }
    }
    return result;
}
} // namespace ecl_reference
)CPP";
} // namespace

std::string ecl_reference(const std::filesystem::path &repo) {
    const auto manager = source(repo / "src/EclManager.hpp",
                                "1df7f926d46d24ad9e303a1a5e9b82cf9674ebec63d9ac5ff770c9c3957812e4");
    const auto integers =
        source(repo / "src/EclOperandsInt.cpp",
               "7a11fcc17dc929484c2eaf32d4eb94a2e78165d592b1d4fa13d590aa9b26ab0a");
    const auto floats = source(repo / "src/EclOperandsFloat.cpp",
                               "5a6ede1121d4387d7e45ba1c421012668d60bba1e898207ff5b3ba778430c726");
    const auto dispatch =
        source(repo / "src/EclRunLow.inl",
               "8c6d23bf4e9daf8f96dbd344f4a03d3ed32d1d200483959682e962cd41ec0045");
    return "namespace ecl_reference {\n" + function(manager, "enum EclOpcode") + ";\n" +
           function(manager, "enum EclOperandId") + ";\n" + adapter +
           "i32 EclOperands::ResolveInt(Enemy*,i32 operand) {switch(operand) {\n" +
           random_cases(integers, false) + "default:return operand;}}\n" +
           "f32 Enemy::ResolveFloat(f32 operand) {switch((i32)operand) {\n" +
           random_cases(floats, true) + "default:return operand;}}\n" +
           section(dispatch, "#define RawInt", "#pragma var_order(angle") +
           "void execute(Enemy* enemy,Instruction* instruction,int opcode) {switch(opcode) {\n" +
           section(dispatch,
                   "    case ECL_OPCODE_SET_INT:", "    case ECL_OPCODE_INT_ADD_ASSIGN:") +
           "default:throw std::runtime_error(\"unsupported source scalar fixture\");}}\n" + tests;
}
