cmake_minimum_required(VERSION 3.28)
project(micro_bootloader_stm32f407_example CXX C ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
enable_testing()

# running the tests off target with qemu
set(CMAKE_CROSSCOMPILING_EMULATOR
    qemu-system-arm
    -cpu cortex-m4 -machine mps2-an386
    -monitor none -nographic
    -serial null -semihosting
    -kernel # command to run here
)
# running tests on target with gdb

add_library(config_base INTERFACE)
add_library(Bootloader::config_base ALIAS config_base)

add_library(bootloader_core OBJECT EXCLUDE_FROM_ALL)
add_library(Bootloader::core_impl ALIAS bootloader_core)

add_library(test_base INTERFACE)
add_library(Bootloader::test_base ALIAS test_base)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../source ${CMAKE_CURRENT_BINARY_DIR}/bootloader)
add_subdirectory(cube)
add_subdirectory(sources)

target_link_options(bootloader PRIVATE
    -T${CMAKE_CURRENT_BINARY_DIR}/sources/memory_spec.ld
    -T${CMAKE_CURRENT_LIST_DIR}/sources/region_alias.ld
    -T${CMAKE_CURRENT_LIST_DIR}/sources/section_mapping.ld
    -eReset_Handler
    -Wl,--print-memory-usage
    -Wl,-Map=$<TARGET_FILE_BASE_NAME:bootloader>.map
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
target_link_options(test_base INTERFACE
    --specs=nosys.specs
    --specs=nano.specs -Wl,-lc
    -specs=rdimon.specs -Wl,-lrdimon
    -T${CMAKE_CURRENT_LIST_DIR}/sources/memory_spec_qemu.ld
    -T${CMAKE_CURRENT_LIST_DIR}/sources/region_alias_qemu.ld
    -T${CMAKE_CURRENT_LIST_DIR}/sources/section_mapping.ld
)
get_target_property(link_dependencies test_base LINK_DEPENDS)
list(FILTER link_dependencies EXCLUDE REGEX "link_dependencies-NOTFOUND")
list(APPEND link_dependencies
    ${CMAKE_CURRENT_LIST_DIR}/sources/memory_spec_qemu.ld
    ${CMAKE_CURRENT_LIST_DIR}/sources/region_alias_qemu.ld
    ${CMAKE_CURRENT_LIST_DIR}/sources/section_mapping.ld
)
set_target_properties(test_base PROPERTIES INTERFACE_LINK_DEPENDS "${link_dependencies}")
