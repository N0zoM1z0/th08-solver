cmake_minimum_required(VERSION 3.16)
file(REMOVE_RECURSE "${WORK}")
file(READ "${INCUMBENT}/result.json" incumbent)
execute_process(COMMAND "${REFINER}" "${DAT}" "${WORK}/run" 0 3 1 23035 0 0
  "${INCUMBENT}/plan.actions" --trials 2 --window 120 --interval 1800
  --timeout-ms 10000 --wall-seconds 90 --target-score 1
  RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 100)
if(NOT result STREQUAL "0")
  message(FATAL_ERROR "Complete Native refinement failed: ${result} ${stdout} ${stderr}")
endif()
file(READ "${WORK}/run/result.json" refined)
if(NOT refined MATCHES "\"outcome\":\"complete\"" OR
   NOT refined MATCHES "\"all_visited_stage_spells_captured\":true")
  message(FATAL_ERROR "Refinement lost its full source goal: ${refined}")
endif()
string(REGEX MATCH "\"score_displayed\":([0-9]+)" unused "${incumbent}")
set(initial_score "${CMAKE_MATCH_1}")
string(REGEX MATCH "\"score_displayed\":([0-9]+)" unused "${refined}")
set(final_score "${CMAKE_MATCH_1}")
if(final_score LESS initial_score OR NOT refined MATCHES
    "\"initial\":\\{\"status\":\"terminal\",\"score_displayed\":${initial_score},")
  message(FATAL_ERROR "Complete oracle changed the initial score or accepted a loss: ${refined}")
endif()
execute_process(COMMAND "${FOLLOWER}" --dat "${DAT}" --actions "${WORK}/run/plan.actions"
  --output "${WORK}/fresh.json" --stage 1 --difficulty 3 --shot 1 --seed 23035
  --story 0 --input-profile sampled-gameplay --frames 70000
  RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 35)
if(NOT result STREQUAL "0")
  message(FATAL_ERROR "Fresh refined execution failed: ${stdout} ${stderr}")
endif()
file(READ "${WORK}/fresh.json" fresh)
string(REGEX MATCH [=["terminal_projection":\[[0-9,]+\]]=] planned_end "${refined}")
string(REGEX MATCH [=["terminal_projection":\[[0-9,]+\]]=] fresh_end "${fresh}")
if(planned_end STREQUAL "" OR NOT planned_end STREQUAL fresh_end)
  message(FATAL_ERROR "Refinement disagreed with fresh execution: ${planned_end} ${fresh_end}")
endif()
execute_process(COMMAND "${CHECKER}" "${DAT}" "${WORK}/run/native.rpy" "${WORK}/natural.rows"
  --score-audit "${WORK}/natural-audit.json" --require-all-captures
  RESULT_VARIABLE result OUTPUT_VARIABLE natural ERROR_VARIABLE stderr TIMEOUT 35)
math(EXPR raw "${final_score} / 10")
if(NOT result STREQUAL "0" OR NOT natural MATCHES "\"natural\":true" OR
   NOT natural MATCHES "\"endScore\":${raw}([,}]|$)" OR
   NOT natural MATCHES "\"player_name_hex\":\"79736f70776e6162\"" OR
   NOT natural MATCHES "\"death_rows\":0" OR NOT natural MATCHES "\"bomb_input_rows\":0")
  message(FATAL_ERROR "Refined original-writer replay failed: ${result} ${natural} ${stderr}")
endif()
# A safe incomplete prefix cannot become an incumbent. A killed complete-suffix
# attempt must retain its failure and charge the full requested work cap.
file(WRITE "${WORK}/short.actions" "5\n5\n5\n5\n")
foreach(kind IN ITEMS incomplete timeout)
  set(tape "${WORK}/short.actions")
  set(timeout 10000)
  set(status tape_exhausted)
  if(kind STREQUAL "timeout")
    set(tape "${INCUMBENT}/plan.actions")
    set(timeout 1)
    set(status timeout)
  endif()
  execute_process(COMMAND "${REFINER}" "${DAT}" "${WORK}/${kind}" 0 3 1 23035 0 0
    "${tape}" --trials 0 --timeout-ms "${timeout}"
    RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 10)
  file(READ "${WORK}/${kind}/result.json" rejected)
  if(NOT result STREQUAL "2" OR EXISTS "${WORK}/${kind}/native.rpy" OR
     NOT rejected MATCHES "\"outcome\":\"invalid_incumbent\"" OR
     NOT rejected MATCHES "\"status\":\"${status}\"" OR
     NOT rejected MATCHES "\"all_captures\":false")
    message(FATAL_ERROR "Incomplete/timeout future certified an incumbent: ${result} ${rejected}")
  endif()
  if(kind STREQUAL "timeout")
    string(REGEX MATCH "\"requested\":([0-9]+)" unused "${rejected}")
    if(NOT rejected MATCHES "\"charged_updates\":${CMAKE_MATCH_1}([,}]|$)")
      message(FATAL_ERROR "Timeout work was hidden: ${rejected}")
    endif()
  endif()
endforeach()
