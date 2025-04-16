ENTRY(Reset_Handler)

SECTIONS
{
    .pre_startup_code : {
        KEEP(*(.isr_vector))
        KEEP(*(.text.Reset_Handler))
        KEEP(*(.text.init_mem))
        *:libc_a-memset.o(.text)
        *:libc_a-memcpy-stub.o(.text)
        *(.text.memset)
        *(.text.memcpy)
    } > FLASH_ISR

    .main : {
    *(.text.main*)
    } > FLASH

    .text : {
        *(.text.*)
    } > FLASH

    .rodata : {
        *(.rodata*)
    } > FLASH

    .data : {
        data_start = .;
        *(.data*)
    } > RAM AT> FLASH
    data_load_start = LOADADDR(.data);
    data_load_end = data_load_start + SIZEOF(.data);

    .bss : {
        bss_start = .;
        *(.bss*)
        *(COMMON)
        bss_end = .;
    } > RAM

    .stack_and_heap : {
        . += 0x400;
    } > RAM
}

/* main gets inlined into init_mem, if not declared "noinline", thus this test fails for optimized builds */
/* NOCROSSREFS_TO(.text .isr_vector); */
