#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include <cstdint>
#include <skip_application.h>

namespace bootloader {
[[noreturn]]
void jump_to_application() noexcept;
bool application_is_valid() noexcept;
bool application_update_is_valid() noexcept;
bool copy_update_to_application() noexcept;
} // namespace bootloader

#endif
