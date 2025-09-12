add_library(cube INTERFACE)
target_include_directories(cube INTERFACE
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    ${CMAKE_CURRENT_LIST_DIR}
)
target_compile_definitions(cube INTERFACE
	USE_FULL_LL_DRIVER
	STM32F407xx
	USE_HAL_DRIVER
)
target_sources(cube INTERFACE
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
    startup.S
    startup.cpp
)
target_link_libraries(linkage_bootloader INTERFACE cube)
target_link_libraries(linkage_test INTERFACE cube)
target_link_libraries(linkage_application INTERFACE cube)

add_executable(stm32f407_bootloader)
target_sources(stm32f407_bootloader PRIVATE reset_arm_cortex.cpp)
target_link_libraries(stm32f407_bootloader PRIVATE
    linkage_bootloader
    flash_layout
    Bootloader::core
    Bootloader::skip_application_impl
    Bootloader::crc_impl
    Bootloader::gzip
)


# this should actually be applied on the application, and not on the bootloader
string(RANDOM LENGTH 32 ALPHABET "0123456789ABCDEF" iv)
add_custom_command(
        OUTPUT stm32f407_bootloader.package
        COMMAND dd ARGS if=/dev/random of=key.bin count=1 bs=16
        COMMAND arm-none-eabi-objcopy ARGS -O binary $<TARGET_FILE:stm32f407_bootloader> $<TARGET_FILE:stm32f407_bootloader>.bin
        # place the key in the bootloader output file!
        COMMAND gzip ARGS -9 -f -n $<TARGET_FILE:stm32f407_bootloader>.bin
        COMMAND openssl ARGS enc -e -kfile key.bin -iv ${iv} -aes-128-cbc -in $<TARGET_FILE:stm32f407_bootloader>.bin.gz -out $<TARGET_FILE:stm32f407_bootloader>.bin.gz.encrypted
        COMMAND sh ARGS -c "'echo' '-n' '${iv}' '|' 'cat' '-' '$<TARGET_FILE:stm32f407_bootloader>.bin.gz.encrypted' '>stm32f407_bootloader.package'"
        COMMAND rm ARGS $<TARGET_FILE:stm32f407_bootloader>.bin.gz $<TARGET_FILE:stm32f407_bootloader>.bin.gz.encrypted
        DEPENDS $<TARGET_FILE:stm32f407_bootloader>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(encrypt ALL DEPENDS stm32f407_bootloader.package)
# head -c 16 /dev/random > iv.bin ; arm-none-eabi-objcopy -O binary bootloader test ; cat test | gzip -c -9 -n - | openssl enc -e -K "$(hexdump -e '16/1 "%02X" /0 "\n"' key.bin)" -iv "$(hexdump -e '16/1 "%02X" /0 "\n"' iv.bin)" -aes-128-cbc -in - -out - | cat iv.bin - > bootloader.secret ; rm test
