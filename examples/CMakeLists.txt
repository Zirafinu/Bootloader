include(cmake/helpers.cmake)

add_subdirectory(common)

if(TARGET_CORE)
    add_subdirectory(${TARGET_CORE})
endif()


_get_all_cmake_targets(all_targets "${CMAKE_CURRENT_LIST_DIR}/..")
foreach (target ${all_targets})
    get_target_property(target_type ${target} TYPE)
    if (${target_type} MATCHES "EXECUTABLE")
        target_link_options(${target} PRIVATE -Wl,-Map=$<TARGET_FILE:${target}>.map)
        set_target_properties(${target} PROPERTIES ADDITIONAL_CLEAN_FILES $<TARGET_FILE_BASE_NAME:${target}>.map)
        if((${target} MATCHES "test_.*") AND (TARGET linkage_test))
            target_link_libraries(${target} PRIVATE linkage_test)
        endif()
    endif ()
endforeach ()
