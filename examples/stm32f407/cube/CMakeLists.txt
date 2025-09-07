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
target_link_libraries(cube INTERFACE flash_layout)
get_property(flash_layout_linker_script TARGET flash_layout PROPERTY LINK_DEPENDS)
target_link_libraries(bootloader_core PRIVATE cube)
target_link_libraries(bootloader_core PUBLIC  libc)

#----------------------------------------------------------------------------------------------------------
# bootloader adaptions
#----------------------------------------------------------------------------------------------------------
target_link_options(bootloader PRIVATE
    -T${flash_layout_linker_script}
    -T${CMAKE_CURRENT_LIST_DIR}/memory_spec_ram.ld
    -T${CMAKE_CURRENT_LIST_DIR}/region_alias.ld
    -T${CMAKE_CURRENT_LIST_DIR}/section_mapping.ld
)
set_property(TARGET bootloader PROPERTY LINK_DEPENDS
    ${flash_layout_linker_script}
    ${CMAKE_CURRENT_LIST_DIR}/memory_spec_ram.ld
    ${CMAKE_CURRENT_LIST_DIR}/region_alias.ld
    ${CMAKE_CURRENT_LIST_DIR}/section_mapping.ld
)

#----------------------------------------------------------------------------------------------------------
# test adaptions
#----------------------------------------------------------------------------------------------------------
target_compile_definitions(test_base PRIVATE STARTUP_WITH_SEMIHOSTING BOOT_WITH_INIT_ARRAY)
target_link_libraries(test_base PRIVATE cube)
target_link_options(test_base PUBLIC
    -specs=nano.specs -Wl,-lc
    -specs=rdimon.specs -Wl,-lrdimon
    -T${flash_layout_linker_script}
    -T${CMAKE_CURRENT_LIST_DIR}/memory_spec_ram.ld
    -T${CMAKE_CURRENT_LIST_DIR}/region_alias_dev_test.ld
    -T${CMAKE_CURRENT_LIST_DIR}/section_mapping.ld
)
set_property(TARGET test_base PROPERTY LINK_DEPENDS
    ${flash_layout_linker_script}
    ${CMAKE_CURRENT_LIST_DIR}/memory_spec_ram.ld
    ${CMAKE_CURRENT_LIST_DIR}/region_alias_dev_test.ld
    ${CMAKE_CURRENT_LIST_DIR}/section_mapping.ld
)


# this should actually be applied on the application, and not on the bootloader
string(RANDOM LENGTH 32 ALPHABET "0123456789ABCDEF" iv)
add_custom_command(
        OUTPUT bootloader.package
        COMMAND dd ARGS if=/dev/random of=key.bin count=1 bs=16
        COMMAND arm-none-eabi-objcopy ARGS -O binary $<TARGET_FILE:bootloader> $<TARGET_FILE:bootloader>.bin
        # place the key in the bootloader output file!
        COMMAND gzip ARGS -9 -f -n $<TARGET_FILE:bootloader>.bin
        COMMAND openssl ARGS enc -e -kfile key.bin -iv ${iv} -aes-128-cbc -in $<TARGET_FILE:bootloader>.bin.gz -out $<TARGET_FILE:bootloader>.bin.gz.encrypted
        COMMAND sh ARGS -c \"echo -n ${iv} | cat - $<TARGET_FILE:bootloader>.bin.gz.encrypted >bootloader.package\"
        COMMAND rm ARGS $<TARGET_FILE:bootloader>.bin.gz $<TARGET_FILE:bootloader>.bin.gz.encrypted
        DEPENDS $<TARGET_FILE:bootloader>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_custom_target(encrypt ALL DEPENDS bootloader.package)
# head -c 16 /dev/random > iv.bin ; arm-none-eabi-objcopy -O binary bootloader test ; cat test | gzip -c -9 -n - | openssl enc -e -K "$(hexdump -e '16/1 "%02X" /0 "\n"' key.bin)" -iv "$(hexdump -e '16/1 "%02X" /0 "\n"' iv.bin)" -aes-128-cbc -in - -out - | cat iv.bin - > bootloader.secret ; rm test
