add_library(cube INTERFACE)

target_include_directories(cube INTERFACE 
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
)
target_compile_definitions(cube INTERFACE
    USE_FULL_LL_DRIVER
    HSE_VALUE=25000000
    HSE_STARTUP_TIMEOUT=100
    LSE_STARTUP_TIMEOUT=5000
    LSE_VALUE=32768
    EXTERNAL_CLOCK_VALUE=12288000
    HSI_VALUE=16000000
    LSI_VALUE=32000
    STM32F429xx
    USE_HAL_DRIVER
)
