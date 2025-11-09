# Append current directory to CMAKE_MODULE_PATH for making device specific cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

set(TARGET_CORE "stm32h743")
include(${CMAKE_CURRENT_LIST_DIR}/generate_flash_layout_files.cmake)


#---------------------------------------------------------------------------------------
# Set compiler/linker flags
#---------------------------------------------------------------------------------------

# CPU Flags
# -mcpu=cortex-m4   generate m4 code
# -mfpu=fpv4-sp-d16 fpu
# -mfloat-abi=hard  float abi
# -mthumb           Generat thumb instructions.
# -mabi=aapcs       Defines enums to be a variable sized type.
# -nostartfiles             Don't link the compilers start files
set(CPU_GEN_FLAGS "-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mabi=aapcs -nostartfiles")

# Warning Flags
# -Wall -Wextra -Wpedantic  Print all warnings
# -Werror                   Treat all warnings as errors
set(WARNING_FLAGS "-Wall -Wextra -Wpedantic -Werror")

# Object build options
# -fno-builtin              Do not use built-in functions provided by GCC.
# -ffunction-sections       Place each function item into its own section in the output file.
# -fdata-sections           Place each data item into its own section in the output file.
# -fomit-frame-pointer      Omit the frame pointer in functions that don’t need one.
set(OBJECT_GEN_FLAGS "${CPU_GEN_FLAGS} ${WARNING_FLAGS} -fno-builtin -ffunction-sections -fdata-sections -fomit-frame-pointer")

set(CMAKE_C_FLAGS_INIT   "${OBJECT_GEN_FLAGS} " CACHE INTERNAL "C Compiler options")
# -fno-rtti                 Don't output rtti for types that are not part of exception handling
# -fno-exceptions           Disable exceptions
# -fno-threadsafe-statics   The application guarantees that static objects are initialized in a thread safe maner
set(CMAKE_CXX_FLAGS_INIT "${OBJECT_GEN_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics " CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS_INIT "${OBJECT_GEN_FLAGS} -x assembler-with-cpp  -MMD -MP " CACHE INTERNAL "ASM Compiler options")


# -Wl,--gc-sections     Perform the dead code elimination.
# -nostartfiles             Don't link the compilers start files
set(CMAKE_EXE_LINKER_FLAGS_INIT " ${CPU_GEN_FLAGS} -Wl,--gc-sections" CACHE INTERNAL "Linker options")

include(${CMAKE_CURRENT_LIST_DIR}/../../toolchains/arm_none_eabi.cmake)


if(NOT TARGET linkage_bootloader)
# --------------------------BOOTLOADER------------------------------------------------------------
add_library(linkage_bootloader INTERFACE)
target_link_options(linkage_bootloader INTERFACE
        -Wl,--print-memory-usage # print memory usage
        -specs=nosys.specs # nosys specs
        -specs=nano.specs -Wl,-lc # reduced libc
        -T${CMAKE_CURRENT_LIST_DIR}/memory_ram.ld # specific ram memory allocation
        -T${CMAKE_BINARY_DIR}/memory_spec.ld # specific flash memory allocation
        -T${CMAKE_CURRENT_LIST_DIR}/region_alias_bootloader.ld # memory mapping for the bootloader
        -T${CMAKE_CURRENT_LIST_DIR}/../../toolchains/section_mapping.ld # default section placement
)
set_property(TARGET linkage_bootloader PROPERTY INTERFACE_LINK_DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/memory_ram.ld
        ${CMAKE_BINARY_DIR}/memory_spec.ld
        ${CMAKE_CURRENT_LIST_DIR}/region_alias_bootloader.ld
        ${CMAKE_CURRENT_LIST_DIR}/../../toolchains/section_mapping.ld
)
# --------------------------SEMIHOSTING-----------------------------------------------------------
add_library(linkage_semihosting INTERFACE)
target_link_options(linkage_semihosting INTERFACE
        -specs=nano.specs -Wl,-lc # reduced libc
        -specs=rdimon.specs -Wl,-lrdimon # enable semihosting
)
target_compile_definitions(linkage_semihosting INTERFACE
        STARTUP_WITH_SEMIHOSTING
        BOOT_WITH_INIT_ARRAY
)
# --------------------------TESTING---------------------------------------------------------------
add_library(linkage_test INTERFACE)
target_link_options(linkage_test INTERFACE
        -Wl,--print-memory-usage # print memory usage
        -specs=nano.specs -Wl,-lc # reduced libc
        -specs=rdimon.specs -Wl,-lrdimon # enable semihosting
        -T${CMAKE_CURRENT_LIST_DIR}/memory_ram.ld # specific ram memory allocation
        -T${CMAKE_BINARY_DIR}/memory_spec.ld # specific flash memory allocation
        -T${CMAKE_CURRENT_LIST_DIR}/region_alias_test.ld # memory mapping for the ram
        -T${CMAKE_CURRENT_LIST_DIR}/../../toolchains/section_mapping.ld # default section placement
)
set_property(TARGET linkage_test PROPERTY INTERFACE_LINK_DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/memory_ram.ld
        ${CMAKE_BINARY_DIR}/memory_spec.ld
        ${CMAKE_CURRENT_LIST_DIR}/region_alias_test.ld
        ${CMAKE_CURRENT_LIST_DIR}/../../toolchains/section_mapping.ld
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
        -T${CMAKE_CURRENT_LIST_DIR}/memory_ram.ld # specific ram memory allocation
        -T${CMAKE_BINARY_DIR}/memory_spec.ld # specific flash memory allocation
        -T${CMAKE_CURRENT_LIST_DIR}/region_alias_application.ld # memory mapping for the application
        -T${CMAKE_CURRENT_LIST_DIR}/../../toolchains/section_mapping.ld # default section placement
)
set_property(TARGET linkage_application PROPERTY INTERFACE_LINK_DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/memory_ram.ld
        ${CMAKE_BINARY_DIR}/memory_spec.ld
        ${CMAKE_CURRENT_LIST_DIR}/region_alias_application.ld
        ${CMAKE_CURRENT_LIST_DIR}/../../toolchains/section_mapping.ld
)
endif()

#---------------------------------------------------------------------------------------
# specify the unit test runner
#---------------------------------------------------------------------------------------
set(CMAKE_CROSSCOMPILING_EMULATOR ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/run_openocd_script.cmake)
