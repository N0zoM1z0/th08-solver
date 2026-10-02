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
      rng_draws rng_seed graze score gauge peak_bullets trace_digest previous_trace_digest
      prefix_frame prefix_trace_digest unused_actions)
    field("${left}" "${key}" a)
    field("${right}" "${key}" b)
    if(NOT a STREQUAL b)
      message(FATAL_ERROR "Fresh replay diverged at ${key}: ${a} vs ${b}")
    endif()
  endforeach()
  string(REGEX MATCH "\"collision\":\\{[^}]+\\}" a "${left}")
  string(REGEX MATCH "\"collision\":\\{[^}]+\\}" b "${right}")
  if(NOT a OR NOT a STREQUAL b)
    message(FATAL_ERROR "Fresh replay collision diagnostics diverged: ${a} vs ${b}")
  endif()
endfunction()

function(scene name stage spell seed strategy budget outcome exit frames)
  set(difficulty 0)
  if(ARGC GREATER 9)
    list(GET ARGN 0 difficulty)
  endif()
  set(args --stage "${stage}" --difficulty "${difficulty}" --seed "${seed}" --frames "${budget}")
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
scene(stage6b-reactive 6b -1 0 reactive 50000 collision 2 854)
# ID85 rotates and translates live pooled lasers from ECL. Preserve the static
# forecast failure, then require the isolated observed rigid-motion profile.
scene(spell85-pooled-laser-baseline 4b 85 0 hazard-reactive 1000 collision 2 631)
file(READ "${WORK}/spell85-pooled-laser-baseline.json" spell85)
foreach(pair IN ITEMS "laser_slot;7" "laser_hitbox_call;4")
  list(GET pair 0 key)
  list(GET pair 1 expected)
  field("${spell85}" "${key}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "ID85 changed pooled-laser source: ${spell85}")
  endif()
endforeach()
foreach(seed IN ITEMS 0 1 65535)
  scene(spell85-portfolio-seed${seed} 4b 85 ${seed} spell-portfolio 4000 complete 0 2792)
  file(READ "${WORK}/spell85-portfolio-seed${seed}.json" report)
  field("${report}" policy_profile_last profile)
  field("${report}" policy_rigid_laser_paths rigid_paths)
  if(NOT profile STREQUAL "\"id85-rigid-laser-motion\"" OR NOT rigid_paths GREATER 0)
    message(FATAL_ERROR "ID85 did not exercise its pooled-laser motion model: ${report}")
  endif()
endforeach()
# Preserve the unadapted ID89 failure: its rotating beam is an ECL-owned
# CalcLaserHitbox call rather than a BulletManager laser.
scene(spell89-direct-laser-baseline 4b 89 0 hazard-reactive 1000 collision 2 393)
file(READ "${WORK}/spell89-direct-laser-baseline.json" spell89)
foreach(pair IN ITEMS "laser_slot;-1" "laser_hitbox_call;0")
  list(GET pair 0 key)
  list(GET pair 1 expected)
  field("${spell89}" "${key}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "ID89 direct-laser ${key} changed: ${actual}")
  endif()
endforeach()
run("${EXECUTABLE}" "${WORK}/spell89-direct-laser-diagnostic.json" 2
  --stage 4b --spell-id 89 --difficulty 0 --seed 0 --frames 1000
  --replay "${WORK}/spell89-direct-laser-baseline.actions" --trace "${WORK}/spell89.tsv")
file(READ "${WORK}/spell89-direct-laser-diagnostic.json" spell89_diagnostic)
agree("${spell89}" "${spell89_diagnostic}")
file(STRINGS "${WORK}/spell89.tsv" direct_hitbox
  REGEX "^393.*after.*laser_hitbox.*-1")
if(NOT direct_hitbox)
  message(FATAL_ERROR "ID89 direct ECL hitbox was absent from the diagnostic trace")
endif()
foreach(seed IN ITEMS 0 1 65535)
  scene(spell89-portfolio-seed${seed} 4b 89 ${seed} spell-portfolio 4000 complete 0 3162)
  file(READ "${WORK}/spell89-portfolio-seed${seed}.json" report)
  field("${report}" policy_profile_last profile)
  field("${report}" policy_direct_laser_constrained_decisions constrained)
  if(NOT profile STREQUAL "\"id89-direct-ecl-laser\"" OR NOT constrained GREATER 0)
    message(FATAL_ERROR "ID89 did not exercise its ECL candidate constraint: ${report}")
  endif()
endforeach()
# Distinct spell profiles guard the measured portfolio boundary. ID195 needs
# vector acceleration; ID199 deliberately retains constant-velocity ranking.
scene(spell193-portfolio extra 193 0 spell-portfolio 5000 complete 0 3692 4)
scene(spell195-portfolio extra 195 0 spell-portfolio 6000 complete 0 4712 4)
scene(spell199-portfolio extra 199 0 spell-portfolio 5000 complete 0 4292 4)
foreach(pair IN ITEMS "spell193-portfolio;source-vector-ranking"
                      "spell195-portfolio;source-vector-ranking"
                      "spell199-portfolio;id199-linear-ranking")
  list(GET pair 0 name)
  list(GET pair 1 expected)
  file(READ "${WORK}/${name}.json" report)
  field("${report}" policy_profile_last actual)
  if(NOT actual STREQUAL "\"${expected}\"")
    message(FATAL_ERROR "${name}: selected ${actual}, expected ${expected}")
  endif()
endforeach()

# Diagnostics must observe the existing failure without changing its trajectory.
set(stage6_tape "${WORK}/stage6b-reactive.actions")
file(READ "${WORK}/stage6b-reactive.json" baseline)
field("${baseline}" trace_digest baseline_digest)
if(NOT baseline_digest STREQUAL "\"4060221407534777929\"")
  message(FATAL_ERROR "The preserved Stage 6b baseline changed")
endif()
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

file(WRITE "${WORK}/illegal.actions" "32768\n")
run("${EXECUTABLE}" "${WORK}/illegal.json" 1 --replay "${WORK}/illegal.actions")
file(READ "${WORK}/stage1-budget.actions" prefix)
file(WRITE "${WORK}/excess.actions" "${prefix}4\n")
run("${EXECUTABLE}" "${WORK}/excess.json" 1 --frames 10 --replay "${WORK}/excess.actions")
# Explicit diagnostic suffix truncation stays a frame-limit failure and reports
# the unused input. Strict replay above still rejects the same tape by default.
run("${EXECUTABLE}" "${WORK}/bounded-prefix.json" 2 --frames 10
  --replay "${WORK}/excess.actions" --allow-unused-actions 1)
file(READ "${WORK}/bounded-prefix.json" bounded)
field("${bounded}" unused_actions unused)
if(NOT unused STREQUAL "1")
  message(FATAL_ERROR "Unused diagnostic suffix was not recorded")
endif()
execute_process(COMMAND "${PROBE_EXECUTABLE}" --executable "${EXECUTABLE}"
  --dat "${DAT}" --actions "${stage6_tape}" --frame 855 --output-dir "${WORK}/invalid-probe"
  RESULT_VARIABLE invalid_probe OUTPUT_QUIET ERROR_QUIET)
if(NOT invalid_probe STREQUAL "1")
  message(FATAL_ERROR "An out-of-tape intervention was accepted")
endif()
# An engine practice wrapper can choose another raw ID for an invalid stage/ID pair.
run("${EXECUTABLE}" "${WORK}/wrong-checkpoint.json" 1
  --stage 6b --spell-id 178 --strategy reactive --frames 2000)

string(JOIN ",\n" records_json ${records})
string(JOIN ",\n" probes_json ${probes})
set(probe_comparison_checked false)
if(COMPARE_EXECUTABLE)
  set(probe_comparison_checked true)
endif()
cmake_host_system_information(RESULT host_cpu QUERY PROCESSOR_DESCRIPTION)
cmake_host_system_information(RESULT host_platform QUERY OS_PLATFORM)
file(WRITE "${WORK}/summary.json"
  "{\"producer\":\"tests/headless_real_data.cmake + th08_headless + th08_headless_probe\",\"host_processor\":\"${host_cpu}\",\"host_system\":\"${CMAKE_HOST_SYSTEM_NAME}\",\"host_architecture\":\"${host_platform}\",\"runs\":[\n${records_json}\n],\"probes\":[\n${probes_json}\n],\"illegal_input_rejected\":true,\"excess_tape_rejected\":true,\"wrong_checkpoint_rejected\":true,\"invalid_probe_rejected\":true,\"diagnostics_preserve_trajectory\":true,\"probe_comparison_checked\":${probe_comparison_checked}}\n")
