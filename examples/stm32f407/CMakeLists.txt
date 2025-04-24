cmake_minimum_required(VERSION 3.28)
project(micro_bootloader_stm32f407_example CXX C ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/helpers.cmake)

include(CTest)
enable_testing()

add_library(config_base INTERFACE)
add_library(bootloader_core OBJECT EXCLUDE_FROM_ALL)
add_library(test_base OBJECT)

if(TEST_ON_TARGET)
# running tests on target with gdb
set(CMAKE_CROSSCOMPILING_EMULATOR ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/cmake/run_openocd_script.cmake)
target_sources(test_base PRIVATE sources/startup.S sources/startup.cpp)
target_compile_definitions(test_base PRIVATE STARTUP_WITH_SEMIHOSTING BOOT_WITH_INIT_ARRAY)
target_compile_options(test_base PUBLIC -nostartfiles)
target_link_options(test_base PUBLIC -nostartfiles)
else()
# running the tests off target with qemu
set(CMAKE_CROSSCOMPILING_EMULATOR
    qemu-system-arm
    -cpu cortex-m4 -machine mps2-an386
    -monitor none -nographic
    -serial null -semihosting
    -kernel # command to run here
)
target_sources(test_base PRIVATE sources/startup_qemu.S)
endif()


add_library(Bootloader::config_base ALIAS config_base)
add_library(Bootloader::core_impl   ALIAS bootloader_core)
add_library(Bootloader::test_base   ALIAS test_base)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../source ${CMAKE_CURRENT_BINARY_DIR}/bootloader)
add_subdirectory(cube)
add_subdirectory(sources)

target_link_options(bootloader PRIVATE
    -T${CMAKE_CURRENT_BINARY_DIR}/sources/memory_spec.ld
    -T${CMAKE_CURRENT_LIST_DIR}/sources/region_alias.ld
    -T${CMAKE_CURRENT_LIST_DIR}/sources/section_mapping.ld
    -eReset_Handler
    -Wl,--print-memory-usage
)
get_target_property(link_dependencies bootloader LINK_DEPENDS)
list(FILTER link_dependencies EXCLUDE REGEX "link_dependencies-NOTFOUND")
list(APPEND link_dependencies
    ${CMAKE_CURRENT_BINARY_DIR}/sources/memory_spec.ld
    ${CMAKE_CURRENT_LIST_DIR}/sources/region_alias.ld
    ${CMAKE_CURRENT_LIST_DIR}/sources/section_mapping.ld
)
set_target_properties(bootloader PROPERTIES LINK_DEPENDS "${link_dependencies}")
set_target_properties(bootloader PROPERTIES ADDITIONAL_CLEAN_FILES $<TARGET_FILE_BASE_NAME:bootloader>.map)

# Flags to qemu emulation
target_link_libraries(test_base PRIVATE cube)
target_link_options(test_base PUBLIC
    --specs=nano.specs -Wl,-lc
    -specs=rdimon.specs -Wl,-lrdimon
    $<IF:${TEST_ON_TARGET},
        -T${CMAKE_CURRENT_BINARY_DIR}/sources/memory_spec.ld
        -T${CMAKE_CURRENT_LIST_DIR}/sources/region_alias_dev_test.ld
        -eReset_Handler
        ,
        -T${CMAKE_CURRENT_LIST_DIR}/sources/memory_spec_qemu.ld
        -T${CMAKE_CURRENT_LIST_DIR}/sources/region_alias_qemu.ld
        -e_start
    >
    -T${CMAKE_CURRENT_LIST_DIR}/sources/section_mapping.ld
    -Wl,--print-memory-usage
)
get_target_property(link_dependencies test_base LINK_DEPENDS)
list(FILTER link_dependencies EXCLUDE REGEX "link_dependencies-NOTFOUND")
list(APPEND link_dependencies
    $<IF:${TEST_ON_TARGET},
        ${CMAKE_CURRENT_BINARY_DIR}/sources/memory_spec.ld
        ${CMAKE_CURRENT_LIST_DIR}/sources/region_alias_dev_test.ld
        ,
        ${CMAKE_CURRENT_LIST_DIR}/sources/memory_spec_qemu.ld
        ${CMAKE_CURRENT_LIST_DIR}/sources/region_alias_qemu.ld
    >
)
set_target_properties(test_base PROPERTIES INTERFACE_LINK_DEPENDS "${link_dependencies}")

_get_all_cmake_targets(all_targets ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "all targets : ${all_targets}")
foreach(target ${all_targets})
    get_target_property(target_type ${target} TYPE)
    if(${target_type} MATCHES "EXECUTABLE")
    message(STATUS "target ${target} ${target_type}")
        target_link_options(${target} PRIVATE
        -Wl,-Map=${target}.map)
    endif()
endforeach()
