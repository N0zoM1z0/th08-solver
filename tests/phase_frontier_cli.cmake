cmake_minimum_required(VERSION 3.16)
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
string(REPEAT "a" 64 request)
string(REPEAT "1" 64 world1)
string(REPEAT "2" 64 world2)
set(header "id\tboundary\tfamily\trequest\tcheckpoint\tscore\tcompleted\tno_miss\tbombs\n")
set(high "high\textra203\tfeedback\t${request}\t${world1}\t100\t1\t1\t0")
set(low "low\textra203\tshoot\t${request}\t${world2}\t1\t1\t1\t0")
file(WRITE "${WORK}/input.tsv" "${header}${high}\n${low}\n")
execute_process(COMMAND "${SELECTOR}" "${WORK}/input.tsv" 2 0 "${WORK}/output.tsv"
  RESULT_VARIABLE result ERROR_VARIABLE error)
file(READ "${WORK}/output.tsv" selected)
if(NOT result STREQUAL "0" OR NOT selected STREQUAL
    "id\tcheckpoint\tscore\nhigh\t${world1}\t100\nlow\t${world2}\t1\n")
  message(FATAL_ERROR "CLI lost distinct strategy worlds: ${result} ${error} ${selected}")
endif()
# A malformed evidence row must fail before opening an existing frontier. This
# guards the campaign's durable selection against partially parsed input.
foreach(bad IN ITEMS "${high}\t" "${high}\n${high}")
  file(WRITE "${WORK}/bad.tsv" "${header}${bad}\n")
  execute_process(COMMAND "${SELECTOR}" "${WORK}/bad.tsv" 2 0 "${WORK}/output.tsv"
    RESULT_VARIABLE rejected ERROR_VARIABLE error)
  file(READ "${WORK}/output.tsv" retained)
  if(NOT rejected STREQUAL "1" OR NOT retained STREQUAL selected)
    message(FATAL_ERROR "Rejected archive replaced the previous frontier: ${rejected} ${error}")
  endif()
endforeach()
