SECTIONS
{
    /* has to match the size of the version struct */
    .version_info : {
        KEEP(*(.rodata.version_info_struct))
    } > FLASH_BOOTLOADER_INFO
    /* has to match the size of the crc */
    .crc : {
        LONG(0);
    } > FLASH_BOOTLOADER_CRC
}

SECTIONS
{
    /* map the embedded application into the application region */
    .app_test_flash : {
        KEEP(*(.rodata*application*isr_vector*))
        KEEP(*(.*application*Reset_Handler*))
    } > FLASH_APPLICATION
    /* has to match the size of the version struct */
    .app_version_info : {
        KEEP(*(.rodata*application*version_info_struct_app*))
    } > FLASH_APPLICATION_INFO
    /* has to match the size of the crc */
    .app_crc : {
        LONG(0);
    } > FLASH_APPLICATION_CRC
}



SECTIONS
{
    /* reserve all the update flash memory */
    .update_place_holder : {
        LONG(0);
        . = ALIGN(LENGTH(FLASH_UPDATE));
    } > FLASH_UPDATE =0
}
