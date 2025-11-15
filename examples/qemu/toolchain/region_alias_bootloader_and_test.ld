REGION_ALIAS("RESET_FLASH", FLASH_BOOTLOADER)
REGION_ALIAS("FLASH", FLASH_BOOTLOADER)
REGION_ALIAS("FAST_DATA_RAM", FLASH_BOOTLOADER)
REGION_ALIAS("FAST_CODE_RAM", FLASH_BOOTLOADER)
REGION_ALIAS("STACK_RAM", RAM_ACTIVE)
REGION_ALIAS("HEAP_RAM", RAM_ACTIVE)
REGION_ALIAS("RAM", RAM_ACTIVE)

ENTRY(_start)

SECTIONS
{
    /* initialize the minimal vector table */
    .ISR_VECTOR_QEMU_PATCH : ALIGN(512) {
        LONG(estack);
        LONG(_start+1);
    } > FLASH

    /* has to match the size of the version struct */
    .version_info : {
        KEEP(*(.rodata.version_info_struct))
    } > FLASH_BOOTLOADER_INFO
    /* has to match the size of the crc */
    .crc : ALIGN(4) {
        KEEP(*(.rodata.bootloader_crc));
    } > FLASH_BOOTLOADER_CRC

    .application_placeholder : {
        KEEP(*(.rodata.application))
    } > FLASH_APPLICATION

    .update_placeholder : {
        KEEP(*(.rodata.update))
    } > FLASH_UPDATE
}
