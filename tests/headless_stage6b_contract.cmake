# Cross-environment v1 boundary contract. The old digest mixes host float bits,
# but its report did not pin solver build, libm or CPU. Keep it as provenance,
# not an unconditional promise of bit identity across numerical environments.
function(stage6b_semantic_contract report metadata)
  foreach(pair IN ITEMS
      "source_revision;\"861bec908b84fa4658382d7526e5a0075f520846\""
      "dat_sha256;\"9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb\""
      "profile;\"native-headless-float32\"" "stage;\"6b\""
      "requested_spell_id;-1" "difficulty;0" "seed;0" "frame_budget;50000"
      "outcome;\"collision\"" "frames;854" "first_hit;854" "spell_id;0"
      "spell_start;0" "deaths;0" "player_state;2" "rng_draws;84" "rng_seed;54466"
      "graze;7" "score;15458" "gauge;0" "peak_bullets;452" "kind;\"bullet\"")
    list(GET pair 0 key)
    list(GET pair 1 expected)
    field("${report}" "${key}" actual)
    if(NOT actual STREQUAL expected)
      message(FATAL_ERROR "stage6b-semantic-v1 changed ${key}: ${actual}, expected ${expected}")
    endif()
  endforeach()
  # Preserve all source collision endpoints, velocity, transforms and latch data.
  # The native JSON emitter's nine-digit float strings round-trip its float32s.
  set(expected "\"collision\":{\"kind\":\"bullet\",\"frame\":854,\"bullet_slot\":664,\"player_bounds\":[373.174988,431.174988,374.825012,432.825012],\"hazard_bounds\":[371.920898,428.774048,375.920898,432.774048],\"vx\":0.907954931,\"vy\":1.78202629,\"active_transforms\":0,\"laser_slot\":-1,\"laser_hitbox_call\":-1,\"laser_center\":[0,0],\"laser_size\":[0,0],\"laser_origin\":[0,0],\"laser_angle\":0,\"movement_input\":133,\"sampled_input\":4165}")
  string(REGEX MATCH "\"collision\":\\{[^}]+\\}" collision "${report}")
  if(NOT collision STREQUAL expected)
    message(FATAL_ERROR "stage6b-semantic-v1 changed source collision geometry or input latch")
  endif()
  field("${report}" trace_digest digest)
  set(historical_match false)
  if(digest STREQUAL "\"4060221407534777929\"")
    set(historical_match true)
  endif()
  set(${metadata}
    "{\"contract\":\"stage6b-semantic-v1\",\"historical_trace_digest\":\"4060221407534777929\",\"observed_trace_digest\":${digest},\"historical_digest_match\":${historical_match},\"numerical_profile_match\":\"unverified\"}"
    PARENT_SCOPE)
endfunction()
