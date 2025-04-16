add_library(crc INTERFACE)
target_include_directories(crc INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_library(sw_crc OBJECT EXCLUDE_FROM_ALL sw_crc.cpp)
target_link_libraries(sw_crc PUBLIC crc)

if(BUILD_TESTING)
add_executable(test_sw_crc EXCLUDE_FROM_ALL test_sw_crc.cpp)
target_link_libraries(test_sw_crc PRIVATE sw_crc Bootloader::test_base)
add_test(NAME test_sw_crc COMMAND $<TARGET_FILE:test_sw_crc>)
endif()

if(NOT TARGET Bootloader::crc_impl)
add_library(Bootloader::crc_impl ALIAS sw_crc)
endif()
