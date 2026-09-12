#include "enemy_motion_source_probe.hpp"
#include <stdexcept>

std::string source(const std::filesystem::path &path, const char *expected);
std::string function(const std::string &text, const std::string &signature);
namespace {
std::string section(const std::string &text, const char *first, const char *after) {
    const auto begin = text.find(first);
    const auto end = text.find(after, begin);
    if (begin == std::string::npos || end == std::string::npos)
        throw std::runtime_error("missing pinned enemy movement section");
    return text.substr(begin, end - begin);
}
constexpr const char *adapter = R"CPP(
using D3DXVECTOR3=Float3;
#define D3DXVECTOR3_PTR(pointer) reinterpret_cast<D3DXVECTOR3*>(pointer)
#define __fastcall
struct Enemy {
    Float3 position,positionOffset,worldPosition,velocity,previousPosition,lastFrameDisplacement;
    Float3 movementInterpolationOrigin,movementInterpolationDelta;
    float movementAngle=0,angularVelocity=0,speed=0,acceleration=0;
    float orbitAngle=0,orbitAngularVelocity=0,orbitRadius=0,radialVelocity=0;
    ZunTimer movementTimer;
    int movementDuration=0;
    unsigned flags1=0;
    struct {Float3 lower,upper;} movementBounds;
    Enemy* parentEnemy=nullptr;
    void UpdateMovement();
    void ClampPosition();
    void IntegrateVelocity();
};
// Only literal operands enter the extracted configuration helpers. Operand
// decoding/shared RNG are a separate source-comparison domain.
struct EclRawInstruction {int duration,easing;float x,y;};
int literal_int(EclRawInstruction* instruction,int index) {
    if(index==0)return instruction->duration;
    if(index==1)return instruction->easing;
    throw std::runtime_error("unexpected source movement integer operand");
}
float literal_float(EclRawInstruction* instruction,int index) {
    if(index==2)return instruction->x;
    if(index==3)return instruction->y;
    throw std::runtime_error("unexpected source movement float operand");
}
#define ReadInt(enemy,instruction,index) literal_int(instruction,index)
#define ReadFloat(enemy,instruction,index) literal_float(instruction,index)
)CPP";
} // namespace
std::string enemy_motion_reference(const std::filesystem::path &repo) {
    const auto header = source(repo / "src/EnemyManager.hpp",
                               "e56633232cfb8e0934fb9e83f592989b577cd045e623df0c2c294eed9b2bf256");
    const auto implementation =
        source(repo / "src/EnemyManager.cpp",
               "e8febe94a833472b33f732e83ee39ee48fdc5097c5d69ff094fd1f1bb8629a7d");
    const auto manager = source(repo / "src/EnemyManagerUpdate.cpp",
                                "5692ab3214e95873626e6ab896f867746217b0556b34737c2556e1b38a454e59");
    const auto helpers = source(repo / "src/EclHelpers.cpp",
                                "64a9318a9a3b89d02f221b1837e618c027c3a7814ed43481a0ca78a5c0b77f73");
    const auto ecl = source(repo / "src/EclManager.hpp",
                            "1df7f926d46d24ad9e303a1a5e9b82cf9674ebec63d9ac5ff770c9c3957812e4");
    std::string generated = "namespace enemy_motion_reference {\n";
    for (const char *name : {"enum EnemyMovementMode", "enum EnemyFlag1Mask",
                             "enum EnemyFlag1Shift", "struct EnemyFlag1Bits"})
        generated += function(header, name) + ";\n";
    generated += function(ecl, "enum EclEasingMode") + ";\n" + adapter;
    for (const char *name : {"void Enemy::UpdateMovement()", "void Enemy::ClampPosition()",
                             "void Enemy::IntegrateVelocity()"})
        generated += function(implementation, name) + '\n';
    generated += function(helpers, "void __fastcall ConfigurePolarMotion(") + '\n' +
                 function(helpers, "void __fastcall ConfigureRelativeMotion(") + '\n';
    generated +=
        "void integrate(Enemy* enemy) {\n" +
        section(manager,
                "        if (!reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->skipMovement)",
                "        if (enemy->alignmentEffect != 0)") +
        "}\n#undef ReadFloat\n#undef ReadInt\n#undef __fastcall\n"
        "#undef D3DXVECTOR3_PTR\n}\n";
    return generated;
}
