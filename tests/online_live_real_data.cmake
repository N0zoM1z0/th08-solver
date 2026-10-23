cmake_minimum_required(VERSION 3.16)
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
# A one-frame serialization/ownership check, not a deadline-performance test.
# Explicit generous owner tolerance prevents host wake-up jitter deciding this
# functional gate. The separate live experiment uses its frozen zero tolerance.
execute_process(COMMAND "${EXECUTABLE}" --dat "${DAT}" --out "${WORK}/one"
  --stage 1 --story 1 --final-route a --difficulty 3 --seed 0 --shot 1
  --horizon 24 --beam 192 --hard-phantoms 1 --motion-envelope 1
  --value-model "${MODEL}" --value-weight 0.25 --history-queue 4
  --scheduler-tolerance-us 1000000 --frames 1 --seconds 2
  RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 30)
if(NOT result STREQUAL "3")
  message(FATAL_ERROR "Live reporting fixture failed: ${result} ${stdout} ${stderr}")
endif()
file(READ "${WORK}/one/online.json" report)
function(expect key expected)
  string(REGEX MATCH "\"${key}\":[ ]*${expected}([,} \r\n\t]|$)" found "${report}")
  if(NOT found)
    message(FATAL_ERROR "Live report ${key} did not match ${expected}")
  endif()
endfunction()
expect(outcome "\"budget\"")
expect(frames 1)
expect(native_updates 1)
expect(native_update_attempts 1)
expect(calculation_updates 1)
expect(control_updates 0)
expect(future_native_updates 0)
expect(rewinds 0)
expect(deadline_enforced true)
expect(strict_wall_complete false)
expect(scheduler_tolerance_us 1000000)
expect(value_model 1)
expect(value_model_continuation "\"held-last\"")
file(SHA256 "${MODEL}" model_hash)
expect(value_model_sha256 "\"${model_hash}\"")
# Mutable std::string ADL must not select std::quoted: it emits raw TSV tabs.
string(FIND "${report}" "\t" raw_tab)
string(FIND "${report}" "\"value_model_header\":\"th08_branch_value_v1\\u000924\\u000964\\u000924\"" escaped_header)
if(NOT raw_tab EQUAL -1 OR escaped_header EQUAL -1)
  message(FATAL_ERROR "Live model metadata is not JSON-escaped")
endif()
file(STRINGS "${WORK}/one/online.actions" actions)
list(LENGTH actions action_count)
file(STRINGS "${WORK}/one/ticks.tsv" ticks)
list(LENGTH ticks tick_count)
if(NOT action_count EQUAL 1 OR NOT tick_count EQUAL 2)
  message(FATAL_ERROR "Live input/tick accounting lost its only submission")
endif()
execute_process(COMMAND "${EXECUTABLE}" --dat "${DAT}" --out "${WORK}/negative"
  --history-queue 0 --frames 1 --seconds 2
  RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 10)
if(NOT result STREQUAL "2" OR EXISTS "${WORK}/negative")
  message(FATAL_ERROR "Invalid live setup started a native case: ${result} ${stdout} ${stderr}")
endif()
