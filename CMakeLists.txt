cmake_minimum_required(VERSION 3.28)
project(micro_bootloader CXX C ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
# include(dependencies.cmake)
add_subdirectory(dependencies)
add_subdirectory(source)

