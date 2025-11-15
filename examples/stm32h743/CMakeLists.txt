add_subdirectory(cube)
target_link_libraries(linkage_bootloader INTERFACE cube)
target_link_libraries(linkage_test INTERFACE cube)
target_link_libraries(linkage_application INTERFACE cube)

target_sources(bootloader_example_test INTERFACE hardware_specific.cpp)
target_sources(bootloader_example PRIVATE hardware_specific.cpp)
add_custom_command(
    OUTPUT bootloader_example.srec
    COMMAND ${CMAKE_OBJCOPY} ARGS --srec-len=255 -O srec $<TARGET_FILE:bootloader_example> bootloader_example.srec
    DEPENDS bootloader_example
)

add_executable(blinky blinky.cpp)
target_link_libraries(blinky PRIVATE
    linkage_application
    Bootloader::version_info
)
update_build_time_and_crc(blinky)
add_custom_command(
    OUTPUT blinky.srec
    COMMAND ${CMAKE_OBJCOPY} ARGS --srec-len=255 -O srec $<TARGET_FILE:blinky> blinky.srec
    DEPENDS blinky
)

encrypt_target(blinky)
join_bootloader_with_application(bootloader_example.srec blinky.srec)
