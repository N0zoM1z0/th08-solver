#pragma once
#include <filesystem>
#include <fstream>
#include <th08/formats.hpp>

namespace th08::audit {
class ResourceReports {
    std::ofstream timeline_, sht_header_, sht_level_, sht_shot_, anm_sprite_, anm_script_,
        anm_instruction_;
    std::ofstream stage_object_, stage_quad_, stage_instance_, stage_instruction_;
    std::filesystem::path output_;
    std::size_t timelines_ = 0, timeline_instructions_ = 0, sht_files_ = 0, sht_levels_ = 0,
                sht_shots_ = 0;
    std::size_t anm_files_ = 0, anm_entries_ = 0, anm_sprites_ = 0, anm_scripts_ = 0,
                anm_instructions_ = 0;
    std::size_t std_files_ = 0, std_objects_ = 0, std_quads_ = 0, std_instances_ = 0,
                std_instructions_ = 0;

  public:
    explicit ResourceReports(const std::filesystem::path &output);
    void ecl(const std::string &file, resources::View bytes, const resources::Ecl &ecl);
    void sht(const std::string &file, const resources::Sht &sht);
    void anm(const std::string &file, resources::View bytes, const resources::Anm &anm);
    void stage(const std::string &file, const resources::Std &stage);
    void finish();
};
} // namespace th08::audit
