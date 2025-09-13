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

function (encrypt_target target)
    get_target_property(out_dir ${target} BINARY_DIR)
    string(RANDOM LENGTH 32 ALPHABET "0123456789ABCDEF" iv)
    add_custom_command(
        OUTPUT "${out_dir}/${target}.package"
        COMMAND arm-none-eabi-objcopy ARGS -O binary $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>.bin
        # place the key in the bootloader output file!
        COMMAND gzip ARGS -9 -f -n $<TARGET_FILE:${target}>.bin
        COMMAND openssl ARGS enc -e -kfile ${CMAKE_CURRENT_LIST_DIR}/../key.bin -iv ${iv} -aes-128-cbc -in $<TARGET_FILE:${target}>.bin.gz -out $<TARGET_FILE:${target}>.bin.gz.encrypted
        COMMAND sh ARGS -c "'echo' '-n' '${iv}' '|' 'cat' '-' '$<TARGET_FILE:${target}>.bin.gz.encrypted' '>${out_dir}/${target}.package'"
        COMMAND rm ARGS $<TARGET_FILE:${target}>.bin.gz $<TARGET_FILE:${target}>.bin.gz.encrypted
        DEPENDS $<TARGET_FILE:${target}>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    # head -c 16 /dev/random > iv.bin ; arm-none-eabi-objcopy -O binary bootloader test ; cat test | gzip -c -9 -n - | openssl enc -e -K "$(hexdump -e '16/1 "%02X" /0 "\n"' key.bin)" -iv "$(hexdump -e '16/1 "%02X" /0 "\n"' iv.bin)" -aes-128-cbc -in - -out - | cat iv.bin - > bootloader.secret ; rm test
    add_custom_target(encrypt_${target} ALL DEPENDS "${out_dir}/${target}.package")
endfunction()
