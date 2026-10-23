# The imported game remains a separate target; existing solver kernels keep their
# own numerical and ownership contracts. This build needs no Python or OpenGL.
find_package(PkgConfig REQUIRED)
pkg_check_modules(TH08_SDL REQUIRED IMPORTED_TARGET sdl2 SDL2_image SDL2_ttf fontconfig)
find_package(Threads REQUIRED)
set(th08_native "${CMAKE_CURRENT_SOURCE_DIR}/third_party/th08")
set(TH08_HEADLESS_OPTIMIZATION "3" CACHE STRING "Native game optimization level (0..3)")
if(NOT TH08_HEADLESS_OPTIMIZATION MATCHES "^[0-3]$")
  message(FATAL_ERROR "TH08_HEADLESS_OPTIMIZATION must be 0..3")
endif()
set(th08_generated "${CMAKE_CURRENT_BINARY_DIR}/headless-generated")
add_executable(th08_headless_i18n tools/headless_i18n.cpp)
target_compile_features(th08_headless_i18n PRIVATE cxx_std_17)
add_custom_command(OUTPUT "${th08_generated}/i18n.hpp"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${th08_generated}"
  COMMAND th08_headless_i18n "${th08_native}/config/i18n.csv" "${th08_generated}/i18n.hpp"
  DEPENDS th08_headless_i18n "${th08_native}/config/i18n.csv" VERBATIM)
# Use the upstream production translation units, including their normal globals.
set(th08_game_units
  AnmManager AsciiManager AsciiManagerBossMarker AsciiManagerGauge
  AsciiManagerGuiMode AsciiManagerScale Background BulletManager EclDependencies
  EclExIns EclGlobals EclHelpers EclManager EclOperandsFloat EclOperandsInt EclRun
  EffectManager Ending EnemyManager EnemyManagerUpdate EnemyTimeline GameManager
  Global Gui ItemManager Midi MusicRoom Player PlayerBomb ReplayManager ResultScreen
  ScoreDat ScreenEffect SoundPlayer Spellcard Supervisor TextHelper TitleScreen main
  utils zwave pbg/Lzss pbg/PbgArchive pbg/PbgFile)
set(th08_game_files)
foreach(unit IN LISTS th08_game_units)
  list(APPEND th08_game_files "${th08_native}/src/${unit}.cpp")
endforeach()
add_library(th08_native_headless STATIC ${th08_game_files}
  "${th08_generated}/i18n.hpp"
  "${th08_native}/src/modern/linux/linux_compat.cpp"
  "${th08_native}/src/modern/linux/d3dx8_compat.cpp"
  "${th08_native}/src/modern/linux/render_audit.cpp"
  "${th08_native}/src/modern/headless/d3d_null.cpp"
  "${th08_native}/src/modern/headless/runtime.cpp")
target_sources(th08_native_headless PRIVATE "${th08_native}/src/modern/headless/session.cpp"
  "${th08_native}/src/modern/headless/body_forecast.cpp"
  "${th08_native}/src/modern/headless/replay_check.cpp")
target_compile_features(th08_native_headless PUBLIC cxx_std_17)
target_compile_definitions(th08_native_headless PUBLIC TH08_MODERN_PORT
  TH08_MODERN_LINUX TH08_PORTABLE_NATIVE_LAYOUT TH08_HEADLESS WIN32_LEAN_AND_MEAN
  _FILE_OFFSET_BITS=64 _TIME_BITS=64)
target_compile_definitions(th08_native_headless PUBLIC
  TH08_HEADLESS_OPTIMIZATION=${TH08_HEADLESS_OPTIMIZATION} TH08_ENABLE_REPLAY_TRACE)
target_include_directories(th08_native_headless PUBLIC "${th08_native}/src"
  "${th08_native}/src/modern/linux/include" "${th08_generated}")
# The headless experiment is optimized native float32. Do not apply fast-math.
# The reconstructed TUs keep ZUN's intentional field-overruns (e.g. the 8-byte
# replay player name); glibc fortify would abort on them at -O1+.
target_compile_options(th08_native_headless PRIVATE -O${TH08_HEADLESS_OPTIMIZATION}
  -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -fpermissive -fno-strict-aliasing
  -ffp-contract=off -Wno-write-strings -Wno-multichar -Wno-unknown-pragmas
  "-include${th08_native}/src/modern/linux/linux_compat.hpp")
target_link_libraries(th08_native_headless PUBLIC PkgConfig::TH08_SDL Threads::Threads dl)
add_executable(th08_headless tools/headless.cpp)
target_link_libraries(th08_headless PRIVATE th08_native_headless)
target_include_directories(th08_headless PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(th08_headless PRIVATE OpenSSL::Crypto)
target_compile_options(th08_headless PRIVATE -ffp-contract=off)
add_executable(th08_headless_repair tools/headless_repair.cpp)
target_compile_features(th08_headless_repair PRIVATE cxx_std_17)
target_link_libraries(th08_headless_repair PRIVATE OpenSSL::Crypto)

add_executable(th08_headless_probe tools/headless_probe.cpp)
target_compile_features(th08_headless_probe PRIVATE cxx_std_17)
add_executable(th08_headless_laser_items tests/headless_laser_items.cpp)
target_link_libraries(th08_headless_laser_items PRIVATE th08_native_headless)
target_compile_options(th08_headless_laser_items PRIVATE -ffp-contract=off
  "-include${th08_native}/src/modern/linux/linux_compat.hpp")

add_executable(th08_headless_bullet_bounds tests/headless_bullet_bounds.cpp)
target_include_directories(th08_headless_bullet_bounds PRIVATE include)
target_link_libraries(th08_headless_bullet_bounds PRIVATE th08_native_headless)
target_compile_options(th08_headless_bullet_bounds PRIVATE -ffp-contract=off
  "-include${th08_native}/src/modern/linux/linux_compat.hpp")

add_executable(th08_headless_body_forecast tests/headless_body_forecast.cpp)
target_link_libraries(th08_headless_body_forecast PRIVATE th08_native_headless)
target_compile_options(th08_headless_body_forecast PRIVATE -ffp-contract=off
  "-include${th08_native}/src/modern/linux/linux_compat.hpp")

set(TH08_HEADLESS_DAT "" CACHE FILEPATH "Private DAT for the optional full-scene regression")
set(TH08_HEADLESS_COMPARE_EXECUTABLE "" CACHE FILEPATH
  "Optional native executable built at another optimization level")

if(BUILD_TESTING AND TH08_HEADLESS_DAT)
  add_executable(th08_headless_routes tests/headless_routes.cpp)
  target_include_directories(th08_headless_routes PRIVATE include)
  target_link_libraries(th08_headless_routes PRIVATE th08_native_headless)
  target_compile_options(th08_headless_routes PRIVATE -ffp-contract=off
    "-include${th08_native}/src/modern/linux/linux_compat.hpp")
  add_test(NAME headless_routes COMMAND th08_headless_routes "${TH08_HEADLESS_DAT}")
endif()

if(BUILD_TESTING)
  add_executable(th08_headless_surface_copy tests/headless_surface_copy.cpp)
  target_link_libraries(th08_headless_surface_copy PRIVATE th08_native_headless)
  target_compile_options(th08_headless_surface_copy PRIVATE -ffp-contract=off
    "-include${th08_native}/src/modern/linux/linux_compat.hpp")
  add_test(NAME headless_surface_copy COMMAND th08_headless_surface_copy)
  add_executable(th08_headless_cancel_regions tests/headless_cancel_regions.cpp)
  target_link_libraries(th08_headless_cancel_regions PRIVATE th08_native_headless)
  target_compile_options(th08_headless_cancel_regions PRIVATE -ffp-contract=off
    "-include${th08_native}/src/modern/linux/linux_compat.hpp")
  add_test(NAME headless_cancel_regions COMMAND th08_headless_cancel_regions)
  add_executable(th08_headless_opcode_guards tests/headless_opcode_guards.cpp)
  target_link_libraries(th08_headless_opcode_guards PRIVATE th08_native_headless)
  target_compile_options(th08_headless_opcode_guards PRIVATE -ffp-contract=off
    "-include${th08_native}/src/modern/linux/linux_compat.hpp")
  add_test(NAME headless_opcode_guards COMMAND th08_headless_opcode_guards)
  add_test(NAME headless_report_contract COMMAND ${CMAKE_COMMAND}
    -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/headless_stage6b_contract_tests.cmake)
  add_executable(th08_headless_repair_child tests/headless_repair_child.cpp)
  target_compile_features(th08_headless_repair_child PRIVATE cxx_std_17)
  add_test(NAME headless_repair_protocol COMMAND ${CMAKE_COMMAND}
    -DREPAIR=$<TARGET_FILE:th08_headless_repair>
    -DCHILD=$<TARGET_FILE:th08_headless_repair_child>
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/repair-protocol
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/headless_repair_protocol.cmake)
endif()

add_executable(th08_replay_check tools/replay_check.cpp)
target_link_libraries(th08_replay_check PRIVATE th08_native_headless OpenSSL::Crypto)
target_include_directories(th08_replay_check PRIVATE "${th08_native}/src")
target_compile_options(th08_replay_check PRIVATE -ffp-contract=off "-include${th08_native}/src/modern/linux/linux_compat.hpp")
add_executable(th08_plan tools/plan.cpp)
target_link_libraries(th08_plan PRIVATE th08_native_headless OpenSSL::Crypto)
target_include_directories(th08_plan PRIVATE "${th08_native}/src" "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options(th08_plan PRIVATE -O3 -ffp-contract=off -fpermissive -Wno-write-strings
  -Wno-unknown-pragmas "-include${th08_native}/src/modern/linux/linux_compat.hpp")

add_library(th08_online_native src/online_native.cpp)
target_link_libraries(th08_online_native PUBLIC th08_online_policy th08_native_headless)
target_compile_options(th08_online_native PRIVATE -ffp-contract=off
  "-include${th08_native}/src/modern/linux/linux_compat.hpp")
add_executable(th08_online tools/online.cpp)
target_link_libraries(th08_online PRIVATE th08_online_native OpenSSL::Crypto)
target_compile_options(th08_online PRIVATE -ffp-contract=off
  "-include${th08_native}/src/modern/linux/linux_compat.hpp")
add_executable(th08_value_collect tools/value_collect.cpp)
add_executable(th08_online_live_cli tools/online_live.cpp)
set_target_properties(th08_online_live_cli PROPERTIES OUTPUT_NAME th08_online_live)
target_link_libraries(th08_online_live_cli PRIVATE th08_online_live th08_online_native OpenSSL::Crypto)
target_compile_options(th08_online_live_cli PRIVATE -ffp-contract=off
  "-include${th08_native}/src/modern/linux/linux_compat.hpp")
target_link_libraries(th08_value_collect PRIVATE th08_online_native OpenSSL::Crypto)
target_compile_options(th08_value_collect PRIVATE -ffp-contract=off
  "-include${th08_native}/src/modern/linux/linux_compat.hpp")

add_executable(th08_replay_target tools/replay_target.cpp)
target_link_libraries(th08_replay_target PRIVATE th08_native_headless th08_resources)
target_include_directories(th08_replay_target PRIVATE "${th08_native}/src")
target_compile_options(th08_replay_target PRIVATE -ffp-contract=off
  "-include${th08_native}/src/modern/linux/linux_compat.hpp")

add_executable(th08_hazard_probe tools/hazard_probe.cpp)
target_link_libraries(th08_hazard_probe PRIVATE th08_native_headless OpenSSL::Crypto)
target_include_directories(th08_hazard_probe PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options(th08_hazard_probe PRIVATE -ffp-contract=off)
if(BUILD_TESTING)
  add_executable(online_policy_tests tests/online_policy_tests.cpp)
  target_link_libraries(online_policy_tests PRIVATE th08_online_policy)
  add_test(NAME headless_online_policy_contract COMMAND online_policy_tests
    ${CMAKE_CURRENT_BINARY_DIR}/online-policy-test
    ${CMAKE_CURRENT_SOURCE_DIR}/models/legacy-risk)
  add_executable(hazard_probe_tests tests/hazard_probe_tests.cpp)
  target_include_directories(hazard_probe_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
  target_compile_features(hazard_probe_tests PRIVATE cxx_std_17)
  add_test(NAME headless_hazard_probe_contract COMMAND hazard_probe_tests)
endif()
