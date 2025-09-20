#ifndef BOOTLOADER_BOOTLOADER_H_
#define BOOTLOADER_BOOTLOADER_H_

#include <bootloader/skip_application.h>
#include <cstdint>

namespace bootloader {
[[noreturn]]
void jump_to_application() noexcept;
bool application_is_valid() noexcept;
bool application_update_is_valid(bool is_application_memory_valid) noexcept;
bool copy_update_to_application() noexcept;
} // namespace bootloader

#endif
