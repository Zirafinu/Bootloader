
#ifndef bootloader_h_
#define bootlaoder_h_

#include <cstdint>
#include <skip_application.h>

namespace bootloader {
[[noreturn]]
void jump_to_application() noexcept;
bool application_is_valid() noexcept;
bool application_backup_is_valid() noexcept;
void copy_backup_to_application() noexcept;
} // namespace bootloader

#endif
