include("${CMAKE_CURRENT_LIST_DIR}/headless_stage6b_contract.cmake")

# Diagnostics must observe the existing failure without changing its trajectory.
set(stage6_tape "${WORK}/stage6b-reactive.actions")
file(READ "${WORK}/stage6b-reactive.json" baseline)
field("${baseline}" trace_digest baseline_digest)
stage6b_semantic_contract("${baseline}" stage6b_contract)
run("${EXECUTABLE}" "${WORK}/stage6b-diagnostic.json" 2
  --stage 6b --frames 50000 --replay "${stage6_tape}" --trace "${WORK}/stage6b.tsv")
file(READ "${WORK}/stage6b-diagnostic.json" diagnostic)
agree("${baseline}" "${diagnostic}")
foreach(pair IN ITEMS "bullet_slot;664" "movement_input;133" "sampled_input;4165")
  list(GET pair 0 key)
  list(GET pair 1 expected)
  field("${diagnostic}" "${key}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "Unexpected collision ${key}: ${actual}")
  endif()
endforeach()

# Same-update input replacement is too late for movement. One update earlier,
# three leftward directions survive the actual collision boundary.
foreach(frame IN ITEMS 854 853)
  set(directory "${WORK}/probe${frame}")
  execute_process(COMMAND "${PROBE_EXECUTABLE}" --executable "${EXECUTABLE}"
    --dat "${DAT}" --stage 6b --difficulty 0 --seed 0 --actions "${stage6_tape}"
    --frame "${frame}" --through-frame 854 --output-dir "${directory}"
    RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
  if(NOT result STREQUAL "0")
    message(FATAL_ERROR "Probe ${frame} failed: ${stdout} ${stderr}")
  endif()
  file(READ "${directory}/summary.json" probe)
  list(APPEND probes "${probe}")
  foreach(branch RANGE 0 8)
    file(READ "${directory}/branch${branch}.json" report)
    field("${report}" outcome outcome)
    field("${report}" prefix_frame prefix_frame)
    math(EXPR expected_prefix "${frame} - 1")
    if(NOT prefix_frame STREQUAL "${expected_prefix}")
      message(FATAL_ERROR "Probe verified the wrong prefix boundary")
    endif()
    if((frame EQUAL 854 AND branch EQUAL 3) OR (frame EQUAL 853 AND branch EQUAL 5))
      field("${report}" trace_digest unchanged_digest)
      if(NOT unchanged_digest STREQUAL baseline_digest)
        message(FATAL_ERROR "The unchanged direction did not reproduce the source baseline")
      endif()
    endif()
    if(frame EQUAL 853 AND branch MATCHES "^(0|3|6)$")
      if(NOT outcome STREQUAL "\"frame_limit\"")
        message(FATAL_ERROR "Early leftward intervention no longer avoids the hit")
      endif()
    elseif(NOT outcome STREQUAL "\"collision\"")
      message(FATAL_ERROR "Preserved collision counterfactual changed")
    endif()
    # A second optimization level checks the exact same independent branch tape.
    if(COMPARE_EXECUTABLE)
      run("${COMPARE_EXECUTABLE}" "${directory}/branch${branch}-comparison.json" 2
        --stage 6b --frames 854 --replay "${directory}/branch${branch}.actions"
        --prefix-frame "${expected_prefix}" --allow-unused-actions 1)
      file(READ "${directory}/branch${branch}-comparison.json" comparison)
      agree("${report}" "${comparison}")
    endif()
  endforeach()
endforeach()
