set(TARGET_CORE "")

# Append current directory to CMAKE_MODULE_PATH for making device specific cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

#---------------------------------------------------------------------------------------
# Set compiler/linker flags
#---------------------------------------------------------------------------------------

# CPU Flags
# -mcpu=cortex-m4   generate m4 code
# -mfpu=fpv4-sp-d16 fpu
# -mfloat-abi=hard  float abi
# -mthumb           Generat thumb instructions.
# -mabi=aapcs       Defines enums to be a variable sized type.
# -fno-exceptions           Disable exceptions
set(CPU_GEN_FLAGS "-mcpu=cortex-m4 -mthumb -mabi=aapcs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fno-exceptions")

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
# The application guarantees that static objects are initialized in a thread safe maner
set(CMAKE_CXX_FLAGS_INIT "${OBJECT_GEN_FLAGS} -fno-threadsafe-statics " CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS_INIT "${OBJECT_GEN_FLAGS} -x assembler-with-cpp  -MMD -MP " CACHE INTERNAL "ASM Compiler options")


# -Wl,--gc-sections     Perform the dead code elimination.
# -fno-exceptions           Disable exceptions
set(CMAKE_EXE_LINKER_FLAGS_INIT " -Wl,--gc-sections -fno-exceptions" CACHE INTERNAL "Linker options")

include(${CMAKE_CURRENT_LIST_DIR}/../../toolchains/arm_none_eabi.cmake)

if(NOT TARGET qemu_linkage)
add_library(qemu_linkage INTERFACE)
target_link_options(qemu_linkage INTERFACE
        -specs=nano.specs -Wl,-lc # reduced libc
        -specs=rdimon.specs -Wl,-lrdimon # arm semi hosting
        -T${CMAKE_CURRENT_LIST_DIR}/memory_spec.ld # qemu target memories
        -T${CMAKE_CURRENT_LIST_DIR}/region_alias.ld # memory mapping
        -T${CMAKE_CURRENT_LIST_DIR}/../../toolchains/section_mapping.ld # default section placement
)

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
