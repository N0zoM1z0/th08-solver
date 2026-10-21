# These are process/fixture contracts, not a replacement physics oracle. The
# known digest was independently generated with jq -cS and openssl dgst -sha256
# over schema + newline + one LE u32(1), followed by 24 zero u32 fields.
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/corpus")
file(WRITE "${WORK}/game.dat" "synthetic protocol fixture")
set(schema "${ROOT}/tests/replay_suite_schema.json")
set(known "ebc547c597e3bbe614016b7092dfd477bf4d34e515f6fdd692ce1ab41aab649a")

function(make_fixture mode)
  file(REMOVE_RECURSE "${WORK}/corpus")
  file(MAKE_DIRECTORY "${WORK}/corpus")
  file(WRITE "${WORK}/corpus/fixture.rpy" "${mode}")
  file(SHA256 "${WORK}/corpus/fixture.rpy" replay_hash)
  file(SIZE "${WORK}/corpus/fixture.rpy" replay_bytes)
  set(entry "{\"id\":\"fixture\",\"file\":\"fixture.rpy\",\"sha256\":\"${replay_hash}\",\"bytes\":${replay_bytes},\"retailTrace\":{\"schemaVersion\":4,\"calculationFrames\":1,\"sha256\":\"${known}\"}}")
  file(WRITE "${WORK}/manifest.json" "{\"version\":1,\"fixtures\":[${entry}]}\n")
  set(entry "${entry}" PARENT_SCOPE)
endfunction()

function(run_suite name expected_success expected_text)
  execute_process(COMMAND "${SUITE}" --checker "${CHILD}" --dat "${WORK}/game.dat"
    --manifest "${WORK}/manifest.json" --schema "${schema}" --corpus "${WORK}/corpus"
    --output "${WORK}/${name}" --expected-count 1 --timeout-seconds 1 ${ARGN}
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 8)
  if(expected_success AND NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "${name}: expected success; got ${result}: ${output}${error}")
  elseif(NOT expected_success AND "${result}" STREQUAL "0")
    message(FATAL_ERROR "${name}: unexpectedly passed: ${output}${error}")
  endif()
  if(NOT "${output}${error}" MATCHES "${expected_text}")
    message(FATAL_ERROR "${name}: missing diagnostic ${expected_text}: ${output}${error}")
  endif()
  if("${output}${error}" MATCHES "${WORK}")
    message(FATAL_ERROR "${name}: CLI disclosed an absolute input/output path")
  endif()
endfunction()

make_fixture(good)
run_suite(good TRUE "verified 1/1 selected fixtures; 1 manifest fixtures")
file(READ "${WORK}/good/summary.json" summary)
if(NOT summary MATCHES "\"outcome\":\"passed\"" OR
   NOT summary MATCHES "\"full_suite\":false" OR
   NOT summary MATCHES "\"sha256\":\"${known}\"")
  message(FATAL_ERROR "Known digest or explicit non-108 coverage missing: ${summary}")
endif()
if(summary MATCHES "${WORK}")
  message(FATAL_ERROR "Public summary disclosed an absolute path")
endif()

execute_process(COMMAND "${CHILD}" unused "${WORK}/corpus/fixture.rpy" "${WORK}/known.rows"
  RESULT_VARIABLE result OUTPUT_QUIET)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Could not materialize independent known row fixture")
endif()
execute_process(COMMAND "${SUITE}" --rows "${WORK}/known.rows" --schema "${schema}"
  --expected-rows 1 --expected-sha256 "${known}"
  RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT result EQUAL 0 OR NOT output MATCHES "\"mode\":\"fingerprint\"")
  message(FATAL_ERROR "Independent known hash failed: ${output}${error}")
endif()
execute_process(COMMAND "${SUITE}" --rows "${WORK}/known.rows" --schema "${schema}"
  --expected-sha256 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
  RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "trace_fingerprint_mismatch")
  message(FATAL_ERROR "Incorrect standalone expected hash was accepted")
endif()

# Native replay fast-forward performs multiple calculation rows in one API step.
# This second independently generated known digest covers calc indexes 1 and 2.
make_fixture(fast-forward)
set(fast_hash "344019865d1c67f3f82f77ee042d422c8326e53f09817a571b9cf89f3fd93fde")
string(REPLACE "\"calculationFrames\":1" "\"calculationFrames\":2" fast_entry "${entry}")
string(REPLACE "${known}" "${fast_hash}" fast_entry "${fast_entry}")
file(WRITE "${WORK}/manifest.json" "{\"version\":1,\"fixtures\":[${fast_entry}]}")
run_suite(fast-forward TRUE "verified 1/1")

# Expected count is never inferred from files on disk. The production default
# remains 108 even when a smaller test manifest happens to be self-consistent.
execute_process(COMMAND "${SUITE}" --checker "${CHILD}" --dat "${WORK}/game.dat"
  --manifest "${WORK}/manifest.json" --schema "${schema}" --corpus "${WORK}/corpus"
  --output "${WORK}/default-count" RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "fixture_count_mismatch")
  message(FATAL_ERROR "Default 108-fixture requirement was bypassed")
endif()

# A plausible replacement with 108 distinct entries still cannot claim the
# reviewed retail gate: the raw pinned manifest identity is required as well.
set(unpinned "")
foreach(index RANGE 1 108)
  string(SHA256 replay_hash "synthetic replay ${index}")
  if(index GREATER 1)
    string(APPEND unpinned ",")
  endif()
  string(APPEND unpinned "{\"id\":\"case-${index}\",\"file\":\"case-${index}.rpy\",\"sha256\":\"${replay_hash}\",\"bytes\":1,\"retailTrace\":{\"schemaVersion\":4,\"calculationFrames\":1,\"sha256\":\"${known}\"}}")
endforeach()
file(WRITE "${WORK}/manifest.json" "{\"version\":1,\"fixtures\":[${unpinned}]}")
execute_process(COMMAND "${SUITE}" --checker "${CHILD}" --dat "${WORK}/game.dat"
  --manifest "${WORK}/manifest.json" --schema "${schema}" --corpus "${WORK}/corpus"
  --output "${WORK}/mutated-108" RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "pinned_fixture_manifest_mismatch")
  message(FATAL_ERROR "Unreviewed 108-fixture manifest was accepted")
endif()

file(WRITE "${WORK}/manifest.json" "{\"version\":1,\"fixtures\":[]}")
run_suite(empty-manifest FALSE "empty_fixture_manifest")
make_fixture(good)
file(REMOVE "${WORK}/corpus/fixture.rpy")
run_suite(empty-corpus FALSE "missing_corpus_input")
make_fixture(good)
file(WRITE "${WORK}/corpus/extra.rpy" "unknown")
run_suite(unknown-input FALSE "unknown_corpus_input")
make_fixture(good)
file(WRITE "${WORK}/corpus/fixture.rpy" "evil")
run_suite(changed-input FALSE "replay_hash_mismatch")
make_fixture(good)
file(WRITE "${WORK}/corpus/fixture.rpy" "goo")
run_suite(truncated-input FALSE "replay_size_mismatch")

make_fixture(good)
file(WRITE "${WORK}/manifest.json" "{\"version\":1,\"fixtures\":[${entry},${entry}]}")
execute_process(COMMAND "${SUITE}" --checker "${CHILD}" --dat "${WORK}/game.dat"
  --manifest "${WORK}/manifest.json" --schema "${schema}" --corpus "${WORK}/corpus"
  --output "${WORK}/duplicate" --expected-count 2 RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "duplicate_fixture_id")
  message(FATAL_ERROR "Duplicate fixture was accepted")
endif()
make_fixture(good)
string(REPLACE "\"calculationFrames\":1" "\"calculationFrames\":0" invalid "${entry}")
file(WRITE "${WORK}/manifest.json" "{\"version\":1,\"fixtures\":[${invalid}]}")
run_suite(zero-expected-rows FALSE "invalid_expected_rows")
file(WRITE "${WORK}/manifest.json" "{\"version\":1,\"fixtures\":[{\"id\":\"fixture\"}]}")
run_suite(incomplete-fixture FALSE "missing_json_field")
file(WRITE "${WORK}/manifest.json" "{\"version\":1,\"fixtures\":[")
run_suite(malformed-manifest FALSE "invalid_json")

make_fixture(good)
run_suite(unknown-selection FALSE "unknown_case_selection" --case absent)
file(MAKE_DIRECTORY "${WORK}/stale")
file(WRITE "${WORK}/stale/fixture.rows" "retained data must not be accepted")
run_suite(stale FALSE "output_directory_already_exists")
if(NOT EXISTS "${WORK}/stale/fixture.rows")
  message(FATAL_ERROR "Rejected stale output directory was modified")
endif()

foreach(pair IN ITEMS
    "empty|empty_capture" "truncated|truncated_capture_row" "missing|capture_missing"
    "hash|trace_fingerprint_mismatch" "exit-error|child_process_failed"
    "timeout|child_timeout" "unfinished|replay_not_naturally_finished"
    "unnatural|replay_not_naturally_finished" "duplicate-json|invalid_json"
    "malformed-json|invalid_json" "runtime-error|child_reported_error"
    "budget|replay_not_naturally_finished" "overbudget|invalid_step_count")
  string(REPLACE "|" ";" parts "${pair}")
  list(GET parts 0 mode)
  list(GET parts 1 diagnostic)
  make_fixture("${mode}")
  run_suite("child-${mode}" FALSE "${diagnostic}")
endforeach()

# Both completed children must appear exactly once, and selected/full evidence
# remains explicit when two workers are used.
make_fixture(good)
file(WRITE "${WORK}/corpus/second.rpy" "good second")
file(SHA256 "${WORK}/corpus/second.rpy" second_hash)
file(SIZE "${WORK}/corpus/second.rpy" second_bytes)
set(second "{\"id\":\"second\",\"file\":\"second.rpy\",\"sha256\":\"${second_hash}\",\"bytes\":${second_bytes},\"retailTrace\":{\"schemaVersion\":4,\"calculationFrames\":1,\"sha256\":\"${known}\"}}")
file(WRITE "${WORK}/manifest.json" "{\"version\":1,\"fixtures\":[${entry},${second}]}")
execute_process(COMMAND "${SUITE}" --checker "${CHILD}" --dat "${WORK}/game.dat"
  --manifest "${WORK}/manifest.json" --schema "${schema}" --corpus "${WORK}/corpus"
  --output "${WORK}/parallel" --expected-count 2 --jobs 2 --timeout-seconds 1
  RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 8)
if(NOT result EQUAL 0 OR NOT output MATCHES "verified 2/2")
  message(FATAL_ERROR "Parallel full selection failed: ${output}${error}")
endif()
file(READ "${WORK}/parallel/summary.json" summary)
if(NOT summary MATCHES "\"passed\":2" OR NOT summary MATCHES "\"full_suite\":false")
  message(FATAL_ERROR "Parallel coverage report is ambiguous: ${summary}")
endif()
execute_process(COMMAND "${SUITE}" --checker "${CHILD}" --dat "${WORK}/game.dat"
  --manifest "${WORK}/manifest.json" --schema "${schema}" --corpus "${WORK}/corpus"
  --output "${WORK}/subset" --expected-count 2 --case second
  RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 8)
if(NOT result EQUAL 0 OR NOT output MATCHES "verified 1/1 selected fixtures; 2 manifest fixtures")
  message(FATAL_ERROR "Explicit subset selection failed: ${output}${error}")
endif()
