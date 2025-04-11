add_library(skip_application INTERFACE)
target_include_directories(skip_application INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_library(skip_application_noinit_ram OBJECT EXCLUDE_FROM_ALL skip_application_noinit_ram.cpp)
target_link_libraries(skip_application_noinit_ram PUBLIC skip_application)


if(NOT TARGET Bootloader::skip_application_impl)
add_library(Bootloader::skip_application_impl ALIAS skip_application_noinit_ram)
endif()
