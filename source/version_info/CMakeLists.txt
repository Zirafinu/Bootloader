add_library(version_info INTERFACE)
target_include_directories(version_info INTERFACE ${CMAKE_CURRENT_LIST_DIR})
add_library(Bootloader::version_info ALIAS version_info)
