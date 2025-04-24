target_include_directories(bootloader_core PRIVATE
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    ${CMAKE_CURRENT_LIST_DIR}
)
target_compile_definitions(bootloader_core PRIVATE
	USE_FULL_LL_DRIVER
	STM32F407xx
	USE_HAL_DRIVER
)
target_sources(bootloader_core PRIVATE
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
)