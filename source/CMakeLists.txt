add_subdirectory(compression)
add_subdirectory(skip_application)
add_subdirectory(crc)
add_subdirectory(core_dummy)
add_subdirectory(crypto)

add_executable(bootloader)
target_sources(bootloader PRIVATE bootloader.cpp)

target_link_libraries(bootloader PRIVATE Bootloader::skip_application_impl)
target_link_libraries(bootloader PRIVATE Bootloader::core_impl)
target_link_libraries(bootloader PRIVATE Bootloader::config_base)
