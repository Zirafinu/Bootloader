cmake_minimum_required(VERSION 3.28)
project(micro_bootloader_stm32f429_example CXX ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(bootloader_core STATIC EXCLUDE_FROM_ALL sources/reset_arm_cortex.cpp sources/startup.S)
add_library(Bootloader::core_impl ALIAS bootloader_core)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../source ${CMAKE_CURRENT_BINARY_DIR}/bootloader)
add_subdirectory(sources)

target_link_libraries(bootloader_core PRIVATE Bootloader::gzip)
target_link_libraries(bootloader_core PRIVATE Bootloader::crc_impl)

target_link_options(bootloader PRIVATE "-Wl,--print-memory-usage")
