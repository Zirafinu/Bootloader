REGION_ALIAS("RESET_FLASH", FLASH_BOOTLOADER)
REGION_ALIAS("FLASH", FLASH_BOOTLOADER)
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
        LONG(0);
    } > FLASH_BOOTLOADER_CRC
}
