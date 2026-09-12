#include "resource_reports.hpp"
#include <iomanip>
#include <stdexcept>

namespace th08::audit {
namespace {
std::ofstream open(const std::filesystem::path &directory, const char *name, const char *header) {
    std::ofstream file(directory / name);
    file.exceptions(std::ios::badbit | std::ios::failbit);
    file << std::setprecision(17) << header << '\n';
    return file;
}
void hex(std::ostream &out, resources::View bytes) {
    constexpr char digits[] = "0123456789abcdef";
    for (std::size_t i = 0; i < bytes.size; ++i)
        out << digits[bytes.data[i] >> 4] << digits[bytes.data[i] & 15];
}
} // namespace
ResourceReports::ResourceReports(const std::filesystem::path &output) : output_(output) {
    timeline_ =
        open(output, "timelines.tsv", "file\ttimeline\toffset\ttime\topcode\tsize\tmask\tpayload");
    sht_header_ = open(output, "sht_headers.tsv",
                       "file\tinitial_bombs\tdeathbomb_frames\thurtbox\tgraze\tauto_collect_"
                       "speed\tcollection_box\tpoint_line\treserved20\tnormal_axis\tfocused_"
                       "axis\tnormal_diagonal\tfocused_diagonal\titem_speed");
    sht_level_ =
        open(output, "sht_levels.tsv", "file\tlevel\tminimum_power\toffset\tterminal\tdescriptors");
    sht_shot_ = open(output, "sht_shots.tsv",
                     "file\tlevel\toffset\tinterval\tframe\tx\ty\thitbox_x\thitbox_"
                     "y\tangle\tspeed\tdamage\tgauge\toption\ttype\tanimation\tsound\tspawn_"
                     "callback\tupdate_callback\tdraw_callback\tcollision_callback");
    anm_sprite_ =
        open(output, "anm_sprites.tsv", "file\tindex\tentry\traw_id\toffset\tx\ty\twidth\theight");
    anm_script_ = open(output, "anm_scripts.tsv",
                       "file\tindex\tentry\traw_id\toffset\tinstructions_including_end");
    anm_instruction_ = open(output, "anm_instructions.tsv",
                            "file\tscript\toffset\topcode\ttime\tsize\tmask\tpayload");
    stage_object_ =
        open(output, "std_objects.tsv",
             "file\tindex\tid\toffset\tz_level\tflags\tx\ty\tz\twidth\theight\tdepth\tquads");
    stage_quad_ =
        open(output, "std_quads.tsv", "file\tobject\toffset\ttype\tsize\tanm_script\tvm\tgeometry");
    stage_instance_ = open(output, "std_instances.tsv", "file\toffset\tobject\tx\ty\tz");
    stage_instruction_ = open(output, "std_instructions.tsv",
                              "file\toffset\ttime\topcode\tpayload_size\tword0\tword1\tword2");
    open(output, "resource_summary.json", "{\"status\":\"INCOMPLETE\"}");
}
void ResourceReports::ecl(const std::string &file, resources::View bytes,
                          const resources::Ecl &ecl) {
    timelines_ += ecl.timelines.size();
    timeline_instructions_ += ecl.timeline_instructions.size();
    for (std::size_t id = 0; id < ecl.timelines.size(); ++id) {
        const auto &timeline = ecl.timelines[id];
        for (std::size_t i = timeline.first; i < timeline.first + timeline.count; ++i) {
            const auto &ins = ecl.timeline_instructions[i];
            timeline_ << file << '\t' << id << '\t' << ins.offset << '\t' << ins.time << '\t'
                      << ins.opcode << '\t' << unsigned(ins.size) << '\t' << unsigned(ins.mask)
                      << '\t';
            hex(timeline_, bytes.sub(ins.offset + 8, ins.size - 8));
            timeline_ << '\n';
        }
    }
}
void ResourceReports::sht(const std::string &file, const resources::Sht &sht) {
    ++sht_files_;
    sht_levels_ += sht.levels.size();
    sht_shots_ += sht.shots.size();
    const auto &h = sht.header;
    sht_header_ << file << '\t' << h.initial_bombs << '\t' << h.deathbomb_frames << '\t'
                << h.hurtbox_size << '\t' << h.graze_size << '\t' << h.item_auto_collect_speed
                << '\t' << h.item_collection_size << '\t' << h.point_value_line << '\t'
                << h.reserved20 << '\t' << h.normal_axis << '\t' << h.focused_axis << '\t'
                << h.normal_diagonal << '\t' << h.focused_diagonal << '\t' << h.item_speed << '\n';
    for (std::size_t id = 0; id < sht.levels.size(); ++id) {
        const auto &level = sht.levels[id];
        sht_level_ << file << '\t' << id << '\t' << level.minimum_power << '\t' << level.offset
                   << '\t' << level.terminal << '\t' << level.count << '\n';
        for (std::size_t i = level.first; i < level.first + level.count; ++i) {
            const auto &shot = sht.shots[i];
            sht_shot_ << file << '\t' << id << '\t' << shot.offset << '\t' << shot.interval << '\t'
                      << shot.frame;
            for (auto value : shot.position)
                sht_shot_ << '\t' << value;
            for (auto value : shot.hitbox)
                sht_shot_ << '\t' << value;
            sht_shot_ << '\t' << shot.angle << '\t' << shot.speed << '\t' << shot.damage << '\t'
                      << shot.gauge_behavior << '\t' << shot.option << '\t' << shot.shot_type
                      << '\t' << shot.animation << '\t' << shot.sound;
            for (auto value : shot.callbacks)
                sht_shot_ << '\t' << value;
            sht_shot_ << '\n';
        }
    }
}
void ResourceReports::anm(const std::string &file, resources::View bytes,
                          const resources::Anm &anm) {
    ++anm_files_;
    anm_entries_ += anm.entries.size();
    anm_sprites_ += anm.sprites.size();
    anm_scripts_ += anm.scripts.size();
    anm_instructions_ += anm.instructions.size() - anm.scripts.size();
    for (std::size_t id = 0; id < anm.sprites.size(); ++id) {
        const auto &s = anm.sprites[id];
        anm_sprite_ << file << '\t' << id << '\t' << s.entry << '\t' << s.raw_id << '\t' << s.offset
                    << '\t' << s.x << '\t' << s.y << '\t' << s.width << '\t' << s.height << '\n';
    }
    for (std::size_t id = 0; id < anm.scripts.size(); ++id) {
        const auto &script = anm.scripts[id];
        anm_script_ << file << '\t' << id << '\t' << script.entry << '\t' << script.raw_id << '\t'
                    << script.offset << '\t' << script.count << '\n';
        for (std::size_t i = script.first; i < script.first + script.count; ++i) {
            const auto &ins = anm.instructions[i];
            anm_instruction_ << file << '\t' << id << '\t' << ins.offset << '\t' << ins.opcode
                             << '\t' << ins.time << '\t' << ins.size << '\t' << ins.mask << '\t';
            if (ins.opcode != -1)
                hex(anm_instruction_, bytes.sub(ins.offset + 8, ins.size - 8));
            anm_instruction_ << '\n';
        }
    }
}
void ResourceReports::stage(const std::string &file, const resources::Std &stage) {
    ++std_files_;
    std_objects_ += stage.objects.size();
    std_quads_ += stage.quads.size();
    std_instances_ += stage.instances.size();
    std_instructions_ += stage.instructions.size();
    for (std::size_t id = 0; id < stage.objects.size(); ++id) {
        const auto &object = stage.objects[id];
        stage_object_ << file << '\t' << id << '\t' << object.id << '\t' << object.offset << '\t'
                      << int(object.z_level) << '\t' << unsigned(object.flags);
        for (auto value : object.position)
            stage_object_ << '\t' << value;
        for (auto value : object.size)
            stage_object_ << '\t' << value;
        stage_object_ << '\t' << object.count << '\n';
        for (std::size_t i = object.first; i < object.first + object.count; ++i) {
            const auto &quad = stage.quads[i];
            stage_quad_ << file << '\t' << id << '\t' << quad.offset << '\t' << quad.type << '\t'
                        << quad.size << '\t' << quad.anm_script << '\t' << quad.vm;
            for (unsigned field = 0; field < unsigned(quad.size - 8) / 4; ++field)
                stage_quad_ << '\t' << quad.geometry[field];
            stage_quad_ << '\n';
        }
    }
    for (const auto &instance : stage.instances) {
        stage_instance_ << file << '\t' << instance.offset << '\t' << instance.object;
        for (auto value : instance.position)
            stage_instance_ << '\t' << value;
        stage_instance_ << '\n';
    }
    for (const auto &ins : stage.instructions) {
        stage_instruction_ << file << '\t' << ins.offset << '\t' << ins.time << '\t' << ins.opcode
                           << '\t' << ins.payload_size;
        for (auto word : ins.words)
            stage_instruction_ << '\t' << word;
        stage_instruction_ << '\n';
    }
}
void ResourceReports::finish() {
    if (sht_files_ != 8 || sht_levels_ != 50 || sht_shots_ != 227 || anm_files_ != 113 ||
        anm_entries_ != 310 || anm_scripts_ != 1151 || anm_sprites_ != 1917 ||
        anm_instructions_ != 15966 || std_files_ != 18 || std_objects_ != 68 || std_quads_ != 552 ||
        std_instances_ != 1332 || std_instructions_ != 641)
        throw std::runtime_error("native auxiliary-resource counts disagree with pinned corpus");
    auto summary = open(output_, "resource_summary.json", "{");
    summary << "  \"status\": \"PASSED\",\n  \"timelines\": " << timelines_
            << ",\n  \"timeline_instructions\": " << timeline_instructions_
            << ",\n  \"sht_files\": 8,\n  \"sht_levels\": 50,\n  \"sht_descriptors\": 227,\n"
            << "  \"anm_files\": 113,\n  \"anm_entries\": 310,\n  \"anm_scripts\": 1151,\n"
            << "  \"anm_sprites\": 1917,\n  \"anm_instructions\": 15966,\n"
            << "  \"std_files\": 18,\n  \"std_objects\": 68,\n  \"std_quads\": 552,\n"
            << "  \"std_instances\": 1332,\n  \"std_instructions\": 641,\n"
            << "  \"scope\": \"native structure and field parsing, not complete resource "
               "execution\"\n}\n";
}
} // namespace th08::audit
