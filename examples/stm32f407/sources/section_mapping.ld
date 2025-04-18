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
    } > RESET_FLASH

    .main : {
        . = ALIGN(4);
        *(.text.main*)
    } > FLASH

    .text : {
        . = ALIGN(4);
        *(.text)           /* .text sections (code) */
        *(.text*)          /* .text* sections (code) */
        *(.glue_7)         /* glue arm to thumb code */
        *(.glue_7t)        /* glue thumb to arm code */
        *(.eh_frame)

        KEEP (*(.init))
        KEEP (*(.fini))

        . = ALIGN(4);
        _etext = .;        /* define a global symbols at end of code */
    } > FLASH

    .rodata : {
        . = ALIGN(4);
        /* .rodata sections (constants, strings, etc.) */
        *(.rodata)
        *(.rodata*)
    } > FLASH

    .ARM.extab (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
    {
        . = ALIGN(4);
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        . = ALIGN(4);
    } >FLASH

    .ARM (READONLY) :
    {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    .preinit_array (READONLY) :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH

    .init_array (READONLY) :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH

    .fini_array (READONLY) :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH


    .data : {
        . = ALIGN(4);
        data_start = .;
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */
        *(.RamFunc)        /* .RamFunc sections */
        *(.RamFunc*)       /* .RamFunc* sections */
    } > RAM AT> FLASH
    data_load_start = LOADADDR(.data);
    data_load_end = data_load_start + SIZEOF(.data);

    .bss : {
        . = ALIGN(4);
        bss_start = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        bss_end = .;
    } > RAM

    .stack_and_heap : {
        . = ALIGN(8);
        . += 0x400;
    } > RAM

    .skip_codes (NOLOAD): {
        . += 4;
    } > SKIP_CODES
}

/* main gets inlined into init_mem, if not declared "noinline", thus this test fails for optimized builds */
/* NOCROSSREFS_TO(.text .pre_startup); */
