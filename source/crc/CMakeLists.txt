add_library(sw_crc OBJECT EXCLUDE_FROM_ALL sw_crc.cpp)

add_executable(test_sw_crc test_sw_crc.cpp)
target_link_libraries(test_sw_crc PRIVATE sw_crc)
add_test(NAME test_sw_crc COMMAND $<TARGET_FILE:test_sw_crc>)
