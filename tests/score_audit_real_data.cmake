cmake_minimum_required(VERSION 3.16)
file(MAKE_DIRECTORY "${WORK}")
# This real NMNB replay is a counterexample to "no miss/no bomb implies all
# captures". Requiring capture coverage must fail while natural playback passes.
execute_process(COMMAND "${CHECKER}" "${DAT}" "${REPLAY}" "${WORK}/audit.rows"
  --score-audit "${WORK}/audit.json" --require-all-captures
  RESULT_VARIABLE result OUTPUT_VARIABLE replay ERROR_VARIABLE stderr TIMEOUT 60)
if(NOT result STREQUAL "3")
  message(FATAL_ERROR "Incomplete capture coverage did not fail explicitly: ${result} ${replay} ${stderr}")
endif()
file(READ "${WORK}/audit.json" audit)
# Per-spell income excludes the interleaved stage/nonspell intervals. The two
# source contact paths have different extra-orb behavior and must stay distinct.
string(REGEX MATCHALL "\\{[^{}]*\"outcome\"[^{}]*\\}" spell_records "${audit}")
set(spell_graze_total 0)
set(previous_stage -1)
set(fast_payouts 0)
foreach(record IN LISTS spell_records)
  foreach(key IN ITEMS stage start_update end_update bonus_displayed
      point_value_at_start point_value_at_end time_orbs_at_start time_orbs_at_end point_items_at_start
      point_items_at_end score_displayed_at_start score_displayed_at_end graze_events
      extreme_youkai_graze_events extra_item_suppressed_graze_events
      ordinary_extreme_youkai_graze_events)
    string(REGEX MATCH "\"${key}\":([0-9]+)" match "${record}")
    if(match STREQUAL "")
      message(FATAL_ERROR "Missing phase boundary/contact metric ${key}: ${record}")
    endif()
    set(phase_${key} "${CMAKE_MATCH_1}")
  endforeach()
  math(EXPR distinct_contacts
    "${phase_ordinary_extreme_youkai_graze_events} + ${phase_extra_item_suppressed_graze_events}")
  if(phase_time_orbs_at_end LESS phase_time_orbs_at_start OR
     phase_point_items_at_end LESS phase_point_items_at_start OR
     phase_score_displayed_at_end LESS phase_score_displayed_at_start OR
     phase_point_value_at_end LESS phase_point_value_at_start OR
     phase_ordinary_extreme_youkai_graze_events GREATER phase_extreme_youkai_graze_events OR
     phase_extra_item_suppressed_graze_events GREATER phase_graze_events OR
     distinct_contacts GREATER phase_graze_events)
    message(FATAL_ERROR "Phase economics violates its source boundaries: ${record}")
  endif()
  if(phase_stage EQUAL previous_stage AND previous_bonus GREATER 0)
    math(EXPR gap "${phase_start_update} - ${previous_end}")
    if(gap LESS 130)
      math(EXPR settled_score "${previous_score} + ${previous_bonus}")
      if(phase_score_displayed_at_start LESS settled_score)
        message(FATAL_ERROR "A preceding capture reward was charged to the next spell: ${record}")
      endif()
      math(EXPR fast_payouts "${fast_payouts} + 1")
    endif()
  endif()
  set(previous_stage "${phase_stage}")
  set(previous_end "${phase_end_update}")
  set(previous_bonus "${phase_bonus_displayed}")
  set(previous_score "${phase_score_displayed_at_end}")
  math(EXPR spell_graze_total "${spell_graze_total} + ${phase_graze_events}")
endforeach()
if(spell_graze_total LESS 1 OR fast_payouts LESS 1)
  message(FATAL_ERROR "Phase regression missed active graze or fast reward settlement")
endif()
function(expect json key value)
  if(NOT "${json}" MATCHES "\"${key}\":${value}([,}]|$)")
    message(FATAL_ERROR "Expected ${key}=${value}: ${json}")
  endif()
endfunction()
expect("${replay}" natural true)
expect("${replay}" death_rows 0)
expect("${replay}" bomb_input_rows 0)
expect("${audit}" complete true)
expect("${audit}" nmnb true)
expect("${audit}" shot 1)
expect("${audit}" score_raw 218996507)
expect("${audit}" score_displayed 2189965070)
expect("${audit}" expected_unique 37)
expect("${audit}" encountered_unique 35)
expect("${audit}" captured_unique 33)
expect("${audit}" unsuccessful_attempts 2)
expect("${audit}" unmatched_events 0)
expect("${audit}" all_visited_stage_spells_captured false)
# Extreme-human graze increments the displayed counter by three. Count actual
# award events separately so an economic comparison does not confuse the two.
foreach(key IN ITEMS graze_events extreme_human_graze_events extreme_youkai_graze_events)
  string(REGEX MATCH "\"${key}\":([0-9]+)" match "${audit}")
  if(match STREQUAL "" OR CMAKE_MATCH_1 LESS 1)
    message(FATAL_ERROR "Missing measured graze events: ${key}")
  endif()
  set(${key} "${CMAKE_MATCH_1}")
endforeach()
math(EXPR extreme_events "${extreme_human_graze_events} + ${extreme_youkai_graze_events}")
math(EXPR minimum_counter "${graze_events} + 2 * ${extreme_human_graze_events}")
if(extreme_events GREATER graze_events OR graze_events GREATER_EQUAL 40784 OR
   minimum_counter GREATER 40784 OR spell_graze_total GREATER graze_events)
  message(FATAL_ERROR "Graze event counts disagree with the frozen40784 counter: ${audit}")
endif()
foreach(card IN ITEMS 6 10)
  if(NOT audit MATCHES "\"card\":${card},\"outcome\":\"timeout\"")
    message(FATAL_ERROR "Ordinary spell timeout ${card} was mislabeled")
  endif()
endforeach()
# Kaguya's timed Last Spells do award captures at zero remaining time. Do not
# infer ordinary timeout failure from the timer alone.
if(NOT audit MATCHES "\"card\":175,\"outcome\":\"captured\"[^}]*\"remaining_frames\":0")
  message(FATAL_ERROR "Timed Last Spell capture was mislabeled")
endif()
foreach(card IN ITEMS 13 32)
  if(NOT audit MATCHES "\"card\":${card},\"last_spell\":true")
    message(FATAL_ERROR "Skipped Last Spell ${card} disappeared from the coverage target")
  endif()
endforeach()
# Read-only auditing must preserve all 25 fields, update ordering and replay
# teardown. This hash is the original frozen story's complete natural rows.
file(SHA256 "${WORK}/audit.rows" rows)
if(NOT rows STREQUAL "17815533a8e343a54de382f108e88a89cebf184e434e57b877f0c9fbec33330e")
  message(FATAL_ERROR "Auditing changed the original natural replay rows: ${rows}")
endif()
