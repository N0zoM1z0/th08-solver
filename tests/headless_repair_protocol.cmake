execute_process(COMMAND "${CHILD}" --parser-test RESULT_VARIABLE parser)
if(NOT parser EQUAL 0)
  message(FATAL_ERROR "Strict child JSON grammar regression")
endif()
file(MAKE_DIRECTORY "${WORK}")
foreach(seed IN ITEMS 0 1 2 3 4 5 6 7 8 9 10 11 12)
  set(directory "${WORK}/case-${seed}")
  # Only generated test artifacts are replaced between local test invocations.
  file(REMOVE_RECURSE "${directory}")
  execute_process(COMMAND "${REPAIR}" --executable "${CHILD}" --dat mock --stage extra
    --spell-id 203 --difficulty 4 --seed ${seed} --frames 200 --seconds 1
    --output-dir "${directory}" RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr
    TIMEOUT 10)
  if(seed EQUAL 0 OR seed EQUAL 5 OR seed EQUAL 10)
    set(expected 0)
  elseif(seed EQUAL 3 OR seed EQUAL 11 OR seed EQUAL 12)
    set(expected 2)
  else()
    set(expected 1)
  endif()
  if(NOT result STREQUAL "${expected}")
    message(FATAL_ERROR "Protocol seed${seed}: ${result}, expected${expected}: ${stdout}${stderr}")
  endif()
  if(NOT EXISTS "${directory}/summary.json")
    message(FATAL_ERROR "Failure/success did not preserve its cost summary")
  endif()
  file(READ "${directory}/summary.json" report)
  if(seed EQUAL 3 AND NOT report MATCHES "\"interrupted_update_upper_bound\":200")
    message(FATAL_ERROR "Interrupted child lost its conservative update charge")
  endif()
endforeach()
# H12 holds last six updates; unsupported horizons and a mixed-horizon unsafe
# transition must not start a search. This fixture verifies the process contract,
# while real DAT runs independently verify the native route.
file(STRINGS "${WORK}/case-10/r1k1a84.prefix" h12_prefix)
list(LENGTH h12_prefix h12_length)
set(h12_hold ${h12_prefix})
list(FILTER h12_hold INCLUDE REGEX "^84$")
list(LENGTH h12_hold h12_hold_length)
if(NOT h12_length EQUAL 31 OR NOT h12_hold_length EQUAL 6)
  message(FATAL_ERROR "H12 repair did not derive a six-update intervention")
endif()
foreach(seed IN ITEMS 11 12)
  file(READ "${WORK}/case-${seed}/summary.json" failure)
  if(NOT failure MATCHES "\"outcome\":\"no-supported-unsafe-transition\"" OR
     NOT failure MATCHES "\"candidates\":0")
    message(FATAL_ERROR "Unsupported or mixed horizon supplied a repair trigger")
  endif()
endforeach()
# A fresh directory is mandatory: no old success report can authorize a rerun.
execute_process(COMMAND "${REPAIR}" --executable "${CHILD}" --dat mock --seed 0
  --output-dir "${WORK}/case-0" RESULT_VARIABLE duplicate OUTPUT_QUIET ERROR_QUIET)
if(NOT duplicate EQUAL 1)
  message(FATAL_ERROR "Existing artifacts were not rejected")
endif()
foreach(seed IN ITEMS 1 6 7)
  file(READ "${WORK}/case-${seed}/summary.json" failure)
  if(NOT failure MATCHES "\"phase\":1,\"attempts\":1,\"verified_native_updates\":0,\"unverified_update_upper_bound\":200")
    message(FATAL_ERROR "Semantically invalid child was charged as verified work")
  endif()
endforeach()

foreach(pair IN ITEMS "8;0" "9;2")
  list(GET pair 0 seed)
  list(GET pair 1 phase)
  file(READ "${WORK}/case-${seed}/summary.json" failure)
  if(NOT failure MATCHES "\"phase\":${phase},\"attempts\":1,\"verified_native_updates\":0,\"unverified_update_upper_bound\":200")
    message(FATAL_ERROR "Initial/verification timeout lost its conservative cost")
  endif()
endforeach()

# A later budget stop or interrupted child cannot erase a validated improvement.
foreach(seed IN ITEMS 13 14)
  set(directory "${WORK}/case-${seed}")
  file(REMOVE_RECURSE "${directory}")
  set(budget 200)
  if(seed EQUAL 14)
    set(budget 1000)
  endif()
  execute_process(COMMAND "${REPAIR}" --executable "${CHILD}" --dat mock --stage extra
    --spell-id 203 --difficulty 4 --seed ${seed} --frames 200 --seconds 1
    --update-budget ${budget} --output-dir "${directory}" RESULT_VARIABLE result
    OUTPUT_QUIET ERROR_VARIABLE stderr TIMEOUT 10)
  if(NOT result EQUAL 2)
    message(FATAL_ERROR "Partial frontier fixture failed: ${stderr}")
  endif()
  file(READ "${directory}/summary.json" summary)
  if(NOT summary MATCHES "\"selected_case\":\"r1k1a84\"" OR
     NOT summary MATCHES "\"candidate_native_updates\":90")
    message(FATAL_ERROR "Partial round discarded its validated best")
  endif()
  if(seed EQUAL 14 AND NOT summary MATCHES "\"interrupted_update_upper_bound\":200")
    message(FATAL_ERROR "Unverified child was not conservatively charged")
  endif()
endforeach()

# Resume reconstructs the world in a fresh child and accounts for that work.
set(directory "${WORK}/resumed")
file(REMOVE_RECURSE "${directory}")
execute_process(COMMAND "${REPAIR}" --executable "${CHILD}" --dat mock --stage extra
  --spell-id 203 --difficulty 4 --seed 13 --frames 200 --seconds 1 --update-budget 200
  --resume-search "${WORK}/case-13" --resume-producer "${REPAIR}"
  --output-dir "${directory}" RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE stderr TIMEOUT 10)
if(NOT result EQUAL 2)
  message(FATAL_ERROR "Resume regeneration failed: ${stderr}")
endif()
file(READ "${directory}/summary.json" summary)
if(NOT summary MATCHES "\"prior_verified_native_updates\":154" OR
   NOT summary MATCHES "\"cumulative_verified_native_updates\":334")
  message(FATAL_ERROR "Resume lost cumulative regeneration/search costs")
endif()

# Reject stale/mutated reports, a fabricated lower frontier and escaped paths.
foreach(kind IN ITEMS report frontier path cumulative identity)
  set(source "${WORK}/tamper-${kind}")
  set(directory "${WORK}/rejected-${kind}")
  file(REMOVE_RECURSE "${source}" "${directory}")
  file(COPY "${WORK}/case-13/" DESTINATION "${source}")
  if(kind STREQUAL "report")
    file(APPEND "${source}/r1k1a84.json" " ")
  else()
    file(READ "${source}/summary.json" summary)
    if(kind STREQUAL "frontier")
      string(REPLACE "\"selected_case\":\"r1k1a84\"" "\"selected_case\":\"initial\"" summary "${summary}")
    elseif(kind STREQUAL "path")
      string(REPLACE "\"selected_case\":\"r1k1a84\"" "\"selected_case\":\"../escape\"" summary "${summary}")
    elseif(kind STREQUAL "cumulative")
      string(REPLACE "\"cumulative_verified_native_updates\":154" "\"cumulative_verified_native_updates\":999" summary "${summary}")
    elseif(kind STREQUAL "identity")
      string(REPLACE "\"child_executable_sha256\":\"" "\"child_executable_sha256\":\"bad" summary "${summary}")
    endif()
    file(WRITE "${source}/summary.json" "${summary}")
  endif()
  execute_process(COMMAND "${REPAIR}" --executable "${CHILD}" --dat mock --stage extra
    --spell-id 203 --difficulty 4 --seed 13 --frames 200 --seconds 1 --update-budget 200
    --resume-search "${source}" --resume-producer "${REPAIR}" --output-dir "${directory}"
    RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE stderr TIMEOUT 10)
  if(NOT result EQUAL 1 OR EXISTS "${directory}/initial.json")
    message(FATAL_ERROR "Unsafe resume ${kind} was executed: ${stderr}")
  endif()
endforeach()
