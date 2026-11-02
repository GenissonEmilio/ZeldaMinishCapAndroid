# Adapt the pinned core's two cart lookups, keeping the engine checkout intact.
# Both normal dispatch and static-coverage probes must select the same corpus.
set(minish_arm "${GBARECOMP_ROOT}/src/armv4t/runtime_arm.cpp")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${minish_arm}")
file(READ "${minish_arm}" arm_source)
minish_replace_once(arm_source "#include \"runtime_arm.h\""
    "#include \"runtime_arm.h\"\n#include \"rom_regions.h\"")
minish_replace_once(arm_source
    "lookup_in(kDispatchTable, kDispatchTableLen, pc, thumb)"
    "lookup_in(minish::cart_dispatch_table(), minish::cart_dispatch_count(), pc, thumb)")
minish_replace_once(arm_source
    "lookup_in(kDispatchTable, kDispatchTableLen, a, thumb != 0)"
    "lookup_in(minish::cart_dispatch_table(), minish::cart_dispatch_count(), a, thumb != 0)")
set(staged "${CMAKE_CURRENT_BINARY_DIR}/minish_renderer/arm.cpp")
file(WRITE "${staged}.tmp" "${arm_source}")
configure_file("${staged}.tmp" "${staged}" COPYONLY)
get_target_property(arm_sources gbarecomp_armv4t SOURCES)
list(REMOVE_ITEM arm_sources src/armv4t/runtime_arm.cpp "${minish_arm}")
set_property(TARGET gbarecomp_armv4t PROPERTY SOURCES "${arm_sources}")
target_sources(gbarecomp_armv4t PRIVATE "${staged}")
target_include_directories(gbarecomp_armv4t PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

foreach(region usa eu)
    if(region STREQUAL "usa")
        set(dir "${CMAKE_CURRENT_SOURCE_DIR}/generated")
        set(prefix "")
    else()
        set(dir "${CMAKE_CURRENT_SOURCE_DIR}/generated/eu")
        set(prefix "eu_")
    endif()
    set_property(SOURCE "${dir}/${prefix}symbol_map.cpp" APPEND PROPERTY COMPILE_DEFINITIONS
        "gba_symbol_register_cart=minish_${region}_symbols")
    set_property(SOURCE "${dir}/${prefix}data_symbol_map.cpp" APPEND PROPERTY COMPILE_DEFINITIONS
        "gba_data_symbol_register_cart=minish_${region}_data")
    set_property(SOURCE "${dir}/${prefix}dispatch_table.cpp" APPEND PROPERTY COMPILE_DEFINITIONS
        "runtime_register_native_ram_templates=minish_${region}_ram")
endforeach()
