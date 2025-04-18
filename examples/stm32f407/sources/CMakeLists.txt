# define flash layouts variables
set(total_begin 0x08000000)
set(bootl_begin 0x08000000)
set(param_begin 0x08002000)
set(event_begin 0x08004000)
set(auxil_begin 0x08006000)
set(appli_begin 0x08008000)
set(updat_begin 0x08080000)

set(total_size 1024*1024)
set(bootl_size 16*1024)
set(param_size 16*1024)
set(event_size 16*1024)
set(auxil_size 16*1024)
set(appli_size 576*1024)
set(updat_size 384*1024)

set(HOMOGENOUS_PAGED_AREA_INITIALIZER "{0x08000000, 0x08010000, 0, 4}, {0x08010000, 0x08020000, 4, 1}, {0x08020000, 0x08100000,5,12}")

configure_file(flash_layout.h.in flash_layout.h)
configure_file(memory_spec.ld.in memory_spec.ld)

target_sources(bootloader_core PRIVATE reset_arm_cortex.cpp startup.S)
target_include_directories(bootloader_core PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(bootloader_core PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(bootloader_core PUBLIC Bootloader::gzip $<TARGET_OBJECTS:Bootloader::gzip>)
target_link_libraries(bootloader_core PUBLIC Bootloader::crc_impl $<TARGET_OBJECTS:Bootloader::crc_impl>)

target_compile_options(bootloader_core PUBLIC
    -nostartfiles
)
target_link_options(bootloader_core PUBLIC
    -nostartfiles
    -T${CMAKE_CURRENT_BINARY_DIR}/memory_spec.ld
    -T${CMAKE_CURRENT_LIST_DIR}/region_alias.ld
    -T${CMAKE_CURRENT_LIST_DIR}/section_mapping.ld
)

get_target_property(link_dependencies bootloader_core LINK_DEPENDS)
list(FILTER link_dependencies EXCLUDE REGEX "link_dependencies-NOTFOUND")
list(APPEND link_dependencies
    ${CMAKE_CURRENT_BINARY_DIR}/memory_spec.ld
    ${CMAKE_CURRENT_LIST_DIR}/region_alias.ld
    ${CMAKE_CURRENT_LIST_DIR}/section_mapping.ld
)
set_target_properties(bootloader_core PROPERTIES
    INTERFACE_LINK_DEPENDS "${link_dependencies}"
)
