if(NOT DEFINED BOOTLOADER_HELPER_SCRIPT_DIR)
    set(BOOTLOADER_HELPER_SCRIPT_DIR "${CMAKE_CURRENT_LIST_DIR}")
endif()

function (_get_all_cmake_targets out_var current_dir)
    get_property(targets DIRECTORY ${current_dir} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(subdirs DIRECTORY ${current_dir} PROPERTY SUBDIRECTORIES)

    foreach(subdir ${subdirs})
        _get_all_cmake_targets(subdir_targets ${subdir})
        list(APPEND targets ${subdir_targets})
    endforeach()

    set(${out_var} ${targets} PARENT_SCOPE)
endfunction()

function (_get_all_tests out_var current_dir)
    get_property(tests DIRECTORY ${current_dir} PROPERTY TESTS)
    get_property(subdirs DIRECTORY ${current_dir} PROPERTY SUBDIRECTORIES)

    foreach(subdir ${subdirs})
        _get_all_cmake_targets(subdir_tests ${subdir})
        list(APPEND tests ${subdir_tests})
    endforeach()

    set(${out_var} ${tests} PARENT_SCOPE)
endfunction()

macro (update_build_time_and_crc target)
    # update build time and crc
    add_custom_command(TARGET ${target}
        COMMENT "Patching the info struct and the crc"
        POST_BUILD
        # update the build time
        COMMAND ${CMAKE_OBJCOPY} ARGS --only-section=.version_info -Obinary $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>.version_info_struct
        COMMAND truncate ARGS --size 8 $<TARGET_FILE:${target}>.version_info_struct
        COMMAND sh ARGS -c 'printf %08X $$\(date +%s\) | head --bytes 8 | tac --regex --separator .. | xxd -r -ps >>$<TARGET_FILE:${target}>.version_info_struct'
        COMMAND ${CMAKE_OBJCOPY} ARGS --update-section ".version_info=$<TARGET_FILE:${target}>.version_info_struct" $<TARGET_FILE:${target}>
        # update the crc
        COMMAND ${CMAKE_OBJCOPY} ARGS --gap-fill 0xFF -O binary $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>.bin.crc
        COMMAND sh ARGS -c 'head --bytes=-4 $<TARGET_FILE:${target}>.bin.crc | gzip | tail --bytes=8 | head --bytes=4 >$<TARGET_FILE:${target}>.crc'
        COMMAND ${CMAKE_OBJCOPY} ARGS --update-section ".crc=$<TARGET_FILE:${target}>.crc" $<TARGET_FILE:${target}>
        COMMAND rm ARGS $<TARGET_FILE:${target}>.bin.crc  $<TARGET_FILE:${target}>.crc $<TARGET_FILE:${target}>.version_info_struct
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_BINARY_DIR}
    )
endmacro ()

function(join_bootloader_with_application bootloader_srec application_srec)
    add_custom_command(
        OUTPUT $<PATH:REPLACE_EXTENSION,LAST_ONLY,${application_srec},.combined.srec>
        COMMAND head ARGS --lines=-1 $<OUTPUT_CONFIG:${bootloader_srec}>  >$<PATH:REPLACE_EXTENSION,LAST_ONLY,${application_srec},.combined.srec>
        COMMAND tail ARGS --lines=+2 $<OUTPUT_CONFIG:${application_srec}> >${application_srec}.no_head
        COMMAND head ARGS --lines=-1 ${application_srec}.no_head >>$<PATH:REPLACE_EXTENSION,LAST_ONLY,${application_srec},.combined.srec>
        COMMAND tail ARGS --lines=1 $<OUTPUT_CONFIG:${bootloader_srec}> >>$<PATH:REPLACE_EXTENSION,LAST_ONLY,${application_srec},.combined.srec>
        COMMAND ${CMAKE_COMMAND} ARGS -E rm ${application_srec}.no_head
        DEPENDS ${bootloader_srec} ${application_srec}
    )
    add_custom_target(combined_${application_srec} ALL DEPENDS "$<PATH:REPLACE_EXTENSION,LAST_ONLY,${application_srec},.combined.srec>")
endfunction()

function (encrypt_target target)
    get_target_property(out_dir ${target} BINARY_DIR)
    string(RANDOM LENGTH 32 ALPHABET "0123456789ABCDEF" iv)
    add_custom_command(
        OUTPUT "${out_dir}/${target}.package"
        COMMAND ${CMAKE_OBJCOPY} ARGS -O binary $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>.bin
        COMMAND ${CMAKE_OBJCOPY} ARGS --only-section=.version_info -Obinary $<TARGET_FILE:${target}> "${out_dir}/${target}.package"
        # place the key in the bootloader output file!
        COMMAND gzip ARGS -9 -f -n $<TARGET_FILE:${target}>.bin
        COMMAND sh ARGS -c 'tail --bytes +11 $<TARGET_FILE:${target}>.bin.gz | head --bytes -8 >$<TARGET_FILE:${target}>.bin.gz.cropped'
        COMMAND openssl ARGS enc -e -K $$\(xxd -ps ${BOOTLOADER_HELPER_SCRIPT_DIR}/../key.bin\) -iv "${iv}" -aes-128-cbc -nosalt -in $<TARGET_FILE:${target}>.bin.gz.cropped -out $<TARGET_FILE:${target}>.bin.gz.encrypted
        COMMAND ${CMAKE_COMMAND} ARGS -E echo_append "${iv}" | xxd -r -ps >>"${out_dir}/${target}.package"
        COMMAND ${CMAKE_COMMAND} ARGS -E cat $<TARGET_FILE:${target}>.bin.gz.encrypted >>"${out_dir}/${target}.package"
        COMMAND ${CMAKE_COMMAND} ARGS -E rm $<TARGET_FILE:${target}>.bin.gz $<TARGET_FILE:${target}>.bin.gz.cropped $<TARGET_FILE:${target}>.bin.gz.encrypted
        DEPENDS $<TARGET_FILE:${target}>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    # head -c 16 /dev/random > iv.bin ; arm-none-eabi-objcopy -O binary bootloader test ; cat test | gzip -c -9 -n - | openssl enc -e -K "$(xxd -ps key.bin)" -iv "$(hexdump -e '16/1 "%02X" /0 "\n"' iv.bin)" -aes-128-cbc -in - -out - | cat iv.bin - > bootloader.secret ; rm test
    add_custom_target(encrypt_${target} ALL DEPENDS "${out_dir}/${target}.package")
endfunction()
