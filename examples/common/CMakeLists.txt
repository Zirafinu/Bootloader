
add_executable(bootloader_example bootloader_example.cpp)
target_include_directories(bootloader_example PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(bootloader_example PRIVATE
    linkage_bootloader
    flash_layout
    Bootloader::core
    Bootloader::skip_application_impl
    Bootloader::crc_impl
    Bootloader::gzip
    Bootloader::version_info
    Bootloader::crypto_impl
)
add_custom_command(TARGET bootloader_example POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}  ARGS --update-section ".secrets=${CMAKE_CURRENT_LIST_DIR}/../key.bin" $<TARGET_FILE:bootloader_example>
)

add_library(bootloader_example_test INTERFACE)
target_include_directories(bootloader_example_test INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_sources(bootloader_example_test INTERFACE bootloader_example.cpp)

if(BUILD_TESTING)
add_subdirectory(tests)
endif()
