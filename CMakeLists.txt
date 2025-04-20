cmake_minimum_required(VERSION 3.28)
project(micro_bootloader CXX ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
enable_testing()

if(NOT TARGET Bootloader::config_base)
    add_library(bootloader_config_base INTERFACE)
    add_library(Bootloader::config_base ALIAS bootloader_config_base)
endif()
if(NOT TARGET Bootloader::test_base)
    add_library(bootloader_test_base INTERFACE)
    add_library(Bootloader::test_base ALIAS bootloader_test_base)
endif()

add_subdirectory(source)
