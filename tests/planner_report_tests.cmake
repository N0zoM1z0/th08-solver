file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/results/spell92-diversity-v3/case0"
  "${WORK}/results/advance-v1/story")
set(dat "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb")
set(binary "01cdab0637be646f45d3e0db12734837a979aa5a8e218fc8dabc7e427f0171e8")
set(revision "02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415")
file(WRITE "${WORK}/fidelity.json"
  "{\"outcome\":\"failed\",\"passed\":106,\"selected_fixtures\":108,\"full_suite\":true,\"dat_sha256\":\"${dat}\",\"private_path\":\"/private/sensitive\",\"cases\":[{\"id\":\"lunatic-1-b\",\"outcome\":\"failed\",\"failure\":\"reported_row_count_mismatch\",\"rows\":0,\"sha256\":\"\"}]}")
set(plan "{\"source_revision\":\"${revision}\",\"outcome\":\"complete\",\"frames\":5,\"deaths\":0,\"spell\":92,\"score\":10,\"rng_seed\":1,\"rng_draws\":2,\"native_updates\":9,\"options\":{\"seed\":0,\"seconds\":180,\"private_path\":\"/private/sensitive\"},\"secret\":\"203.0.113.42\"}")
set(verify "{\"source_revision\":\"${revision}\",\"verified\":true,\"frames\":5,\"complete_at\":5,\"death_updates\":0,\"score\":10,\"rng_seed\":1,\"rng_draws\":2,\"native_updates\":5,\"options\":{\"seed\":0},\"private_path\":\"/private/sensitive\"}")
file(WRITE "${WORK}/results/spell92-diversity-v3/case0/plan.json" "${plan}")
file(WRITE "${WORK}/results/spell92-diversity-v3/case0.exit" "0\n")
# A pending job's plan must never be consumed, even if it happens to say complete.
file(WRITE "${WORK}/results/advance-v1/story/plan.json" "{unfinished and private}")

function(run_report name success)
  execute_process(COMMAND "${REPORT}" --root "${WORK}/results" --fidelity "${WORK}/fidelity.json"
    --output "${WORK}/ledger-${name}.json" --known-binary-sha256 "${binary}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
  if(success AND NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "${name}: expected success: ${output}${error}")
  elseif(NOT success AND "${result}" STREQUAL "0")
    message(FATAL_ERROR "${name}: incomplete evidence unexpectedly passed")
  endif()
  file(READ "${WORK}/ledger-${name}.json" ledger)
  foreach(sensitive IN ITEMS "/private/sensitive" "203.0.113.42" "private_path" "secret" "${WORK}")
    string(FIND "${ledger}${output}${error}" "${sensitive}" position)
    if(NOT position EQUAL -1)
      message(FATAL_ERROR "${name}: sensitive or unknown metadata was disclosed")
    endif()
  endforeach()
  set(ledger "${ledger}" PARENT_SCOPE)
endfunction()

run_report(missing-verification FALSE)
if(NOT ledger MATCHES "\"verified_complete_cases\":0" OR
   NOT ledger MATCHES "\"failure\":\"missing_verification\"")
  message(FATAL_ERROR "A complete plan without verification was counted")
endif()
file(MAKE_DIRECTORY "${WORK}/results/spell92-verified/case0")
file(WRITE "${WORK}/results/spell92-verified/case0/verify.json" "${verify}")
run_report(verified TRUE)
if(NOT ledger MATCHES "\"verified_complete_cases\":1" OR
   NOT ledger MATCHES "\"pending_cases\":1" OR
   NOT ledger MATCHES "\"passed\":106" OR
   NOT ledger MATCHES "\"selected_fixtures\":108" OR
   NOT ledger MATCHES "\"outcome\":\"failed\"" OR
   NOT ledger MATCHES "\"native_updates\":14" OR
   NOT ledger MATCHES "\"binary_sha256\":\"${binary}\"")
  message(FATAL_ERROR "Verified/pending counts, costs, binary provenance or fidelity changed")
endif()
string(REPLACE "\"rng_draws\":2" "\"rng_draws\":3" mismatch "${verify}")
file(WRITE "${WORK}/results/spell92-verified/case0/verify.json" "${mismatch}")
run_report(rng-mismatch FALSE)
if(NOT ledger MATCHES "verification_state_mismatch")
  message(FATAL_ERROR "Mismatched RNG was accepted")
endif()
file(WRITE "${WORK}/results/spell92-verified/case0/verify.json" "${verify}")

file(MAKE_DIRECTORY "${WORK}/results/spell92-diversity-v2")
file(WRITE "${WORK}/results/spell92-diversity-v2/rejected.exit" "2\n")
file(WRITE "${WORK}/results/spell92-diversity-v2/rejected.log"
  "invalid planner arguments: unknown option: --diversity-cell\n")
run_report(setup-rejection TRUE)
if(NOT ledger MATCHES "\"setup_rejected_cases\":1" OR
   NOT ledger MATCHES "\"setup_rejection_native_costs_recorded\":false")
  message(FATAL_ERROR "Proven argument rejection was lost or assigned invented native costs")
endif()
file(WRITE "${WORK}/results/spell92-diversity-v2/rejected.log" "some other failure\n")
run_report(unknown-missing-report FALSE)
if(NOT ledger MATCHES "\"missing_report_cases\":1" OR
   NOT ledger MATCHES "unverified_missing_report")
  message(FATAL_ERROR "An unknown missing report was silently treated as a setup rejection")
endif()
file(WRITE "${WORK}/results/spell92-diversity-v2/rejected.log"
  "invalid planner arguments: unknown option: --diversity-cell\n")

file(MAKE_DIRECTORY "${WORK}/results/practice-v1/stage1"
  "${WORK}/results/practice-v1/stage1-verify")
string(REPLACE "\"spell\":92" "\"spell\":-1" stage_plan "${plan}")
file(WRITE "${WORK}/results/practice-v1/stage1/plan.json" "${stage_plan}")
file(WRITE "${WORK}/results/practice-v1/stage1.exit" "0")
file(WRITE "${WORK}/results/practice-v1/stage1-verify/verify.json" "${verify}")
run_report(missing-natural-replay FALSE)
if(NOT ledger MATCHES "missing_natural_replay" OR NOT ledger MATCHES "\"verified_complete_cases\":1")
  message(FATAL_ERROR "Stage success without a natural .rpy replay was counted")
endif()
set(natural_replay "{\"rows\":5,\"steps\":3,\"finished\":true,\"natural\":true,\"death_rows\":0,\"bomb_input_rows\":0,\"stages\":[{\"stage\":0,\"endScore\":10}]}")
file(WRITE "${WORK}/results/practice-v1/stage1-rpy.json" "${natural_replay}")
run_report(natural-replay TRUE)
if(NOT ledger MATCHES "\"verified_complete_cases\":2" OR
   NOT ledger MATCHES "\"native_updates\":28" OR
   NOT ledger MATCHES "\"rpy_calculation_rows\":5" OR
   NOT ledger MATCHES "\"rpy_session_steps\":3")
  message(FATAL_ERROR "Natural replay or its separate costs were lost")
endif()
foreach(counter IN ITEMS death_rows bomb_input_rows)
  string(REPLACE "\"${counter}\":0" "\"${counter}\":1" non_nmnb_replay "${natural_replay}")
  file(WRITE "${WORK}/results/practice-v1/stage1-rpy.json" "${non_nmnb_replay}")
  run_report("natural-${counter}" FALSE)
  if(NOT ledger MATCHES "natural_replay_not_nmnb")
    message(FATAL_ERROR "A replay with ${counter} was counted as NMNB")
  endif()
  string(REPLACE "\"${counter}\":0," "" missing_counter "${natural_replay}")
  file(WRITE "${WORK}/results/practice-v1/stage1-rpy.json" "${missing_counter}")
  run_report("missing-${counter}" FALSE)
endforeach()
file(WRITE "${WORK}/results/practice-v1/stage1-rpy.json"
  "{\"rows\":5,\"steps\":3,\"finished\":true,\"natural\":false}")
run_report(unnatural-replay FALSE)
if(NOT ledger MATCHES "natural_replay_mismatch")
  message(FATAL_ERROR "Unnatural replay was accepted")
endif()

# A continuous replay's last score is authoritative; an earlier matching score
# must not hide a different final result. A still-running job remains pending.
file(WRITE "${WORK}/results/practice-v1/stage1-rpy.json" "${natural_replay}")
file(WRITE "${WORK}/results/advance-v1/story/plan.json" "${stage_plan}")
file(MAKE_DIRECTORY "${WORK}/results/advance-v1/story-verify")
file(WRITE "${WORK}/results/advance-v1/story-verify/verify.json" "${verify}")
set(story_replay "{\"rows\":5,\"steps\":3,\"finished\":true,\"natural\":true,\"death_rows\":0,\"bomb_input_rows\":0,\"stages\":[{\"stage\":0,\"endScore\":3},{\"stage\":1,\"endScore\":10}]}")
file(WRITE "${WORK}/results/advance-v1/story-rpy.json" "${story_replay}")
run_report(story-without-exit TRUE)
if(NOT ledger MATCHES "\"verified_complete_cases\":2" OR NOT ledger MATCHES "\"pending_cases\":1")
  message(FATAL_ERROR "An intermediate story plan was consumed before its exit marker")
endif()
file(WRITE "${WORK}/results/advance-v1/story.exit" "0")
run_report(story-final-score TRUE)
if(NOT ledger MATCHES "\"verified_complete_cases\":3" OR
   NOT ledger MATCHES "\"group\":\"advance-v1\",\"id\":\"story\",\"status\":\"verified_complete\"[^}]*\"binary_sha256\":\"${binary}\"")
  message(FATAL_ERROR "Story completion or retained binary identity was lost")
endif()
string(REPLACE "\"endScore\":3" "\"endScore\":10" wrong_story "${story_replay}")
string(REPLACE "{\"stage\":1,\"endScore\":10}" "{\"stage\":1,\"endScore\":11}" wrong_story "${wrong_story}")
file(WRITE "${WORK}/results/advance-v1/story-rpy.json" "${wrong_story}")
run_report(story-wrong-final-score FALSE)
if(NOT ledger MATCHES "natural_replay_score_mismatch" OR NOT ledger MATCHES "\"verified_complete_cases\":2")
  message(FATAL_ERROR "An earlier matching score hid a mismatched final story score")
endif()
string(REPLACE "[{\"stage\":0,\"endScore\":3},{\"stage\":1,\"endScore\":10}]" "[]" empty_stages "${story_replay}")
file(WRITE "${WORK}/results/advance-v1/story-rpy.json" "${empty_stages}")
run_report(story-empty-stages FALSE)
if(NOT ledger MATCHES "natural_replay_missing_stages")
  message(FATAL_ERROR "A replay with no stage evidence was counted")
endif()
file(WRITE "${WORK}/results/advance-v1/story-rpy.json" "${story_replay}")

# New efficiency runs use their producer's own recorded identity in preference
# to the separately retained binary for the earlier campaign.
file(MAKE_DIRECTORY "${WORK}/results/spell92-efficiency-v1/efficient"
  "${WORK}/results/spell92-efficiency-v1/efficient-verify")
set(new_binary "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
string(REPLACE "\"outcome\":\"complete\"" "\"binary_sha256\":\"${new_binary}\",\"outcome\":\"complete\"" efficient_plan "${plan}")
string(REPLACE "\"verified\":true" "\"binary_sha256\":\"${new_binary}\",\"verified\":true" efficient_verify "${verify}")
file(WRITE "${WORK}/results/spell92-efficiency-v1/efficient/plan.json" "${efficient_plan}")
file(WRITE "${WORK}/results/spell92-efficiency-v1/efficient.exit" "0")
file(WRITE "${WORK}/results/spell92-efficiency-v1/efficient-verify/verify.json" "${efficient_verify}")
run_report(efficiency-group TRUE)
if(NOT ledger MATCHES "\"verified_complete_cases\":4" OR
   NOT ledger MATCHES "\"group\":\"spell92-efficiency-v1\",\"id\":\"efficient\",\"status\":\"verified_complete\"[^}]*\"binary_sha256\":\"${new_binary}\"")
  message(FATAL_ERROR "Efficiency group or its producer binary identity was lost")
endif()

# A successful continuation must prove that every requested prefix action was
# reconstructed before search, in addition to replaying the full final tape.
file(MAKE_DIRECTORY "${WORK}/results/story-prefix-v1/resumed"
  "${WORK}/results/story-prefix-v1/resumed-verify")
string(REPLACE "\"seconds\":180" "\"seconds\":180,\"prefix\":1" resumed_plan "${stage_plan}")
string(REPLACE "\"outcome\":\"complete\""
  "\"prefix_complete\":true,\"prefix_sha256\":\"${dat}\",\"prefix_requested_frames\":3,\"prefix_native_updates\":3,\"prefix_native_update_attempts\":3,\"outcome\":\"complete\""
  resumed_plan "${resumed_plan}")
file(WRITE "${WORK}/results/story-prefix-v1/resumed/plan.json" "${resumed_plan}")
file(WRITE "${WORK}/results/story-prefix-v1/resumed.exit" "0")
file(WRITE "${WORK}/results/story-prefix-v1/resumed-verify/verify.json" "${verify}")
file(WRITE "${WORK}/results/story-prefix-v1/resumed-rpy.json" "${natural_replay}")
run_report(resumed TRUE)
if(NOT ledger MATCHES "\"verified_complete_cases\":5" OR
   NOT ledger MATCHES "\"prefix_native_updates\":3")
  message(FATAL_ERROR "Resumed completion or its reconstruction costs were lost")
endif()
string(REPLACE "\"prefix_complete\":true" "\"prefix_complete\":false" incomplete_prefix "${resumed_plan}")
file(WRITE "${WORK}/results/story-prefix-v1/resumed/plan.json" "${incomplete_prefix}")
run_report(incomplete-prefix FALSE)
if(NOT ledger MATCHES "incomplete_prefix_reconstruction")
  message(FATAL_ERROR "An incomplete prefix was counted as a completed continuation")
endif()
