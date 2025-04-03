
#ifndef bootloader_h_
#define bootlaoder_h_

#include <cstdint>

uint32_t crc(const void *mem_begin, const void *mem_end) noexcept;

extern "C"
{
    bool application_is_valid() noexcept;
    bool application_backup_is_valid() noexcept;
    bool application_requested_skipping() noexcept;
    void reset_application_skipping_request() noexcept;
    [[noreturn]]
    void jump_to_application() noexcept;
    void copy_backup_to_application() noexcept;
}

#endif