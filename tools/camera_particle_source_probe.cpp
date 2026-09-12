#include "camera_particle_source_probe.hpp"

std::string source(const std::filesystem::path &, const char *);
std::string function(const std::string &, const std::string &);

std::string camera_particle_reference(const std::filesystem::path &repo) {
    const auto effect = source(repo / "src/EffectManager.cpp",
                               "63d45a213956008b44874bc4707c971a7799a9c551b07e732bf1f55282c2209e");
    const auto ecl = source(repo / "src/EclManager.hpp",
                            "1df7f926d46d24ad9e303a1a5e9b82cf9674ebec63d9ac5ff770c9c3957812e4");
    (void)ecl; // Pins the Effect layout/template identity, not a complete adapter layout.
    const auto enemy = source(repo / "src/EnemyManager.cpp",
                              "e8febe94a833472b33f732e83ee39ee48fdc5097c5d69ff094fd1f1bb8629a7d");
    const auto enemy_header =
        source(repo / "src/EnemyManager.hpp",
               "e56633232cfb8e0934fb9e83f592989b577cd045e623df0c2c294eed9b2bf256");
    const auto math = source(repo / "src/ZunMath.hpp",
                             "ba187178ec936c2492f3e311e8d6d634421c35bc5abd74ab4af77a4c81ddb3de");
    const auto background =
        source(repo / "src/Background.cpp",
               "36889a17a6f0c314eaf3751a2238e778c82d13bffd3a18e55051b49f3d8fd285");
    source(repo / "src/Background.hpp",
           "bbfa9022f52c5b5332f8e690d42c7338ec97f062b43a3bfcd6dc33190484efe8");
    const auto color = source(repo / "src/ZunColor.hpp",
                              "e8561d8b0f2770566bc0658b9ad2e8cb2dfbebc7fcea1cf72a59b80487503af2");
    const auto supervisor =
        source(repo / "src/Supervisor.cpp",
               "67b761377ae38aec18581920ea07ff31fb4dd3c0de0d15acdd42530d19fa1a5e");
    const auto global = source(repo / "src/Global.cpp",
                               "8df17616c935d684b6636619d4726889e68f7d2d7000e27c25aebc4bc460b74b");
    const auto global_header =
        source(repo / "src/Global.hpp",
               "ce49422a53e5ba33b63d803d17e7051ba2a5ad7a33ae531910c048a091f37592");
    const auto d3dx = source(repo / "src/modern/linux/d3dx8_compat.cpp",
                             "8e9649ef554dcc2ac7d0218974a48d4583bae32591f4a7cd5b16ecdca3b62388");
    const auto d3dx_header =
        source(repo / "src/modern/linux/include/d3dx8.h",
               "c9f5b34bf60903919726bfed0c2055b0c203ea630994a13bb5434925b56dd20e");
    std::string result = R"CPP(
#include <cmath>
#include <climits>
#include <cstdint>
namespace camera_particle_reference {
using f32=float; using u32=std::uint32_t; using i32=std::int32_t;
using u16=std::uint16_t; using u8=std::uint8_t; using FLOAT=float;
#define TH08_MODERN_PORT
)CPP";
    result += function(math, "struct Float3") + ";\n#undef TH08_MODERN_PORT\n";
    result += function(supervisor, "inline Float3::Float3(") + '\n';
    for (const auto *signature :
         {"Float3 Float3::operator+(", "Float3 Float3::operator-(", "Float3 Float3::operator*("})
        result += function(background, signature) + '\n';
    result += function(effect, "Float3 Float3::operator-()") + '\n';
    result +=
        "struct D3DVECTOR {float x,y,z;};\n" + function(d3dx_header, "struct D3DXVECTOR3") + ";\n";
    for (const auto *signature : {"FLOAT D3DXVec3Dot(", "FLOAT D3DXVec3LengthSq(",
                                  "FLOAT D3DXVec3Length(", "D3DXVECTOR3 *D3DXVec3Normalize("})
        result += function(d3dx, signature) + '\n';
    result += function(global_header, "class Rng") + ";\n";
    for (const auto *signature :
         {"void Rng::SetSeed(", "void Rng::ResetGenerationCount(", "u16 Rng::GetSeed(",
          "u16 Rng::GetRandomU16(", "u32 Rng::GetRandomU32(", "f32 Rng::GetRandomF32(",
          "f32 Rng::GetRandomF32Signed("})
        result += function(global, signature) + '\n';
    result += function(color, "union ZunColor") + ";\n";
    result += function(enemy_header, "enum EnemyFlag1Shift") + ";\n";
    result += R"CPP(
struct Animation {Float3 pos2,posInitial,posFinal,rotateInitial;ZunColor color1,color2;u32 flags;};
struct Effect {Animation vm;Float3 position,vector1,vector2,vector3,vector4;std::int8_t drawGroup;};
struct Camera {Float3 position,lookAtOffset,forward;};
struct Background {Camera cameraCurrent;Animation stageTextVm;} g_Background;
struct Enemy {Float3 position;u32 flags1;};
struct EnemyManager {Enemy* bosses[8]{};i32 HasBoss();} g_EnemyManager;
struct Supervisor {float framerateMultiplier;} g_Supervisor;
Rng g_Rng;
#define __fastcall
#define D3DXVECTOR3_PTR(pointer) reinterpret_cast<D3DXVECTOR3*>(pointer)
)CPP";
    result += function(enemy, "i32 EnemyManager::HasBoss()") + '\n';
    // Match definitions, not the earlier callback forward declarations.
    const auto body = effect.substr(effect.find("// FUNCTION: th08 0x426280"));
    result += function(body, "i32 __fastcall InitializeTintedBossTrackingCameraParticle(") + '\n';
    result += function(body, "i32 __fastcall UpdateTintedBossTrackingCameraParticle(") + '\n';
    return result + "#undef D3DXVECTOR3_PTR\n#undef __fastcall\n}\n";
}
