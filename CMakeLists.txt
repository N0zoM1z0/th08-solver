cmake_minimum_required(VERSION 3.16)
project(th08_solver VERSION 0.1.0 LANGUAGES CXX)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)
find_package(Threads REQUIRED)
include(CTest)
include(cmake/ReplaySuite.cmake)
include(cmake/PlannerTests.cmake)
option(TH08_WINE_SHADOW "Build the pinned 32-bit Wine observer and owned launcher" OFF)
if(TH08_WINE_SHADOW)
  include(cmake/WineShadow.cmake)
endif()
add_library(th08_online_policy src/online.cpp src/online_model.cpp src/online_value.cpp)
target_compile_features(th08_online_policy PUBLIC cxx_std_17)
target_include_directories(th08_online_policy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TH08_ONLINE_ARCH "" CACHE STRING "Optional online-policy CPU target: native or x86-64-v3")
set_property(CACHE TH08_ONLINE_ARCH PROPERTY STRINGS "" native x86-64-v3)
if(NOT TH08_ONLINE_ARCH STREQUAL "")
  if(MSVC OR NOT TH08_ONLINE_ARCH MATCHES "^(native|x86-64-v3)$")
    message(FATAL_ERROR "TH08_ONLINE_ARCH requires a supported GCC/Clang native or x86-64-v3 target")
  endif()
  include(CheckCXXCompilerFlag)
  string(MAKE_C_IDENTIFIER "TH08_ARCH_${TH08_ONLINE_ARCH}" arch_check)
  check_cxx_compiler_flag("-march=${TH08_ONLINE_ARCH}" ${arch_check})
  if(NOT ${arch_check})
    message(FATAL_ERROR "Compiler does not support -march=${TH08_ONLINE_ARCH}")
  endif()
  target_compile_options(th08_online_policy PRIVATE "-march=${TH08_ONLINE_ARCH}")
  target_compile_definitions(th08_online_policy PRIVATE TH08_ONLINE_ARCH_NAME="${TH08_ONLINE_ARCH}")
endif()
if(MSVC)
  target_compile_options(th08_online_policy PRIVATE /fp:strict)
else()
  target_compile_options(th08_online_policy PRIVATE -ffp-contract=off)
endif()
add_executable(th08_planner_report tools/planner_report.cpp)
target_compile_features(th08_planner_report PRIVATE cxx_std_17)
target_include_directories(th08_planner_report PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(th08_planner_report PRIVATE OpenSSL::Crypto)
add_executable(th08_plan_matrix tools/plan_matrix.cpp)
target_compile_features(th08_plan_matrix PRIVATE cxx_std_17)
target_include_directories(th08_plan_matrix PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(th08_spell_bank tools/spell_bank.cpp)
target_compile_features(th08_spell_bank PRIVATE cxx_std_17)
target_link_libraries(th08_spell_bank PRIVATE OpenSSL::Crypto)
add_library(th08_online_live src/online_live.cpp)
target_link_libraries(th08_online_live PUBLIC th08_online_policy Threads::Threads)
target_compile_features(th08_online_live PUBLIC cxx_std_17)
if(MSVC)
  target_compile_options(th08_online_live PRIVATE /fp:strict)
else()
  target_compile_options(th08_online_live PRIVATE -ffp-contract=off)
endif()
add_executable(th08_value_train tools/value_train.cpp)
target_link_libraries(th08_value_train PRIVATE th08_online_policy OpenSSL::Crypto)
if(MSVC)
  target_compile_options(th08_value_train PRIVATE /fp:strict)
else()
  target_compile_options(th08_value_train PRIVATE -ffp-contract=off)
endif()
if(BUILD_TESTING)
  add_executable(shadow_protocol_tests tests/shadow_protocol_tests.cpp)
  target_link_libraries(shadow_protocol_tests PRIVATE th08_geometry)
  target_include_directories(shadow_protocol_tests PRIVATE include)
  target_compile_features(shadow_protocol_tests PRIVATE cxx_std_17)
  add_test(NAME shadow_protocol_contract COMMAND shadow_protocol_tests)
  add_executable(online_live_tests tests/online_live_tests.cpp)
  target_link_libraries(online_live_tests PRIVATE th08_online_live)
  add_test(NAME online_live_contract COMMAND online_live_tests)
  add_executable(value_model_tests tests/value_model_tests.cpp)
  target_link_libraries(value_model_tests PRIVATE th08_online_policy)
  add_test(NAME value_model_contract COMMAND value_model_tests
    ${CMAKE_CURRENT_BINARY_DIR}/value-model-contract)
  add_test(NAME value_training_contract COMMAND ${CMAKE_COMMAND}
    -DTRAINER=$<TARGET_FILE:th08_value_train>
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/value-training-contract
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/value_training_tests.cmake)
  add_test(NAME spell_bank_contract COMMAND ${CMAKE_COMMAND}
    -DANALYZER=$<TARGET_FILE:th08_spell_bank>
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/spell-bank-contract
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/spell_bank_tests.cmake)
  add_test(NAME planner_report_contract COMMAND ${CMAKE_COMMAND}
    -DREPORT=$<TARGET_FILE:th08_planner_report>
    -DMATRIX=$<TARGET_FILE:th08_plan_matrix>
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/planner-report-contract
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/planner_report_tests.cmake)
endif()
option(TH08_HEADLESS "Build the imported native TH08 runtime without a display" OFF)
if(TH08_HEADLESS)
  include(cmake/Headless.cmake)
endif()
option(TH08_SANITIZERS "Enable address and undefined-behavior sanitizers" OFF)
add_library(th08_geometry INTERFACE)
target_include_directories(th08_geometry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(th08_geometry INTERFACE cxx_std_17)
if(MSVC)
  target_compile_options(th08_geometry INTERFACE /W4 /fp:strict)
else()
  target_compile_options(th08_geometry INTERFACE -Wall -Wextra -Wpedantic -ffp-contract=off)
  if(TH08_SANITIZERS)
    target_compile_options(th08_geometry INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(th08_geometry INTERFACE -fsanitize=address,undefined)
  endif()
endif()
add_library(th08_resources
  src/resources.cpp
  src/schema.cpp
  src/formats.cpp
  src/emitter.cpp
  src/animation.cpp
  src/animation_control.cpp
  src/world_motion.cpp
  src/practice_entry.cpp
  src/practice_camera.cpp
  src/effect_pool.cpp
  src/effect_animation.cpp
  src/timeline.cpp)
target_link_libraries(th08_resources PUBLIC th08_geometry OpenSSL::Crypto)
add_library(th08_scenarios src/scenario.cpp)
target_link_libraries(th08_scenarios PUBLIC th08_geometry)
add_executable(th08_scenario_cases tools/scenario_cases.cpp)
target_link_libraries(th08_scenario_cases PRIVATE th08_scenarios)
add_library(th08_spell_scenarios src/spell_scenario.cpp)
target_link_libraries(th08_spell_scenarios PUBLIC th08_resources)
add_executable(th08_spell_cases tools/spell_cases.cpp)
target_link_libraries(th08_spell_cases PRIVATE th08_spell_scenarios)
add_executable(th08_audit tools/audit.cpp tools/resource_reports.cpp)
target_link_libraries(th08_audit PRIVATE th08_resources)
add_executable(th08_slices tools/slices.cpp)
target_link_libraries(th08_slices PRIVATE th08_resources)
add_executable(th08_motion_cases tools/motion_cases.cpp)
target_link_libraries(th08_motion_cases PRIVATE th08_resources)
add_executable(th08_animation_cases tools/animation_cases.cpp)
target_link_libraries(th08_animation_cases PRIVATE th08_resources)
add_executable(th08_timeline_cases tools/timeline_cases.cpp)
target_link_libraries(th08_timeline_cases PRIVATE th08_resources)
add_executable(th08_first_spell tools/first_spell.cpp)
target_link_libraries(th08_first_spell PRIVATE th08_resources)
set(TH08_REFERENCE_SOURCE "" CACHE PATH "Optional pinned reconstruction checkout for source oracle")
if(TH08_REFERENCE_SOURCE)
  add_library(th08_source_support STATIC tools/source_probe_support.cpp)
  target_link_libraries(th08_source_support PUBLIC th08_resources)
  add_executable(effect_pool_source_probe tools/effect_pool_source_probe.cpp tools/camera_particle_source_probe.cpp)
  target_link_libraries(effect_pool_source_probe PRIVATE th08_source_support)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/source_effect_pool.cpp
    COMMAND effect_pool_source_probe ${TH08_REFERENCE_SOURCE} ${CMAKE_CURRENT_BINARY_DIR}/source_effect_pool.cpp
    DEPENDS effect_pool_source_probe ${TH08_REFERENCE_SOURCE}/src/EffectManager.cpp
      ${TH08_REFERENCE_SOURCE}/src/AnmManager.cpp ${TH08_REFERENCE_SOURCE}/src/AsciiManager.cpp
      ${TH08_REFERENCE_SOURCE}/src/Global.cpp
      ${TH08_REFERENCE_SOURCE}/src/EclManager.hpp ${TH08_REFERENCE_SOURCE}/src/EnemyManager.cpp
      ${TH08_REFERENCE_SOURCE}/src/EnemyManager.hpp ${TH08_REFERENCE_SOURCE}/src/ZunMath.hpp
      ${TH08_REFERENCE_SOURCE}/src/Background.cpp ${TH08_REFERENCE_SOURCE}/src/Background.hpp
      ${TH08_REFERENCE_SOURCE}/src/ZunColor.hpp ${TH08_REFERENCE_SOURCE}/src/Supervisor.cpp
      ${TH08_REFERENCE_SOURCE}/src/Global.hpp
      ${TH08_REFERENCE_SOURCE}/src/modern/linux/d3dx8_compat.cpp
      ${TH08_REFERENCE_SOURCE}/src/modern/linux/include/d3dx8.h
    VERBATIM)
  add_executable(source_effect_pool ${CMAKE_CURRENT_BINARY_DIR}/source_effect_pool.cpp)
  target_link_libraries(source_effect_pool PRIVATE th08_resources)
  target_include_directories(source_effect_pool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
  if(NOT MSVC)
    target_compile_options(source_effect_pool PRIVATE -Wno-unused-parameter -Wno-unknown-pragmas -fno-strict-aliasing)
  endif()
  add_executable(th08_source_probe
    tools/source_probe.cpp
    tools/ecl_source_probe.cpp
    tools/transform_source_probe.cpp
    tools/animation_source_probe.cpp
    tools/enemy_motion_source_probe.cpp
    tools/spawn_source_probe.cpp
    tools/world_motion_source_probe.cpp
    tools/timeline_source_probe.cpp
    tools/camera_particle_source_probe.cpp)
  target_link_libraries(th08_source_probe PRIVATE th08_source_support)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/source_oracle.cpp
    COMMAND th08_source_probe ${TH08_REFERENCE_SOURCE} ${CMAKE_CURRENT_BINARY_DIR}/source_oracle.cpp
    DEPENDS th08_source_probe ${TH08_REFERENCE_SOURCE}/src/Player.cpp ${TH08_REFERENCE_SOURCE}/src/Global.cpp ${TH08_REFERENCE_SOURCE}/src/Global.hpp ${TH08_REFERENCE_SOURCE}/src/BulletManager.cpp ${TH08_REFERENCE_SOURCE}/src/ZunMath.hpp ${TH08_REFERENCE_SOURCE}/src/Supervisor.cpp ${TH08_REFERENCE_SOURCE}/src/Supervisor.hpp
      ${TH08_REFERENCE_SOURCE}/src/EclManager.hpp ${TH08_REFERENCE_SOURCE}/src/EclOperandsInt.cpp ${TH08_REFERENCE_SOURCE}/src/EclOperandsFloat.cpp ${TH08_REFERENCE_SOURCE}/src/EclRunLow.inl
      ${TH08_REFERENCE_SOURCE}/src/PlayerBomb.cpp ${TH08_REFERENCE_SOURCE}/src/Background.cpp
      ${TH08_REFERENCE_SOURCE}/src/BulletManager.hpp
      ${TH08_REFERENCE_SOURCE}/src/GameManager.cpp
      ${TH08_REFERENCE_SOURCE}/src/AnmManager.cpp ${TH08_REFERENCE_SOURCE}/src/AnmManager.hpp
      ${TH08_REFERENCE_SOURCE}/src/AsciiManager.cpp
      ${TH08_REFERENCE_SOURCE}/src/EnemyManager.cpp ${TH08_REFERENCE_SOURCE}/src/EnemyManager.hpp
      ${TH08_REFERENCE_SOURCE}/src/EnemyManagerUpdate.cpp ${TH08_REFERENCE_SOURCE}/src/EclHelpers.cpp
      ${TH08_REFERENCE_SOURCE}/src/EclRun.cpp ${TH08_REFERENCE_SOURCE}/src/modern/linux/d3dx8_compat.cpp
      ${TH08_REFERENCE_SOURCE}/src/EclDependencies.cpp
      ${TH08_REFERENCE_SOURCE}/src/EnemyTimeline.cpp ${TH08_REFERENCE_SOURCE}/src/Gui.cpp
      ${TH08_REFERENCE_SOURCE}/src/AsciiManagerScale.cpp
      ${TH08_REFERENCE_SOURCE}/src/EffectManager.cpp ${TH08_REFERENCE_SOURCE}/src/Background.hpp
      ${TH08_REFERENCE_SOURCE}/src/ZunColor.hpp ${TH08_REFERENCE_SOURCE}/src/modern/linux/include/d3dx8.h
    VERBATIM)
  add_executable(source_oracle ${CMAKE_CURRENT_BINARY_DIR}/source_oracle.cpp)
  target_link_libraries(source_oracle PRIVATE th08_resources)
  target_include_directories(source_oracle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
  if(NOT MSVC)
    # Unchanged pinned bodies include an unused ZunTimer operator with no return,
    # intentional switch fallthrough, and retail bitcast-through-pointer expressions.
    target_compile_options(source_oracle PRIVATE -Wno-unused-parameter -Wno-unused-label -Wno-implicit-fallthrough -Wno-return-type -fno-strict-aliasing)
  endif()
  # Explicit targets replace ad-hoc drivers formerly kept in ignored build trees.
  # Reuse the hash-checked generator and unchanged test headers, not generated TUs.
  foreach(component IN ITEMS enemy_motion world_motion camera_particle spawn)
    set(component_source ${CMAKE_CURRENT_BINARY_DIR}/source_${component}.cpp)
    add_custom_command(OUTPUT ${component_source}
      COMMAND th08_source_probe ${TH08_REFERENCE_SOURCE} ${component_source} ${component}
      DEPENDS th08_source_probe ${CMAKE_CURRENT_BINARY_DIR}/source_oracle.cpp
      VERBATIM)
    add_executable(source_${component} EXCLUDE_FROM_ALL ${component_source})
    target_link_libraries(source_${component} PRIVATE th08_resources)
    target_include_directories(source_${component} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    if(NOT MSVC)
      target_compile_options(source_${component} PRIVATE -Wno-unused-parameter -Wno-unused-label -Wno-implicit-fallthrough -Wno-return-type -fno-strict-aliasing)
    endif()
  endforeach()
endif()
if(BUILD_TESTING)
  add_test(NAME model_artifacts COMMAND ${CMAKE_COMMAND}
    -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/model_artifacts.cmake)
  add_executable(geometry_tests tests/geometry_tests.cpp)
  target_link_libraries(geometry_tests PRIVATE th08_geometry)
  add_test(NAME geometry COMMAND geometry_tests)
  add_executable(scenario_tests tests/scenario_tests.cpp)
  target_link_libraries(scenario_tests PRIVATE th08_scenarios)
  add_test(NAME scenario COMMAND scenario_tests)
  add_executable(planner_tests tests/planner_tests.cpp)
  target_link_libraries(planner_tests PRIVATE th08_geometry)
  add_test(NAME planner COMMAND planner_tests)
  add_executable(native_policy_tests tests/native_policy_tests.cpp)
  target_link_libraries(native_policy_tests PRIVATE th08_geometry)
  add_test(NAME native_policy COMMAND native_policy_tests)
  add_executable(resource_tests tests/resource_tests.cpp)
  target_link_libraries(resource_tests PRIVATE th08_resources)
  add_test(NAME resources COMMAND resource_tests)
  add_executable(formats_tests tests/formats_tests.cpp)
  target_link_libraries(formats_tests PRIVATE th08_resources)
  add_test(NAME formats COMMAND formats_tests)
  add_executable(lzss_decode_tests tests/lzss_decode_tests.cpp)
  target_link_libraries(lzss_decode_tests PRIVATE th08_geometry)
  target_include_directories(lzss_decode_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/th08/src)
  add_test(NAME lzss_decode COMMAND lzss_decode_tests)
  add_executable(emitter_tests tests/emitter_tests.cpp)
  target_link_libraries(emitter_tests PRIVATE th08_resources)
  add_test(NAME emitter COMMAND emitter_tests)
  add_executable(context_storage_tests tests/context_storage_tests.cpp)
  target_link_libraries(context_storage_tests PRIVATE th08_resources)
  add_test(NAME context_storage COMMAND context_storage_tests)
  add_executable(enemy_motion_tests tests/enemy_motion_tests.cpp)
  target_link_libraries(enemy_motion_tests PRIVATE th08_geometry)
  add_test(NAME enemy_motion COMMAND enemy_motion_tests)
  add_executable(world_motion_tests tests/world_motion_tests.cpp)
  target_link_libraries(world_motion_tests PRIVATE th08_resources)
  add_test(NAME world_motion COMMAND world_motion_tests)
  add_executable(timeline_tests tests/timeline_tests.cpp)
  target_link_libraries(timeline_tests PRIVATE th08_resources)
  add_test(NAME timeline COMMAND timeline_tests)
  add_executable(practice_entry_tests tests/practice_entry_tests.cpp)
  target_link_libraries(practice_entry_tests PRIVATE th08_resources)
  add_test(NAME practice_entry COMMAND practice_entry_tests)
  add_executable(practice_camera_tests tests/practice_camera_tests.cpp)
  target_link_libraries(practice_camera_tests PRIVATE th08_resources)
  add_test(NAME practice_camera COMMAND practice_camera_tests)
  add_executable(effect_pool_tests tests/effect_pool_tests.cpp)
  target_link_libraries(effect_pool_tests PRIVATE th08_resources)
  add_test(NAME effect_pool COMMAND effect_pool_tests)
  add_executable(effect_entry_tests tests/effect_entry_tests.cpp)
  target_link_libraries(effect_entry_tests PRIVATE th08_resources)
  add_test(NAME effect_entry COMMAND effect_entry_tests)
  add_executable(camera_particle_tests tests/camera_particle_tests.cpp)
  target_link_libraries(camera_particle_tests PRIVATE th08_geometry)
  add_test(NAME camera_particle COMMAND camera_particle_tests)
  add_executable(kinematics_tests tests/kinematics_tests.cpp)
  target_link_libraries(kinematics_tests PRIVATE th08_geometry)
  add_test(NAME kinematics COMMAND kinematics_tests)
  add_executable(animation_tests tests/animation_tests.cpp)
  target_link_libraries(animation_tests PRIVATE th08_resources)
  add_test(NAME animation COMMAND animation_tests)
  add_executable(animation_control_tests tests/animation_control_tests.cpp)
  target_link_libraries(animation_control_tests PRIVATE th08_resources)
  add_test(NAME animation_control COMMAND animation_control_tests)
  add_executable(bullet_motion_tests tests/bullet_motion_tests.cpp)
  target_link_libraries(bullet_motion_tests PRIVATE th08_geometry)
  add_test(NAME bullet_motion COMMAND bullet_motion_tests)
  add_executable(acceleration_tests tests/acceleration_tests.cpp)
  target_link_libraries(acceleration_tests PRIVATE th08_geometry)
  add_test(NAME acceleration COMMAND acceleration_tests)
  add_executable(transform_program_tests tests/transform_program_tests.cpp)
  target_link_libraries(transform_program_tests PRIVATE th08_geometry)
  add_test(NAME transform_program COMMAND transform_program_tests)
  add_executable(bullet_slots_tests tests/bullet_slots_tests.cpp)
  target_link_libraries(bullet_slots_tests PRIVATE th08_geometry)
  add_test(NAME bullet_slots COMMAND bullet_slots_tests)
  add_executable(laser_motion_tests tests/laser_motion_tests.cpp)
  target_link_libraries(laser_motion_tests PRIVATE th08_geometry)
  add_test(NAME laser_motion COMMAND laser_motion_tests)
  add_executable(rng_tests tests/rng_tests.cpp)
  target_link_libraries(rng_tests PRIVATE th08_geometry)
  add_test(NAME rng COMMAND rng_tests)
  if(TARGET source_oracle)
    add_test(NAME source_oracle COMMAND source_oracle)
    add_test(NAME source_effect_pool COMMAND source_effect_pool)
  endif()
  add_executable(documentation_tests tests/documentation_tests.cpp)
  target_link_libraries(documentation_tests PRIVATE th08_geometry)
  get_property(th08_core_tests DIRECTORY PROPERTY TESTS)
  list(REMOVE_ITEM th08_core_tests source_oracle source_effect_pool headless_repair_protocol headless_report_contract)
  list(FILTER th08_core_tests EXCLUDE REGEX "^headless_")
  list(LENGTH th08_core_tests th08_core_test_count)
  math(EXPR th08_core_test_count "${th08_core_test_count} + 1")
  add_test(NAME documentation COMMAND documentation_tests
    ${CMAKE_CURRENT_SOURCE_DIR} ${th08_core_test_count})
  if(TH08_HEADLESS AND TH08_HEADLESS_DAT)
    if(NOT EXISTS "${TH08_HEADLESS_DAT}")
      message(FATAL_ERROR "TH08_HEADLESS_DAT does not exist")
    endif()
    add_test(NAME headless_shadow_follow_real_data COMMAND ${CMAKE_COMMAND}
      -DFOLLOWER=$<TARGET_FILE:th08_shadow_follow>
      -DROLLOUT=$<TARGET_FILE:th08_shadow_rollout_test>
      -DPRODUCER=$<TARGET_FILE:th08_headless>
      -DSTORY_ACTIONS=${CMAKE_CURRENT_SOURCE_DIR}/reports/native/replays/online-story-4a-v1--s0.actions
      -DDAT=${TH08_HEADLESS_DAT}
      -DWORK=${CMAKE_CURRENT_BINARY_DIR}/shadow-follow-regression
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/shadow_follow_real_data.cmake)
    set_tests_properties(headless_shadow_follow_real_data PROPERTIES TIMEOUT 100)
    set(TH08_SHADOW_FAILURE_TRACE "" CACHE FILEPATH "Optional retained seed42386 Stage4B journal")
    if(TH08_SHADOW_FAILURE_TRACE)
      add_test(NAME headless_shadow_retained_4b COMMAND th08_shadow_rollout_test
        "${TH08_HEADLESS_DAT}" "${TH08_SHADOW_FAILURE_TRACE}" retained4b)
      set_tests_properties(headless_shadow_retained_4b PROPERTIES TIMEOUT 60)
    endif()
    set(TH08_SHADOW_PULSE_TRACE "" CACHE FILEPATH "Optional retained seed56891 Stage4B journal")
    if(TH08_SHADOW_PULSE_TRACE)
      add_test(NAME headless_shadow_retained_pulse COMMAND th08_shadow_rollout_test
        "${TH08_HEADLESS_DAT}" "${TH08_SHADOW_PULSE_TRACE}" retainedpulse)
      set_tests_properties(headless_shadow_retained_pulse PROPERTIES TIMEOUT 60)
    endif()
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
    add_test(NAME headless_real_data COMMAND ${CMAKE_COMMAND}
      -DEXECUTABLE=$<TARGET_FILE:th08_headless>
      -DPROBE_EXECUTABLE=$<TARGET_FILE:th08_headless_probe>
      -DREPAIR_EXECUTABLE=$<TARGET_FILE:th08_headless_repair>
      -DLASER_ITEMS_EXECUTABLE=$<TARGET_FILE:th08_headless_laser_items>
      -DBULLET_BOUNDS_EXECUTABLE=$<TARGET_FILE:th08_headless_bullet_bounds>
      -DBODY_FORECAST_EXECUTABLE=$<TARGET_FILE:th08_headless_body_forecast>
      -DDAT=${TH08_HEADLESS_DAT}
      -DWORK=${CMAKE_CURRENT_BINARY_DIR}/headless-regression
      -DCOMPARE_EXECUTABLE=${TH08_HEADLESS_COMPARE_EXECUTABLE}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/headless_real_data.cmake)
    # Complete scenes plus the bounded ID203 search/replay run in this aggregate.
    # This is only the aggregate test process timeout, not a solver/frame budget.
    set_tests_properties(headless_real_data PROPERTIES TIMEOUT 1200)
    add_test(NAME headless_planner_prefix COMMAND ${CMAKE_COMMAND}
      -DEXECUTABLE=$<TARGET_FILE:th08_plan>
      -DDAT=${TH08_HEADLESS_DAT}
      -DWORK=${CMAKE_CURRENT_BINARY_DIR}/planner-prefix-regression
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/planner_prefix_real_data.cmake)
    set_tests_properties(headless_planner_prefix PROPERTIES TIMEOUT 300)
    add_test(NAME headless_replay_export_contract COMMAND ${CMAKE_COMMAND}
      -DEXECUTABLE=$<TARGET_FILE:th08_plan>
      -DSHADOW_EXPORTER=$<TARGET_FILE:th08_shadow_export>
      -DCHECKER=$<TARGET_FILE:th08_replay_check>
      -DTARGETER=$<TARGET_FILE:th08_replay_target>
      -DDAT=${TH08_HEADLESS_DAT}
      -DACTIONS=${CMAKE_CURRENT_SOURCE_DIR}/reports/native/replays/practice-v1--stage1-d3-s0.actions
      -DLEGACY_REPLAY=${CMAKE_CURRENT_SOURCE_DIR}/reports/native/replays/practice-v1--stage1-d3-s0.rpy
      -DWORK=${CMAKE_CURRENT_BINARY_DIR}/replay-export-regression
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/replay_export_real_data.cmake)
    set_tests_properties(headless_replay_export_contract PROPERTIES TIMEOUT 120)
  endif()
endif()
add_executable(geometry_bench benchmarks/geometry_bench.cpp)
target_link_libraries(geometry_bench PRIVATE th08_geometry)
add_executable(planner_bench benchmarks/planner_bench.cpp)
target_link_libraries(planner_bench PRIVATE th08_geometry)
add_executable(bullet_slots_bench benchmarks/bullet_slots_bench.cpp)
target_link_libraries(bullet_slots_bench PRIVATE th08_geometry)
