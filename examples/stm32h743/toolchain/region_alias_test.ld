REGION_ALIAS("RESET_FLASH", RAM_ACTIVE)
REGION_ALIAS("FLASH", RAM_ACTIVE)
REGION_ALIAS("FAST_DATA_RAM", DTCMRAM)
REGION_ALIAS("FAST_CODE_RAM", ITCMRAM)
REGION_ALIAS("STACK_RAM", DTCMRAM)
REGION_ALIAS("HEAP_RAM", DTCMRAM)
REGION_ALIAS("RAM", DTCMRAM)

SECTIONS
{
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
