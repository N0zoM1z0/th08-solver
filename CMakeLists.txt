cmake_minimum_required(VERSION 3.16)
project(th08_solver VERSION 0.2.0 LANGUAGES CXX)

find_package(OpenSSL REQUIRED COMPONENTS Crypto)
find_package(Threads REQUIRED)
include(CTest)

option(TH08_HEADLESS "Build the imported native TH08 runtime and the Native scoring tools" OFF)
option(TH08_WINE_SHADOW "Build the pinned 32-bit Wine seed observer and owned launcher" OFF)
option(TH08_SANITIZERS "Enable address and undefined-behavior sanitizers for asset-free tests" OFF)

# Common options for project-owned code that does not link the imported game.
# Float contraction stays disabled everywhere: proposal geometry must not drift
# from the native float32 kernels it imitates.
add_library(th08_strict INTERFACE)
target_include_directories(th08_strict INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(th08_strict INTERFACE cxx_std_17)
if(MSVC)
  target_compile_options(th08_strict INTERFACE /W4 /fp:strict)
else()
  target_compile_options(th08_strict INTERFACE -Wall -Wextra -Wpedantic -ffp-contract=off)
  if(TH08_SANITIZERS)
    target_compile_options(th08_strict INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(th08_strict INTERFACE -fsanitize=address,undefined)
  endif()
endif()

# Current-memory proposal policy and immutable stage/spell strategies. This
# library owns no native world; the Native adapter copies observations into it.
add_library(th08_online_policy src/online.cpp src/online_model.cpp src/online_value.cpp
  src/phase_strategy.cpp)
target_compile_features(th08_online_policy PUBLIC cxx_std_17)
target_include_directories(th08_online_policy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(MSVC)
  target_compile_options(th08_online_policy PRIVATE /fp:strict)
else()
  target_compile_options(th08_online_policy PRIVATE -ffp-contract=off)
endif()

# Bounded selection over declared completed-segment evidence.
add_library(th08_phase_search src/phase_archive.cpp)
target_compile_features(th08_phase_search PUBLIC cxx_std_17)
target_include_directories(th08_phase_search PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(th08_phase_frontier tools/phase_frontier.cpp)
target_link_libraries(th08_phase_frontier PRIVATE th08_phase_search)

# Compare fresh native replay captures with a pinned upstream retail corpus.
# Only the corpus gate itself needs the native checker and game data.
if(UNIX)
  add_executable(th08_replay_suite tools/replay_suite.cpp)
  target_link_libraries(th08_replay_suite PRIVATE th08_strict OpenSSL::Crypto)
endif()

if(BUILD_TESTING)
  add_executable(phase_strategy_tests tests/phase_strategy_tests.cpp)
  target_link_libraries(phase_strategy_tests PRIVATE th08_online_policy th08_phase_search)
  add_test(NAME phase_strategy_archive_contract COMMAND phase_strategy_tests)
  add_test(NAME phase_frontier_cli COMMAND ${CMAKE_COMMAND}
    -DSELECTOR=$<TARGET_FILE:th08_phase_frontier>
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/phase-frontier-regression
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/phase_frontier_cli.cmake)

  add_executable(online_policy_tests tests/online_policy_tests.cpp)
  target_link_libraries(online_policy_tests PRIVATE th08_online_policy)
  add_test(NAME online_policy_contract COMMAND online_policy_tests
    ${CMAKE_CURRENT_BINARY_DIR}/online-policy-test)

  foreach(test IN ITEMS geometry kinematics bullet_motion direct_laser_policy)
    add_executable(${test}_tests tests/${test}_tests.cpp)
    target_link_libraries(${test}_tests PRIVATE th08_strict)
    add_test(NAME ${test} COMMAND ${test}_tests)
  endforeach()

  add_executable(shadow_protocol_tests tests/shadow_protocol_tests.cpp)
  target_link_libraries(shadow_protocol_tests PRIVATE th08_strict)
  add_test(NAME shadow_protocol_contract COMMAND shadow_protocol_tests)

  add_executable(lzss_decode_tests tests/lzss_decode_tests.cpp)
  target_link_libraries(lzss_decode_tests PRIVATE th08_strict)
  target_include_directories(lzss_decode_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/th08/src)
  add_test(NAME lzss_decode COMMAND lzss_decode_tests)

  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Pure rollout/child-process contracts need no assets or native runtime.
    add_executable(plan_contract_tests tests/plan_contract_tests.cpp)
    target_compile_features(plan_contract_tests PRIVATE cxx_std_17)
    target_include_directories(plan_contract_tests PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/third_party/th08/src)
    add_test(NAME plan_contract COMMAND plan_contract_tests)
  endif()

  if(TARGET th08_replay_suite)
    add_executable(th08_replay_suite_child tests/replay_suite_child.cpp)
    target_compile_features(th08_replay_suite_child PRIVATE cxx_std_17)
    add_test(NAME replay_suite_contract COMMAND ${CMAKE_COMMAND}
      -DSUITE=$<TARGET_FILE:th08_replay_suite>
      -DCHILD=$<TARGET_FILE:th08_replay_suite_child>
      -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
      -DWORK=${CMAKE_CURRENT_BINARY_DIR}/replay-suite-contract
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/replay_suite_tests.cmake)
    set_tests_properties(replay_suite_contract PROPERTIES TIMEOUT 30)
  endif()

  # Local links, anchors and prose spacing in every maintained Markdown file.
  add_executable(documentation_tests tests/documentation_tests.cpp)
  target_link_libraries(documentation_tests PRIVATE th08_strict)
  add_test(NAME documentation COMMAND documentation_tests ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(TH08_HEADLESS)
  include(cmake/Headless.cmake)
endif()
if(TH08_WINE_SHADOW)
  include(cmake/WineShadow.cmake)
endif()
