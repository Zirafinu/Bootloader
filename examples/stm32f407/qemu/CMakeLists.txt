target_sources(bootloader_core PRIVATE dummy.cpp)
target_link_options(bootloader_core PUBLIC
    -specs=nano.specs -Wl,-lc
    -specs=rdimon.specs -Wl,-lrdimon
    -T${CMAKE_CURRENT_LIST_DIR}/memory_spec.ld
    -T${CMAKE_CURRENT_LIST_DIR}/region_alias.ld
    -T${CMAKE_CURRENT_LIST_DIR}/section_mapping.ld
)


target_sources(test_base PRIVATE startup_qemu.S)
target_link_libraries(test_base PUBLIC flash_layout)
target_link_options(test_base PUBLIC
    -specs=nano.specs -Wl,-lc
    -specs=rdimon.specs -Wl,-lrdimon
    -T${CMAKE_CURRENT_LIST_DIR}/memory_spec.ld
    -T${CMAKE_CURRENT_LIST_DIR}/region_alias.ld
    -T${CMAKE_CURRENT_LIST_DIR}/section_mapping.ld
)

set_property(TARGET test_base PROPERTY LINK_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/memory_spec.ld
    ${CMAKE_CURRENT_LIST_DIR}/region_alias.ld
    ${CMAKE_CURRENT_LIST_DIR}/section_mapping.ld
)
