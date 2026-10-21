# Pure planner/process contracts need no assets or native runtime linkage.
if(BUILD_TESTING AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_executable(plan_contract_tests tests/plan_contract_tests.cpp)
  target_compile_features(plan_contract_tests PRIVATE cxx_std_17)
  target_include_directories(plan_contract_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/th08/src")
  add_test(NAME plan_contract COMMAND plan_contract_tests)
endif()
