SECTIONS
{
    .pre_startup : ALIGN(512) {
        KEEP(*(.isr_vector))
        KEEP(*(.text.Reset_Handler))
        KEEP(*(.text._startup))
        KEEP(*(.text.init_mem))
        *:libc_a-memset.o(.text)
        *:libc_a-memcpy-stub.o(.text)
        *:libc_a-memcpy.o(.text)
        *(.text.memset)
        *(.text.memcpy)

        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        KEEP(*(.init_array))      /* static constructors */
        KEEP(*(.ctors))           /* static constructors */
        PROVIDE_HIDDEN (__init_array_end = .);
    } > RESET_FLASH

    .main : ALIGN(4) {
        *(.text.main*)
    } > FLASH

    .fast_text : ALIGN(4) {
        fast_text_start = .;
        *(.text.*crc*)
        fast_text_end = ALIGN(4);
    } > FAST_CODE_RAM AT>FLASH
    fast_text_load_start = LOADADDR(.fast_text);
    fast_text_load_size = SIZEOF(.fast_text);

    .fast_data : ALIGN(4) {
        fast_data_start = .;
        *(.rodata.*crc*)
        . = ALIGN(4);
        *(.data.*crc*)
        fast_data_end = ALIGN(4);
    } > FAST_DATA_RAM AT>FLASH
    fast_data_load_start = LOADADDR(.fast_data);
    fast_data_load_size = SIZEOF(.fast_data);

    .text : ALIGN(4) {
        *(.text)           /* .text sections (code) */
        *(.text*)          /* .text* sections (code) */
        *(.glue_7)         /* glue arm to thumb code */
        *(.glue_7t)        /* glue thumb to arm code */
        *(.eh_frame)

        KEEP (*(.init))
        KEEP (*(.fini))

        _etext = ALIGN(4);        /* define a global symbols at end of code */
    } > FLASH

    .rodata : ALIGN(4) {
        /* .rodata sections (constants, strings, etc.) */
        *(.rodata)
        *(.rodata*)
    } > FLASH

    .ARM.extab (READONLY) : ALIGN(4) /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >FLASH

    .ARM (READONLY) : ALIGN(4) {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH


    .fini_array (READONLY) : ALIGN(4) {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH


    .data : ALIGN(4) {
        data_start = .;
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */
        *(.RamFunc)        /* .RamFunc sections */
        *(.RamFunc*)       /* .RamFunc* sections */
        data_end = ALIGN(4);
    } > RAM AT> FLASH
    data_load_start = LOADADDR(.data);
    data_load_size = SIZEOF(.data);

    .bss (NOLOAD): ALIGN(4) {
        bss_start = .;
        __bss_start__ = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        bss_end = ALIGN(4);
        __bss_end__ = ALIGN(4);
    } > RAM
    bss_size = SIZEOF(.bss);

    .heap (NOLOAD): ALIGN(8) {
        heap_start = .;
        . += 0x00;
        heap_end = ALIGN(8);
        end = .;
        __end__ = .;
    } > HEAP_RAM

    .stack (NOLOAD): ALIGN(8) {
        stack_top = .;
        . += 10K;
        estack = ALIGN(8);
    } > STACK_RAM

    .skip_codes (NOLOAD): ALIGN(4) {
        . += 4;
    } > SKIP_CODES
}

/* main gets inlined into init_mem, if not declared "noinline", thus this test fails for optimized builds */
/* NOCROSSREFS_TO(.text .pre_startup); */
