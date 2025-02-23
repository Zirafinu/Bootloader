add_executable(bootloader)
target_sources(bootloader PRIVATE bootloader.cpp)

add_subdirectory(arm_m_profile)
add_subdirectory(crc)
target_link_libraries(bootloader PRIVATE arm_m_profile sw_crc)


target_link_options(bootloader PRIVATE 
    -Wl,-Map=$<TARGET_FILE_BASE_NAME:bootloader>.map
    -Wl,--print-memory-usage
)
