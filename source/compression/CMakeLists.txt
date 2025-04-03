add_library(gzip OBJECT EXCLUDE_FROM_ALL gzip.cpp)
target_include_directories(gzip PUBLIC ${CMAKE_CURRENT_LIST_DIR})
