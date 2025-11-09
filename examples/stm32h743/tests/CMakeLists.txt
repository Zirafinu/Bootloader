
add_executable(application_v2 application_v2.cpp)
target_link_libraries(application_v2 PRIVATE
    linkage_application
    linkage_semihosting
    Bootloader::version_info
)
update_build_time_and_crc(application_v2)
encrypt_target(application_v2)

get_target_property(out_dir application_v2 BINARY_DIR)
add_custom_command(
    OUTPUT "${out_dir}/application_v2.package.update"
    DEPENDS  $<TARGET_FILE:application_v2>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_executable(test_implementation test_implementation.cpp)
target_link_libraries(test_implementation PRIVATE
    flash_layout
    Bootloader::skip_application_impl
    Bootloader::crc_impl
    Bootloader::gzip
    Bootloader::version_info
    Bootloader::crypto_impl
)
target_link_options(test_implementation PRIVATE
    -T${CMAKE_BINARY_DIR}/memory_spec.ld # add the generated flash layout
    -T${CMAKE_CURRENT_LIST_DIR}/test_bootloader_implementation.ld # store an embedded application in the app memory
)
set_property(TARGET test_implementation PROPERTY LINK_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/test_bootloader_implementation.ld
)
add_custom_command(TARGET test_implementation POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}  ARGS --update-section ".secrets=${CMAKE_CURRENT_LIST_DIR}/../../key.bin" $<TARGET_FILE:test_implementation>
)
update_build_time_and_crc(test_implementation)
# update build time and crc of the embedded application
add_custom_command(TARGET test_implementation
    COMMENT "Patching the info struct and the crc of the application"
    POST_BUILD
    # update the build time
    COMMAND ${CMAKE_OBJCOPY} ARGS --only-section=.app_version_info -Obinary $<TARGET_FILE:test_implementation> $<TARGET_FILE:test_implementation>.app_version_info
    COMMAND truncate ARGS --size 8 $<TARGET_FILE:test_implementation>.app_version_info
    COMMAND sh ARGS -c 'printf %08X $$\(date +%s\) | head --bytes 8 | tac --regex --separator .. | xxd -r -ps >>$<TARGET_FILE:test_implementation>.app_version_info'
    COMMAND ${CMAKE_OBJCOPY} ARGS --update-section ".app_version_info=$<TARGET_FILE:test_implementation>.app_version_info" $<TARGET_FILE:test_implementation>
    # update the crc
    COMMAND ${CMAKE_OBJCOPY} ARGS --only-section .app_test_flash --only-section .app_version_info --gap-fill 0xFF -O binary $<TARGET_FILE:test_implementation> $<TARGET_FILE:test_implementation>.bin.app_crc
    COMMAND sh ARGS -c 'cat $<TARGET_FILE:test_implementation>.bin.app_crc | gzip | tail --bytes=8 | head --bytes=4 >$<TARGET_FILE:test_implementation>.app_crc'
    COMMAND ${CMAKE_OBJCOPY} ARGS --update-section ".app_crc=$<TARGET_FILE:test_implementation>.app_crc" $<TARGET_FILE:test_implementation>
    # clean up
    COMMAND rm ARGS $<TARGET_FILE:test_implementation>.bin.app_crc  $<TARGET_FILE:test_implementation>.app_crc $<TARGET_FILE:test_implementation>.app_version_info
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_BINARY_DIR}
)

math(EXPR update_section_size "${updat_size} - 4" OUTPUT_FORMAT DECIMAL) # compute the size of the update area
# add the application_v2 into the update area, as if it was downloaded
add_custom_command(
    TARGET test_implementation
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E copy "${out_dir}/application_v2.package" "${out_dir}/application_v2.package.update"
    COMMAND truncate -s ${update_section_size} "${out_dir}/application_v2.package.update"
    COMMAND sh ARGS -c 'cat ${out_dir}/application_v2.package.update | gzip | tail --bytes=8 | head --bytes=4 >>${out_dir}/application_v2.package.update'
    COMMAND COMMAND ${CMAKE_OBJCOPY} ARGS --update-section ".update_place_holder=${out_dir}/application_v2.package.update" $<TARGET_FILE:test_implementation>
    DEPENDS ${out_dir}/application_v2.package
    BYPRODUCTS ${out_dir}/application_v2.package.update
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(NAME test_implementation COMMAND test_implementation)
