# Available without game assets; only the corpus gate needs the native checker.
if(UNIX)
  add_executable(th08_replay_suite tools/replay_suite.cpp)
  target_compile_features(th08_replay_suite PRIVATE cxx_std_17)
  target_link_libraries(th08_replay_suite PRIVATE OpenSSL::Crypto)
  if(NOT MSVC)
    target_compile_options(th08_replay_suite PRIVATE -Wall -Wextra -Wpedantic)
  endif()
  if(BUILD_TESTING)
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
endif()
