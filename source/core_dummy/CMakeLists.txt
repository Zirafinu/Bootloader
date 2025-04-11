add_library(bootloader_core_dummy OBJECT EXCLUDE_FROM_ALL dummy.cpp)

if(NOT TARGET Bootloader::core_impl)
add_library(Bootloader::core_impl ALIAS bootloader_core_dummy)
endif()
