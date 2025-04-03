add_library(gzip OBJECT EXCLUDE_FROM_ALL gzip.cpp)
target_include_directories(gzip PUBLIC ${CMAKE_CURRENT_LIST_DIR})


if(NOT CMAKE_CROSSCOMPILING)
add_executable(test_gzip test_gzip.cpp)
target_link_libraries(test_gzip PRIVATE gzip)
add_test(NAME test_gzip COMMAND $<TARGET_FILE:test_gzip>)
endif()
