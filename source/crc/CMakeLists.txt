add_library(crc INTERFACE)
target_include_directories(crc INTERFACE ${CMAKE_CURRENT_LIST_DIR})
add_library(Bootloader::crc ALIAS crc)

add_library(sw_crc INTERFACE)
target_sources(sw_crc INTERFACE sw_crc.cpp)
target_link_libraries(sw_crc INTERFACE crc)
add_library(Bootloader::crc_impl ALIAS sw_crc)

if(BUILD_TESTING)
add_executable(test_sw_crc test_sw_crc.cpp sw_crc.cpp)
target_link_libraries(test_sw_crc PRIVATE sw_crc)
add_test(NAME test_sw_crc COMMAND test_sw_crc)
endif()
