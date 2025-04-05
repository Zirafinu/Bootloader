
#ifndef bootloader_h_
#define bootlaoder_h_

#include <cstdint>
#include <skip_application.h>

extern "C"
{
    bool application_is_valid() noexcept;
    bool application_backup_is_valid() noexcept;
    [[noreturn]]
    void jump_to_application() noexcept;
    void copy_backup_to_application() noexcept;
}

#endif