#---------------------------------------------------------------------------------------
# configure flash_layout library
#---------------------------------------------------------------------------------------
set(total_begin 0x08000000)
set(bootl_begin 0x08000000)
set(param_begin 0x08002000)
set(event_begin 0x08004000)
set(recrs_begin 0x08006000)
set(appli_begin 0x08008000)
set(updat_begin 0x08080000)

set(total_size 1024*1024)
set(bootl_size 16*1024)
set(param_size 16*1024)
set(event_size 16*1024)
set(recrs_size 16*1024)
set(appli_size 576*1024)
set(updat_size 384*1024)

set(HOMOGENOUS_PAGED_AREA_INITIALIZER "{0x08000000, 0x08010000, 0, 4}, {0x08010000, 0x08020000, 4, 1}, {0x08020000, 0x08100000,5,12}")

# generate the memory map file and the header
configure_file(${CMAKE_CURRENT_LIST_DIR}/../../toolchains/in.memory_spec.ld ${CMAKE_BINARY_DIR}/memory_spec.ld)
configure_file(${CMAKE_CURRENT_LIST_DIR}/../../toolchains/in.flash_layout.h ${CMAKE_BINARY_DIR}/flash_layout.h)

# create the library for easy access
if(NOT TARGET flash_layout)
    add_library(flash_layout INTERFACE)
    target_include_directories(flash_layout INTERFACE ${CMAKE_BINARY_DIR})
endif()
