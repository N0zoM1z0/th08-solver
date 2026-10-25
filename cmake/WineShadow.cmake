# Optional 32-bit observer and owned-process launcher. This is an independent
# MinGW custom build, not a cross-compiled copy of the native runtime.
find_program(TH08_MINGW_CXX i686-w64-mingw32-g++)
if(NOT TH08_MINGW_CXX)
  message(FATAL_ERROR "TH08_WINE_SHADOW requires i686-w64-mingw32-g++")
endif()
set(_wine_shadow ${CMAKE_CURRENT_SOURCE_DIR}/tools/wine-shadow)
set(_wine_output ${CMAKE_CURRENT_BINARY_DIR}/wine-shadow)
set(_wine_flags -std=c++17 -O2 -static -ffp-contract=off)
file(GLOB_RECURSE _wine_abi_headers CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/th08/src/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/th08/src/*.h)
add_custom_command(OUTPUT ${_wine_output}/launch.exe
  COMMAND ${CMAKE_COMMAND} -E make_directory ${_wine_output}
  COMMAND ${TH08_MINGW_CXX} ${_wine_flags} ${_wine_shadow}/launch.cpp
    -ladvapi32 -o ${_wine_output}/launch.new.exe
  COMMAND ${CMAKE_COMMAND} -E rename ${_wine_output}/launch.new.exe ${_wine_output}/launch.exe
  DEPENDS tools/wine-shadow/launch.cpp tools/wine-shadow/identity.hpp
  VERBATIM)
add_custom_command(OUTPUT ${_wine_output}/observer.dll
  COMMAND ${CMAKE_COMMAND} -E make_directory ${_wine_output}
  COMMAND ${TH08_MINGW_CXX} ${_wine_flags} -c ${_wine_shadow}/d3d8_timing_abi.cpp
    -o ${_wine_output}/d3d8_timing_abi.o
  COMMAND ${TH08_MINGW_CXX} ${_wine_flags} -shared -DTH08_MODERN_PORT
    -fpermissive -fms-extensions -mms-bitfields -Wno-invalid-offsetof
    -I${_wine_shadow}/compat -I${CMAKE_CURRENT_SOURCE_DIR}/third_party/th08/src
    -I${CMAKE_CURRENT_SOURCE_DIR}/include ${_wine_shadow}/observer.cpp
    -ladvapi32 -Wl,--kill-at -o ${_wine_output}/observer.new.dll
  COMMAND ${CMAKE_COMMAND} -E rename ${_wine_output}/observer.new.dll ${_wine_output}/observer.dll
  DEPENDS tools/wine-shadow/observer.cpp tools/wine-shadow/identity.hpp
    tools/wine-shadow/retail_view.hpp tools/wine-shadow/compat/d3dx8.h
    tools/wine-shadow/compat/d3dx8math.h include/th08/shadow_bridge.hpp
    include/th08/shadow_protocol.hpp include/th08/shadow_start.hpp include/th08/render_timing.hpp
    tools/wine-shadow/render_timing.hpp tools/wine-shadow/d3d8_timing_abi.cpp ${_wine_abi_headers}
  VERBATIM)
add_custom_target(th08_wine_shadow ALL
  DEPENDS ${_wine_output}/launch.exe ${_wine_output}/observer.dll)
