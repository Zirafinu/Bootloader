cmake_minimum_required(VERSION 3.28)
project(micro_bootloader_stm32f429_example CXX C ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(bootloader_core STATIC EXCLUDE_FROM_ALL
    cube/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
    cube/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    cube/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
)

add_library(Bootloader::core_impl ALIAS bootloader_core)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../source ${CMAKE_CURRENT_BINARY_DIR}/bootloader)
add_subdirectory(cube)
add_subdirectory(sources)

target_link_libraries(bootloader_core PRIVATE cube)

target_link_options(bootloader PRIVATE
    -Wl,--print-memory-usage
    -Wl,-Map=$<TARGET_FILE_BASE_NAME:bootloader>.map
)
