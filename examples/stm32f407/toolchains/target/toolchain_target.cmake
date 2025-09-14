# Append current directory to CMAKE_MODULE_PATH for making device specific cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/generate_flash_layout_files.cmake)

# Object build options
# -fno-exceptions           Disable exceptions
# -nostartfiles             Don't link the compilers start files
set(ADDITIONAL_COMPILE_AND_LINK_FLAGS "-fno-exceptions -nostartfiles")

include(${CMAKE_CURRENT_LIST_DIR}/../toolchain_base_arm_none_eabi.cmake)

set(TARGET_CORE "Device")

if(NOT TARGET linkage_bootloader)
# --------------------------BOOTLOADER------------------------------------------------------------
add_library(linkage_bootloader INTERFACE)
target_link_options(linkage_bootloader INTERFACE
        -Wl,--print-memory-usage # print memory usage
        -specs=nosys.specs # nosys specs
        -specs=nano.specs -Wl,-lc # reduced libc
        -T${CMAKE_BINARY_DIR}/memory_spec.ld # specific flash memory allocation
        -T${CMAKE_CURRENT_LIST_DIR}/region_alias_bootloader.ld # memory mapping for the bootloader
        -T${CMAKE_CURRENT_LIST_DIR}/../section_mapping.ld # default section placement
)
set_property(TARGET linkage_bootloader PROPERTY INTERFACE_LINK_DEPENDS
        ${CMAKE_BINARY_DIR}/memory_spec.ld
        ${CMAKE_CURRENT_LIST_DIR}/region_alias_bootloader.ld
        ${CMAKE_CURRENT_LIST_DIR}/../section_mapping.ld
)
# --------------------------TESTING---------------------------------------------------------------
add_library(linkage_test INTERFACE)
target_link_options(linkage_test INTERFACE
        -Wl,--print-memory-usage # print memory usage
        -specs=nano.specs -Wl,-lc # reduced libc
        -specs=rdimon.specs -Wl,-lrdimon # enable semihosting
        -T${CMAKE_BINARY_DIR}/memory_spec.ld # specific flash memory allocation
        -T${CMAKE_CURRENT_LIST_DIR}/region_alias_test.ld # memory mapping for the ram
        -T${CMAKE_CURRENT_LIST_DIR}/../section_mapping.ld # default section placement
)
set_property(TARGET linkage_test PROPERTY INTERFACE_LINK_DEPENDS
        ${CMAKE_BINARY_DIR}/memory_spec.ld
        ${CMAKE_CURRENT_LIST_DIR}/region_alias_test.ld
        ${CMAKE_CURRENT_LIST_DIR}/../section_mapping.ld
)
target_compile_definitions(linkage_test INTERFACE
        STARTUP_WITH_SEMIHOSTING
        BOOT_WITH_INIT_ARRAY
)
# --------------------------APPLICATION------------------------------------------------------------
add_library(linkage_application INTERFACE)
target_link_options(linkage_application INTERFACE
        -Wl,--print-memory-usage # print memory usage
        -specs=nano.specs -Wl,-lc # reduced libc
        -T${CMAKE_BINARY_DIR}/memory_spec.ld # specific flash memory allocation
        -T${CMAKE_CURRENT_LIST_DIR}/region_alias_application.ld # memory mapping for the application
        -T${CMAKE_CURRENT_LIST_DIR}/../section_mapping.ld # default section placement
)
set_property(TARGET linkage_application PROPERTY INTERFACE_LINK_DEPENDS
        ${CMAKE_BINARY_DIR}/memory_spec.ld
        ${CMAKE_CURRENT_LIST_DIR}/region_alias_application.ld
        ${CMAKE_CURRENT_LIST_DIR}/../section_mapping.ld
)
endif()

#---------------------------------------------------------------------------------------
# specify the unit test runner
#---------------------------------------------------------------------------------------
set(CMAKE_CROSSCOMPILING_EMULATOR ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/run_openocd_script.cmake)
