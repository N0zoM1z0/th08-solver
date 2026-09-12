// Materialize a native test TU from the exact pinned reconstruction functions.
// Only generated build files are written; the source checkout remains untouched.
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <th08/resources.hpp>

namespace res = th08::resources;
std::string source(const std::filesystem::path &path, const char *expected) {
    const auto bytes = res::read_file(path);
    if (res::sha256(res::view(bytes)) != expected)
        throw std::runtime_error("reference source hash mismatch: " + path.string());
    return {bytes.begin(), bytes.end()};
}
std::string function(const std::string &text, const std::string &signature) {
    const auto start = text.find(signature);
    if (start == std::string::npos)
        throw std::runtime_error("missing reference function");
    auto opening = text.find('{', start);
    if (opening == std::string::npos)
        throw std::runtime_error("missing function body");
    unsigned depth = 1;
    auto end = opening + 1;
    for (; end < text.size() && depth; ++end) {
        if (text[end] == '{')
            ++depth;
        if (text[end] == '}')
            --depth;
    }
    if (depth)
        throw std::runtime_error("unterminated reference function");
    return text.substr(start, end - start);
}
constexpr const char *prefix = R"CPP(
#include <th08/geometry.hpp>
#include <th08/kinematics.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <random>
#include <cstring>
using f32=float;
using i32=int32_t;
using u32=uint32_t;
constexpr float ZUN_PI=3.14159265358979323846f;
constexpr float ZUN_2PI=ZUN_PI*2.0f;
enum {BULLET_AIM_FAN_AIMED,BULLET_AIM_FAN,BULLET_AIM_CIRCLE_AIMED,BULLET_AIM_CIRCLE,
      BULLET_AIM_OFFSET_CIRCLE_AIMED,BULLET_AIM_OFFSET_CIRCLE,BULLET_AIM_RANDOM_ANGLE,
      BULLET_AIM_RANDOM_SPEED,BULLET_AIM_RANDOM};
struct BulletSpawnDescriptor {
    int aimMode,count1,count2;
    float speed1,speed2,angle,angleStep;
};
struct RandomTrace {
    float values[2];
    unsigned index=0;
    float GetRandomF32InRange(float range) {
        if(index>=2) throw std::runtime_error("unexpected reference RNG consumption");
        return values[index++]*range;
    }
} g_Rng;
struct Float3 {
    float x=0,y=0,z=0;
    Float3()=default;
    Float3(float a,float b,float c=0):x(a),y(b),z(c){}
    Float3 operator+(Float3 b)const{return {x+b.x,y+b.y,z+b.z};}
    Float3 operator-(Float3 b)const{return {x-b.x,y-b.y,z-b.z};}
    Float3 operator/(float s)const{return {x/s,y/s,z/s};}
};
struct PlayerCollisionRegion {
    Float3 center,size;
    float radius=0,angle=0;
    int active=0,collisionValue=0,hitAccumulator=0;
};
#define ARRAY_SIZE_SIGNED(x) int(sizeof(x)/sizeof((x)[0]))
enum {PLAYER_STATE_ALIVE,PLAYER_STATE_SPAWNING,PLAYER_STATE_DYING,PLAYER_STATE_INVULNERABLE};
constexpr u32 REPLAY_FRAME_EVENT_PLAYER_HIT=1;
struct Replay {u32 frameEventFlags=0;} replay;
Replay* g_ReplayManager=&replay;
struct Game {void RandomizeAntiTamper(){}} g_GameManager;
struct Player {
    Float3 position,hurtboxHalfSize,hurtboxBoundsMin,hurtboxBoundsMax,grazeBoundsMin,grazeBoundsMax;
    PlayerCollisionRegion cancelRegions[192];
    int bulletCancelItemType=0,playerState=0,deaths=0,grazes=0;
    void Die(){++deaths;playerState=PLAYER_STATE_DYING;}
    void AwardGraze(Float3*,int){++grazes;}
    i32 CheckBulletCancelCollision(Float3*,Float3*);
    i32 CheckBulletCollision(Float3*,Float3*);
    u32 CalcLaserHitbox(Float3*,Float3*,Float3*,f32,i32);
};
)CPP";
constexpr const char *suffix = R"CPP(
int main(int argc,char** argv) {
    std::mt19937 rng(20260912);
    auto uniform=[&](float low,float high) {
        return std::uniform_real_distribution<float>(low,high)(rng);
    };
    std::uint64_t mismatches=0;
    std::uint64_t launch_mismatches=0;
    for(int i=0;i<180000;++i) {
        using namespace th08::kinematics;
        const unsigned count_limit=i%2?1536:16;
        const Pattern pattern{Aim(i%9),int(rng()%count_limit+1),int(rng()%count_limit+1),
                              uniform(-5,20),uniform(-5,20),uniform(-200,200),uniform(-3,3)};
        const int index1=int(rng()%unsigned(pattern.count1));
        const int index2=int(rng()%unsigned(pattern.count2));
        const float aim=uniform(-pi,pi), multiplier=uniform(.25f,2);
        RandomPair random{uniform(0,1),uniform(0,1)};
        g_Rng.index=0;
        g_Rng.values[0]=pattern.aim==Aim::random_speed?random.speed:random.angle;
        g_Rng.values[1]=random.speed;
        BulletSpawnDescriptor descriptor{int(pattern.aim),pattern.count1,pattern.count2,
                                         pattern.speed1,pattern.speed2,pattern.angle,pattern.angle_step};
        const auto expected=reference_launch(&descriptor,index1,index2,aim,multiplier);
        Launch actual{};
        const unsigned draws=pattern.aim==Aim::random_angle_speed?2:unsigned(pattern.aim)>=6?1:0;
        auto same=[](float a,float b){return std::memcmp(&a,&b,sizeof(float))==0;};
        if(launch(pattern,index1,index2,aim,multiplier,random,actual)!=Status::ready ||
           !same(actual.raw_angle,expected.raw_angle) || !same(actual.angle,expected.angle) ||
           !same(actual.speed,expected.speed) || !same(actual.velocity_x,expected.velocity_x) ||
           !same(actual.velocity_y,expected.velocity_y) || g_Rng.index!=draws)
            ++launch_mismatches;
    }
    for(int i=0;i<300000;++i) {
        Player player;
        player.position={uniform(-40,424),uniform(-40,488)};
        player.hurtboxHalfSize={uniform(.5f,3),uniform(.5f,3)};
        player.hurtboxBoundsMin=player.position-player.hurtboxHalfSize;
        player.hurtboxBoundsMax=player.position+player.hurtboxHalfSize;
        Float3 center{uniform(-100,484),uniform(-100,548)};
        Float3 size{uniform(0,590),uniform(0,288)};
        Float3 origin{uniform(-100,484),uniform(-100,548)};
        float angle=uniform(-3.142f,3.142f);
        const int state=int(rng()%4);
        player.playerState=state;
        replay.frameEventFlags=0;
        player.CheckBulletCollision(&center,&size);
        const auto p=th08::geometry::Vec2{player.position.x,player.position.y};
        const auto half=th08::geometry::Vec2{player.hurtboxHalfSize.x,player.hurtboxHalfSize.y};
        const auto box=th08::geometry::Box{{center.x,center.y},{size.x,size.y}};
        const bool box_hit=th08::geometry::box_hit(p,half,box);
        if(box_hit!=bool(replay.frameEventFlags&1)) ++mismatches;
        if((player.deaths!=0)!=(box_hit && state==PLAYER_STATE_ALIVE)) ++mismatches;
        player.playerState=state;
        player.deaths=0;
        replay.frameEventFlags=0;
        player.CalcLaserHitbox(&center,&size,&origin,angle,0);
        const bool laser_hit=th08::geometry::laser_hit(p,half,{box,{origin.x,origin.y},angle});
        if(laser_hit!=bool(replay.frameEventFlags&1)) ++mismatches;
        if((player.deaths!=0)!=(laser_hit && state==PLAYER_STATE_ALIVE)) ++mismatches;
    }
    std::ofstream file;
    if(argc==2) {
        file.open(argv[1]);
        if(!file) return 2;
    }
    std::ostream& out=argc==2?file:std::cout;
    out << "{\"scope\":\"pinned reconstructed bodies versus maintained native predicates; not game execution\","
        << "\"random_cases\":300000,\"predicate_comparisons\":600000,\"mismatches\":"
        << mismatches << ",\"launch_cases\":180000,\"launch_mismatches\":" << launch_mismatches
        << ",\"velocity_profile\":\"TH08_MODERN_PORT float32; not retail x87\"}\n";
    return mismatches||launch_mismatches?1:0;
}
)CPP";
int main(int argc, char **argv) try {
    if (argc != 3)
        throw std::runtime_error("Usage: th08_source_probe reference-repo output.cpp");
    const std::filesystem::path repo = argv[1];
    const auto player = source(repo / "src/Player.cpp",
                               "80c6829a41a30fcce47837edaa8da90bb11779130b5c443db842c7623745242c");
    const auto global = source(repo / "src/Global.cpp",
                               "8df17616c935d684b6636619d4726889e68f7d2d7000e27c25aebc4bc460b74b");
    const auto bullet = source(repo / "src/BulletManager.cpp",
                               "77562e578c4fd2b2fd55f836f3b2e9e0ade16198208f81e94eb1d1a837207dc1");
    source(repo / "src/ZunMath.hpp",
           "ba187178ec936c2492f3e311e8d6d634421c35bc5abd74ab4af77a4c81ddb3de");
    const auto launch_start =
        bullet.find("    angle = 0.0f;", bullet.find("i32 BulletManager::SpawnSingleBullet("));
    const auto launch_end = bullet.find("    bullet->state = BULLET_STATE_FIRED;", launch_start);
    if (launch_start == std::string::npos || launch_end == std::string::npos)
        throw std::runtime_error("missing pinned launch block");
    std::ofstream out(argv[2]);
    out.exceptions(std::ios::badbit | std::ios::failbit);
    out << prefix << function(global, "f32 AddNormalizeAngle(") << '\n'
        << "th08::kinematics::Launch reference_launch(BulletSpawnDescriptor* descriptor, "
           "i32 index1,i32 index2,f32 angleToPlayer,float multiplier) { float angle,speed;\n"
        << bullet.substr(launch_start, launch_end - launch_start)
        << "return {angle,AddNormalizeAngle(angle,0),speed,cosf(angle)*(speed*multiplier),"
           "sinf(angle)*(speed*multiplier)}; }\n"
        << function(global, "void Rotate(Float3 *") << '\n'
        << function(player, "i32 Player::CheckBulletCancelCollision(") << '\n'
        << function(player, "i32 Player::CheckBulletCollision(") << '\n'
        << function(player, "u32 Player::CalcLaserHitbox(") << '\n'
        << suffix;
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
