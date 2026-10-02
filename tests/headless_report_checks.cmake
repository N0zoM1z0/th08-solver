function(field report key result)
  string(REGEX MATCH "\"${key}\":[ ]*(\"[^\"]*\"|-?[0-9]+)([,} \r\n\t]|$)" match "${report}")
  if(NOT match)
    message(FATAL_ERROR "Missing report field ${key}")
  endif()
  set(${result} "${CMAKE_MATCH_1}" PARENT_SCOPE)
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
