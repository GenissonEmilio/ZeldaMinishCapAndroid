# Android owns packaging and the launcher; the shared core owns execution.
set(GBARECOMP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/gbarecomp" CACHE PATH "Engine")
set(GBARECOMP_GENERATED_BIOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated/open_bios" CACHE PATH "Open firmware corpus" FORCE)
set(GBARECOMP_RUNTIME_UI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/recomp-ui" CACHE PATH "Runtime UI")
set(GBARECOMP_ENABLE_MODS ON CACHE BOOL "Mod catalog" FORCE)
set(RECOMP_UI_ENABLE_MODS ON CACHE BOOL "Mod UI" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Host tests are separate" FORCE)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/dispatch_table.cpp" OR
   NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/eu/eu_dispatch_table.cpp" OR
   NOT EXISTS "${GBARECOMP_GENERATED_BIOS_DIR}/bios_dispatch_table.cpp")
    message(FATAL_ERROR "Generate USA, EU and open BIOS first: tools/android/prepare.py --rom <USA.gba> --rom <EU.gba>")
endif()
add_subdirectory("${GBARECOMP_ROOT}" gbarecomp_build EXCLUDE_FROM_ALL)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/AndroidRenderer.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/AndroidRegions.cmake")
file(GLOB MINISH_GENERATED CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/eu/*.cpp")
add_library(main SHARED src/main.cpp src/game_config.cpp src/minish_extended_view.cpp
    src/mods/minish_adaptive_view_plugin.cpp src/android_port.cpp
    src/photo_menu.cpp src/game_photo_album.cpp src/rom_regions.cpp ${MINISH_GENERATED})
set_source_files_properties(src/main.cpp PROPERTIES COMPILE_DEFINITIONS "main=MinishMain")
target_include_directories(main PRIVATE src "${GBARECOMP_ROOT}/src/armv4t"
    "${GBARECOMP_ROOT}/src/gba" "${GBARECOMP_ROOT}/src/runtime" "${GBARECOMP_ROOT}/src/debug")
target_link_libraries(main PRIVATE "-Wl,--start-group" gbarecomp_runtime gbarecomp_debug
    gbarecomp_gba gbarecomp_armv4t "-Wl,--end-group" SDL2::SDL2 log android)
target_compile_definitions(main PRIVATE GBARECOMP_DEFAULT_DEBUG_PORT=19842
    GBARECOMP_WINDOW_TITLE="MinishCap" GBARECOMP_DEFAULT_GAME_CONFIG="variants/minishcap/game.toml")
include("${GBARECOMP_RUNTIME_UI_ROOT}/recomp_ui.cmake")
recomp_target_launcher_ui(main CONSOLE gba)
set_property(TARGET main PROPERTY CXX_STANDARD 20)
# The pinned launcher uses UTF-8 literals as const char* (C++17 contract).
# Keep its sources at that standard without weakening the game's C++20 core.
get_target_property(MINISH_ALL_SOURCES main SOURCES)
foreach(source IN LISTS MINISH_ALL_SOURCES)
    if(source MATCHES "/recomp-ui/.*\\.cpp$")
        set_property(SOURCE "${source}" APPEND PROPERTY COMPILE_OPTIONS "-std=c++17")
    endif()
endforeach()
target_link_options(main PRIVATE "-Wl,-z,max-page-size=16384")
