ENTRY(Reset_Handler)

SECTIONS
{
    .pre_startup : {
        KEEP(*(.isr_vector))
        KEEP(*(.text.Reset_Handler))
        KEEP(*(.text.init_mem))
        *:libc_a-memset.o(.text)
        *:libc_a-memcpy-stub.o(.text)
        *(.text.memset)
        *(.text.memcpy)

        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(.init_array))      /* static constructors */
        KEEP(*(.ctors))           /* static constructors */
        __init_array_end = .;
    } > FLASH

    .main : {
        *(.text.main*)
    } > FLASH

    .text : {
        *(.text.*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
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

    .skip_codes (NOLOAD): {
        . += 4;
    } > SKIP_CODES

    /* C++ exception unwinding stuff--needed by some toolchains */
    .ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH


    .ARM.exidx : {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH
}

/* main gets inlined into init_mem, if not declared "noinline", thus this test fails for optimized builds */
/* NOCROSSREFS_TO(.text .pre_startup); */
