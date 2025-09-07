configure_file(flash_layout.h.in flash_layout.h)
configure_file(memory_spec.ld.in memory_spec.ld)

add_library(flash_layout INTERFACE)
target_include_directories(flash_layout INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

# for other compilers this has to look differently
set_property(TARGET flash_layout PROPERTY LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/memory_spec.ld)
