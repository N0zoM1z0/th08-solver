# Imported upstream TH08 runtime (headless) plus the offline Native scoring tools.
# This build needs no Python or OpenGL.
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
  "${th08_native}/src/modern/headless/score_audit.cpp"
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

# Project code that includes reconstructed game headers shares their compat
# prelude and keeps float contraction disabled.
function(th08_native_options target)
  target_compile_options(${target} PRIVATE -ffp-contract=off
    "-include${th08_native}/src/modern/linux/linux_compat.hpp")
endfunction()

# Natural replay playback, row streaming and the source capture audit.
add_executable(th08_replay_check tools/replay_check.cpp)
target_link_libraries(th08_replay_check PRIVATE th08_native_headless OpenSSL::Crypto)
th08_native_options(th08_replay_check)

# Native observation adapter for the proposal policy (no native futures).
add_library(th08_online_native src/online_native.cpp)
target_link_libraries(th08_online_native PUBLIC th08_online_policy th08_native_headless)
th08_native_options(th08_online_native)

# Fork-owned true-RNG futures, wire projection and common-start following.
add_library(th08_shadow_native src/shadow_native.cpp src/shadow_process.cpp src/shadow_rollout.cpp)
target_include_directories(th08_shadow_native PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(th08_shadow_native PUBLIC th08_native_headless)
th08_native_options(th08_shadow_native)
add_executable(th08_shadow_follow tools/shadow_follow.cpp)
target_link_libraries(th08_shadow_follow PRIVATE th08_shadow_native OpenSSL::Crypto)
th08_native_options(th08_shadow_follow)

# Proposal generation over Native observations and the offline scoring CLI.
add_library(th08_shadow_proposals src/shadow_proposal.cpp src/shadow_recorded.cpp)
set_source_files_properties(src/shadow_recorded.cpp PROPERTIES COMPILE_OPTIONS "-fpermissive")
target_link_libraries(th08_shadow_proposals PUBLIC th08_shadow_native th08_online_native)
th08_native_options(th08_shadow_proposals)
add_executable(th08_shadow_style tools/shadow_style.cpp)
target_link_libraries(th08_shadow_style PRIVATE th08_shadow_proposals)
th08_native_options(th08_shadow_style)

# Complete-tape terminal-score refinement.
add_library(th08_score_refinement src/score_refine.cpp)
target_link_libraries(th08_score_refinement PUBLIC th08_shadow_native)
th08_native_options(th08_score_refinement)
add_executable(th08_score_refine tools/score_refine.cpp)
target_link_libraries(th08_score_refine PRIVATE th08_score_refinement)
th08_native_options(th08_score_refine)

# Online MPC (retained, not the current focus). th08_online is the forward-only
# current-memory controller, th08_online_live its 60 Hz worker, th08_shadow_online
# the live Wine true-RNG MPC client and th08_shadow_export its replay exporter.
# th08_plan is the earlier fork planner; the online regressions use its --verify
# mode as an independent fresh verifier.
add_executable(th08_plan tools/plan.cpp)
target_link_libraries(th08_plan PRIVATE th08_native_headless OpenSSL::Crypto)
target_include_directories(th08_plan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(th08_plan PRIVATE -fpermissive -Wno-write-strings -Wno-unknown-pragmas)
th08_native_options(th08_plan)
add_executable(th08_online tools/online.cpp)
target_link_libraries(th08_online PRIVATE th08_online_native OpenSSL::Crypto)
th08_native_options(th08_online)
add_executable(th08_online_live_cli tools/online_live.cpp)
set_target_properties(th08_online_live_cli PROPERTIES OUTPUT_NAME th08_online_live)
target_link_libraries(th08_online_live_cli PRIVATE th08_online_live th08_online_native OpenSSL::Crypto)
th08_native_options(th08_online_live_cli)
add_executable(th08_value_collect tools/value_collect.cpp)
target_link_libraries(th08_value_collect PRIVATE th08_online_native OpenSSL::Crypto)
th08_native_options(th08_value_collect)
add_executable(th08_shadow_online tools/shadow_online.cpp)
target_link_libraries(th08_shadow_online PRIVATE th08_shadow_proposals OpenSSL::Crypto)
th08_native_options(th08_shadow_online)
add_executable(th08_shadow_export tools/shadow_export.cpp)
target_link_libraries(th08_shadow_export PRIVATE th08_shadow_native OpenSSL::Crypto)
target_compile_options(th08_shadow_export PRIVATE -fpermissive)
th08_native_options(th08_shadow_export)

if(NOT BUILD_TESTING)
  return()
endif()

# Session-level contracts that need no game data.
foreach(test IN ITEMS surface_copy cancel_regions opcode_guards)
  add_executable(th08_headless_${test} tests/headless_${test}.cpp)
  target_link_libraries(th08_headless_${test} PRIVATE th08_native_headless)
  th08_native_options(th08_headless_${test})
  add_test(NAME headless_${test} COMMAND th08_headless_${test})
endforeach()

set(TH08_HEADLESS_DAT "" CACHE FILEPATH "Private th08.dat for the real-data regressions")
if(NOT TH08_HEADLESS_DAT)
  return()
endif()
if(NOT EXISTS "${TH08_HEADLESS_DAT}")
  message(FATAL_ERROR "TH08_HEADLESS_DAT does not exist")
endif()

# Real-DAT route transitions and laser-to-item conversion.
foreach(test IN ITEMS routes laser_items)
  add_executable(th08_headless_${test} tests/headless_${test}.cpp)
  target_include_directories(th08_headless_${test} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(th08_headless_${test} PRIVATE th08_native_headless)
  th08_native_options(th08_headless_${test})
  add_test(NAME headless_${test} COMMAND th08_headless_${test} "${TH08_HEADLESS_DAT}")
endforeach()

add_test(NAME headless_score_audit COMMAND ${CMAKE_COMMAND}
  -DCHECKER=$<TARGET_FILE:th08_replay_check>
  -DDAT=${TH08_HEADLESS_DAT}
  -DREPLAY=${CMAKE_CURRENT_SOURCE_DIR}/reports/native/replays/online/shadow-recording-v3--story-4a-finalb-d3-s1-seed40755.rpy
  -DWORK=${CMAKE_CURRENT_BINARY_DIR}/score-audit-regression
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/score_audit_real_data.cmake)
set_tests_properties(headless_score_audit PROPERTIES TIMEOUT 75)

add_executable(th08_shadow_rollout_test tests/shadow_rollout_real_data.cpp)
target_link_libraries(th08_shadow_rollout_test PRIVATE th08_shadow_proposals)
th08_native_options(th08_shadow_rollout_test)
add_test(NAME headless_shadow_follow COMMAND ${CMAKE_COMMAND}
  -DFOLLOWER=$<TARGET_FILE:th08_shadow_follow>
  -DROLLOUT=$<TARGET_FILE:th08_shadow_rollout_test>
  -DID179_ACTIONS=${CMAKE_CURRENT_SOURCE_DIR}/tests/data/id179-stage6b-d0-s0.actions
  -DSTORY_ACTIONS=${CMAKE_CURRENT_SOURCE_DIR}/reports/native/replays/online/online-story-4a-v1--s0.actions
  -DDAT=${TH08_HEADLESS_DAT}
  -DWORK=${CMAKE_CURRENT_BINARY_DIR}/shadow-follow-regression
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/shadow_follow_real_data.cmake)
set_tests_properties(headless_shadow_follow PROPERTIES TIMEOUT 100)

add_executable(th08_shadow_capture_goal_test tests/shadow_capture_goal_real_data.cpp)
target_link_libraries(th08_shadow_capture_goal_test PRIVATE th08_shadow_native)
th08_native_options(th08_shadow_capture_goal_test)
add_test(NAME headless_shadow_capture_goal COMMAND th08_shadow_capture_goal_test
  "${TH08_HEADLESS_DAT}")
add_test(NAME headless_shadow_capture_handoff COMMAND th08_shadow_capture_goal_test
  "${TH08_HEADLESS_DAT}"
  "${CMAKE_CURRENT_SOURCE_DIR}/reports/native/replays/magic/lunatic/story-native-nmnb-4a-finalb-3903603650-seed40755.rpy")
set_tests_properties(headless_shadow_capture_goal headless_shadow_capture_handoff
  PROPERTIES TIMEOUT 45)

# Complete offline scoring fixture, then refinement and bomb-permission
# variants that reuse its incumbent tape. These are correctness budgets,
# independent of production search budgets.
add_test(NAME headless_shadow_style COMMAND ${CMAKE_COMMAND}
  -DSOLVER=$<TARGET_FILE:th08_shadow_style>
  -DONLINE=$<TARGET_FILE:th08_shadow_online>
  -DFOLLOWER=$<TARGET_FILE:th08_shadow_follow>
  -DCHECKER=$<TARGET_FILE:th08_replay_check>
  -DDAT=${TH08_HEADLESS_DAT}
  -DWORK=${CMAKE_CURRENT_BINARY_DIR}/shadow-style-regression
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/shadow_style_real_data.cmake)
set_tests_properties(headless_shadow_style PROPERTIES TIMEOUT 900
  FIXTURES_SETUP native_score_incumbent)
add_test(NAME headless_score_refine COMMAND ${CMAKE_COMMAND}
  -DREFINER=$<TARGET_FILE:th08_score_refine>
  -DFOLLOWER=$<TARGET_FILE:th08_shadow_follow>
  -DCHECKER=$<TARGET_FILE:th08_replay_check>
  -DDAT=${TH08_HEADLESS_DAT}
  -DINCUMBENT=${CMAKE_CURRENT_BINARY_DIR}/shadow-style-regression
  -DWORK=${CMAKE_CURRENT_BINARY_DIR}/score-refine-regression
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/score_refine_real_data.cmake)
set_tests_properties(headless_score_refine PROPERTIES TIMEOUT 120
  FIXTURES_REQUIRED native_score_incumbent)
add_test(NAME headless_bomb_scoring COMMAND ${CMAKE_COMMAND}
  -DSOLVER=$<TARGET_FILE:th08_shadow_style>
  -DREFINER=$<TARGET_FILE:th08_score_refine>
  -DFOLLOWER=$<TARGET_FILE:th08_shadow_follow>
  -DCHECKER=$<TARGET_FILE:th08_replay_check>
  -DDAT=${TH08_HEADLESS_DAT}
  -DINCUMBENT=${CMAKE_CURRENT_BINARY_DIR}/shadow-style-regression
  -DWORK=${CMAKE_CURRENT_BINARY_DIR}/bomb-scoring-regression
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/bomb_scoring_real_data.cmake)
set_tests_properties(headless_bomb_scoring PROPERTIES TIMEOUT 900
  FIXTURES_REQUIRED native_score_incumbent)

# Retained online MPC regressions.
add_test(NAME headless_online_real_data COMMAND ${CMAKE_COMMAND}
  -DEXECUTABLE=$<TARGET_FILE:th08_online>
  -DVERIFIER=$<TARGET_FILE:th08_plan>
  -DCHECKER=$<TARGET_FILE:th08_replay_check>
  -DDAT=${TH08_HEADLESS_DAT}
  -DWORK=${CMAKE_CURRENT_BINARY_DIR}/online-regression
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/online_real_data.cmake)
set_tests_properties(headless_online_real_data PROPERTIES TIMEOUT 1100)
add_test(NAME headless_online_motion_real_data COMMAND ${CMAKE_COMMAND}
  -DEXECUTABLE=$<TARGET_FILE:th08_online>
  -DVERIFIER=$<TARGET_FILE:th08_plan>
  -DDAT=${TH08_HEADLESS_DAT}
  -DWORK=${CMAKE_CURRENT_BINARY_DIR}/online-motion-regression
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/online_motion_real_data.cmake)
set_tests_properties(headless_online_motion_real_data PROPERTIES TIMEOUT 400)
add_test(NAME headless_value_collection_real_data COMMAND ${CMAKE_COMMAND}
  -DCOLLECTOR=$<TARGET_FILE:th08_value_collect>
  -DDAT=${TH08_HEADLESS_DAT}
  -DWORK=${CMAKE_CURRENT_BINARY_DIR}/value-collection-regression
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/value_collection_real_data.cmake)
set_tests_properties(headless_value_collection_real_data PROPERTIES TIMEOUT 120)
add_test(NAME headless_online_live_real_data COMMAND ${CMAKE_COMMAND}
  -DEXECUTABLE=$<TARGET_FILE:th08_online_live_cli>
  -DDAT=${TH08_HEADLESS_DAT}
  -DMODEL=${CMAKE_CURRENT_SOURCE_DIR}/models/branch-value-v1/weights.tsv
  -DWORK=${CMAKE_CURRENT_BINARY_DIR}/online-live-regression
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/online_live_real_data.cmake)
set_tests_properties(headless_online_live_real_data PROPERTIES TIMEOUT 60)
# Optional retained live Wine journals (Stage 4B seeds 42386 and 56891).
set(TH08_SHADOW_FAILURE_TRACE "" CACHE FILEPATH "Optional retained seed42386 Stage4B journal")
if(TH08_SHADOW_FAILURE_TRACE)
  add_test(NAME headless_shadow_retained_4b COMMAND th08_shadow_rollout_test
    "${TH08_HEADLESS_DAT}" "${TH08_SHADOW_FAILURE_TRACE}" retained4b)
endif()
set(TH08_SHADOW_PULSE_TRACE "" CACHE FILEPATH "Optional retained seed56891 Stage4B journal")
if(TH08_SHADOW_PULSE_TRACE)
  add_test(NAME headless_shadow_retained_pulse COMMAND th08_shadow_rollout_test
    "${TH08_HEADLESS_DAT}" "${TH08_SHADOW_PULSE_TRACE}" retainedpulse)
endif()
