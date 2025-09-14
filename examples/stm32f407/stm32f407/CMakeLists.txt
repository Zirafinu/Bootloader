
add_subdirectory(cube)

target_link_libraries(linkage_bootloader INTERFACE cube)
target_link_libraries(linkage_test INTERFACE cube)
target_link_libraries(linkage_application INTERFACE cube)

add_executable(stm32f407_bootloader bootloader_impl.cpp)
target_link_libraries(stm32f407_bootloader PRIVATE
    linkage_bootloader
    flash_layout
    Bootloader::core
    Bootloader::skip_application_impl
    Bootloader::crc_impl
    Bootloader::gzip
    Bootloader::version_info
)
update_build_time_and_crc(stm32f407_bootloader)

add_executable(blinky blinky.cpp)
target_link_libraries(blinky PRIVATE
    linkage_application
    Bootloader::version_info
)

update_build_time_and_crc(blinky)
encrypt_target(blinky)
