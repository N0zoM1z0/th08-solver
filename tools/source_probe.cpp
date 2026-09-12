// Materialize a native test TU from the exact pinned reconstruction functions.
// Only generated build files are written; the source checkout remains untouched.
#include "ecl_source_probe.hpp"
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
#include <th08/bullet_motion.hpp>
#include <th08/laser_motion.hpp>
#include <th08/rng.hpp>
#include <th08/emitter.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <random>
#include <cstring>
#include <climits>
using f32=float;
using i32=int32_t;
using u32=uint32_t;
using u16=uint16_t;
using ZunBool=int;
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
    void FromAngleMagnitude(float a,float m){x=cosf(a)*m;y=sinf(a)*m;}
};
constexpr unsigned BULLET_TRANSFORM_CHANGE_DIRECTION_RELATIVE=0x40;
constexpr unsigned BULLET_TRANSFORM_CHANGE_DIRECTION_AIMED=0x80;
constexpr unsigned BULLET_TRANSFORM_CHANGE_DIRECTION_ABSOLUTE=0x100;
constexpr int BULLET_TRANSFORM_STATE_DIRECTION_CHANGE=0;
using SoundIdx=int;
struct Sound {void PlaySoundByIdx(int,int){}} g_SoundPlayer;
struct Supervisor {float framerateMultiplier=1; void TickTimer(int*,float*);} g_Supervisor;
struct TurnState {
    ZunTimer timer;
    int directionChangeIntervalFrames=0,directionChangeRepeatCount=0,directionChangesCompleted=0;
    float directionChangeAngle=0,directionChangeSpeed=0;
};
struct Bullet {
    Float3 position,velocity;
    float angle=0,speed=0;
    int transformSound=-1;
    unsigned activeTransformFlags=0;
    TurnState exStates[1];
    void UpdateRelativeDirectionChange();
    void UpdateAbsoluteDirectionChange();
    void UpdateAimedDirectionChange();
};
float supplied_target_angle=0;
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
    float AngleToPoint(Float3*){return supplied_target_angle;}
    i32 CheckBulletCancelCollision(Float3*,Float3*);
    i32 CheckBulletCollision(Float3*,Float3*);
    u32 CalcLaserHitbox(Float3*,Float3*,Float3*,f32,i32);
};
Player g_Player;
enum {LASER_STATE_STARTING,LASER_STATE_ACTIVE,LASER_STATE_DESPAWNING};
struct DummySprite {float widthPx=16,heightPx=16;} dummy_sprite;
struct LaserVm {
    Float3 scale;
    DummySprite* loadedSprite=&dummy_sprite;
    struct {unsigned d3dColor=0;} color1;
    void SetZRotation(float){}
};
struct SourceLaser {
    Float3 position;
    float angle=0,startOffset=0,endOffset=0,startLength=0,width=0,speed=0,currentWidth=0;
    ZunTimer timer;
    int startTime=0,duration=0,despawnDuration=0,hitboxStartTime=0,hitboxEndDelay=0,state=0;
    bool inUse=true;
    unsigned flags=0;
    LaserVm bodyVm;
};
struct DummyAnm {int ExecuteScript(LaserVm*){return 0;}} dummy_anm;
DummyAnm* g_AnmManager=&dummy_anm;
#define FLOAT3_PTR(p) reinterpret_cast<Float3*>(p)
struct LaserRecorder {
    th08::laser::Result trace;
    int CalcLaserHitbox(Float3* center,Float3* size,Float3* origin,float angle,int graze) {
        if(trace.count>=3) throw std::runtime_error("reference exceeded laser call bound");
        trace.calls[trace.count++]={{{{center->x,center->y},{size->x,size->y}},
                                    {origin->x,origin->y},angle},graze!=0};
        return 0;
    }
};
)CPP";
constexpr const char *suffix = R"CPP(
int main(int argc,char** argv) {
    const auto ecl_random=ecl_reference::compare();
    std::mt19937 rng(20260912);
    auto uniform=[&](float low,float high) {
        return std::uniform_real_distribution<float>(low,high)(rng);
    };
    std::uint64_t mismatches=0;
    std::uint64_t rng_mismatches=0,rng_operations=0;
    for(unsigned seed=0;seed<65536;++seed) {
        Rng reference;
        reference.SetSeed(u16(seed));
        reference.ResetGenerationCount();
        th08::random::Rng actual{u16(seed)};
        for(unsigned operation=0;operation<8;++operation) {
            switch(operation%4) {
            case 0: if(reference.GetRandomU16()!=actual.next_u16()) ++rng_mismatches; break;
            case 1: if(reference.GetRandomU32()!=actual.next_u32()) ++rng_mismatches; break;
            case 2: {
                const float a=reference.GetRandomF32(),b=actual.unit();
                if(std::memcmp(&a,&b,sizeof(float))) ++rng_mismatches;
                break;
            }
            case 3: {
                const float a=reference.GetRandomF32Signed(),b=actual.signed_unit();
                if(std::memcmp(&a,&b,sizeof(float))) ++rng_mismatches;
                break;
            }
            }
            if(reference.GetSeed()!=actual.seed()) ++rng_mismatches;
            ++rng_operations;
        }
    }
    std::uint64_t launch_mismatches=0;
    std::uint64_t turn_mismatches=0,turn_frames=0;
    std::uint64_t laser_mismatches=0,laser_frames=0;
    for(int scenario=0;scenario<5000;++scenario) {
        th08::laser::State state;
        state.position={uniform(-100,484),uniform(-100,548)};
        state.angle=uniform(-3,3);
        state.end_offset=uniform(10,100);
        state.length=uniform(100,500);
        state.width=uniform(1,80);
        state.speed=uniform(0,8);
        state.start_time=int(rng()%60+1);
        state.duration=int(rng()%120);
        state.despawn_duration=int(rng()%40);
        state.hitbox_start_time=int(rng()%80);
        state.hitbox_end_delay=int(rng()%50);
        state.fade_alpha=(scenario&1)!=0;
        state.phase=th08::laser::Phase::starting;
        SourceLaser reference;
        reference.position={state.position.x,state.position.y};
        reference.angle=state.angle;
        reference.endOffset=state.end_offset;
        reference.startLength=state.length;
        reference.width=state.width;
        reference.speed=state.speed;
        reference.startTime=state.start_time;
        reference.duration=state.duration;
        reference.despawnDuration=state.despawn_duration;
        reference.hitboxStartTime=state.hitbox_start_time;
        reference.hitboxEndDelay=state.hitbox_end_delay;
        reference.flags=state.fade_alpha;
        g_Supervisor.framerateMultiplier=uniform(.25f,2);
        for(int frame=0;frame<300 && state.in_use;++frame) {
            const auto before=state;
            const auto expected=reference_laser(&reference);
            const auto actual=th08::laser::advance(state,g_Supervisor.framerateMultiplier);
            auto same=[](float a,float b){return std::memcmp(&a,&b,sizeof(float))==0;};
            if(actual.status==th08::laser::Status::invalid || actual.count!=expected.count ||
               !same(state.start_offset,reference.startOffset) || !same(state.end_offset,reference.endOffset) ||
               state.timer!=int(reference.timer) || !same(state.subframe,reference.timer.subFrame) ||
               int(state.phase)!=reference.state || state.in_use!=reference.inUse)
            {
                if(laser_mismatches<3)
                    std::cerr << "laser divergence: scenario=" << scenario << " frame=" << frame
                              << " phase=" << int(before.phase) << " timer=" << before.timer
                              << " width=" << std::hexfloat << before.width << std::defaultfloat
                              << " despawn=" << before.despawn_duration << " gate=" << before.hitbox_end_delay
                              << " expected_calls=" << expected.count << " actual_calls=" << actual.count
                              << " expected_x=" << (expected.count?expected.calls[0].geometry.box.size.x:0) << '\n';
                ++laser_mismatches;
            }
            for(unsigned i=0;i<std::min(actual.count,expected.count);++i) {
                const auto& a=actual.calls[i]; const auto& b=expected.calls[i];
                if(!same(a.geometry.box.center.x,b.geometry.box.center.x) ||
                   !same(a.geometry.box.center.y,b.geometry.box.center.y) ||
                   !same(a.geometry.box.size.x,b.geometry.box.size.x) ||
                   !same(a.geometry.box.size.y,b.geometry.box.size.y) || a.allow_graze!=b.allow_graze)
                    ++laser_mismatches;
            }
            ++laser_frames;
        }
    }
    for(int scenario=0;scenario<3000;++scenario) {
        using namespace th08::bullet;
        const auto mode=TurnMode(scenario%3);
        DirectionChange turn{mode,true,int(rng()%100),int(rng()%5+1),0,0,
                             uniform(-5,5),uniform(-2,8)};
        Flight flight{0,0,0,0,uniform(-5,5),uniform(-2,8)};
        Bullet reference;
        reference.angle=flight.angle;
        reference.speed=flight.speed;
        reference.activeTransformFlags=mode==TurnMode::relative?0x40:mode==TurnMode::absolute?0x100:0x80;
        reference.exStates[0].directionChangeIntervalFrames=turn.interval;
        reference.exStates[0].directionChangeRepeatCount=turn.repeats;
        reference.exStates[0].directionChangeAngle=turn.angle;
        reference.exStates[0].directionChangeSpeed=turn.speed;
        g_Supervisor.framerateMultiplier=uniform(.25f,2);
        for(int frame=0;frame<600 && turn.active;++frame) {
            supplied_target_angle=uniform(-3,3);
            if(mode==TurnMode::relative) reference.UpdateRelativeDirectionChange();
            else if(mode==TurnMode::absolute) reference.UpdateAbsoluteDirectionChange();
            else reference.UpdateAimedDirectionChange();
            const auto status=advance_direction(flight,turn,g_Supervisor.framerateMultiplier,supplied_target_angle);
            auto same=[](float a,float b){return std::memcmp(&a,&b,sizeof(float))==0;};
            auto& expected=reference.exStates[0];
            if(status!=Status::advanced || !same(flight.angle,reference.angle) ||
               !same(flight.speed,reference.speed) || !same(flight.velocity_x,reference.velocity.x) ||
               !same(flight.velocity_y,reference.velocity.y) || turn.timer!=int(expected.timer) ||
               !same(turn.subframe,expected.timer.subFrame) ||
               turn.completed!=expected.directionChangesCompleted ||
               turn.active!=(reference.activeTransformFlags!=0)) ++turn_mismatches;
            ++turn_frames;
        }
    }
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
        Float3 laser_size=size;
        if(i%13==0) laser_size.x=-uniform(0,8);
        player.CalcLaserHitbox(&center,&laser_size,&origin,angle,0);
        const auto laser_box=th08::geometry::Box{{center.x,center.y},{laser_size.x,laser_size.y}};
        const bool laser_hit=th08::geometry::laser_hit(p,half,{laser_box,{origin.x,origin.y},angle});
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
        << ",\"direction_frames\":" << turn_frames << ",\"direction_mismatches\":" << turn_mismatches
        << ",\"laser_frames\":" << laser_frames << ",\"laser_mismatches\":" << laser_mismatches
        << ",\"rng_operations\":" << rng_operations << ",\"rng_mismatches\":" << rng_mismatches
        << ",\"ecl_random_operations\":" << ecl_random.operations
        << ",\"ecl_random_mismatches\":" << ecl_random.mismatches
        << ",\"velocity_profile\":\"TH08_MODERN_PORT float32; not retail x87\"}\n";
    return mismatches||launch_mismatches||turn_mismatches||laser_mismatches||rng_mismatches||ecl_random.mismatches?1:0;
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
    const auto global_header =
        source(repo / "src/Global.hpp",
               "ce49422a53e5ba33b63d803d17e7051ba2a5ad7a33ae531910c048a091f37592");
    const auto bullet = source(repo / "src/BulletManager.cpp",
                               "77562e578c4fd2b2fd55f836f3b2e9e0ade16198208f81e94eb1d1a837207dc1");
    source(repo / "src/ZunMath.hpp",
           "ba187178ec936c2492f3e311e8d6d634421c35bc5abd74ab4af77a4c81ddb3de");
    const auto supervisor =
        source(repo / "src/Supervisor.cpp",
               "67b761377ae38aec18581920ea07ff31fb4dd3c0de0d15acdd42530d19fa1a5e");
    const auto supervisor_header =
        source(repo / "src/Supervisor.hpp",
               "985ed6b9c210ba4cc7e6c54e033df4e882ad46567e71ccd8c6fadcbe2efac976");
    const auto launch_start =
        bullet.find("    angle = 0.0f;", bullet.find("i32 BulletManager::SpawnSingleBullet("));
    const auto launch_end = bullet.find("    bullet->state = BULLET_STATE_FIRED;", launch_start);
    if (launch_start == std::string::npos || launch_end == std::string::npos)
        throw std::runtime_error("missing pinned launch block");
    const auto laser_start =
        bullet.find("            laser->endOffset +=",
                    bullet.find("ChainCallbackResult BulletManager::OnUpdate("));
    const auto laser_last =
        bullet.find("            g_AnmManager->ExecuteScript(&laser->bodyVm);", laser_start);
    if (laser_start == std::string::npos || laser_last == std::string::npos)
        throw std::runtime_error("missing pinned laser update block");
    const auto laser_end = bullet.find('\n', laser_last);
    std::ofstream out(argv[2]);
    out.exceptions(std::ios::badbit | std::ios::failbit);
    std::string preamble = prefix;
    preamble.insert(preamble.find("struct RandomTrace"),
                    function(global_header, "class Rng") + ";\n");
    preamble.insert(preamble.find("struct TurnState"),
                    function(supervisor_header, "struct ZunTimer") + ";\n");
    out << preamble << function(supervisor, "void Supervisor::TickTimer(") << '\n'
        << function(global, "void Rng::SetSeed(") << '\n'
        << function(global, "void Rng::ResetGenerationCount(") << '\n'
        << function(global, "u16 Rng::GetSeed(") << '\n'
        << function(global, "u16 Rng::GetRandomU16(") << '\n'
        << function(global, "u32 Rng::GetRandomU32(") << '\n'
        << function(global, "f32 Rng::GetRandomF32(") << '\n'
        << function(global, "f32 Rng::GetRandomF32Signed(") << '\n'
        << function(global, "f32 AddNormalizeAngle(") << '\n'
        << "th08::kinematics::Launch reference_launch(BulletSpawnDescriptor* descriptor, "
           "i32 index1,i32 index2,f32 angleToPlayer,float multiplier) { float angle,speed;\n"
        << bullet.substr(launch_start, launch_end - launch_start)
        << "return {angle,AddNormalizeAngle(angle,0),speed,cosf(angle)*(speed*multiplier),"
           "sinf(angle)*(speed*multiplier)}; }\n"
        << function(bullet, "void Bullet::UpdateRelativeDirectionChange()") << '\n'
        << function(bullet, "void Bullet::UpdateAbsoluteDirectionChange()") << '\n'
        << function(bullet, "void Bullet::UpdateAimedDirectionChange()") << '\n'
        << "th08::laser::Result reference_laser(SourceLaser* laser) { LaserRecorder g_Player; "
           "float laserSize[3],laserCenter[3],currentWidth; int alpha,rampWindow; "
           "for(int once=0;once<1;++once) {\n"
        << bullet.substr(laser_start, laser_end - laser_start) << "\n} return g_Player.trace; }\n"
        << function(global, "void Rotate(Float3 *") << '\n'
        << function(player, "i32 Player::CheckBulletCancelCollision(") << '\n'
        << function(player, "i32 Player::CheckBulletCollision(") << '\n'
        << function(player, "u32 Player::CalcLaserHitbox(") << '\n'
        << ecl_reference(repo) << suffix;
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
