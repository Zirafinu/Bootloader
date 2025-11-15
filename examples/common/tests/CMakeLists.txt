
add_executable(demo_application demo_application.cpp)
target_link_libraries(demo_application PRIVATE
    linkage_application
    linkage_semihosting
    Bootloader::version_info
)
update_build_time_and_crc(demo_application)
encrypt_target(demo_application)
get_target_property(out_dir demo_application BINARY_DIR)

add_executable(test_implementation test_implementation.cpp)
target_link_libraries(test_implementation PRIVATE
    flash_layout
    bootloader_example_test
    Bootloader::skip_application_impl
    Bootloader::crc_impl
    Bootloader::gzip
    Bootloader::version_info
    Bootloader::crypto_impl
)
add_custom_command(TARGET test_implementation POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}  ARGS --update-section ".secrets=${CMAKE_CURRENT_LIST_DIR}/../../key.bin" $<TARGET_FILE:test_implementation>
)
update_build_time_and_crc(test_implementation)

math(EXPR update_section_size "${updat_size} - 4" OUTPUT_FORMAT DECIMAL) # compute the size of the update area
# add the demo_application into the update area, as if it was downloaded
add_custom_command(
    TARGET test_implementation
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E copy "${out_dir}/demo_application.package" "${out_dir}/demo_application.package.update"
    COMMAND truncate -s ${update_section_size} "${out_dir}/demo_application.package.update"
    COMMAND sh ARGS -c 'cat ${out_dir}/demo_application.package.update | gzip | tail --bytes=8 | head --bytes=4 >>${out_dir}/demo_application.package.update'
    COMMAND COMMAND ${CMAKE_OBJCOPY} ARGS --update-section ".update_placeholder=${out_dir}/demo_application.package.update" $<TARGET_FILE:test_implementation>
    DEPENDS ${out_dir}/demo_application.package
    BYPRODUCTS ${out_dir}/demo_application.package.update
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(NAME test_implementation COMMAND test_implementation)
