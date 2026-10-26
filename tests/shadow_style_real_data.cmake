cmake_minimum_required(VERSION 3.16)
file(REMOVE_RECURSE "${WORK}")
# Reject schedules that could publish beyond native verification or overwrite
# their immutable lead. Validation must happen before waiting for a live owner.
foreach(controls IN ITEMS "--publish-horizon;129" "--lead-updates;128"
                          "--lead-updates;33"
                          "--search-budget-ms;0" "--human-bias;nan"
                          "--recovery-style;unknown" "--recovery-fire;unknown")
  execute_process(COMMAND "${ONLINE}" "${DAT}" "${WORK}/absent-owner"
    "${WORK}/invalid.json" mpc 60 behavioral h128-held score-harvest ${controls}
    RESULT_VARIABLE rejected ERROR_VARIABLE error TIMEOUT 3)
  if(NOT rejected STREQUAL "1" OR NOT error MATCHES
      "invalid_shadow_control|incompatible_shadow_controls")
    message(FATAL_ERROR "Unsafe live controls were not rejected before owner wait: ${rejected} ${error}")
  endif()
endforeach()
execute_process(COMMAND "${SOLVER}" "${DAT}" "${WORK}" 0 3 1 23035 score 1 0 1
  --replay-name ysopwnable RESULT_VARIABLE rejected ERROR_VARIABLE error TIMEOUT 3)
if(NOT rejected STREQUAL "1" OR NOT error MATCHES "native_replay_name_requires")
  message(FATAL_ERROR "Oversized name could overwrite the retail replay mode: ${rejected} ${error}")
endif()
execute_process(COMMAND "${SOLVER}" "${DAT}" "${WORK}" 0 3 1 23035 score 100 0 1
  --prefix-updates 1 RESULT_VARIABLE rejected ERROR_VARIABLE error TIMEOUT 3)
if(NOT rejected STREQUAL "1" OR NOT error MATCHES "native_prefix_requires")
  message(FATAL_ERROR "Unpaired checkpoint controls were accepted: ${rejected} ${error}")
endif()
foreach(controls IN ITEMS "--lead-updates;0" "--verify-horizon;96"
                          "--nominal-warm-updates;4" "--replan-updates;65")
  execute_process(COMMAND "${SOLVER}" "${DAT}" "${WORK}" 0 3 1 23035 score 100 0 1
    ${controls} RESULT_VARIABLE rejected ERROR_VARIABLE error TIMEOUT 3)
  if(NOT rejected STREQUAL "1")
    message(FATAL_ERROR "Invalid offline prefix/search schedule was accepted: ${controls} ${error}")
  endif()
endforeach()
execute_process(COMMAND "${SOLVER}" "${DAT}" "${WORK}" 0 3 1 23035 score 70000 0 180
  --collect-items 1 --aim-boss 1 --require-all-captures 1 --target-score 1
  RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 200)
if(NOT result STREQUAL "0")
  message(FATAL_ERROR "Complete style scene failed: ${result} ${stdout} ${stderr}")
endif()
file(READ "${WORK}/result.json" original)
if(NOT original MATCHES "\"collect_items\":true" OR
   NOT original MATCHES "\"aim_boss\":true" OR
   NOT original MATCHES "\"score_goal_revision\":1")
  message(FATAL_ERROR "Score experiment did not record its independent controls: ${original}")
endif()
if(NOT original MATCHES "\"replay_name\":\"ysopwnab\"" OR
   NOT original MATCHES "\"objective_met\":true")
  message(FATAL_ERROR "Qualified Native objective or owner replay identity missing: ${original}")
endif()
# Resume through the original owned world, warming observer history from the
# actual tape. Verify the regenerated continuation rather than only rechecking
# an unchanged copied suffix.
file(STRINGS "${WORK}/plan.actions" prefix_inputs)
list(LENGTH prefix_inputs input_count)
math(EXPR checkpoint "${input_count} - 64")
execute_process(COMMAND "${SOLVER}" "${DAT}" "${WORK}/resumed" 0 3 1 23035 score 70000 0 180
  --collect-items 1 --aim-boss 1 --require-all-captures 1 --target-score 1
  --verify-horizon 512 --publish-horizon 256 --lead-updates 1 --replan-updates 128
  --policy-horizon 48 --policy-beam 256 --verify-captures 1
  --nominal-warm-updates 1 --search-budget-ms 2000
  --prefix-actions "${WORK}/plan.actions" --prefix-updates "${checkpoint}"
  RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 200)
if(NOT result STREQUAL "0")
  message(FATAL_ERROR "Owned Native checkpoint continuation failed: ${result} ${stdout} ${stderr}")
endif()
file(READ "${WORK}/resumed/result.json" original)
if(NOT original MATCHES "\"prefix_updates_requested\":${checkpoint}," OR
   NOT original MATCHES "\"prefix_updates_executed\":${checkpoint}," OR
   NOT original MATCHES "\"lead\":1," OR NOT original MATCHES "\"horizon\":512," OR
   NOT original MATCHES "\"publication_horizon\":256," OR
   NOT original MATCHES "\"replan_updates\":128," OR
   NOT original MATCHES "\"policy_horizon\":48," OR
   NOT original MATCHES "\"verify_captures\":true," OR
   NOT original MATCHES "\"nominal_warm_updates\":1," OR
   NOT original MATCHES "\"search_budget_ms\":2000,")
  message(FATAL_ERROR "Checkpoint reconstruction was not charged: ${original}")
endif()
file(STRINGS "${WORK}/resumed/plan.actions" resumed_inputs)
list(SUBLIST prefix_inputs 0 ${checkpoint} prefix_part)
list(SUBLIST resumed_inputs 0 ${checkpoint} resumed_part)
if(NOT prefix_part STREQUAL resumed_part)
  message(FATAL_ERROR "Owned checkpoint changed the selected historical prefix")
endif()
set(RUN "${WORK}/resumed")
file(READ "${RUN}/score-audit.json" audit)
foreach(pair IN ITEMS "complete:true" "nmnb:true" "expected_unique:4" "captured_unique:4"
                      "unsuccessful_attempts:0" "unmatched_events:0"
                      "all_visited_stage_spells_captured:true")
  string(REPLACE ":" ";" parts "${pair}")
  list(GET parts 0 key)
  list(GET parts 1 value)
  if(NOT audit MATCHES "\"${key}\":${value}([,}]|$)")
    message(FATAL_ERROR "Full style scene must have ${pair}: ${audit}")
  endif()
endforeach()
execute_process(COMMAND "${FOLLOWER}" --dat "${DAT}" --actions "${RUN}/plan.actions"
  --output "${WORK}/fresh.json" --stage 1 --difficulty 3 --shot 1 --seed 23035
  --story 0 --input-profile sampled-gameplay --frames 70000
  RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 60)
if(NOT result STREQUAL "0")
  message(FATAL_ERROR "Fresh style execution failed: ${stdout} ${stderr}")
endif()
file(READ "${WORK}/fresh.json" fresh)
string(REGEX MATCH [=["terminal_projection":\[[0-9,]+\]]=] original_end "${original}")
string(REGEX MATCH [=["terminal_projection":\[[0-9,]+\]]=] fresh_end "${fresh}")
if(original_end STREQUAL "" OR NOT original_end STREQUAL fresh_end)
  message(FATAL_ERROR "Style search and fresh literal execution disagree: ${original_end} ${fresh_end}")
endif()
execute_process(COMMAND "${CHECKER}" "${DAT}" "${RUN}/native.rpy" "${WORK}/natural.rows"
  --score-audit "${WORK}/natural-audit.json" --require-all-captures
  RESULT_VARIABLE result OUTPUT_VARIABLE replay ERROR_VARIABLE stderr TIMEOUT 60)
if(NOT result STREQUAL "0" OR NOT replay MATCHES "\"natural\":true" OR
   NOT replay MATCHES "\"death_rows\":0" OR NOT replay MATCHES "\"bomb_input_rows\":0")
  message(FATAL_ERROR "Natural style replay/captures failed: ${result} ${replay} ${stderr}")
endif()
if(NOT replay MATCHES "\"player_name_hex\":\"79736f70776e6162\"")
  message(FATAL_ERROR "Retail-format player name did not preserve the selected eight bytes: ${replay}")
endif()
if(NOT replay MATCHES "\"slowdown_percent\":null,")
  message(FATAL_ERROR "Unmeasured Native slowdown must remain unavailable: ${replay}")
endif()
string(REGEX MATCH "\"score_raw\":([0-9]+)" unused "${original}")
set(score "${CMAKE_MATCH_1}")
if(NOT replay MATCHES "\"endScore\":${score}([,}]|$)")
  message(FATAL_ERROR "Natural style replay score disagrees: ${replay}")
endif()
file(READ "${WORK}/plan.actions" full_prefix)
file(WRITE "${WORK}/trailing.actions" "${full_prefix}5\n")
math(EXPR trailing_count "${input_count} + 1")
execute_process(COMMAND "${SOLVER}" "${DAT}" "${WORK}/trailing" 0 3 1 23035 score 70000 0 30
  --prefix-actions "${WORK}/trailing.actions" --prefix-updates "${trailing_count}"
  RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 35)
file(READ "${WORK}/trailing/result.json" trailing)
if(NOT result STREQUAL "2" OR EXISTS "${WORK}/trailing/native.rpy" OR NOT trailing MATCHES
    "\"outcome\":\"prefix_terminal_before_requested_boundary\"")
  message(FATAL_ERROR "Checkpoint silently ignored a requested input beyond terminal: ${trailing}")
endif()
