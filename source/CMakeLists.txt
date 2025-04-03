add_executable(bootloader)
target_sources(bootloader PRIVATE bootloader.cpp)

add_subdirectory(compression)

add_subdirectory(crc)
target_link_libraries(bootloader PRIVATE sw_crc)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
add_subdirectory(core_arm_m_profile)
target_link_libraries(bootloader PRIVATE arm_m_profile)
target_link_options(bootloader PRIVATE
    -Wl,-Map=$<TARGET_FILE_BASE_NAME:bootloader>.map
    -Wl,--print-memory-usage
)
else()
add_subdirectory(core_dummy)
target_link_libraries(bootloader PRIVATE core_dummy)
endif()