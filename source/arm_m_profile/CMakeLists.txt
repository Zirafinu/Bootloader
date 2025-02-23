add_library(arm_m_profile OBJECT EXCLUDE_FROM_ALL reset_arm_cortex.cpp startup.S)

set(total_begin 0x08000000)
set(bootl_begin 0x08000000)
set(param_begin 0x08004000)
set(param_backup_begin 0x08008000)
set(aux_1_begin 0x0800C000)
set(event_begin 0x08010000)
set(appl_begin  0x08020000)
set(appl_backup_begin 0x08080000)
set(aux_2_begin 0x080E0000)
set(aux_3_begin 0x08100000)

set(total_size 1024*1024 )
set(bootl_size 16*1024)
set(param_size 16*1024)
set(param_backup_size 16*1024)
set(event_size 64*1024)
set(appl_size 384*1024)
set(appl_backup_size 384*1024)
set(aux_1_size 16*1024)
set(aux_2_size 128*1024)
set(aux_3_size 0)

set(HOMOGENOUS_PAGED_AREA_INITIALIZER "{0x08000000, 0x08010000, 0, 4}, {0x08010000, 0x08020000, 4, 1}, {0x08020000, 0x08100000,5,12}")

configure_file(flash_layout.h.in flash_layout.h )
configure_file(memory_spec.ld.in memory_spec.ld )

# -Wl,-Map=$<TARGET>.map
target_link_options(arm_m_profile PUBLIC
    -nostartfiles
    -T${CMAKE_CURRENT_BINARY_DIR}/memory_spec.ld
    -T${CMAKE_CURRENT_LIST_DIR}/region_alias.ld
    -T${CMAKE_CURRENT_LIST_DIR}/section_mapping.ld
)
target_compile_options(arm_m_profile PUBLIC
    -nostartfiles
)
target_include_directories(arm_m_profile PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
