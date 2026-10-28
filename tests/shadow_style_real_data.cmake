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
foreach(controls IN ITEMS "--lead-updates;0" "--verify-horizon;96" "--economic-proposals;5"
                          "--phase-plan;8:0:208:7:600:feedback:youkai"
                          "--phase-plan;0:2:208:7:600:feedback:youkai"
                          "--stop-after-card;257"
                          "--stop-after-card;2;--stop-after-card;2"
                          "--spell-horizon;4" "--spell-horizon;96"
                          "--spell-horizon;64;--lead-updates;32"
                          "--spell-horizon;128;--spell-horizon;128"
                          "--verify-horizon;256;--spell-horizon;64;--policy-horizon;128"
                          "--verify-horizon;256;--spell-horizon;64;--nominal-warm-updates;80"
                          "--economic-movement;5"
                          "--economic-movement;1;--economic-movement;1"
                          "--farm-to-timeout;1"
                          "--farm-charge-fire;1"
                          "--checkpoint-updates;63"
                          "--upper-flow;1"
                          "--drop-follow;1"
                          "--parent-chain-bias;5"
                          "--parent-chain-bias;nan;--drop-target;1"
                          "--farm-to-timeout;1;--attack-pacing;1;--require-all-captures;1"
                          "--nominal-warm-updates;4" "--replan-updates;65")
  execute_process(COMMAND "${SOLVER}" "${DAT}" "${WORK}" 0 3 1 23035 score 100 0 1
    ${controls} RESULT_VARIABLE rejected ERROR_VARIABLE error TIMEOUT 3)
  if(NOT rejected STREQUAL "1")
    message(FATAL_ERROR "Invalid offline prefix/search schedule was accepted: ${controls} ${error}")
  endif()
endforeach()
# This fixture verifies complete execution, not the 110ms default's performance.
# Use the declared offline recovery profile so host scheduling does not turn a
# timed-out proposal into a missing incumbent for all dependent regressions.
execute_process(COMMAND "${SOLVER}" "${DAT}" "${WORK}" 0 3 1 23035 score 70000 0 600
  --collect-items 1 --aim-boss 1 --drop-target 1 --economic-proposals 4 --require-all-captures 1 --target-score 1
  --verify-horizon 128 --publish-horizon 64 --lead-updates 1 --replan-updates 32
  --nominal-warm-updates 1 --search-budget-ms 2000 --held-recovery 1 --baseline-recovery 1
  --checkpoint-updates 128
  RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 660)
if(NOT result STREQUAL "0")
  message(FATAL_ERROR "Complete style scene failed: ${result} ${stdout} ${stderr}")
endif()
file(READ "${WORK}/result.json" original)
if(NOT original MATCHES "\"collect_items\":true" OR
   NOT original MATCHES "\"aim_boss\":true" OR
   NOT original MATCHES "\"drop_target\":true" OR
   NOT original MATCHES "\"drop_target_revision\":1" OR
   NOT original MATCHES "\"economic_proposals\":4" OR
   NOT original MATCHES "\"economic_movement\":0," OR
   NOT original MATCHES "\"motion_economic_attempts\":0," OR
   NOT original MATCHES "\"economic_attempts\":[1-9][0-9]*" OR
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
file(SHA256 "${WORK}/plan.actions" final_tape_hash)
file(SHA256 "${WORK}/checkpoint.inputs.actions" checkpoint_tape_hash)
file(SIZE "${WORK}/checkpoint.inputs.actions" checkpoint_bytes)
file(READ "${WORK}/checkpoint.json" checkpoint_metadata)
if(NOT final_tape_hash STREQUAL checkpoint_tape_hash OR
   NOT checkpoint_metadata MATCHES "\"saved_updates\":${input_count}," OR
   NOT checkpoint_metadata MATCHES "\"saved_bytes\":${checkpoint_bytes}," OR
   NOT checkpoint_metadata MATCHES "\"gameplay_certificate\":false,")
  message(FATAL_ERROR "Executed-input checkpoint changed the owner tape or claimed a certificate: ${checkpoint_metadata}")
endif()
# A deterministic budget-limited owner executes a known literal prefix. Saving
# it must neither export a replay nor turn an incomplete duration into success.
execute_process(COMMAND "${SOLVER}" "${DAT}" "${WORK}/partial-checkpoint" 0 3 1 23035 score 1024 0 180
  --prefix-actions "${WORK}/plan.actions" --prefix-updates 1024 --checkpoint-updates 128
  RESULT_VARIABLE partial_result OUTPUT_VARIABLE partial_stdout ERROR_VARIABLE partial_stderr TIMEOUT 30)
if(NOT partial_result STREQUAL "2")
  message(FATAL_ERROR "Partial checkpoint changed the incomplete-owner exit: ${partial_result} ${partial_stdout} ${partial_stderr}")
endif()
file(READ "${WORK}/partial-checkpoint/result.json" partial_report)
file(READ "${WORK}/partial-checkpoint/checkpoint.json" partial_metadata)
file(STRINGS "${WORK}/partial-checkpoint/checkpoint.inputs.actions" partial_inputs)
list(SUBLIST prefix_inputs 0 1024 expected_partial_inputs)
if(NOT partial_inputs STREQUAL expected_partial_inputs OR
   NOT partial_report MATCHES "\"outcome\":\"budget\"" OR
   NOT partial_report MATCHES "\"replay_exported\":false" OR
   NOT partial_metadata MATCHES "\"saved_updates\":1024," OR
   NOT partial_metadata MATCHES "\"gameplay_certificate\":false," OR
   EXISTS "${WORK}/partial-checkpoint/native.rpy")
  message(FATAL_ERROR "A partial input checkpoint certified a replay or changed its executed inputs: ${partial_report} ${partial_metadata}")
endif()
# Cross the first real spell boundary from the complete fixture's actual world.
# Compare omitted/zero controls byte-for-byte; an enabled shorter profile must
# preserve the prefix and replay its newly planned suffix to the same 41 words.
# These owners deliberately stop early: changing profiles cannot certify a run
# or export a replay merely because its checked futures were safe.
file(READ "${WORK}/score-audit.json" profile_source_audit)
string(REGEX MATCH "\"card\":2,[^}]*\"start_update\":([0-9]+)" profile_spell "${profile_source_audit}")
if(NOT profile_spell OR CMAKE_MATCH_1 LESS 32)
  message(FATAL_ERROR "Complete source fixture has no usable first-spell boundary")
endif()
math(EXPR profile_prefix "${CMAKE_MATCH_1} - 32")
math(EXPR profile_limit "${profile_prefix} + 192")
foreach(profile_mode IN ITEMS default disabled active)
  set(profile_options)
  if(profile_mode STREQUAL "disabled")
    set(profile_options --spell-horizon 0)
  elseif(profile_mode STREQUAL "active")
    set(profile_options --spell-horizon 64)
  endif()
  set(profile_run "${WORK}/profile-${profile_mode}")
  execute_process(COMMAND "${SOLVER}" "${DAT}" "${profile_run}" 0 3 1 23035 score ${profile_limit} 0 180
    --prefix-actions "${WORK}/plan.actions" --prefix-updates ${profile_prefix}
    --verify-horizon 128 --publish-horizon 64 --lead-updates 1 --replan-updates 32
    --nominal-warm-updates 1 --search-budget-ms 2000 --held-recovery 1 --baseline-recovery 1
    --allow-bombs 0 --require-all-captures 0 ${profile_options}
    RESULT_VARIABLE profile_result OUTPUT_VARIABLE profile_stdout ERROR_VARIABLE profile_stderr TIMEOUT 60)
  if(NOT profile_result STREQUAL "2")
    message(FATAL_ERROR "Profile owner changed partial-run failure meaning: ${profile_mode} ${profile_result} ${profile_stdout} ${profile_stderr}")
  endif()
  file(READ "${profile_run}/result.json" profile_report)
  file(STRINGS "${profile_run}/plan.actions" profile_inputs)
  list(SUBLIST profile_inputs 0 ${profile_prefix} profile_actual_prefix)
  list(SUBLIST prefix_inputs 0 ${profile_prefix} profile_expected_prefix)
  if(NOT profile_actual_prefix STREQUAL profile_expected_prefix OR
     NOT profile_report MATCHES "\"outcome\":\"budget\"" OR
     NOT profile_report MATCHES "\"updates\":${profile_limit}," OR
     NOT profile_report MATCHES "\"no_miss\":true," OR
     NOT profile_report MATCHES "\"bombs_used\":0," OR
     NOT profile_report MATCHES "\"replay_exported\":false" OR
     EXISTS "${profile_run}/native.rpy")
    message(FATAL_ERROR "Profile owner altered its prefix, escaped NMNB or certified an incomplete run: ${profile_report}")
  endif()
  string(REGEX MATCH "\"terminal_projection\":(\\[[^]]+\\])" profile_projection "${profile_report}")
  set(profile_projection "${CMAKE_MATCH_1}")
  if(profile_mode STREQUAL "default")
    file(SHA256 "${profile_run}/plan.actions" profile_default_hash)
    set(profile_default_projection "${profile_projection}")
  elseif(profile_mode STREQUAL "disabled")
    file(SHA256 "${profile_run}/plan.actions" profile_disabled_hash)
    if(NOT profile_disabled_hash STREQUAL profile_default_hash OR
       NOT profile_projection STREQUAL profile_default_projection OR
       NOT profile_report MATCHES "\"profile_switches\":0," OR
       NOT profile_report MATCHES "\"discarded_pending_inputs\":0,")
      message(FATAL_ERROR "Zero spell horizon changed the default input tape or terminal: ${profile_report}")
    endif()
  else()
    foreach(pair IN ITEMS "road_verify_horizon:128" "road_publication:64" "road_replan:32"
                          "spell_verify_horizon:64" "spell_publication:32" "spell_replan:16")
      string(REPLACE ":" ";" parts "${pair}")
      list(GET parts 0 key)
      list(GET parts 1 value)
      if(NOT profile_report MATCHES "\"${key}\":${value},")
        message(FATAL_ERROR "Profile effective schedule missing ${pair}: ${profile_report}")
      endif()
    endforeach()
    if(NOT profile_report MATCHES "\"profile_switches\":[1-9][0-9]*," OR
       NOT profile_report MATCHES "\"road_owner_inputs\":[1-9][0-9]*," OR
       NOT profile_report MATCHES "\"spell_owner_inputs\":[1-9][0-9]*,")
      message(FATAL_ERROR "Profile fixture did not cross a real spell boundary: ${profile_report}")
    endif()
    execute_process(COMMAND "${FOLLOWER}" --dat "${DAT}" --actions "${profile_run}/plan.actions"
      --output "${profile_run}/fresh.json" --stage 1 --difficulty 3 --shot 1 --seed 23035
      --story 0 --input-profile sampled-gameplay --frames ${profile_limit}
      RESULT_VARIABLE profile_fresh_result OUTPUT_VARIABLE profile_fresh_stdout ERROR_VARIABLE profile_fresh_stderr TIMEOUT 60)
    file(READ "${profile_run}/fresh.json" profile_fresh)
    string(REGEX MATCH "\"terminal_projection\":(\\[[^]]+\\])" profile_fresh_projection "${profile_fresh}")
    if(NOT profile_fresh_result STREQUAL "0" OR profile_projection STREQUAL "" OR
       NOT profile_projection STREQUAL CMAKE_MATCH_1)
      message(FATAL_ERROR "Profile change failed fresh exact Native execution: ${profile_fresh_result} ${profile_fresh_stdout} ${profile_fresh_stderr} ${profile_fresh}")
    endif()
  endif()
endforeach()
# A phase checkpoint is an executed boundary, never a complete replay. The
# omitted/empty plan must preserve literal inputs, while an active proposal must
# regenerate the world after the same original prefix and pass a fresh execution.
string(REGEX MATCH "\"card\":2,[^}]*\"end_update\":([0-9]+)" phase_end "${profile_source_audit}")
if(NOT phase_end)
  message(FATAL_ERROR "Missing first-spell end for phase checkpoint regression")
endif()
set(phase_literal_end "${CMAKE_MATCH_1}")
math(EXPR phase_limit "${phase_literal_end} + 5000")
foreach(phase_mode IN ITEMS default empty active)
  set(phase_options)
  set(phase_prefix "${phase_literal_end}")
  if(phase_mode STREQUAL "empty")
    set(phase_options --phase-plan none)
  elseif(phase_mode STREQUAL "active")
    set(phase_options --phase-plan 0:2:208:7:600:feedback:youkai)
    set(phase_prefix "${profile_prefix}")
  endif()
  set(phase_run "${WORK}/phase-${phase_mode}")
  execute_process(COMMAND "${SOLVER}" "${DAT}" "${phase_run}" 0 3 1 23035 score ${phase_limit} 0 180
    --prefix-actions "${WORK}/plan.actions" --prefix-updates ${phase_prefix}
    --verify-horizon 128 --publish-horizon 64 --lead-updates 1 --replan-updates 32
    --nominal-warm-updates 1 --search-budget-ms 2000 --held-recovery 1 --baseline-recovery 1
    --attack-pacing 1 --allow-bombs 0 --require-all-captures 0 --stop-after-card 2 ${phase_options}
    RESULT_VARIABLE phase_result OUTPUT_VARIABLE phase_stdout ERROR_VARIABLE phase_stderr TIMEOUT 90)
  file(READ "${phase_run}/result.json" phase_report)
  file(STRINGS "${phase_run}/plan.actions" phase_inputs)
  list(LENGTH phase_inputs phase_count)
  list(SUBLIST phase_inputs 0 ${phase_prefix} phase_actual_prefix)
  list(SUBLIST prefix_inputs 0 ${phase_prefix} phase_expected_prefix)
  if(NOT phase_result STREQUAL "2" OR NOT phase_actual_prefix STREQUAL phase_expected_prefix OR
     NOT phase_report MATCHES "\"outcome\":\"phase_boundary\"" OR
     NOT phase_report MATCHES "\"no_miss\":true," OR
     NOT phase_report MATCHES "\"bombs_used\":0," OR
     NOT phase_report MATCHES "\"objective_met\":false," OR
     NOT phase_report MATCHES "\"replay_exported\":false" OR EXISTS "${phase_run}/native.rpy")
    message(FATAL_ERROR "Phase checkpoint changed prefix/constraints or certified a replay: ${phase_result} ${phase_stdout} ${phase_stderr} ${phase_report}")
  endif()
  if(phase_mode STREQUAL "default")
    file(SHA256 "${phase_run}/plan.actions" phase_default_hash)
    string(REGEX MATCH "\"terminal_projection\":(\\[[^]]+\\])" phase_default_projection "${phase_report}")
    set(phase_default_projection "${CMAKE_MATCH_1}")
  elseif(phase_mode STREQUAL "empty")
    file(SHA256 "${phase_run}/plan.actions" phase_empty_hash)
    string(REGEX MATCH "\"terminal_projection\":(\\[[^]]+\\])" phase_empty_projection "${phase_report}")
    if(NOT phase_empty_hash STREQUAL phase_default_hash OR
       phase_default_projection STREQUAL "" OR NOT phase_default_projection STREQUAL CMAKE_MATCH_1)
      message(FATAL_ERROR "Explicit empty phase plan changed the default execution")
    endif()
  else()
    if(NOT phase_report MATCHES "\"planned_inputs\":[1-9][0-9]*," OR
       NOT phase_report MATCHES "\"actual_inputs\":[1-9][0-9]*,")
      message(FATAL_ERROR "Targeted phase proposal was not exercised: ${phase_report}")
    endif()
    execute_process(COMMAND "${FOLLOWER}" --dat "${DAT}" --actions "${phase_run}/plan.actions"
      --output "${phase_run}/fresh.json" --stage 1 --difficulty 3 --shot 1 --seed 23035
      --story 0 --input-profile sampled-gameplay --frames ${phase_count}
      RESULT_VARIABLE phase_fresh_result OUTPUT_VARIABLE phase_fresh_stdout ERROR_VARIABLE phase_fresh_stderr TIMEOUT 60)
    file(READ "${phase_run}/fresh.json" phase_fresh)
    string(REGEX MATCH "\"terminal_projection\":(\\[[^]]+\\])" phase_projection "${phase_report}")
    set(phase_projection "${CMAKE_MATCH_1}")
    string(REGEX MATCH "\"terminal_projection\":(\\[[^]]+\\])" phase_fresh_projection "${phase_fresh}")
    if(NOT phase_fresh_result STREQUAL "0" OR phase_projection STREQUAL "" OR
       NOT phase_projection STREQUAL CMAKE_MATCH_1)
      message(FATAL_ERROR "Phase boundary failed fresh exact execution: ${phase_fresh_result} ${phase_fresh_stdout} ${phase_fresh_stderr} ${phase_fresh}")
    endif()
  endif()
endforeach()
# Exercise the opt-in comparison inside a real active spell, independently of
# fire/form proposals. The original complete fixture owns this literal prefix;
# the new owner must execute it unchanged and may certify only its new futures.
file(READ "${WORK}/score-audit.json" movement_source_audit)
string(REGEX MATCH "\"card\":13,[^}]*\"start_update\":([0-9]+)" movement_spell "${movement_source_audit}")
if(NOT movement_spell)
  message(FATAL_ERROR "Complete source fixture has no last-spell checkpoint")
endif()
set(movement_prefix "${CMAKE_MATCH_1}")
math(EXPR movement_limit "${movement_prefix} + 256")
execute_process(COMMAND "${SOLVER}" "${DAT}" "${WORK}/movement" 0 3 1 23035 score ${movement_limit} 0 180
  --prefix-actions "${WORK}/plan.actions" --prefix-updates ${movement_prefix}
  --economic-proposals 0 --economic-movement 4 --verify-horizon 128
  --lead-updates 1 --replan-updates 32 --search-budget-ms 2000
  --held-recovery 1 --baseline-recovery 1 --allow-bombs 0 --require-all-captures 0
  RESULT_VARIABLE movement_result OUTPUT_VARIABLE movement_stdout ERROR_VARIABLE movement_stderr TIMEOUT 60)
if(NOT movement_result STREQUAL "2")
  message(FATAL_ERROR "Movement comparison changed partial-owner failure meaning: ${movement_result} ${movement_stdout} ${movement_stderr}")
endif()
file(READ "${WORK}/movement/result.json" movement_report)
file(STRINGS "${WORK}/movement/plan.actions" movement_inputs)
list(SUBLIST prefix_inputs 0 ${movement_prefix} movement_expected_prefix)
list(SUBLIST movement_inputs 0 ${movement_prefix} movement_actual_prefix)
if(NOT movement_expected_prefix STREQUAL movement_actual_prefix OR
   NOT movement_report MATCHES "\"outcome\":\"budget\"" OR
   NOT movement_report MATCHES "\"updates\":${movement_limit}," OR
   NOT movement_report MATCHES "\"economic_proposals\":0," OR
   NOT movement_report MATCHES "\"economic_attempts\":0," OR
   NOT movement_report MATCHES "\"economic_movement\":4," OR
   NOT movement_report MATCHES "\"motion_economic_attempts\":[1-9][0-9]*," OR
   NOT movement_report MATCHES "\"no_miss\":true," OR
   NOT movement_report MATCHES "\"bombs_used\":0," OR
   NOT movement_report MATCHES "\"replay_exported\":false" OR
   EXISTS "${WORK}/movement/native.rpy")
  message(FATAL_ERROR "Movement comparison altered its literal prefix, escaped constraints or certified an incomplete run: ${movement_report}")
endif()
execute_process(COMMAND "${FOLLOWER}" --dat "${DAT}" --actions "${WORK}/movement/plan.actions"
  --output "${WORK}/movement-fresh.json" --stage 1 --difficulty 3 --shot 1 --seed 23035
  --story 0 --prior-final-a-clear 0 --input-profile sampled-gameplay --frames ${movement_limit}
  RESULT_VARIABLE movement_fresh_result OUTPUT_VARIABLE movement_fresh_stdout ERROR_VARIABLE movement_fresh_stderr TIMEOUT 60)
file(READ "${WORK}/movement-fresh.json" movement_fresh)
string(REGEX MATCH "\"terminal_projection\":(\\[[^]]+\\])" movement_projection "${movement_report}")
set(movement_projection "${CMAKE_MATCH_1}")
string(REGEX MATCH "\"terminal_projection\":(\\[[^]]+\\])" movement_fresh_projection "${movement_fresh}")
# MATCHES conditions below overwrite CMAKE_MATCH_1; retain both captures first.
set(movement_fresh_projection "${CMAKE_MATCH_1}")
if(NOT movement_fresh_result STREQUAL "0" OR
   NOT movement_fresh MATCHES "\"outcome\":\"budget\"" OR
   NOT movement_fresh MATCHES "\"deaths\":0," OR
   movement_projection STREQUAL "" OR NOT movement_projection STREQUAL movement_fresh_projection)
  message(FATAL_ERROR "Movement tape failed fresh exact Native execution: ${movement_fresh_result} ${movement_fresh_stdout} ${movement_fresh_stderr} ${movement_fresh}")
endif()
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
if(EXISTS "${WORK}/resumed/checkpoint.inputs.actions" OR
   EXISTS "${WORK}/resumed/checkpoint.json")
  message(FATAL_ERROR "Default-off checkpointing wrote files without an explicit interval")
endif()
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
