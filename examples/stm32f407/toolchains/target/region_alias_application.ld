REGION_ALIAS("RESET_FLASH", FLASH_APPLICATION)
REGION_ALIAS("FLASH", FLASH_APPLICATION)
REGION_ALIAS("FAST_DATA_RAM", CCMRAM)
REGION_ALIAS("FAST_CODE_RAM", RAM_ACTIVE)
REGION_ALIAS("STACK_RAM", CCMRAM)
REGION_ALIAS("HEAP_RAM", CCMRAM)
REGION_ALIAS("RAM", CCMRAM)

SECTIONS
{
    /* has to match the size of the version struct */
    .version_info : {
        KEEP(*(.rodata.version_info_struct))
    } > FLASH_APPLICATION_INFO
    /* has to match the size of the crc */
    .crc : {
        LONG(0);
    } > FLASH_APPLICATION_CRC
}
