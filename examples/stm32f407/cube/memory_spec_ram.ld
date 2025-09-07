MEMORY
{
    RAM_TOTAL (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
    CCMRAM (xrw)     : ORIGIN = 0x10000000, LENGTH = 64K
}

MEMORY {
    RAM_ACTIVE(rwx): ORIGIN = ORIGIN(RAM_TOTAL)                     , LENGTH = LENGTH(RAM_TOTAL) - 4
    SKIP_CODES (rw): ORIGIN = ORIGIN(RAM_TOTAL) + LENGTH(RAM_ACTIVE), LENGTH = 4
}

application_skip_request = ORIGIN(SKIP_CODES);
application_entry_function = ORIGIN(FLASH_APPLICATION) + 4;
