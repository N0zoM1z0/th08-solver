# Opt-in proprietary-data coverage; no asset extraction or window is involved.
# Each launch owns fresh process globals and an isolated configuration directory.
file(MAKE_DIRECTORY "${WORK}")
get_filename_component(DAT "${DAT}" ABSOLUTE)

function(field report key result)
  string(REGEX MATCH "\"${key}\":[ ]*(\"[^\"]*\"|-?[0-9]+)" match "${report}")
  if(NOT match)
    message(FATAL_ERROR "Missing report field ${key}")
  endif()
  set(${result} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()

function(run exe report expected)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E env --unset=DISPLAY --unset=WAYLAND_DISPLAY
    "${exe}" --dat "${DAT}" --output "${report}" ${ARGN}
    RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
  if(NOT result STREQUAL "${expected}")
    message(FATAL_ERROR "${report}: expected exit ${expected}, got ${result}: ${stdout} ${stderr}")
  endif()
endfunction()

function(agree left right)
  # Compare execution feedback and the frame-by-frame projection, not timings.
  foreach(key IN ITEMS source_revision profile dat_sha256 stage requested_spell_id difficulty
      seed frame_budget outcome frames spell_id spell_start first_hit deaths player_state
      rng_draws rng_seed graze score gauge peak_bullets trace_digest)
    field("${left}" "${key}" a)
    field("${right}" "${key}" b)
    if(NOT a STREQUAL b)
      message(FATAL_ERROR "Fresh replay diverged at ${key}: ${a} vs ${b}")
    endif()
  endforeach()
endfunction()

function(scene name stage spell seed strategy budget outcome exit frames)
  set(args --stage "${stage}" --difficulty 0 --seed "${seed}" --frames "${budget}")
  if(NOT spell LESS 0)
    list(APPEND args --spell-id "${spell}")
  endif()
  set(tape "${WORK}/${name}.actions")
  run("${EXECUTABLE}" "${WORK}/${name}.json" "${exit}"
    ${args} --strategy "${strategy}" --actions "${tape}")
  file(READ "${WORK}/${name}.json" original)
  field("${original}" outcome actual_outcome)
  field("${original}" frames actual_frames)
  if(NOT actual_outcome STREQUAL "\"${outcome}\"" OR NOT actual_frames STREQUAL "${frames}")
    message(FATAL_ERROR "${name}: changed full-scene boundary: ${original}")
  endif()
  run("${EXECUTABLE}" "${WORK}/${name}-replay.json" "${exit}"
    ${args} --replay "${tape}")
  file(READ "${WORK}/${name}-replay.json" replay)
  agree("${original}" "${replay}")
  set(record "{\"case\":\"${name}\",\"execution\":${original},\"replay\":${replay}")
  if(COMPARE_EXECUTABLE)
    run("${COMPARE_EXECUTABLE}" "${WORK}/${name}-comparison.json" "${exit}"
      ${args} --replay "${tape}")
    file(READ "${WORK}/${name}-comparison.json" comparison)
    agree("${original}" "${comparison}")
    string(APPEND record ",\"comparison_replay\":${comparison}")
  endif()
  string(APPEND record "}")
  set(records ${records} "${record}" PARENT_SCOPE)
  message(STATUS "${name}: ${outcome}, ${frames} frames; fresh replay agrees")
endfunction()

scene(stage1 1 -1 0 reactive 30000 complete 0 22176)
foreach(seed IN ITEMS 0 1 65535)
  scene(spell179-seed${seed} 6b 179 ${seed} reactive 2000 complete 0 1292)
endforeach()
# Preserve a genuine bad strategy and a bounded run as failures, including replay.
scene(spell179-stationary 6b 179 0 stationary 2000 collision 2 382)
scene(stage1-budget 1 -1 0 reactive 10 frame_limit 2 10)

file(WRITE "${WORK}/illegal.actions" "32768\n")
run("${EXECUTABLE}" "${WORK}/illegal.json" 1 --replay "${WORK}/illegal.actions")
file(READ "${WORK}/stage1-budget.actions" prefix)
file(WRITE "${WORK}/excess.actions" "${prefix}4\n")
run("${EXECUTABLE}" "${WORK}/excess.json" 1 --frames 10 --replay "${WORK}/excess.actions")
# An engine practice wrapper can choose another raw ID for an invalid stage/ID pair.
run("${EXECUTABLE}" "${WORK}/wrong-checkpoint.json" 1
  --stage 6b --spell-id 178 --strategy reactive --frames 2000)

string(JOIN ",\n" records_json ${records})
cmake_host_system_information(RESULT host_cpu QUERY PROCESSOR_DESCRIPTION)
cmake_host_system_information(RESULT host_platform QUERY OS_PLATFORM)
file(WRITE "${WORK}/summary.json"
  "{\"producer\":\"tests/headless_real_data.cmake + th08_headless\",\"host_processor\":\"${host_cpu}\",\"host_system\":\"${CMAKE_HOST_SYSTEM_NAME}\",\"host_architecture\":\"${host_platform}\",\"runs\":[\n${records_json}\n],\"illegal_input_rejected\":true,\"excess_tape_rejected\":true,\"wrong_checkpoint_rejected\":true}\n")
