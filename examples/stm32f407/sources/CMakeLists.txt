
target_sources(bootloader_core PRIVATE reset_arm_cortex.cpp)

target_link_libraries(bootloader_core PUBLIC Bootloader::gzip $<TARGET_OBJECTS:Bootloader::gzip>)
target_link_libraries(bootloader_core PUBLIC Bootloader::crc_impl $<TARGET_OBJECTS:Bootloader::crc_impl>)
target_link_libraries(bootloader_core PUBLIC config_base)

