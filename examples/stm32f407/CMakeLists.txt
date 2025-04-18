cmake_minimum_required(VERSION 3.28)
project(micro_bootloader_stm32f407_example CXX C ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(bootloader_core STATIC EXCLUDE_FROM_ALL)
add_library(Bootloader::core_impl ALIAS bootloader_core)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../source ${CMAKE_CURRENT_BINARY_DIR}/bootloader)
add_subdirectory(cube)
add_subdirectory(sources)

target_link_options(bootloader PRIVATE
    -Wl,--print-memory-usage
    -Wl,-Map=$<TARGET_FILE_BASE_NAME:bootloader>.map
)
set_target_properties(bootloader PROPERTIES
    ADDITIONAL_CLEAN_FILES $<TARGET_FILE_BASE_NAME:bootloader>.map
)
