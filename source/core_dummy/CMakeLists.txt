if(NOT TARGET Bootloader::core_impl)
add_library(bootloader_core_dummy OBJECT EXCLUDE_FROM_ALL dummy.cpp)
target_link_libraries(bootloader_core_dummy PUBLIC Bootloader::config_base)

add_library(Bootloader::core_impl ALIAS bootloader_core_dummy)
endif()
