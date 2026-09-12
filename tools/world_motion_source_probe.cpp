#include "world_motion_source_probe.hpp"
#include <stdexcept>
#include <vector>

std::string source(const std::filesystem::path &path, const char *expected);
std::string function(const std::string &text, const std::string &signature);
namespace {
std::string section(const std::string &text, const char *first, const char *after) {
    const auto begin = text.find(first);
    const auto end = text.find(after, begin);
    if (begin == std::string::npos || end == std::string::npos)
        throw std::runtime_error("missing pinned world movement section");
    return text.substr(begin, end - begin);
}
std::string operand_cases(const std::string &text, bool floating) {
    // Only this explicit selector domain enters the comparison. Missing globals
    // throw rather than receiving fabricated source-adapter defaults.
    std::vector<std::string> ids = {"RANDOM_NONNEGATIVE_INT",
                                    "RANDOM_UNIT_FLOAT",
                                    "RANDOM_RAW_INT",
                                    "RANDOM_SIGNED_UNIT_FLOAT",
                                    "ENEMY_POSITION_X",
                                    "ENEMY_POSITION_Y",
                                    "ENEMY_POSITION_Z",
                                    "PLAYER_POSITION_X",
                                    "PLAYER_POSITION_Y",
                                    "PLAYER_POSITION_Z",
                                    "INTERPOLATION_ORIGIN_X",
                                    "INTERPOLATION_ORIGIN_Y",
                                    "INTERPOLATION_ORIGIN_Z",
                                    "LAST_FRAME_DISPLACEMENT_X",
                                    "LAST_FRAME_DISPLACEMENT_Y",
                                    "LAST_FRAME_DISPLACEMENT_Z",
                                    "ANGLE_TO_PLAYER",
                                    "DISTANCE_TO_PLAYER",
                                    "MOVEMENT_ANGLE",
                                    "ANGULAR_VELOCITY",
                                    "SPEED",
                                    "ACCELERATION",
                                    "ORBIT_RADIUS",
                                    "ORBIT_ANGLE",
                                    "ORBIT_ANGULAR_VELOCITY"};
    if (floating)
        ids.insert(ids.end(), {"RANDOM_ANGLE", "INTERPOLATION_DELTA_X", "INTERPOLATION_DELTA_Y",
                               "INTERPOLATION_DELTA_Z"});
    std::string result;
    for (const auto &id : ids) {
        const auto begin = text.find("    case ECL_OPERAND_" + id + ':');
        auto end = text.find("    case ", begin + 5);
        const auto fallback = text.find("    default:", begin);
        if (fallback < end)
            end = fallback;
        if (begin == std::string::npos || end == std::string::npos)
            throw std::runtime_error("missing pinned movement operand case " + id);
        result += text.substr(begin, end - begin);
    }
    return result;
}
constexpr const char *adapter = R"CPP(
using namespace enemy_motion_reference;
using D3DXVECTOR3=Float3;
using FLOAT=float;
#define D3DXVECTOR3_PTR(pointer) reinterpret_cast<D3DXVECTOR3*>(pointer)
#define __fastcall
struct Enemy:enemy_motion_reference::Enemy {
    float ResolveFloat(float operand);
};
struct Player {
    Float3 position;
    float AngleToPoint(Float3* position);
} g_Player;
Rng g_Rng;
struct EclRawInstruction {unsigned short operandFlags; alignas(4) unsigned char operands[32];};
namespace EclOperands {int ResolveInt(Enemy* enemy,int operand);}
)CPP";
} // namespace
std::string world_motion_reference(const std::filesystem::path &repo) {
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
    const auto scheduler =
        source(repo / "src/EclRun.cpp",
               "010049211263e47d8245c7335f56b17a8502ca0f84595c8b035926a495d90b57");
    const auto helpers = source(repo / "src/EclHelpers.cpp",
                                "64a9318a9a3b89d02f221b1837e618c027c3a7814ed43481a0ca78a5c0b77f73");
    const auto dependencies =
        source(repo / "src/EclDependencies.cpp",
               "019f9cd6abdb73223d3d41cc8a6317641e6fe6bfbd7777d126a4bace3e14e2e4");
    const auto player = source(repo / "src/Player.cpp",
                               "80c6829a41a30fcce47837edaa8da90bb11779130b5c443db842c7623745242c");
    const auto d3dx = source(repo / "src/modern/linux/d3dx8_compat.cpp",
                             "8e9649ef554dcc2ac7d0218974a48d4583bae32591f4a7cd5b16ecdca3b62388");
    std::string generated = "namespace world_motion_reference {\n" +
                            function(manager, "enum EclOpcode") + ";\n" +
                            function(manager, "enum EclOperandId") + ";\n" + adapter;
    generated += function(player, "f32 Player::AngleToPoint(") + '\n';
    for (const char *signature :
         {"FLOAT D3DXVec3Dot(", "FLOAT D3DXVec3LengthSq(", "FLOAT D3DXVec3Length("})
        generated += function(d3dx, signature) + '\n';
    generated +=
        "i32 EclOperands::ResolveInt(Enemy* enemy,i32 operand) {switch(operand) {\n" +
        operand_cases(integers, false) +
        "default: if(operand>=10000 && operand<=10100 && !(operand>=10079 && operand<=10082))"
        " throw std::runtime_error(\"integer selector outside source movement fixture\");"
        "return operand;}}\n";
    generated += "f32 Enemy::ResolveFloat(f32 operand) {switch((i32)operand) {\n" +
                 operand_cases(floats, true) +
                 "default: if(operand>=10000 && operand<10100 && i32(operand)!=10098)"
                 " throw std::runtime_error(\"float selector outside source movement fixture\");"
                 "return operand;}}\n";
    generated += section(dispatch, "#define RawInt", "#pragma var_order(angle") +
                 "namespace EclHelpers {\n" +
                 function(helpers, "void __fastcall ConfigurePolarMotion(") + '\n' +
                 function(helpers, "void __fastcall ConfigureRelativeMotion(") + "\n}\n";
    generated += "namespace EclRunLow {\n" +
                 section(dependencies, "#define DEP_READ_INT", "// FUNCTION: th08 0x4222b0") +
                 function(dependencies, "void __fastcall StartTimedPolarDisplacement(") + '\n' +
                 function(dependencies, "void __fastcall BeginBoundaryAwareMove(") + '\n' +
                 function(dependencies, "void __fastcall ApplyRandomBiasedMove(") +
                 "\n#undef DEP_READ_INT\n#undef DEP_READ_FLOAT\n}\n"
                 "using EclRunLow::BeginBoundaryAwareMove;\n"
                 "using EclRunLow::ApplyRandomBiasedMove;\n"
                 "#define TH08_ECL_RUN_LOW_BODY\n"
                 "#define TH08_ECL_CONTEXT_ENEMY(unused) enemy\n"
                 "#define TH08_ECL_CONTEXT_INSTRUCTION(unused) instruction\n";
    const auto publication =
        section(scheduler, "        *D3DXVECTOR3_PTR(&enemy->worldPosition) =",
                "        if ((int)enemy->activeEclContext->secondaryTime > 0)");
    generated +=
        "void execute(Enemy* enemy,EclRawInstruction* instruction,int opcode) {\n" + publication +
        "switch(opcode) {\n" +
        section(dispatch, "    case ECL_OPCODE_SET_POSITION:",
                "    case ECL_OPCODE_SET_AIMED_DIRECTION_AND_SPEED:") +
        section(dispatch, "    case ECL_OPCODE_SET_AIMED_DIRECTION_AND_SPEED:",
                "    case ECL_OPCODE_SET_HITBOX:") +
        "default:throw std::runtime_error(\"unsupported source movement instruction\");}\n" +
        publication +
        "\n}\n"
        "#undef RawInt\n#undef RawFloat\n#undef ReadInt\n#undef ReadFloat\n"
        "#undef ReadFloatRawArg\n#undef WriteInt\n#undef WriteFloat\n"
        "#undef TH08_ECL_RUN_LOW_BODY\n#undef TH08_ECL_CONTEXT_ENEMY\n"
        "#undef TH08_ECL_CONTEXT_INSTRUCTION\n"
        "#undef __fastcall\n#undef D3DXVECTOR3_PTR\n}\n";
    return generated;
}
