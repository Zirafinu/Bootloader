#---------------------------------------------------------------------------------------
# configure flash_layout library
#---------------------------------------------------------------------------------------
set(total_begin 0x00000000)
set(bootl_begin 0x00000000)
set(appli_begin 0x00100000)
set(updat_begin 0x00180000)
set(param_begin 0x00200000)
set(event_begin 0x00240000)
set(recrs_begin 0x00280000)

set(total_size 3*1024*1024)
set(bootl_size 1024*1024)
set(appli_size 512*1024)
set(updat_size 512*1024 CACHE INTERNAL "the size of the update memory")
set(param_size 256*1024)
set(event_size 256*1024)
set(recrs_size 512*1024)

set(HOMOGENOUS_PAGED_AREA_INITIALIZER "{0x00000000, 0x00300000, 0, 24}")

# generate the memory map file and the header
configure_file(${CMAKE_CURRENT_LIST_DIR}/../../toolchains/in.memory_spec.ld ${CMAKE_BINARY_DIR}/memory_spec.ld)
configure_file(${CMAKE_CURRENT_LIST_DIR}/../../toolchains/in.flash_layout.h ${CMAKE_BINARY_DIR}/flash_layout.h)

# create the library for easy access
if(NOT TARGET flash_layout)
    add_library(flash_layout INTERFACE)
    target_include_directories(flash_layout INTERFACE ${CMAKE_BINARY_DIR})
endif()
