ENTRY(Reset_Handler)

SECTIONS
{
   .isr_vector : {
       KEEP(*(.isr_vector))
       KEEP(*(.text.Reset_Handler))
   } > FLASH_ISR

   .text : {
       *(.text*)
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
