cmake_minimum_required(VERSION 3.28)
project(micro_bootloader_stm32f407_example CXX C ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/helpers.cmake)

include(CTest)
enable_testing()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../source ${CMAKE_CURRENT_BINARY_DIR}/bootloader)

if(TARGET_CORE STREQUAL "Device")
    add_subdirectory(stm32f407)
endif()

_get_all_cmake_targets(all_targets ${CMAKE_CURRENT_LIST_DIR})
foreach (target ${all_targets})
    get_target_property(target_type ${target} TYPE)
    if (${target_type} MATCHES "EXECUTABLE")
        target_link_options(${target} PRIVATE -Wl,-Map=${target}.map)
        set_target_properties(${target} PROPERTIES ADDITIONAL_CLEAN_FILES $<TARGET_FILE_BASE_NAME:${target}>.map)
        if((${target} MATCHES "test_.*") AND (TARGET linkage_test))
            target_link_libraries(${target} PRIVATE linkage_test)
        endif()
    endif ()
endforeach ()
