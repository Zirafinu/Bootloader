
# Append current directory to CMAKE_MODULE_PATH for making device specific cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

set(TARGET_CORE "Simulation")

if(NOT TARGET qemu_linkage)
add_library(qemu_linkage INTERFACE)
target_link_options(qemu_linkage INTERFACE
        -specs=nano.specs -Wl,-lc # reduced libc
        -specs=rdimon.specs -Wl,-lrdimon # arm semi hosting
        -T${CMAKE_CURRENT_LIST_DIR}/memory_spec.ld # qemu target memories
        -T${CMAKE_CURRENT_LIST_DIR}/region_alias.ld # memory mapping
        -T${CMAKE_CURRENT_LIST_DIR}/../section_mapping.ld # default section placement
)
include(${CMAKE_CURRENT_LIST_DIR}/../toolchain_base_arm_none_eabi.cmake)

add_library(linkage_bootloader INTERFACE)
target_link_libraries(linkage_bootloader INTERFACE qemu_linkage)

add_library(linkage_test INTERFACE)
target_link_libraries(linkage_test INTERFACE qemu_linkage)

add_library(linkage_application INTERFACE)
target_link_libraries(linkage_application INTERFACE qemu_linkage)
endif()
#---------------------------------------------------------------------------------------
# specify the unit test runner
#---------------------------------------------------------------------------------------
set(CMAKE_CROSSCOMPILING_EMULATOR
        qemu-system-arm
        -cpu cortex-m4 -machine mps2-an386
        -monitor none -nographic
        -serial null -semihosting
        -kernel # command to run here
)
