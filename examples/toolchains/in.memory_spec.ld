MEMORY
{
    FLASH_TOTAL (rx) : ORIGIN = ${total_begin},  LENGTH = ${total_size}
}

application_skip_request = ORIGIN(SKIP_CODES);
application_entry_function = ORIGIN(FLASH_APPLICATION) + 4;

MEMORY {
    FLASH_BOOTLOADER  (rx) : ORIGIN = ${bootl_begin}, LENGTH = ${bootl_size} - 16
    FLASH_APPLICATION (rx) : ORIGIN = ${appli_begin}, LENGTH = ${appli_size} - 16
    FLASH_UPDATE      (rx) : ORIGIN = ${updat_begin}, LENGTH = ${updat_size}
    FLASH_PARAMETER   (rx) : ORIGIN = ${param_begin}, LENGTH = ${param_size}
    FLASH_EVENT_LOG   (rx) : ORIGIN = ${event_begin}, LENGTH = ${event_size}
    FLASH_RESOURCES   (rx) : ORIGIN = ${recrs_begin}, LENGTH = ${recrs_size}
}

MEMORY {
    FLASH_BOOTLOADER_INFO  (rx) : ORIGIN = ORIGIN(FLASH_BOOTLOADER)  + LENGTH(FLASH_BOOTLOADER),  LENGTH = 12
    FLASH_APPLICATION_INFO (rx) : ORIGIN = ORIGIN(FLASH_APPLICATION) + LENGTH(FLASH_APPLICATION), LENGTH = 12

    FLASH_BOOTLOADER_CRC   (rx) : ORIGIN = ORIGIN(FLASH_BOOTLOADER_INFO)  + LENGTH(FLASH_BOOTLOADER_INFO),  LENGTH = 4
    FLASH_APPLICATION_CRC  (rx) : ORIGIN = ORIGIN(FLASH_APPLICATION_INFO) + LENGTH(FLASH_APPLICATION_INFO), LENGTH = 4
}

ENTRY(Reset_Handler)
