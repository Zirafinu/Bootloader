#---------------------------------------------------------------------------------------
# configure flash_layout library
#---------------------------------------------------------------------------------------
set(total_begin 0x08000000)
set(bootl_begin 0x08000000)
set(param_begin 0x08020000)
set(event_begin 0x08040000)
set(recrs_begin 0x08060000)
set(appli_begin 0x08080000)
set(updat_begin 0x08180000)

set(total_size 2*1024*1024)
set(bootl_size 128*1024)
set(param_size 128*1024)
set(event_size 128*1024)
set(recrs_size 128*1024)
set(appli_size 8*128*1024)
set(updat_size 4*128*1024)

set(HOMOGENOUS_PAGED_AREA_INITIALIZER "{0x08000000, 0x08200000, 0, 16}")

# generate the memory map file and the header
configure_file(${CMAKE_CURRENT_LIST_DIR}/../../toolchains/in.memory_spec.ld ${CMAKE_BINARY_DIR}/memory_spec.ld)
configure_file(${CMAKE_CURRENT_LIST_DIR}/../../toolchains/in.flash_layout.h ${CMAKE_BINARY_DIR}/flash_layout.h)

# create the library for easy access
if(NOT TARGET flash_layout)
    add_library(flash_layout INTERFACE)
    target_include_directories(flash_layout INTERFACE ${CMAKE_BINARY_DIR})
endif()
