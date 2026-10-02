# The imported game remains a separate target; existing solver kernels keep their
# own numerical and ownership contracts. This build needs no Python or OpenGL.
find_package(PkgConfig REQUIRED)
pkg_check_modules(TH08_SDL REQUIRED IMPORTED_TARGET sdl2 SDL2_image SDL2_ttf fontconfig)
find_package(Threads REQUIRED)
set(th08_native "${CMAKE_CURRENT_SOURCE_DIR}/third_party/th08")
set(TH08_HEADLESS_OPTIMIZATION "3" CACHE STRING "Native game optimization level (0..3)")
if(NOT TH08_HEADLESS_OPTIMIZATION MATCHES "^[0-3]$")
  message(FATAL_ERROR "TH08_HEADLESS_OPTIMIZATION must be 0..3")
endif()
set(th08_generated "${CMAKE_CURRENT_BINARY_DIR}/headless-generated")
add_executable(th08_headless_i18n tools/headless_i18n.cpp)
target_compile_features(th08_headless_i18n PRIVATE cxx_std_17)
add_custom_command(OUTPUT "${th08_generated}/i18n.hpp"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${th08_generated}"
  COMMAND th08_headless_i18n "${th08_native}/config/i18n.csv" "${th08_generated}/i18n.hpp"
  DEPENDS th08_headless_i18n "${th08_native}/config/i18n.csv" VERBATIM)
# Use the upstream production translation units, including their normal globals.
set(th08_game_units
  AnmManager AsciiManager AsciiManagerBossMarker AsciiManagerGauge
  AsciiManagerGuiMode AsciiManagerScale Background BulletManager EclDependencies
  EclExIns EclGlobals EclHelpers EclManager EclOperandsFloat EclOperandsInt EclRun
  EffectManager Ending EnemyManager EnemyManagerUpdate EnemyTimeline GameManager
  Global Gui ItemManager Midi MusicRoom Player PlayerBomb ReplayManager ResultScreen
  ScoreDat ScreenEffect SoundPlayer Spellcard Supervisor TextHelper TitleScreen main
  utils zwave pbg/Lzss pbg/PbgArchive pbg/PbgFile)
set(th08_game_files)
foreach(unit IN LISTS th08_game_units)
  list(APPEND th08_game_files "${th08_native}/src/${unit}.cpp")
endforeach()
add_library(th08_native_headless STATIC ${th08_game_files}
  "${th08_generated}/i18n.hpp"
  "${th08_native}/src/modern/linux/linux_compat.cpp"
  "${th08_native}/src/modern/linux/d3dx8_compat.cpp"
  "${th08_native}/src/modern/linux/render_audit.cpp"
  "${th08_native}/src/modern/headless/d3d_null.cpp"
  "${th08_native}/src/modern/headless/runtime.cpp")
target_sources(th08_native_headless PRIVATE "${th08_native}/src/modern/headless/session.cpp")
target_compile_features(th08_native_headless PUBLIC cxx_std_17)
target_compile_definitions(th08_native_headless PUBLIC TH08_MODERN_PORT
  TH08_MODERN_LINUX TH08_PORTABLE_NATIVE_LAYOUT TH08_HEADLESS WIN32_LEAN_AND_MEAN
  _FILE_OFFSET_BITS=64 _TIME_BITS=64)
target_compile_definitions(th08_native_headless PUBLIC
  TH08_HEADLESS_OPTIMIZATION=${TH08_HEADLESS_OPTIMIZATION})
target_include_directories(th08_native_headless PUBLIC "${th08_native}/src"
  "${th08_native}/src/modern/linux/include" "${th08_generated}")
# The headless experiment is optimized native float32. Do not apply fast-math.
target_compile_options(th08_native_headless PRIVATE -O${TH08_HEADLESS_OPTIMIZATION}
  -fpermissive -fno-strict-aliasing
  -ffp-contract=off -Wno-write-strings -Wno-multichar -Wno-unknown-pragmas
  "-include${th08_native}/src/modern/linux/linux_compat.hpp")
target_link_libraries(th08_native_headless PUBLIC PkgConfig::TH08_SDL Threads::Threads dl)
add_executable(th08_headless tools/headless.cpp)
target_link_libraries(th08_headless PRIVATE th08_native_headless)
target_include_directories(th08_headless PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(th08_headless PRIVATE OpenSSL::Crypto)
target_compile_options(th08_headless PRIVATE -ffp-contract=off)
add_executable(th08_headless_probe tools/headless_probe.cpp)
target_compile_features(th08_headless_probe PRIVATE cxx_std_17)

set(TH08_HEADLESS_DAT "" CACHE FILEPATH "Private DAT for the optional full-scene regression")
set(TH08_HEADLESS_COMPARE_EXECUTABLE "" CACHE FILEPATH
  "Optional native executable built at another optimization level")
